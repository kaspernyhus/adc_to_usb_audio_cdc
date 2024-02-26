/**
 * @file i2s.c
 * @author Kasper Nyhus Kaae (KANK)
 * @brief
 * @version 0.1
 * @date 2022-10-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "i2s.h"

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"

#include "config/pin_config.h"
#include <stdint.h>

#define I2S_READ_TIMEOUT_MS 1000

static const char* TAG = "i2s";

typedef enum {
    I2S_ERROR_INVALID_CONFIG,
    I2S_ERROR_DMA_OVERFLOW,
    I2S_ERROR_STREAMBUF_WRITE,
} i2s_status_t;

typedef struct {
    i2s_chan_handle_t i2s_chan_rx_handle;
    audio_config_t audio_config;
    QueueSetHandle_t status_queue;
} i2s_ctx_t;

static i2s_ctx_t ctx = { 0 };

static bool post_status(i2s_status_t status)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (ctx.status_queue != NULL) {
        xQueueSendFromISR(ctx.status_queue, &status, &xHigherPriorityTaskWoken);
    }   
    return xHigherPriorityTaskWoken;
}

static bool i2s_rx_q_ovf(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
    return post_status(I2S_ERROR_DMA_OVERFLOW);
}

static bool i2s_rx_callback(i2s_chan_handle_t handle, i2s_event_data_t* event, void* user_ctx)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint8_t* audio_data = (event->data)+12; //! Workaround. 

    xStreamBufferSendFromISR(
        ctx.audio_config.stream_buffer_handle,
        audio_data,
        event->size,
        &xHigherPriorityTaskWoken);
        
    return xHigherPriorityTaskWoken;
}

esp_err_t i2s_init(audio_config_t* audio_cfg)
{
    esp_err_t result = ESP_FAIL;

    ctx.audio_config = *audio_cfg;

    if (ctx.audio_config.stream_buffer_handle == NULL) {
        ESP_LOGE(TAG, "Streambuffer NULL");
        return ESP_FAIL;
    }

    /* Make sure a valid Audio format has been set */
    if (ctx.audio_config.audio_format == PCM_FORMAT_UNKNOWN) {
        return ESP_FAIL;
    }

    i2s_chan_config_t rx_chan_cfg = {
        .id = I2S_NUM_AUTO,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 320,
        .auto_clear = false,
    };
    result = i2s_new_channel(&rx_chan_cfg, NULL, &ctx.i2s_chan_rx_handle);
    if (result != ESP_OK) {
        i2s_del_channel(ctx.i2s_chan_rx_handle);
        return result;
    }

    i2s_std_config_t i2s_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_32BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_BOTH,
            .ws_width = 32,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK_IO,
            .ws = I2S_LRCLK_IO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_ADC_DATA_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    result = i2s_channel_init_std_mode(ctx.i2s_chan_rx_handle, &i2s_cfg);

    i2s_event_callbacks_t cbs = {
        .on_recv_q_ovf = i2s_rx_q_ovf
    };
    esp_err_t ovf_monitor = i2s_channel_register_event_callback(ctx.i2s_chan_rx_handle, &cbs, NULL);
    if (ovf_monitor == ESP_OK) {
        ESP_LOGI(TAG, "I2S DMA overflow monitoring registered");
    }

    return result;
}

esp_err_t i2s_deinit(void)
{
    esp_err_t result = ESP_FAIL;
    result = i2s_del_channel(ctx.i2s_chan_rx_handle);
    return result;
}

esp_err_t i2s_enable_clk(void)
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGI(TAG, "Enabling I2S clock");
    ret = i2s_channel_enable(ctx.i2s_chan_rx_handle);
    return ret;
}

esp_err_t i2s_disable_clk(void)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Disabling I2S clock");
    ret = i2s_channel_disable(ctx.i2s_chan_rx_handle);
    return ret;
}

esp_err_t i2s_read(uint8_t* audio_data, size_t* bytes_read)
{
    return i2s_channel_read(
        ctx.i2s_chan_rx_handle,
        audio_data,
        ctx.audio_config.i2s_dma_size,
        bytes_read,
        pdMS_TO_TICKS(I2S_READ_TIMEOUT_MS));
}

esp_err_t i2s_enable_async_read(void)
{
    i2s_event_callbacks_t cbs = {
        .on_recv = i2s_rx_callback,
        .on_recv_q_ovf = NULL,
        .on_send_q_ovf = NULL,
        .on_sent = NULL
    };
    return i2s_channel_register_event_callback(ctx.i2s_chan_rx_handle, &cbs, NULL);
}

esp_err_t i2s_disable_async_read(void)
{
    i2s_event_callbacks_t cbs = {
        .on_recv = NULL,
    };
    return i2s_channel_register_event_callback(ctx.i2s_chan_rx_handle, &cbs, NULL);
}

void i2s_monitor_task(void* pvParam)
{
    ctx.status_queue = xQueueCreate(10, sizeof(i2s_status_t));
    if (ctx.status_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create status queue");
    }

    while(1) {
        i2s_status_t status;
        xQueueReceive(ctx.status_queue, &status, portMAX_DELAY);
        ESP_LOGI(TAG, "Status: %u", status);
    }
}
