/**
 * @file audio_pipeline_msg.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

typedef enum {
    PIPELINE_STATUS_OVERRUN,
    PIPELINE_STATUS_UNDERRUN,
    PIPELINE_STATUS_OVERRUN_CLOSE,
    PIPELINE_STATUS_UNDERRUN_CLOSE,
    PIPELINE_STATUS_SOURCE_INVALID_CONFIG,
    PIPELINE_STATUS_SOURCE_NOT_ENOUGH_DATA,
    PIPELINE_STATUS_SOURCE_READ_ERROR,
    PIPELINE_STATUS_SOURCE_DMA_OVF,
    PIPELINE_STATE_STOPPED,
    PIPELINE_STATE_BUFFERING,
    PIPELINE_STATE_RUNNING,
} audio_pipeline_message_t;

void audio_pipeline_msg_post(audio_pipeline_message_t msg);

void audio_pipeline_msg_post_from_isr(audio_pipeline_message_t msg);
