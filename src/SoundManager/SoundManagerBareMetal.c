/*
 * SoundManagerBareMetal.c - Bare-metal Sound Manager implementation
 *
 * Minimal Sound Manager for bare-metal x86 environment.
 * Provides SysBeep() and API stubs without threading dependencies.
 */

#include "SystemTypes.h"
#include <stdint.h>
#include <stdbool.h>

/* Error codes */
#define unimpErr -4  /* Unimplemented trap */

/* PC Speaker hardware functions */
extern int PCSpkr_Init(void);
extern void PCSpkr_Shutdown(void);
extern void PCSpkr_Beep(uint32_t frequency, uint32_t duration_ms);

/* Sound Manager state */
static bool g_soundManagerInitialized = false;

/*
 * SoundManagerInit - Initialize Sound Manager
 *
 * Called during system startup from SystemInit.c
 * Returns noErr on success
 */
OSErr SoundManagerInit(void) {
    extern void serial_printf(const char* fmt, ...);

    serial_printf("SoundManagerInit: ENTRY (initialized=%d)\n", g_soundManagerInitialized);

    if (g_soundManagerInitialized) {
        serial_printf("SoundManagerInit: Already initialized, returning\n");
        return noErr;
    }

    serial_printf("SoundManagerInit: Initializing bare-metal Sound Manager\n");

    /* Initialize PC speaker hardware */
    int pcspkr_result = PCSpkr_Init();
    serial_printf("SoundManagerInit: PCSpkr_Init returned %d\n", pcspkr_result);

    if (pcspkr_result != 0) {
        serial_printf("SoundManagerInit: Failed to initialize PC speaker\n");
        return -1;
    }

    g_soundManagerInitialized = true;
    serial_printf("SoundManagerInit: Sound Manager initialized successfully (flag=%d)\n", g_soundManagerInitialized);

    return noErr;
}

/*
 * SoundManagerShutdown - Shut down Sound Manager
 */
void SoundManagerShutdown(void) {
    if (!g_soundManagerInitialized) {
        return;
    }

    PCSpkr_Shutdown();
    g_soundManagerInitialized = false;
}

/*
 * SysBeep - System beep sound
 *
 * @param duration - Duration in ticks (1/60th second)
 *
 * Classic Mac OS standard beep sound (1000 Hz tone).
 */
void SysBeep(short duration) {
    extern void serial_printf(const char* fmt, ...);

    if (!g_soundManagerInitialized) {
        serial_printf("SysBeep: Sound Manager not initialized\n");
        return;
    }

    /* Convert duration from ticks (1/60 sec) to milliseconds */
    uint32_t duration_ms = (duration * 1000) / 60;

    /* Default to 200ms if duration is 0 or very short */
    if (duration_ms < 50) {
        duration_ms = 200;
    }

    serial_printf("SysBeep: duration=%d ticks (%u ms)\n", duration, duration_ms);

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
    extern void serial_printf(const char* fmt, ...);

    if (!g_soundManagerInitialized) {
        serial_printf("StartupChime: Sound Manager not initialized\n");
        return;
    }

    serial_printf("StartupChime: Playing System 7 startup chime\n");

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
    serial_printf("StartupChime: Playing C4 (262 Hz)\n");
    PCSpkr_Beep(262, 300);  /* C4 - 300ms */

    serial_printf("StartupChime: Playing E4 (330 Hz)\n");
    PCSpkr_Beep(330, 300);  /* E4 - 300ms */

    serial_printf("StartupChime: Playing G4 (392 Hz)\n");
    PCSpkr_Beep(392, 300);  /* G4 - 300ms */

    serial_printf("StartupChime: Playing C5 (523 Hz)\n");
    PCSpkr_Beep(523, 600);  /* C5 - 600ms (longer sustain on final note) */

    serial_printf("StartupChime: Complete\n");
}

/*
 * Sound Manager API Stubs
 *
 * These functions return unimplemented errors for now.
 * They can be filled in later as needed.
 */

OSErr SndNewChannel(void** chan, short synth, long init, void* userRoutine) {
    return unimpErr;
}

OSErr SndDisposeChannel(void* chan, Boolean quietNow) {
    return unimpErr;
}

OSErr SndDoCommand(void* chan, const SndCommand* cmd, Boolean noWait) {
    return unimpErr;
}

OSErr SndDoImmediate(void* chan, const SndCommand* cmd) {
    return unimpErr;
}

OSErr SndPlay(void* chan, void* sndHandle, Boolean async) {
    return unimpErr;
}

OSErr SndControl(short id, void* cmd) {
    return unimpErr;
}

/* Legacy Sound Manager 1.0 stubs */
void StartSound(const void* soundPtr, long numBytes, void* completionRtn) {
    /* No-op */
}

void StopSound(void) {
    /* No-op */
}

Boolean SoundDone(void) {
    return true;  /* Always done */
}

/* Volume control stubs */
void GetSysBeepVolume(long* level) {
    if (level) *level = 7;  /* Maximum volume */
}

void SetSysBeepVolume(long level) {
    /* No-op - PC speaker has no volume control */
}

void GetDefaultOutputVolume(long* level) {
    if (level) *level = 255;  /* Maximum volume */
}

void SetDefaultOutputVolume(long level) {
    /* No-op */
}

void GetSoundVol(short* level) {
    if (level) *level = 7;
}

void SetSoundVol(short level) {
    /* No-op */
}
