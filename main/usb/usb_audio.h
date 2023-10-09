/**
 * @file usb_audio.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include "config/audio_config.h"
#include "esp_err.h"

/**
 * @brief Initialize USB audio
 *
 */
void usb_audio_init();

esp_err_t usb_audio_start(audio_config_t* audio_cfg);

esp_err_t usb_audio_stop();
