/**
 * @file audio.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdbool.h>
#include <stdint.h>

#include "audio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

#include "esp_log.h"

#include "adc/adc.h"
#include "config/audio_config.h"
#include "usb/usb_audio.h"

static const char* TAG = "AUDIO";

static audio_config_t audio_config;
static StreamBufferHandle_t stream_buffer_handle = NULL;

esp_err_t audio_pipeline_init()
{
    audio_config = create_audio_config(I2S_BITS_PER_CHAN_32BIT, I2S_BITS_PER_SAMPLE_24BIT);

    stream_buffer_handle = xStreamBufferCreate(audio_config.stream_buffer_total_size, audio_config.stream_buffer_trigger_size);
    if (stream_buffer_handle == NULL) {
        ESP_LOGE(TAG, "Error creating xStreamBuffer!");
        return ESP_FAIL;
    }
    audio_config.stream_buffer_handle = stream_buffer_handle;

    adc_start(&audio_config);
    usb_audio_start(&audio_config);

    return ESP_OK;
}
