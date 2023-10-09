/**
 * @file usb.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "usb.h"
#include "tinyusb.h"
#include "tusb_tasks.h"
#include "usb_audio.h"

#include "esp_log.h"

static const char* TAG = "USB TASK";

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB mounted.");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "USB unmounted.");
}

void usb_init(void)
{
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    tinyusb_driver_install(&tusb_cfg);

    usb_audio_init();
}

void usb_deinit(void)
{
    // There is atm no tinyusb_driver_uninstall(void); (IDF-1474)
    ESP_LOGI(TAG, "Stopping tinyusb task");
    tusb_stop_task();
}
