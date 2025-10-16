/*
 * HDMI Audio Driver Interface for Raspberry Pi 4/5
 * Public API for audio output via VideoCore GPU
 */

#ifndef ARM_AUDIO_HDMI_H
#define ARM_AUDIO_HDMI_H

#include <stdint.h>

/* Audio configuration constants */
#define AUDIO_HDMI_SAMPLE_RATE  48000
#define AUDIO_HDMI_CHANNELS     2
#define AUDIO_HDMI_BITS         16

/* Initialize HDMI audio driver
 * Returns: 0 on success, -1 on failure
 */
int audio_hdmi_init(void);

/* Enable/disable HDMI audio output */
int audio_hdmi_enable(void);
int audio_hdmi_disable(void);
int audio_hdmi_is_enabled(void);

/* Write PCM audio samples to buffer
 * samples: Pointer to 16-bit stereo samples (interleaved LRLR...)
 * sample_count: Number of stereo samples (each = 4 bytes)
 * Returns: Number of samples written, -1 on error
 */
int audio_hdmi_write_samples(const int16_t *samples, uint32_t sample_count);

/* Flush audio buffer to hardware
 * Returns: 0 on success, -1 on error
 */
int audio_hdmi_flush(void);

/* Query buffer state */
uint32_t audio_hdmi_get_buffer_size(void);
uint32_t audio_hdmi_get_buffer_used(void);
uint32_t audio_hdmi_get_buffer_free(void);

/* Reset audio buffer to empty */
void audio_hdmi_reset_buffer(void);

/* Get audio configuration */
uint32_t audio_hdmi_get_sample_rate(void);
uint32_t audio_hdmi_get_channels(void);
uint32_t audio_hdmi_get_bits_per_sample(void);

/* Shutdown audio */
void audio_hdmi_shutdown(void);

/* Generate test tone (debugging) */
void audio_hdmi_test_tone(void);

#endif /* ARM_AUDIO_HDMI_H */
