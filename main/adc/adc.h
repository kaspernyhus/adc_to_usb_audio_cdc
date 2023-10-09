/**
 * @file audio.h
 * @author Eduard Steven Macías Uriña (ESMU)
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

/**
 * @brief FreeRTOS task to manage I2S audio stream coming from the ADC
 *
 * @param pvParams not used
 */
void adc_task(void* pvParams);

/**
 * @brief Start audio processing with a provided audio configuration
 *
 * @param audio_config audio configuration used to install i2s driver
 * @return ESP_OK if i2s driver is successfully installed, ESP_FAIL otherwise
 */
esp_err_t adc_start(audio_config_t* audio_config);

/**
 * @brief Stop audio processing and wait for the audio task to uninstall the i2s driver
 *        and release the memory used for the DMA buffers
 * @note Blocks calling task until audio task has completed its shutdown
 * @return ESO_OK on successful shutdown, ESP_FAIL otherwise
 */
esp_err_t adc_stop(void);
