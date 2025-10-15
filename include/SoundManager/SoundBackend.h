/*
 * SoundBackend.h - Runtime-selectable sound backends
 */

#pragma once

#include "SystemTypes.h"

typedef enum SoundBackendType {
    kSoundBackendNone = 0,
    kSoundBackendHDA,
    kSoundBackendSB16
} SoundBackendType;

typedef struct SoundBackendOps {
    SoundBackendType type;
    const char* name;
    OSErr (*init)(void);
    void (*shutdown)(void);
    OSErr (*play_pcm)(const uint8_t* data,
                      uint32_t sizeBytes,
                      uint32_t sampleRate,
                      uint8_t channels,
                      uint8_t bitsPerSample);
    void (*stop)(void);
} SoundBackendOps;

const SoundBackendOps* SoundBackend_GetOps(SoundBackendType type);
const char* SoundBackend_Name(SoundBackendType type);

