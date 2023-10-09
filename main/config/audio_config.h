/**
 * File: audio_config.h
 * Project:     Classics Adapter
 * Author:      Peter Boné (PEBO)
 * Created:     25/01/2023
 *
 * (c) Copyright BANG & OLUFSEN A/S.
 **/

#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

#define I2S_NUM I2S_NUM_0
#define I2S_SAMPLE_RATE_HZ 48000
#define I2S_SAMPLES_PER_MS (I2S_SAMPLE_RATE_HZ / 1000)
#define I2S_NUM_CHANNELS 2
#define AUDIO_BUFFER_TRANSMIT_LEN_MS 20
#define AUDIO_BUFFER_MAX_LEN_MS AUDIO_BUFFER_TRANSMIT_LEN_MS * 5

typedef enum {
    I2S_BITS_PER_CHAN_16BIT = 16,
    I2S_BITS_PER_CHAN_32BIT = 32,
} i2s_bits_per_chan_t;

typedef enum {
    I2S_BITS_PER_SAMPLE_16BIT = 16,
    I2S_BITS_PER_SAMPLE_24BIT = 24,
    I2S_BITS_PER_SAMPLE_32BIT = 32,
} i2s_bits_per_sample_t;

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
    i2s_bits_per_chan_t i2s_bits_per_chan;
    i2s_bits_per_sample_t i2s_bits_per_sample;
    audio_format_t audio_format;
    uint32_t audio_data_pr_ms; // Size in bytes of total audio data stream per ms
    uint16_t i2s_read_buffer_size; // Size in bytes of the DMA buffer used for reading from I2S on each iteration
    StreamBufferHandle_t stream_buffer_handle; // Handle of the StreamBuffer
    uint32_t stream_buffer_total_size; // Total size of the StreamBuffer in bytes
    uint32_t stream_buffer_trigger_size; // Number of bytes to read from the StreamBuffer on each iteration
} audio_config_t;

/**
 * @brief Create a configuration struct with various parameters relating to reading i2s audio and streaming it afterwards.
 * Some values are given as parameters and others are calculated based on these.
 * @param i2s_bits_per_chan Number of bits in each channel when reading from I2S
 * @param i2s_bits_per_sample  Number of bits in each sample when reading from I2S (can be smaller than the channel)
 * @return Struct with configuration values
 */
static inline audio_config_t create_audio_config(i2s_bits_per_chan_t i2s_bits_per_chan, i2s_bits_per_sample_t i2s_bits_per_sample)
{
    const uint32_t audio_data_pr_ms = (I2S_SAMPLES_PER_MS * (i2s_bits_per_chan / 8) * I2S_NUM_CHANNELS);
    const uint16_t SIZE_OF_I2S_DMA_BUFFER = 1024; // Maybe this is defined in the ESP SDK somewhere? At least we get an error if it is above that value.
    // Calculate how many ms of audio data can fit into the I2S DMA buffer. Equates to 5ms when reading 32 bit and 2ms when reading 16bit data.
    const uint16_t i2s_read_ms = SIZE_OF_I2S_DMA_BUFFER / audio_data_pr_ms;

    audio_config_t audio_config = {
        .i2s_bits_per_chan = i2s_bits_per_chan,
        .i2s_bits_per_sample = i2s_bits_per_sample,
        .audio_data_pr_ms = audio_data_pr_ms,
        .i2s_read_buffer_size = audio_data_pr_ms * i2s_read_ms,
        .stream_buffer_total_size = audio_data_pr_ms * AUDIO_BUFFER_MAX_LEN_MS,
        .stream_buffer_trigger_size = audio_data_pr_ms * AUDIO_BUFFER_TRANSMIT_LEN_MS
    };

    if (i2s_bits_per_chan == I2S_BITS_PER_CHAN_16BIT) {
        audio_config.audio_format = PCM_FORMAT_16BIT;
    } else if ((i2s_bits_per_chan == I2S_BITS_PER_CHAN_32BIT) && (i2s_bits_per_sample == I2S_BITS_PER_SAMPLE_24BIT)) {
        audio_config.audio_format = PCM_FORMAT_24BIT_32BIT;
    } else if ((i2s_bits_per_chan == I2S_BITS_PER_CHAN_32BIT) && (i2s_bits_per_sample == I2S_BITS_PER_SAMPLE_32BIT)) {
        audio_config.audio_format = PCM_FORMAT_32BIT;
    } else {
        audio_config.audio_format = PCM_FORMAT_UNKNOWN;
    }

    return audio_config;
}
