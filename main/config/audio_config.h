/**
 * File: audio_config.h
 * Project:     Classics Adapter
 * Author:      Peter Boné (PEBO)
 * Created:     25/01/2023
 *
 * @copyright Copyright BANG & OLUFSEN A/S (c) 2024
 **/

#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define SAMPLES_PER_MS (SAMPLE_RATE / 1000)
#define STREAMBUFFER_TOTAL_LEN_MS 300

typedef enum {
    BITS_PER_SAMPLE_16BIT = 16,
    BITS_PER_SAMPLE_24BIT = 24,
    BITS_PER_SAMPLE_32BIT = 32,
} bits_per_sample_t;

typedef enum {
    PCM_FORMAT_UNKNOWN,
    PCM_FORMAT_16BIT,
    PCM_FORMAT_24BIT_32BIT,
    PCM_FORMAT_32BIT
} audio_format_t;

/**
 * Configuration parameters used for setting up various buffers related to audio reading and streaming.
 */
typedef struct {
    audio_format_t audio_format;
    uint32_t audio_bytes_per_ms; // Size in bytes of total audio data stream per ms
    uint32_t i2s_dma_size;
    uint32_t stream_buffer_total_size; // Total size of the StreamBuffer in bytes
    StreamBufferHandle_t stream_buffer_handle; // Handle of the StreamBuffer
} audio_config_t;

/**
 * @brief Create a configuration struct with various audio parameters.
 *        Some values are given as parameters and others are calculated based on these.
 * @param format audio format (16/24/32bit)
 * @return Struct with configuration values
 */
static inline audio_config_t create_audio_config(audio_format_t format)
{
    // default values
    uint8_t BYTES_PER_SAMPLE_PIPELINE = 3;

    switch (format) {
    case PCM_FORMAT_16BIT:
        BYTES_PER_SAMPLE_PIPELINE = 2;
        break;
    case PCM_FORMAT_24BIT_32BIT:
        BYTES_PER_SAMPLE_PIPELINE = 3;
        break;
    case PCM_FORMAT_32BIT:
        BYTES_PER_SAMPLE_PIPELINE = 4;
        break;
    default:
        break;
    }

    const uint32_t audio_bytes_per_ms = (SAMPLES_PER_MS * BYTES_PER_SAMPLE_PIPELINE * NUM_CHANNELS); // number of audio BYTES per ms audio for pipeline

    audio_config_t audio_config = {
        .audio_format = format,
        .audio_bytes_per_ms = audio_bytes_per_ms,
        .i2s_dma_size = 2560,
        .stream_buffer_total_size = audio_bytes_per_ms * STREAMBUFFER_TOTAL_LEN_MS,
        .stream_buffer_handle = NULL,
    };

    return audio_config;
}
