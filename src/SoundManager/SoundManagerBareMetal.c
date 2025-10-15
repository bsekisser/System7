/*
 * SoundManagerBareMetal.c - Bare-metal Sound Manager implementation
 *
 * Minimal Sound Manager for bare-metal x86 environment.
 * Provides SysBeep() and API stubs without threading dependencies.
 */

#include "SystemTypes.h"
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include "SoundManager/SoundManager.h"
#include "SoundManager/SoundLogging.h"
#include "SoundManager/SoundBlaster16.h"
#include "SoundManager/boot_chime_data.h"
#include "TimeManager/MicrosecondTimer.h"
#include "SystemInternal.h"

/* Error codes */
#define unimpErr -4  /* Unimplemented trap */

/* PC Speaker hardware functions */
extern int PCSpkr_Init(void);
extern void PCSpkr_Shutdown(void);
extern void PCSpkr_Beep(uint32_t frequency, uint32_t duration_ms);

/* Sound Manager state */
static bool g_soundManagerInitialized = false;
static bool g_sb16Available = false;

/* DMA playback scratch buffer (<= 120 KB per transfer, 32-byte aligned) */
#define SB16_DMA_CHUNK_BYTES 120000U
static uint8_t g_sb16DMABuffer[SB16_DMA_CHUNK_BYTES] __attribute__((aligned(32)));

static uint64_t smb_udiv64(uint64_t num, uint64_t den) {
    if (den == 0) {
        return 0;
    }
    uint64_t quot = 0;
    uint64_t bit = 1ULL << 63;
    while (bit && !(den & bit)) {
        bit >>= 1;
    }
    while (bit) {
        if (num >= den) {
            num -= den;
            quot |= bit;
        }
        den >>= 1;
        bit >>= 1;
    }
    return quot;
}

/*
 * SoundManagerInit - Initialize Sound Manager
 *
 * Called during system startup from SystemInit.c
 * Returns noErr on success
 */
OSErr SoundManagerInit(void) {

    SND_LOG_TRACE("SoundManagerInit: ENTRY (initialized=%d)\n", g_soundManagerInitialized);

    if (g_soundManagerInitialized) {
        SND_LOG_DEBUG("SoundManagerInit: Already initialized, returning\n");
        return noErr;
    }

    SND_LOG_INFO("SoundManagerInit: Initializing bare-metal Sound Manager\n");

    /* Initialize PC speaker hardware */
    int pcspkr_result = PCSpkr_Init();
    SND_LOG_DEBUG("SoundManagerInit: PCSpkr_Init returned %d\n", pcspkr_result);

    if (pcspkr_result != 0) {
        SND_LOG_ERROR("SoundManagerInit: Failed to initialize PC speaker\n");
        return -1;
    }

    /* Attempt to initialize Sound Blaster 16 for PCM playback */
    int sb16_result = SB16_Init();
    if (sb16_result == 0) {
        g_sb16Available = true;
        SND_LOG_INFO("SoundManagerInit: SB16 available for PCM playback\n");
    } else {
        SND_LOG_WARN("SoundManagerInit: SB16 unavailable (code=%d), falling back to PC speaker only\n",
                     sb16_result);
    }

    g_soundManagerInitialized = true;
    SND_LOG_INFO("SoundManagerInit: Sound Manager initialized successfully (flag=%d)\n", g_soundManagerInitialized);

    return noErr;
}

/*
 * SoundManagerShutdown - Shut down Sound Manager
 */
OSErr SoundManagerShutdown(void) {
    if (!g_soundManagerInitialized) {
        return noErr;
    }

    PCSpkr_Shutdown();
    if (g_sb16Available) {
        SB16_Shutdown();
        g_sb16Available = false;
    }
    g_soundManagerInitialized = false;
    return noErr;
}

/*
 * SysBeep - System beep sound
 *
 * @param duration - Duration in ticks (1/60th second)
 *
 * Classic Mac OS standard beep sound (1000 Hz tone).
 */
void SysBeep(short duration) {

    if (!g_soundManagerInitialized) {
        SND_LOG_WARN("SysBeep: Sound Manager not initialized\n");
        return;
    }

    /* Convert duration from ticks (1/60 sec) to milliseconds */
    uint32_t duration_ms = (duration * 1000) / 60;

    /* Default to 200ms if duration is 0 or very short */
    if (duration_ms < 50) {
        duration_ms = 200;
    }

    SND_LOG_TRACE("SysBeep: duration=%d ticks (%u ms)\n", duration, duration_ms);

    /* Generate 1000 Hz beep (classic Mac beep frequency) */
    PCSpkr_Beep(1000, duration_ms);
}

/*
 * StartupChime - Classic System 7 startup sound
 *
 * Plays the iconic Macintosh startup chime - a C major chord arpeggio.
 * This recreates the classic "boooong" sound that Mac users know and love.
 */
void StartupChime(void) {

    if (!g_soundManagerInitialized) {
        SND_LOG_WARN("StartupChime: Sound Manager not initialized\n");
        return;
    }

    /* Attempt high-fidelity playback first */
    if (g_sb16Available) {
        const uint8_t* src = gBootChimePCM;
        uint32_t remaining = BOOT_CHIME_DATA_SIZE;
        const uint32_t bytesPerFrame = (BOOT_CHIME_BITS_PER_SAMPLE / 8) * BOOT_CHIME_CHANNELS;
        int sbErr = 0;

        SND_LOG_INFO("StartupChime: Playing PCM boot chime via SB16 (%u bytes)\n",
                     BOOT_CHIME_DATA_SIZE);

        while (remaining > 0) {
            uint32_t chunk = (remaining > SB16_DMA_CHUNK_BYTES) ? SB16_DMA_CHUNK_BYTES : remaining;
            chunk -= chunk % bytesPerFrame;
            if (chunk == 0) {
                break;
            }

            memcpy(g_sb16DMABuffer, src, chunk);
            sbErr = SB16_PlayWAV(g_sb16DMABuffer,
                                 chunk,
                                 BOOT_CHIME_SAMPLE_RATE,
                                 BOOT_CHIME_CHANNELS,
                                 BOOT_CHIME_BITS_PER_SAMPLE);
            if (sbErr != 0) {
                SND_LOG_WARN("StartupChime: SB16 playback chunk failed (err=%d)\n", sbErr);
                break;
            }

            /* Wait for chunk duration so DMA can finish before queueing next block */
            uint64_t frames = chunk / bytesPerFrame;
            uint64_t chunkUs = smb_udiv64(frames * 1000000ULL, BOOT_CHIME_SAMPLE_RATE);
            if (chunkUs > 0) {
                if (chunkUs > UINT32_MAX) {
                    /* Break into multiple microsecond delays if necessary */
                    uint64_t remainingUs = chunkUs;
                    while (remainingUs > 0) {
                        UInt32 stepUs = (remainingUs > UINT32_MAX) ? UINT32_MAX : (UInt32)remainingUs;
                        MicrosecondDelay(stepUs);
                        remainingUs -= stepUs;
                    }
                } else {
                    MicrosecondDelay((UInt32)chunkUs);
                }
            }

            src += chunk;
            remaining -= chunk;
        }

        if (sbErr == 0) {
            SND_LOG_INFO("StartupChime: SB16 playback complete\n");
            return;
        }

        SND_LOG_WARN("StartupChime: Falling back to PC speaker chime\n");
    }

    /* Fallback: PC speaker arpeggio */
    SND_LOG_INFO("StartupChime: Playing System 7 startup chime\n");

    /* Classic Mac startup chime - C major chord arpeggio
     * Played as a quick succession of notes to create the iconic sound
     *
     * Musical notes (in Hz):
     * C4 = 261.63 Hz (middle C)
     * E4 = 329.63 Hz (major third)
     * G4 = 392.00 Hz (perfect fifth)
     * C5 = 523.25 Hz (octave)
     */

    /* Play the chord as an arpeggio with longer sustain for audibility */
    SND_LOG_TRACE("StartupChime: Playing C4 (262 Hz)\n");
    PCSpkr_Beep(262, 300);  /* C4 - 300ms */

    SND_LOG_TRACE("StartupChime: Playing E4 (330 Hz)\n");
    PCSpkr_Beep(330, 300);  /* E4 - 300ms */

    SND_LOG_TRACE("StartupChime: Playing G4 (392 Hz)\n");
    PCSpkr_Beep(392, 300);  /* G4 - 300ms */

    SND_LOG_TRACE("StartupChime: Playing C5 (523 Hz)\n");
    PCSpkr_Beep(523, 600);  /* C5 - 600ms (longer sustain on final note) */

    SND_LOG_INFO("StartupChime: Complete\n");
}

/*
 * Sound Manager API Stubs
 *
 * These functions return unimplemented errors for now.
 * They can be filled in later as needed.
 */

OSErr SndNewChannel(SndChannelPtr* chan, SInt16 synth, SInt32 init, SndCallBackProcPtr userRoutine) {
    return unimpErr;
}

OSErr SndDisposeChannel(SndChannelPtr chan, Boolean quietNow) {
    return unimpErr;
}

OSErr SndDoCommand(SndChannelPtr chan, const SndCommand* cmd, Boolean noWait) {
    return unimpErr;
}

OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd) {
    return unimpErr;
}

OSErr SndPlay(SndChannelPtr chan, SndListHandle sndHandle, Boolean async) {
    return unimpErr;
}

OSErr SndControl(SInt16 id, SndCommand* cmd) {
    return unimpErr;
}

/* Legacy Sound Manager 1.0 stubs */
void StartSound(const void* soundPtr, SInt32 numBytes, SoundCompletionUPP completionRtn) {
    /* No-op */
}

void StopSound(void) {
    /* No-op */
}

Boolean SoundDone(void) {
    return true;  /* Always done */
}

/* Volume control stubs */
void GetSysBeepVolume(SInt32* level) {
    if (level) *level = 7;  /* Maximum volume */
}

OSErr SetSysBeepVolume(SInt32 level) {
    /* No-op - PC speaker has no volume control */
    return noErr;
}

void GetDefaultOutputVolume(SInt32* level) {
    if (level) *level = 255;  /* Maximum volume */
}

OSErr SetDefaultOutputVolume(SInt32 level) {
    /* No-op */
    return noErr;
}

void GetSoundVol(SInt16* level) {
    if (level) *level = 7;
}

void SetSoundVol(SInt16 level) {
    /* No-op */
}
