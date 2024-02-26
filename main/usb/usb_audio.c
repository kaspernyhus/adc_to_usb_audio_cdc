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

#include "freertos/portmacro.h"
#include "projdefs.h"
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
        audio_bytes_read = xStreamBufferReceive(
            audio_config.stream_buffer_handle, 
            (void*)audio_data,
            audio_config.audio_bytes_per_ms,
            0);
        
        if (audio_bytes_read != audio_config.audio_bytes_per_ms) {
            ESP_LOGW(TAG, "Expected %lu bytes from StreamBuffer, Sending %zu bytes to USB",
                audio_config.audio_bytes_per_ms,
                audio_bytes_read);
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
    audio_data = pvPortMalloc(audio_config.audio_bytes_per_ms);
    ESP_LOGI(TAG, "Created audio_data buffer of size: %lu", audio_config.audio_bytes_per_ms);
    if (audio_data != NULL) {
        usb_audio_stream_running = true;
        ret = ESP_OK;
    }
    memset(audio_data, 0, audio_config.audio_bytes_per_ms);
    return ret;
}

esp_err_t usb_audio_stop()
{
    usb_audio_stream_running = false;
    return ESP_OK;
}
