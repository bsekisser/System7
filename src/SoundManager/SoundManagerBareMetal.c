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
#include "MemoryMgr/MemoryManager.h"

/* Error codes */
#define unimpErr -4      /* Unimplemented trap */
#define badChannel -233  /* Bad sound channel */
#define nullCmd 0        /* Null command */
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

/* Channel management - bare-metal simple linked list */
static SndChannelPtr g_firstChannel = NULL;
static SInt16 g_channelCount = 0;

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

    /* Dispose any remaining channels */
    while (g_firstChannel != NULL) {
        SndChannelPtr chan = g_firstChannel;
        g_firstChannel = chan->nextChan;
        /* Clear channel state */
        chan->nextChan = NULL;
        chan->callBack = NULL;
    }
    g_channelCount = 0;

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

/* ============================================================================
 * Sound Manager Channel Management
 * ============================================================================ */

/*
 * SndNewChannel - Create a new sound channel
 *
 * @param chan - Pointer to receive channel pointer
 * @param synth - Synthesizer type (sampledSynth, squareWaveSynth, etc.)
 * @param init - Initialization bits (channel initialization data)
 * @param userRoutine - Optional callback routine (not supported in bare-metal)
 * @return noErr on success, memFullErr if out of memory
 *
 * Allocates and initializes a new sound channel for audio playback.
 * The bare-metal implementation uses a simple linked list of channels.
 */
OSErr SndNewChannel(SndChannelPtr* chan, SInt16 synth, SInt32 init, SndCallBackProcPtr userRoutine) {
    SndChannelPtr newChan;

    if (chan == NULL) {
        return paramErr;
    }

    if (!g_soundManagerInitialized) {
        return notOpenErr;
    }

    /* Allocate new channel structure from system heap */
    newChan = (SndChannelPtr)NewPtr(sizeof(SndChannel));
    if (newChan == NULL) {
        return memFullErr;
    }

    /* Initialize channel structure */
    newChan->nextChan = NULL;
    newChan->firstMod = NULL;
    newChan->callBack = userRoutine;
    newChan->userInfo = 0;
    newChan->wait = 0;
    newChan->cmdInProgress.cmd = nullCmd;
    newChan->cmdInProgress.param1 = 0;
    newChan->cmdInProgress.param2 = 0;
    newChan->flags = 0;
    newChan->qLength = 0;
    newChan->qHead = 0;
    newChan->qTail = 0;

    /* Clear command queue */
    int i;
    for (i = 0; i < 128; i++) {
        newChan->queue[i].cmd = 0;
        newChan->queue[i].param1 = 0;
        newChan->queue[i].param2 = 0;
    }

    /* Add to global channel list (head insertion) */
    newChan->nextChan = g_firstChannel;
    g_firstChannel = newChan;
    g_channelCount++;

    *chan = newChan;
    return noErr;
}

/*
 * SndDisposeChannel - Dispose of a sound channel
 *
 * @param chan - Channel to dispose
 * @param quietNow - If true, stop any playing sound immediately
 * @return noErr on success, badChannel if channel not found
 *
 * Releases a sound channel and frees its resources.
 * Optionally stops any sound currently playing.
 */
OSErr SndDisposeChannel(SndChannelPtr chan, Boolean quietNow) {
    SndChannelPtr *chanPtr;
    (void)quietNow; /* Parameter not used in bare-metal */

    if (chan == NULL) {
        return badChannel;
    }

    if (!g_soundManagerInitialized) {
        return badChannel;
    }

    /* Find and remove channel from linked list */
    chanPtr = &g_firstChannel;
    while (*chanPtr != NULL) {
        if (*chanPtr == chan) {
            /* Found it - remove from list */
            *chanPtr = chan->nextChan;
            g_channelCount--;

            /* Free the channel memory */
            DisposePtr((Ptr)chan);
            return noErr;
        }
        chanPtr = &((*chanPtr)->nextChan);
    }

    /* Channel not found in list */
    return badChannel;
}

/* ============================================================================
 * Sound Command Definitions
 * ============================================================================ */

/* Sound command opcodes */
#define freqCmd         1       /* Set frequency (param2 = frequency in Hz) */
#define ampCmd          2       /* Set amplitude */
#define timbreCmd       3       /* Set timbre */
#define waveCmd         4       /* Set waveform */
#define quietCmd        5       /* Turn off sound */
#define restCmd         6       /* Rest for duration (param2 = duration in ms) */
#define noteCmd         7       /* Play note (param1 = MIDI note, param2 = amplitude) */

/*
 * Process a single sound command
 * Internal helper function that executes command logic
 */
static void SndProcessCommand(SndChannelPtr chan, const SndCommand* cmd) {
    if (!cmd) return;

    SND_LOG_DEBUG("SndDoCommand: Processing cmd=%d param1=%d param2=%d\n",
                  cmd->cmd, cmd->param1, cmd->param2);

    switch (cmd->cmd) {
        case freqCmd:
            /* Set frequency for next sound */
            if (chan) {
                chan->userInfo = cmd->param2;  /* Store frequency in userInfo */
            }
            break;

        case restCmd:
            /* Play tone with frequency from userInfo and param2 duration */
            if (chan && chan->userInfo > 0 && cmd->param2 > 0) {
                PCSpkr_Beep((UInt32)chan->userInfo, (UInt32)cmd->param2);
                chan->userInfo = 0;  /* Reset after playing */
            }
            break;

        case quietCmd:
            /* Silence */
            if (chan) {
                chan->userInfo = 0;
            }
            break;

        case noteCmd:
            /* MIDI note - convert to frequency */
            if (cmd->param1 >= 0 && cmd->param1 <= 127) {
                UInt32 noteFreq = 440;  /* A4 as default */
                if (cmd->param1 >= 57 && cmd->param1 <= 69) {
                    /* C4 to A4 range */
                    static const UInt16 freqTable[] = {
                        262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523
                    };
                    int idx = cmd->param1 - 57;
                    if (idx >= 0 && idx < 13) {
                        noteFreq = freqTable[idx];
                    }
                }
                UInt32 duration = cmd->param2 > 0 ? (UInt32)cmd->param2 : 200;
                PCSpkr_Beep(noteFreq, duration);
            }
            break;

        case ampCmd:
        case timbreCmd:
        case waveCmd:
            /* Amplitude/timbre/waveform commands - not implemented for PC speaker */
            SND_LOG_DEBUG("SndDoCommand: Unsupported command %d (amplitude/timbre/waveform)\n", cmd->cmd);
            break;

        default:
            SND_LOG_DEBUG("SndDoCommand: Unknown command %d\n", cmd->cmd);
            break;
    }
}

/*
 * SndDoCommand - Queue a command to a sound channel
 *
 * @param chan - Target channel
 * @param cmd - Command to queue
 * @param noWait - If true, don't wait for completion
 * @return noErr on success
 *
 * Queues commands to a sound channel. In a bare-metal environment without
 * threading support, commands are executed immediately.
 */
OSErr SndDoCommand(SndChannelPtr chan, const SndCommand* cmd, Boolean noWait) {
    if (chan == NULL || cmd == NULL) {
        return paramErr;
    }

    if (!g_soundManagerInitialized) {
        return notOpenErr;
    }

    /* In bare-metal environment without threading, execute immediately */
    /* Queue management for compatibility, but execution is synchronous */

    /* Add command to queue */
    if (chan->qLength < 128) {
        /* Calculate next tail position */
        SInt16 nextTail = (chan->qTail + 1) % 128;

        /* Queue the command */
        chan->queue[chan->qTail] = *cmd;
        chan->qTail = nextTail;
        chan->qLength++;

        SND_LOG_DEBUG("SndDoCommand: Queued command, qLength=%d\n", chan->qLength);
    } else {
        /* Queue overflow */
        SND_LOG_WARN("SndDoCommand: Command queue full for channel %p\n", chan);
        return qErr;
    }

    /* In bare-metal, execute commands immediately */
    /* This provides synchronous behavior without threading overhead */
    if (chan->qLength > 0 && chan->qHead != chan->qTail) {
        /* Process commands from queue */
        while (chan->qHead != chan->qTail) {
            SndCommand qcmd = chan->queue[chan->qHead];
            chan->qHead = (chan->qHead + 1) % 128;
            chan->qLength--;

            /* Process the command */
            SndProcessCommand(chan, &qcmd);
        }
    }

    return noErr;
}

/*
 * SndDoImmediate - Execute a sound command immediately
 *
 * @param chan - Target channel
 * @param cmd - Command to execute
 * @return noErr on success
 *
 * Executes a sound command immediately without queueing.
 * Useful for real-time control or when queue synchronization is not needed.
 */
OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd) {
    if (chan == NULL || cmd == NULL) {
        return paramErr;
    }

    if (!g_soundManagerInitialized) {
        return notOpenErr;
    }

    /* Execute command immediately */
    SndProcessCommand(chan, cmd);

    return noErr;
}

/* ============================================================================
 * 'snd ' Resource Structures
 * ============================================================================ */

/* 'snd ' resource format 1 - synthesized sound (square wave) */
typedef struct SndCommand_Res {
    UInt16 cmd;        /* Command opcode */
    SInt16 param1;     /* Parameter 1 */
    SInt32 param2;     /* Parameter 2 */
} SndCommand_Res;

/* 'snd ' resource header */
typedef struct SndResourceHeader {
    UInt16 format;             /* Format: 1 = synth, 2 = sampled */
    UInt16 numDataFormats;     /* Number of data formats (or numSynths for format 1) */
} SndResourceHeader;

/* Format 1 synthesizer descriptor */
typedef struct SynthDesc {
    UInt16 synthID;            /* Synthesizer ID (1 = square wave) */
    UInt32 initBits;           /* Initialization options */
} SynthDesc;

/* Format 1 command list */
typedef struct SndFormat1 {
    UInt16 numCmds;            /* Number of commands */
    SndCommand_Res cmds[1];    /* Variable length array of commands */
} SndFormat1;

/* ============================================================================
 * Sound Playback Implementation
 * ============================================================================ */

/* Parse and play a format 1 'snd ' resource (square wave synthesis) */
static OSErr SndPlay_Format1(const UInt8* sndData, Size dataSize) {
    if (!sndData || dataSize < 10) {
        return paramErr;
    }

    const UInt8* ptr = sndData + 2;  /* Skip format field (already checked) */

    /* Read number of synths */
    UInt16 numSynths = (ptr[0] << 8) | ptr[1];
    ptr += 2;

    SND_LOG_DEBUG("SndPlay_Format1: numSynths=%d\n", numSynths);

    /* Skip synth descriptors (6 bytes each) */
    if (dataSize < 4 + (numSynths * 6) + 2) {
        return paramErr;
    }
    ptr += numSynths * 6;

    /* Read number of commands */
    UInt16 numCmds = (ptr[0] << 8) | ptr[1];
    ptr += 2;

    SND_LOG_DEBUG("SndPlay_Format1: numCmds=%d\n", numCmds);

    /* Process commands */
    UInt32 currentFreq = 0;
    UInt32 currentDuration = 0;

    for (UInt16 i = 0; i < numCmds && (ptr + 8) <= (sndData + dataSize); i++) {
        /* Read command */
        UInt16 cmd = (ptr[0] << 8) | ptr[1];
        SInt16 param1 = (SInt16)((ptr[2] << 8) | ptr[3]);
        SInt32 param2 = (SInt32)(((UInt32)ptr[4] << 24) |
                                 ((UInt32)ptr[5] << 16) |
                                 ((UInt32)ptr[6] << 8) |
                                 (UInt32)ptr[7]);
        ptr += 8;

        SND_LOG_DEBUG("SndPlay_Format1: cmd=%d param1=%d param2=%d\n",
                      cmd, param1, param2);

        switch (cmd) {
            case freqCmd:
                /* Set frequency for next sound */
                currentFreq = (UInt32)param2;
                break;

            case restCmd:
                /* Play tone with current frequency and param2 duration */
                currentDuration = (UInt32)param2;
                if (currentFreq > 0 && currentDuration > 0) {
                    PCSpkr_Beep(currentFreq, currentDuration);
                    currentFreq = 0;  /* Reset after playing */
                }
                break;

            case quietCmd:
                /* Silence */
                currentFreq = 0;
                break;

            case noteCmd:
                /* MIDI note - convert to frequency */
                /* MIDI note 60 (middle C) = 261.63 Hz */
                /* Formula: freq = 440 * 2^((note-69)/12) */
                if (param1 >= 0 && param1 <= 127) {
                    /* Simplified: use lookup or approximation */
                    /* For now, use simple mapping */
                    UInt32 noteFreq = 440;  /* A4 as default */
                    if (param1 >= 57 && param1 <= 69) {
                        /* C4 to A4 range */
                        static const UInt16 freqTable[] = {
                            262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523
                        };
                        int idx = param1 - 57;
                        if (idx >= 0 && idx < 13) {
                            noteFreq = freqTable[idx];
                        }
                    }
                    currentFreq = noteFreq;
                    currentDuration = 200;  /* Default duration */
                    PCSpkr_Beep(currentFreq, currentDuration);
                }
                break;

            default:
                /* Ignore unknown commands */
                SND_LOG_DEBUG("SndPlay_Format1: Unknown command %d\n", cmd);
                break;
        }
    }

    return noErr;
}

/* Main SndPlay implementation */
OSErr SndPlay(SndChannelPtr chan, SndListHandle sndHandle, Boolean async) {
    /* For now, ignore channel and async parameters - just play the sound */
    (void)chan;
    (void)async;

    if (!sndHandle || !*sndHandle) {
        SND_LOG_ERROR("SndPlay: Invalid sound handle\n");
        return paramErr;
    }

    /* Lock the handle and get the resource data */
    HLock((Handle)sndHandle);

    const UInt8* sndData = (const UInt8*)*sndHandle;
    Size dataSize = GetHandleSize((Handle)sndHandle);

    if (dataSize < 4) {
        HUnlock((Handle)sndHandle);
        SND_LOG_ERROR("SndPlay: Sound resource too small\n");
        return paramErr;
    }

    /* Read format */
    UInt16 format = (sndData[0] << 8) | sndData[1];

    SND_LOG_INFO("SndPlay: Playing sound, format=%d, size=%d\n", format, dataSize);

    OSErr result = noErr;

    switch (format) {
        case 1:
            /* Format 1: Synthesized sound (square wave) */
            result = SndPlay_Format1(sndData, dataSize);
            break;

        case 2:
            /* Format 2: Sampled sound - not implemented yet */
            SND_LOG_WARN("SndPlay: Format 2 (sampled sound) not yet implemented\n");
            /* Fall back to simple beep */
            PCSpkr_Beep(1000, 200);
            result = noErr;
            break;

        default:
            SND_LOG_ERROR("SndPlay: Unknown sound format %d\n", format);
            result = paramErr;
            break;
    }

    HUnlock((Handle)sndHandle);

    return result;
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
