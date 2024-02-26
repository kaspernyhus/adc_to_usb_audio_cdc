#include <stdio.h>

#include "esp_netif.h"
#include "esp_err.h"
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"

#include "i2s/i2s.h"
#include "usb/usb.h"
#include "usb/usb_audio.h"
#include "config/audio_config.h"
#include "audio_pipeline/audio_pipeline.h"


void app_main(void)
{
    usb_init();
    
    audio_config_t audio_config = create_audio_config(PCM_FORMAT_32BIT);
    ESP_ERROR_CHECK(audio_pipeline_init(&audio_config));
    
    xTaskCreate(i2s_monitor_task, "i2s mon task", 4096, NULL, 1, NULL);
    
    ESP_ERROR_CHECK(i2s_init(&audio_config));
    ESP_ERROR_CHECK(i2s_enable_async_read());
    ESP_ERROR_CHECK(i2s_enable_clk());
    ESP_ERROR_CHECK(usb_audio_start(&audio_config));

    // xTaskCreate(audio_pipeline_task, "pipeline task", 4096, NULL, 6, NULL);
}
