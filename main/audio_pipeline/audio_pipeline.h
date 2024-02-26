/**
 * @file audio_pipeline.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "config/audio_config.h"

esp_err_t audio_pipeline_init(audio_config_t* audio_config);

esp_err_t audio_pipeline_flush(void);

void audio_pipeline_task(void* pvParams);
