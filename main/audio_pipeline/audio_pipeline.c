/**
 * @file audio_pipeline.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "audio_pipeline.h"
#include "audio_pipeline_msg.h"
#include "config/audio_config.h"
#include "i2s/i2s.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "portable.h"
#include <stdint.h>

typedef struct {
    QueueHandle_t msg_queue;
    audio_config_t audio_config;
    uint8_t* audio_buffer;
} pipeline_ctx_t;

static pipeline_ctx_t ctx = { 0 };

static const char* TAG = "audio-pipeline";

esp_err_t audio_pipeline_init(audio_config_t* audio_config)
{
    esp_err_t ret = ESP_FAIL;

    audio_config->stream_buffer_handle = xStreamBufferCreate(
        audio_config->stream_buffer_total_size,
        audio_config->audio_bytes_per_ms);
    
    if (audio_config->stream_buffer_handle != NULL) {
        ret = ESP_OK;
        ESP_LOGI(TAG, "Created streambuffer size: %lu bytes", audio_config->stream_buffer_total_size);
    }
    
    ctx.audio_config = *audio_config;
    
    return ret;
}

void audio_pipeline_task(void* pvParams)
{
    ctx.msg_queue = xQueueCreate( 10, sizeof(audio_pipeline_message_t) );
    if (ctx.msg_queue != NULL) {
        ESP_LOGI(TAG, "Message queue created");
    }

    ctx.audio_buffer = (uint8_t*)pvPortMalloc(ctx.audio_config.i2s_dma_size);
    size_t bytes_read = 0;

    while (1) {
        // Send to buffer

        i2s_read(ctx.audio_buffer, &bytes_read);

        size_t bytes_written = xStreamBufferSend(
            ctx.audio_config.stream_buffer_handle,
            ctx.audio_buffer,
            ctx.audio_config.i2s_dma_size,
            0);

        if (bytes_written != ctx.audio_config.i2s_dma_size) {
            ESP_LOGE(TAG, "bytes_written != i2s_dma_size");
        }


        // audio_pipeline_message_t message;
        // xQueueReceive(ctx.msg_queue, &message, portMAX_DELAY);
        // ESP_LOGI(TAG, "Mesage ID: %lu", message);
    }
}

void audio_pipeline_msg_post(audio_pipeline_message_t msg)
{
    if (ctx.msg_queue != NULL) {
        xQueueSend(ctx.msg_queue, (void*)msg, 0);
    }
}

void audio_pipeline_msg_post_from_isr(audio_pipeline_message_t msg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (ctx.msg_queue != NULL) {
        xQueueSendFromISR(ctx.msg_queue, (void*)msg, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken) {
        // TODO: yield
    }
}

esp_err_t audio_pipeline_flush(void)
{
    esp_err_t ret = ESP_FAIL;

    if (ctx.audio_config.stream_buffer_handle == NULL) {
        return ESP_FAIL;
    }

    ESP_LOGW(TAG, "Flushing pipeline");
    if (xStreamBufferReset(ctx.audio_config.stream_buffer_handle) == pdTRUE) {
        ret = ESP_OK;
    }
    return ret;
}