/**
 * @file adc.c
 * @author Kasper Nyhus Kaae (KANK)
 * @brief
 * @version 0.1
 * @date 2022-10-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"

#include "adc.h"
#include "config/audio_config.h"
#include "config/pin_config.h"
#include "hal/i2s_types.h"

#ifdef CONFIG_SIG_GEN_AUDIO_SOURCE
#include "esp_signal_generator.h"
#define USE_SIG_GEN
esp_sig_gen_t sig_gen;
#endif

#define SHUTDOWN_WAIT_TIMEOUT_TICKS 100

static const char* TAG = "ADC";

#ifndef USE_SIG_GEN
static i2s_chan_handle_t i2s_chan_rx_handle;
#endif

static TaskHandle_t adc_task_handle = NULL;
static SemaphoreHandle_t semaphore_wait;
static uint8_t* i2s_data;
static audio_config_t audio_config = {};
bool audio_running = false;

static esp_err_t adc_i2s_init(void)
{
    /* Make sure a valid Audio format has been set */
    if (audio_config.audio_format == PCM_FORMAT_UNKNOWN) {
        return ESP_FAIL;
    }

    /* Make sure a valid streambuffer handle is present */
    if (audio_config.stream_buffer_handle == NULL) {
        return ESP_FAIL;
    }

    i2s_data = pvPortMalloc(audio_config.i2s_read_buffer_size);
    if (i2s_data == NULL) {
        ESP_LOGE(TAG, "Failed to allocated i2s read buffer.");
        return ESP_FAIL;
    }

#ifndef USE_SIG_GEN
    esp_err_t result_create_channel = ESP_FAIL;
    esp_err_t result_channel_init = ESP_FAIL;
    esp_err_t result_channel_enable = ESP_FAIL;

    i2s_chan_config_t rx_chan_cfg = {
        .id = I2S_NUM_AUTO,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 16,
        .dma_frame_num = audio_config.i2s_read_buffer_size
    };
    result_create_channel = i2s_new_channel(&rx_chan_cfg, NULL, &i2s_chan_rx_handle);

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
    result_channel_init = i2s_channel_init_std_mode(i2s_chan_rx_handle, &i2s_cfg);
    result_channel_enable = i2s_channel_enable(i2s_chan_rx_handle);
    return (result_create_channel && result_channel_init && result_channel_enable);

#else // USE_SIG_GEN
    esp_sig_gen_config_t sig_gen_cfg = {
        .sample_rate = I2S_SAMPLE_RATE_HZ,
        .wave = ESP_SIG_GEN_WAVE_SINE_TAB,
        .channels = 2,
        .volume = 0.2,
        .freq[ESP_SIG_GEN_CH_1] = 440,
        .freq[ESP_SIG_GEN_CH_2] = 552,
        .layout = ESP_SIG_GEN_LAYOUT_INTERLEAVED,
        .method = ESP_SIG_GEN_PUSH_METHOD,
        .format = ESP_SIG_GEN_AUDIO_FMT_S32LE,
        .push_interval_ms = 2,
    };
    return esp_sig_gen_init(&sig_gen, &sig_gen_cfg);
#endif // USE_SIG_GEN
}

static esp_err_t adc_i2s_deinit(void)
{
    esp_err_t ret = ESP_FAIL;
#ifndef USE_SIG_GEN
    esp_err_t driver_uninstall = i2s_del_channel(i2s_chan_rx_handle);
    ;
    if (i2s_data != NULL) {
        free(i2s_data);
        i2s_data = NULL;
    }
    if ((driver_uninstall == ESP_OK) && (i2s_data == NULL)) {
        ret = ESP_OK;
    }
#else // USE_SIG_GEN
    ret = esp_sig_gen_deinit(&sig_gen);
#endif
    return ret;
}

esp_err_t adc_start(audio_config_t* audio_cfg)
{
    esp_err_t ret = ESP_FAIL;
    audio_config = *audio_cfg;
    ESP_LOGI(TAG, "Audio configuration: %dbits / %dbits per channel. DMA size: %d bytes.",
        audio_config.i2s_bits_per_sample, audio_config.i2s_bits_per_chan, audio_config.i2s_read_buffer_size);
    if (adc_i2s_init() == ESP_OK) {
        audio_running = true;
        ESP_LOGI(TAG, "Resuming adc task");
        vTaskResume(adc_task_handle);
        ret = ESP_OK;
    }
    return ret;
}

esp_err_t adc_stop(void)
{
    esp_err_t ret = ESP_OK;
    audio_running = false;
    // block until task has suspended itself
    ESP_LOGD(TAG, "Waiting on task to shut down");
    if (xSemaphoreTake(semaphore_wait, SHUTDOWN_WAIT_TIMEOUT_TICKS) != pdTRUE) {
        ESP_LOGE(TAG, "Shutdown wait time expired");
        ret = ESP_FAIL;
    }
    return ret;
}

static void adc_run()
{
    esp_err_t result_i2s;
    size_t i2s_bytes_read = 0;
    size_t stream_buffer_written = 0;

#ifndef USE_SIG_GEN
    result_i2s = i2s_channel_read(i2s_chan_rx_handle, i2s_data, audio_config.i2s_read_buffer_size, &i2s_bytes_read, pdMS_TO_TICKS(1000));
#else
    result_i2s = esp_sig_gen_read(&sig_gen, i2s_data, audio_config.i2s_read_buffer_size, &i2s_bytes_read, pdMS_TO_TICKS(1000));
#endif

    if (result_i2s == ESP_OK) {
        if (i2s_bytes_read != audio_config.i2s_read_buffer_size) {
            ESP_LOGW(TAG, "Expected %d bytes from I2S, Received %zu bytes from I2S driver.", audio_config.i2s_read_buffer_size, i2s_bytes_read);
        }
        stream_buffer_written = xStreamBufferSend(audio_config.stream_buffer_handle, i2s_data, i2s_bytes_read, pdMS_TO_TICKS(1000));
        if (stream_buffer_written != i2s_bytes_read) {
            ESP_LOGW(TAG, "Expected to write %zu bytes from I2S. Only able to write %zu bytes to stream buffer.",
                i2s_bytes_read,
                stream_buffer_written);
        }
    } else {
        ESP_LOGE(TAG, "Error while reading I2S.");
    }
}

void adc_task(void* pvParams)
{
    adc_task_handle = xTaskGetCurrentTaskHandle();
    semaphore_wait = xSemaphoreCreateBinary();
    if (semaphore_wait == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore.");
    }

    // Suspend Non-FSM Tasks by default
    vTaskSuspend(NULL);

    for (;;) {
        if (audio_running) {
            adc_run();
        } else {
            if (adc_i2s_deinit() == ESP_OK) {
                ESP_LOGI(TAG, "Releasing audio resources and suspending task");
            } else {
                ESP_LOGE(TAG, "Failed to release audio resources");
            }
            xSemaphoreGive(semaphore_wait);
            vTaskSuspend(NULL);
        }
    }
}
