#include <stdio.h>

#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc/adc.h"
#include "audio/audio.h"
#include "usb/usb.h"


void app_main(void)
{
    xTaskCreatePinnedToCore(adc_task, "ADC TASK", 4192, NULL, 6, NULL, APP_CPU_NUM);

    usb_init();
    audio_pipeline_init();
}
