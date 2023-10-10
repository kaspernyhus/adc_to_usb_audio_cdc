/**
 * @file usb_cdc.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-10-09
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tinyusb.h"
#include "tusb_cdc_acm.h"

#include "usb_cdc.h"

static const char* TAG = "USB-CDC";

static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

static bool wanted_char_flag = false;
static uint8_t message[2048] = { 0 };
static size_t msg_idx = 0;

esp_err_t tusb_serial_write(const char* buffer, size_t length)
{
    ESP_LOGD(TAG, "Send:    %s", buffer);
    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, (uint8_t*)buffer, length);
    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
    return ESP_OK;
}

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t* event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        if (wanted_char_flag) {
            wanted_char_flag = false;
            ESP_LOGD(TAG, "Message: %s", message);
            tusb_serial_write((char*)message, msg_idx);
            msg_idx = 0;
            memset(message, 0, sizeof(message));
        } else {
            memcpy(message + msg_idx, buf, rx_size);
            msg_idx += rx_size;
        }
    } else {
        ESP_LOGE(TAG, "Read error");
    }
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

void wanted_char_cb(int itf, cdcacm_event_t* event)
{
    wanted_char_flag = true;
}

void usb_cdc_init(void)
{
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .callback_rx = &tinyusb_cdc_rx_callback,
        .callback_rx_wanted_char = &wanted_char_cb,
        .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    tud_cdc_n_set_wanted_char(TINYUSB_CDC_ACM_0, '\r');
}