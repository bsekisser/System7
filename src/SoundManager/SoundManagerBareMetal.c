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
#include "SoundManager/SoundEffects.h"
#include "SoundManager/SoundBackend.h"
#include "SoundManager/boot_chime_data.h"
#include "config/sound_config.h"
#include "SystemInternal.h"

/* Error codes */
#define unimpErr -4  /* Unimplemented trap */
#ifndef notOpenErr
#define notOpenErr (-28)
#endif

/* PC Speaker hardware functions */
extern int PCSpkr_Init(void);
extern void PCSpkr_Shutdown(void);
extern void PCSpkr_Beep(uint32_t frequency, uint32_t duration_ms);

/* Sound Manager state */
static bool g_soundManagerInitialized = false;
static const SoundBackendOps* g_soundBackendOps = NULL;
static SoundBackendType g_soundBackendType = kSoundBackendNone;
static bool gStartupChimeAttempted = false;
static bool gStartupChimePlayed = false;

OSErr SoundManager_PlayPCM(const uint8_t* data,
                           uint32_t sizeBytes,
                           uint32_t sampleRate,
                           uint8_t channels,
                           uint8_t bitsPerSample)
{
    if (!data || sizeBytes == 0) {
        return paramErr;
    }

    if (!g_soundManagerInitialized || !g_soundBackendOps || !g_soundBackendOps->play_pcm) {
        return notOpenErr;
    }

    return g_soundBackendOps->play_pcm(data, sizeBytes, sampleRate, channels, bitsPerSample);
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

    /* Attempt to initialize configured sound backend */
    const SoundBackendOps* candidate = SoundBackend_GetOps(DEFAULT_SOUND_BACKEND);
    if (candidate && candidate->init) {
        OSErr initErr = candidate->init();
        if (initErr == noErr) {
            g_soundBackendOps = candidate;
            g_soundBackendType = DEFAULT_SOUND_BACKEND;
            SND_LOG_INFO("SoundManagerInit: Selected %s backend\n", candidate->name);
        } else {
            SND_LOG_WARN("SoundManagerInit: Backend %s init failed (err=%d), falling back to speaker\n",
                         candidate->name, initErr);
        }
    }

    if (!g_soundBackendOps) {
        SND_LOG_WARN("SoundManagerInit: No advanced sound backend available, using PC speaker only\n");
    }

    gStartupChimeAttempted = false;
    gStartupChimePlayed = false;

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
    gStartupChimeAttempted = false;
    gStartupChimePlayed = false;
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
    (void)duration;
    (void)SoundEffects_Play(kSoundEffectBeep);
}

/*
 * StartupChime - Classic System 7 startup sound
 *
 * Plays the iconic Macintosh startup chime - a C major chord arpeggio.
 * This recreates the classic "boooong" sound that Mac users know and love.
 */
void StartupChime(void) {

    if (gStartupChimeAttempted) {
        SND_LOG_DEBUG("StartupChime: Already attempted, skipping\n");
        return;
    }

    gStartupChimeAttempted = true;

    OSErr err = SoundEffects_Play(kSoundEffectStartupChime);
    if (err == noErr) {
        gStartupChimePlayed = true;
    }
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
