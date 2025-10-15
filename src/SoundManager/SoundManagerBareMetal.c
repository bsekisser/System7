/*
 * SoundManagerBareMetal.c - Bare-metal Sound Manager implementation
 *
 * Minimal Sound Manager for bare-metal x86 environment.
 * Provides SysBeep() and API stubs without threading dependencies.
 */

#include "SystemTypes.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "SoundManager/SoundManager.h"
#include "SoundManager/SoundLogging.h"
#include "SoundManager/SoundBackend.h"
#include "SoundManager/boot_chime_data.h"
#include "config/sound_config.h"
#include "SystemInternal.h"

/* Error codes */
#define unimpErr -4  /* Unimplemented trap */

/* PC Speaker hardware functions */
extern int PCSpkr_Init(void);
extern void PCSpkr_Shutdown(void);
extern void PCSpkr_Beep(uint32_t frequency, uint32_t duration_ms);

/* Sound Manager state */
static bool g_soundManagerInitialized = false;
static const SoundBackendOps* g_soundBackendOps = NULL;
static SoundBackendType g_soundBackendType = kSoundBackendNone;

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

    /* Attempt to initialize configured sound backend */
    SoundBackendType candidates[2] = { DEFAULT_SOUND_BACKEND, kSoundBackendSB16 };
    size_t candidateCount = (DEFAULT_SOUND_BACKEND == kSoundBackendSB16) ? 1 : 2;

    for (size_t i = 0; i < candidateCount; ++i) {
        SoundBackendType type = candidates[i];
        const SoundBackendOps* ops = SoundBackend_GetOps(type);
        if (!ops || !ops->init) {
            continue;
        }

        OSErr initErr = ops->init();
        if (initErr == noErr) {
            g_soundBackendOps = ops;
            g_soundBackendType = type;
            SND_LOG_INFO("SoundManagerInit: Selected %s backend\n", ops->name);
            break;
        }

        SND_LOG_WARN("SoundManagerInit: Backend %s init failed (err=%d)\n",
                     ops->name, initErr);
    }

    if (!g_soundBackendOps) {
        SND_LOG_WARN("SoundManagerInit: No advanced sound backend available, using PC speaker only\n");
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

    if (g_soundBackendOps && g_soundBackendOps->shutdown) {
        g_soundBackendOps->shutdown();
    }
    g_soundBackendOps = NULL;
    g_soundBackendType = kSoundBackendNone;
    PCSpkr_Shutdown();
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

    if (g_soundBackendOps && g_soundBackendOps->play_pcm) {
        SND_LOG_INFO("StartupChime: Trying %s backend (%u bytes)\n",
                     g_soundBackendOps->name, BOOT_CHIME_DATA_SIZE);

        OSErr backendErr = g_soundBackendOps->play_pcm(gBootChimePCM,
                                                       BOOT_CHIME_DATA_SIZE,
                                                       BOOT_CHIME_SAMPLE_RATE,
                                                       BOOT_CHIME_CHANNELS,
                                                       BOOT_CHIME_BITS_PER_SAMPLE);
        if (g_soundBackendOps->stop) {
            g_soundBackendOps->stop();
        }

        if (backendErr == noErr) {
            SND_LOG_INFO("StartupChime: Playback complete via %s backend\n",
                         g_soundBackendOps->name);
            return;
        }

        SND_LOG_WARN("StartupChime: Backend %s failed (err=%d), falling back to PC speaker\n",
                     g_soundBackendOps->name, backendErr);
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
