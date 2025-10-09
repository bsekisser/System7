/*
 * PCSpkr.h - PC Speaker Hardware Abstraction
 *
 * Provides low-level PC speaker control for system beeps
 * and simple audio output on x86 platforms.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef __PCSPKR_H__
#define __PCSPKR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PC Speaker initialization and shutdown */
int PCSpkr_Init(void);
void PCSpkr_Shutdown(void);

/* PC Speaker beep output */
void PCSpkr_Beep(uint32_t frequency, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* __PCSPKR_H__ */
