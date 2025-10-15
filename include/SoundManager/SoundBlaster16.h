/*
 * SoundBlaster16.h - Sound Blaster 16 driver interface
 *
 * Provides simple PCM playback support for the bare-metal Sound Manager.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* DSP accessors (shared with DMA controller) */
bool sb16_dsp_write(uint8_t value);

/* High-level driver entry points */
int  SB16_Init(void);
void SB16_Shutdown(void);
void SB16_StopPlayback(void);
int  SB16_PlayWAV(const uint8_t* data, uint32_t size,
                  uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample);
int  SB16_PlayDMA(const uint8_t* data, uint32_t size,
                  uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample);
