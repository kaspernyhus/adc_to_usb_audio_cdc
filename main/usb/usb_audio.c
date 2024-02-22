/**
 * @file usb_audio.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdint.h>

#include "tusb_audio.h"
#include "usb_audio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

#include "esp_log.h"

static const char* TAG = "USB-AUDIO";

static audio_config_t audio_config;
static uint8_t* audio_data = NULL;
static size_t audio_bytes_read;
static bool usb_audio_stream_running = false;

esp_err_t usb_audio_transfer_data()
{
    if (usb_audio_stream_running) {
        tud_audio_write(audio_data, CFG_TUD_AUDIO_EP_SZ_IN);
    }

    return ESP_OK;
}

esp_err_t usb_audio_prepare_data()
{
    if (usb_audio_stream_running) {
        audio_bytes_read = xStreamBufferReceive(audio_config.stream_buffer_handle, (void*)audio_data, audio_config.audio_data_pr_ms, 0);
        if (audio_bytes_read != audio_config.audio_data_pr_ms) {
            ESP_LOGW(TAG, "Expected %lu bytes from StreamBuffer, Sending %zu bytes to USB",
                audio_config.stream_buffer_trigger_size,
                audio_bytes_read);
        }
    }

    if (audio_config.audio_format == PCM_FORMAT_24BIT_32BIT) {
        uint8_t* destination = audio_data;
        uint32_t data_length = audio_config.audio_data_pr_ms;
        for (int i = 0; i < (data_length / 4); i++) {
            uint16_t src_offset = i * 4;
            uint16_t dst_offset = i * 3;
            destination[dst_offset] = audio_data[src_offset + 1];
            destination[dst_offset + 1] = audio_data[src_offset + 2];
            destination[dst_offset + 2] = audio_data[src_offset + 3];
        }
    }

    return ESP_OK;
}

void usb_audio_init()
{
    tinyusb_audio_config_t cfg = {
        .on_post_callback = usb_audio_prepare_data,
        .on_pre_callback = usb_audio_transfer_data,
    };
    tusb_audio_init(&cfg);
}

esp_err_t usb_audio_start(audio_config_t* audio_cfg)
{
    esp_err_t ret = ESP_FAIL;
    audio_config = *audio_cfg;
    audio_data = pvPortMalloc(audio_config.audio_data_pr_ms);
    ESP_LOGI(TAG, "Created audio_data buffer of size: %lu", audio_config.audio_data_pr_ms);
    if (audio_data != NULL) {
        usb_audio_stream_running = true;
        ret = ESP_OK;
    }

    return ret;
}

esp_err_t usb_audio_stop()
{
    usb_audio_stream_running = false;
    return ESP_OK;
}
