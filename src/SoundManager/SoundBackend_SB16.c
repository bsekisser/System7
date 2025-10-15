#include "SystemTypes.h"
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "SoundManager/SoundBackend.h"
#include "SoundManager/SoundBlaster16.h"
#include "SoundManager/SoundLogging.h"
#include "TimeManager/MicrosecondTimer.h"

#ifndef unimpErr
#define unimpErr (-4)
#endif
#ifndef notOpenErr
#define notOpenErr (-28)
#endif
#ifndef qErr
#define qErr (-1)
#endif

#define SB16_DMA_CHUNK_BYTES 120000U

static uint8_t g_sb16ChunkBuffer[SB16_DMA_CHUNK_BYTES] __attribute__((aligned(32)));
static bool g_sb16Ready = false;

static uint64_t sb16_div_u64_32(uint64_t num, uint32_t den)
{
    if (den == 0) {
        return 0;
    }

    uint64_t quotient = 0;
    uint64_t remainder = 0;

    for (int i = 63; i >= 0; --i) {
        remainder = (remainder << 1) | ((num >> i) & 1ULL);
        if (remainder >= den) {
            remainder -= den;
            quotient |= (1ULL << i);
        }
    }
    return quotient;
}

static OSErr SoundBackendSB16_Init(void)
{
    if (SB16_Init() == 0) {
        g_sb16Ready = true;
        SND_LOG_INFO("SoundBackend(SB16): Initialized\n");
        return noErr;
    }
    g_sb16Ready = false;
    SND_LOG_WARN("SoundBackend(SB16): Initialization failed\n");
    return unimpErr;
}

static void SoundBackendSB16_Shutdown(void)
{
    if (!g_sb16Ready) return;
    SB16_Shutdown();
    g_sb16Ready = false;
}

static OSErr SoundBackendSB16_PlayPCM(const uint8_t* data,
                                      uint32_t sizeBytes,
                                      uint32_t sampleRate,
                                      uint8_t channels,
                                      uint8_t bitsPerSample)
{
    if (!g_sb16Ready) {
        return notOpenErr;
    }

    if (!data || sizeBytes == 0) {
        return paramErr;
    }

    const uint8_t* src = data;
    uint32_t remaining = sizeBytes;
    const uint32_t frameBytes = (bitsPerSample / 8) * channels;

    while (remaining > 0) {
        uint32_t chunk = (remaining > SB16_DMA_CHUNK_BYTES) ? SB16_DMA_CHUNK_BYTES : remaining;
        if (frameBytes > 0) {
            chunk -= chunk % frameBytes;
            if (chunk == 0) {
                break;
            }
        }

        memcpy(g_sb16ChunkBuffer, src, chunk);

        int err = SB16_PlayWAV(g_sb16ChunkBuffer,
                               chunk,
                               sampleRate,
                               channels,
                               bitsPerSample);
        if (err != 0) {
            SND_LOG_WARN("SoundBackend(SB16): Playback chunk failed (err=%d)\n", err);
            SB16_StopPlayback();
            return qErr;
        }

        if (frameBytes > 0) {
            uint64_t frames = chunk / frameBytes;
            uint64_t usec64 = sb16_div_u64_32(frames * 1000000ULL, sampleRate);
            if (usec64 > 0) {
                UInt32 clamped = (usec64 > UINT32_MAX) ? UINT32_MAX : (UInt32)usec64;
                MicrosecondDelay(clamped);
            }
        }

        src += chunk;
        remaining -= chunk;
    }

    SB16_StopPlayback();
    return noErr;
}

static void SoundBackendSB16_Stop(void)
{
    if (!g_sb16Ready) return;
    SB16_StopPlayback();
}

const SoundBackendOps kSoundBackendOps_SB16 = {
    .type = kSoundBackendSB16,
    .name = "Sound Blaster 16",
    .init = SoundBackendSB16_Init,
    .shutdown = SoundBackendSB16_Shutdown,
    .play_pcm = SoundBackendSB16_PlayPCM,
    .stop = SoundBackendSB16_Stop
};
