/**
 * @file i2s.h
 * @author Kasper Nyhus Kaae (KANK)
 * @brief
 * @version 0.1
 * @date 2022-10-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "config/audio_config.h"
#include "esp_err.h"
#include "driver/i2s_types.h"

/**
 * @brief Initialize I2S driver with audio configuration
 *
 * @param audio_config
 * @return ESP_OK if i2s driver is successfully installed, ESP_FAIL otherwise
 */
esp_err_t i2s_init(audio_config_t* audio_config);

/**
 * @brief Deinitialize I2S driver
 *
 * @return ESP_OK if i2s driver is successfully uninstalled, ESP_FAIL otherwise
 */
esp_err_t i2s_deinit(void);

/**
 * @brief Start audio processing
 *
 * @return
 */
esp_err_t i2s_enable_clk(void);

/**
 * @brief Stop audio processing
 *
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t i2s_disable_clk(void);

/**
 * @brief Blocking read of audio data from the DMA buffers
 *
 * @return esp_err_t
 */
esp_err_t i2s_read(uint8_t* audio_data, size_t* bytes_read);

/**
 * @brief 
 * 
 * @return esp_err_t 
 */
esp_err_t i2s_enable_async_read(void);

/**
 * @brief 
 * 
 */
esp_err_t i2s_disable_async_read(void);

void i2s_monitor_task(void* pvParam);
