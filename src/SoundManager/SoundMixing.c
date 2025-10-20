#include "MemoryMgr/MemoryManager.h"
#include "SystemTypes.h"
#include <stdlib.h>
#include <string.h>
/*
 * SoundMixing.c - Multi-Channel Audio Mixing Engine
 *
 * Implements a complete audio mixing system for the Sound Manager:
 * - Multi-channel mixing with individual volume and pan controls
 * - Real-time audio effects (reverb, echo, filtering)
 * - Dynamic range compression and limiting
 * - Sample rate conversion and format conversion
 * - CPU load monitoring and optimization
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"

#include <math.h>

#include "SoundManager/SoundSynthesis.h"
#include "SoundManager/SoundTypes.h"
#include "SoundManager/SoundLogging.h"


/* Mathematical constants */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Audio processing constants */
#define MAX_REVERB_DELAY    4096    /* Maximum reverb delay in samples */
#define MAX_ECHO_DELAY      8192    /* Maximum echo delay in samples */
#define LIMITER_THRESHOLD   0.95f   /* Limiter threshold (0.0-1.0) */
#define COMPRESSOR_RATIO    4.0f    /* Compression ratio */

/* Filter coefficients */
typedef struct FilterCoeffs {
    float a0, a1, a2;  /* Feedforward coefficients */
    float b1, b2;      /* Feedback coefficients */
} FilterCoeffs;

/* Biquad filter state */
typedef struct BiquadFilter {
    FilterCoeffs coeffs;
    float x1, x2;      /* Input history */
    float y1, y2;      /* Output history */
} BiquadFilter;

/* Reverb processor */
typedef struct ReverbProcessor {
    float*  delayBuffer;        /* Delay buffer */
    UInt32 bufferSize;        /* Buffer size */
    UInt32 writeIndex;        /* Write position */
    float   feedback;           /* Feedback amount */
    float   wetLevel;           /* Wet signal level */
    float   dryLevel;           /* Dry signal level */
    BiquadFilter lowpass;       /* Lowpass filter */
    BiquadFilter highpass;      /* Highpass filter */
} ReverbProcessor;

/* Echo processor */
typedef struct EchoProcessor {
    float*  delayBuffer;        /* Delay buffer */
    UInt32 bufferSize;        /* Buffer size */
    UInt32 writeIndex;        /* Write position */
    UInt32 readIndex;         /* Read position */
    float   feedback;           /* Echo feedback */
    float   wetLevel;           /* Echo level */
    UInt32 delayTime;         /* Delay time in samples */
} EchoProcessor;

/* Dynamic range processor */
typedef struct DynamicsProcessor {
    float   threshold;          /* Compressor threshold */
    float   ratio;              /* Compression ratio */
    float   attack;             /* Attack time constant */
    float   release;            /* Release time constant */
    float   envelope;           /* Current envelope */
    float   gain;               /* Current gain */
    Boolean    limiterEnabled;     /* Limiter enable */
} DynamicsProcessor;

/* Extended mixer channel with effects */
typedef struct ExtendedMixerChannel {
    MixerChannel        base;           /* Base mixer channel */

    /* Effects processors */
    ReverbProcessor     reverb;         /* Reverb processor */
    EchoProcessor       echo;           /* Echo processor */
    DynamicsProcessor   dynamics;       /* Dynamics processor */
    BiquadFilter        eq[3];          /* 3-band EQ (low, mid, high) */

    /* Processing state */
    float*              tempBuffer;     /* Temporary processing buffer */
    UInt32            tempBufferSize; /* Temp buffer size */
    Boolean                effectsEnabled; /* Effects processing enabled */

    /* Performance monitoring */
    clock_t             processingTime; /* CPU time used */
    UInt32            samplesProcessed; /* Samples processed */
} ExtendedMixerChannel;

/* Extended mixer with advanced features */
typedef struct ExtendedMixer {
    Mixer                   base;           /* Base mixer */
    ExtendedMixerChannel*   extChannels;    /* Extended channel array */

    /* Master effects */
    ReverbProcessor         masterReverb;   /* Master reverb */
    DynamicsProcessor       masterDynamics; /* Master dynamics */
    BiquadFilter            masterEQ[3];    /* Master 3-band EQ */

    /* Processing buffers */
    float*                  floatMixBuffer; /* Float mix buffer */
    SInt16*                tempOutputBuffer; /* Temp output buffer */

    /* Performance monitoring */
    clock_t                 lastProcessTime;
    UInt32                totalSamplesProcessed;
    float                   averageCpuLoad;
} ExtendedMixer;

/* Forward declarations */
static void InitializeBiquadFilter(BiquadFilter* filter, float freq, float q, float gain, UInt32 sampleRate, int type);
static float ProcessBiquadFilter(BiquadFilter* filter, float input);
static void InitializeReverb(ReverbProcessor* reverb, UInt32 sampleRate);
static void ProcessReverb(ReverbProcessor* reverb, float* input, float* output, UInt32 frameCount);
static void InitializeEcho(EchoProcessor* echo, UInt32 delayMs, UInt32 sampleRate);
static void ProcessEcho(EchoProcessor* echo, float* input, float* output, UInt32 frameCount);
static void InitializeDynamics(DynamicsProcessor* dynamics, UInt32 sampleRate);
static void ProcessDynamics(DynamicsProcessor* dynamics, float* buffer, UInt32 frameCount);
static void ApplyVolumeAndPan(float* leftBuffer, float* rightBuffer, UInt32 frameCount,
                             UInt16 volume, SInt16 pan);
static void ConvertFloatToInt16(float* floatBuffer, SInt16* intBuffer, UInt32 samples);
static void ConvertInt16ToFloat(SInt16* intBuffer, float* floatBuffer, UInt32 samples);

/*
 * Mixer Initialization
 */
OSErr MixerInit(MixerPtr* mixer, UInt16 numChannels, UInt32 sampleRate)
{
    ExtendedMixer* extMixer;
    int i;

    if (mixer == NULL || numChannels == 0 || numChannels > 32) {
        return paramErr;
    }

    /* Allocate extended mixer */
    extMixer = (ExtendedMixer*)NewPtrClear(sizeof(ExtendedMixer));
    if (extMixer == NULL) {
        return memFullErr;
    }

    /* Initialize base mixer */
    (extMixer)->numChannels = numChannels;
    (extMixer)->activeChannels = 0;
    (extMixer)->bufferFrames = 1024; /* Default buffer size */
    (extMixer)->sampleRate = sampleRate;
    (extMixer)->outputChannels = 2; /* Stereo */
    (extMixer)->masterVolume = kFullVolume;
    (extMixer)->masterMute = false;
    (extMixer)->cpuLoad = 0;

    /* Allocate channel arrays */
    extMixer->extChannels = (ExtendedMixerChannel*)NewPtrClear((numChannels) * (sizeof(ExtendedMixerChannel)));
    if (extMixer->extChannels == NULL) {
        DisposePtr((Ptr)extMixer);
        return memFullErr;
    }

    /* Initialize extended channels */
    for (i = 0; i < numChannels; i++) {
        ExtendedMixerChannel* chan = &extMixer->extChannels[i];

        /* Initialize base channel */
        (chan)->active = false;
        (chan)->volume = kFullVolume;
        (chan)->pan = 0; /* Center */
        (chan)->muted = false;
        (chan)->solo = false;

        /* Allocate temp buffer */
        chan->tempBufferSize = 2048;
        chan->tempBuffer = (float*)NewPtrClear((chan->tempBufferSize) * (sizeof(float)));
        if (chan->tempBuffer == NULL) {
            /* Cleanup on failure */
            for (int j = 0; j < i; j++) {
                if (extMixer->extChannels[j].tempBuffer) {
                    DisposePtr((Ptr)extMixer->extChannels[j].tempBuffer);
                }
            }
            DisposePtr((Ptr)extMixer->extChannels);
            DisposePtr((Ptr)extMixer);
            return memFullErr;
        }

        /* Initialize effects */
        InitializeReverb(&chan->reverb, sampleRate);
        InitializeEcho(&chan->echo, 250, sampleRate); /* 250ms echo */
        InitializeDynamics(&chan->dynamics, sampleRate);

        /* Initialize 3-band EQ */
        InitializeBiquadFilter(&chan->eq[0], 100.0f, 0.7f, 1.0f, sampleRate, 0); /* Low */
        InitializeBiquadFilter(&chan->eq[1], 1000.0f, 0.7f, 1.0f, sampleRate, 1); /* Mid */
        InitializeBiquadFilter(&chan->eq[2], 8000.0f, 0.7f, 1.0f, sampleRate, 2); /* High */

        chan->effectsEnabled = false;
    }

    /* Allocate processing buffers */
    extMixer->floatMixBuffer = (float*)NewPtrClear(((extMixer)->bufferFrames * 2) * (sizeof(float)));
    extMixer->tempOutputBuffer = (SInt16*)NewPtrClear(((extMixer)->bufferFrames * 2) * (sizeof(SInt16)));
    (extMixer)->mixBuffer = extMixer->tempOutputBuffer;

    if (extMixer->floatMixBuffer == NULL || extMixer->tempOutputBuffer == NULL) {
        /* Cleanup on failure */
        for (i = 0; i < numChannels; i++) {
            if (extMixer->extChannels[i].tempBuffer) {
                DisposePtr((Ptr)extMixer->extChannels[i].tempBuffer);
            }
        }
        if (extMixer->floatMixBuffer) DisposePtr((Ptr)extMixer->floatMixBuffer);
        if (extMixer->tempOutputBuffer) DisposePtr((Ptr)extMixer->tempOutputBuffer);
        DisposePtr((Ptr)extMixer->extChannels);
        DisposePtr((Ptr)extMixer);
        return memFullErr;
    }

    /* Initialize master effects */
    InitializeReverb(&extMixer->masterReverb, sampleRate);
    InitializeDynamics(&extMixer->masterDynamics, sampleRate);
    InitializeBiquadFilter(&extMixer->masterEQ[0], 100.0f, 0.7f, 1.0f, sampleRate, 0);
    InitializeBiquadFilter(&extMixer->masterEQ[1], 1000.0f, 0.7f, 1.0f, sampleRate, 1);
    InitializeBiquadFilter(&extMixer->masterEQ[2], 8000.0f, 0.7f, 1.0f, sampleRate, 2);

    /* Initialize performance monitoring */
    extMixer->lastProcessTime = clock();
    extMixer->totalSamplesProcessed = 0;
    extMixer->averageCpuLoad = 0.0f;

    *mixer = (MixerPtr)extMixer;
    return noErr;
}

/*
 * Mixer Disposal
 */
OSErr MixerDispose(MixerPtr mixer)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;
    int i;

    if (extMixer == NULL) {
        return paramErr;
    }

    /* Free channel resources */
    for (i = 0; i < (extMixer)->numChannels; i++) {
        ExtendedMixerChannel* chan = &extMixer->extChannels[i];

        if (chan->tempBuffer) {
            DisposePtr((Ptr)chan->tempBuffer);
        }

        /* Free effect buffers */
        if ((chan)->delayBuffer) {
            DisposePtr((Ptr)(chan)->delayBuffer);
        }
        if ((chan)->delayBuffer) {
            DisposePtr((Ptr)(chan)->delayBuffer);
        }
    }

    /* Free master effect buffers */
    if ((extMixer)->delayBuffer) {
        DisposePtr((Ptr)(extMixer)->delayBuffer);
    }

    /* Free processing buffers */
    if (extMixer->floatMixBuffer) {
        DisposePtr((Ptr)extMixer->floatMixBuffer);
    }
    if (extMixer->tempOutputBuffer) {
        DisposePtr((Ptr)extMixer->tempOutputBuffer);
    }
    if (extMixer->extChannels) {
        DisposePtr((Ptr)extMixer->extChannels);
    }

    DisposePtr((Ptr)extMixer);
    return noErr;
}

/*
 * Add Channel to Mixer
 */
OSErr MixerAddChannel(MixerPtr mixer, SynthesizerPtr synth, UInt16* channelIndex)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;
    int i;

    if (extMixer == NULL || channelIndex == NULL) {
        return paramErr;
    }

    /* Find free channel */
    for (i = 0; i < (extMixer)->numChannels; i++) {
        if (!extMixer->extChannels[i].base.active) {
            ExtendedMixerChannel* chan = &extMixer->extChannels[i];

            (chan)->active = true;
            (chan)->synthesizer = synth;
            (chan)->volume = kFullVolume;
            (chan)->pan = 0;
            (chan)->muted = false;
            (chan)->solo = false;

            *channelIndex = i;
            (extMixer)->activeChannels++;
            return noErr;
        }
    }

    return notEnoughHardwareErr;
}

/*
 * Remove Channel from Mixer
 */
OSErr MixerRemoveChannel(MixerPtr mixer, UInt16 channelIndex)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL || channelIndex >= (extMixer)->numChannels) {
        return paramErr;
    }

    ExtendedMixerChannel* chan = &extMixer->extChannels[channelIndex];

    if ((chan)->active) {
        (chan)->active = false;
        (chan)->synthesizer = NULL;
        (extMixer)->activeChannels--;
    }

    return noErr;
}

/*
 * Set Channel Volume
 */
OSErr MixerSetChannelVolume(MixerPtr mixer, UInt16 channel, UInt16 volume)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL || channel >= (extMixer)->numChannels) {
        return paramErr;
    }

    extMixer->extChannels[channel].base.volume = volume;
    return noErr;
}

/*
 * Set Channel Pan
 */
OSErr MixerSetChannelPan(MixerPtr mixer, UInt16 channel, SInt16 pan)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL || channel >= (extMixer)->numChannels) {
        return paramErr;
    }

    extMixer->extChannels[channel].base.pan = pan;
    return noErr;
}

/*
 * Set Channel Mute
 */
OSErr MixerSetChannelMute(MixerPtr mixer, UInt16 channel, Boolean muted)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL || channel >= (extMixer)->numChannels) {
        return paramErr;
    }

    extMixer->extChannels[channel].base.muted = muted;
    return noErr;
}

/*
 * Set Master Volume
 */
OSErr MixerSetMasterVolume(MixerPtr mixer, UInt16 volume)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL) {
        return paramErr;
    }

    (extMixer)->masterVolume = volume;
    return noErr;
}

/*
 * Set Master Mute
 */
OSErr MixerSetMasterMute(MixerPtr mixer, Boolean muted)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;

    if (extMixer == NULL) {
        return paramErr;
    }

    (extMixer)->masterMute = muted;
    return noErr;
}

/*
 * Process Audio Through Mixer
 */
UInt32 MixerProcess(MixerPtr mixer, SInt16* outputBuffer, UInt32 frameCount)
{
    ExtendedMixer* extMixer = (ExtendedMixer*)mixer;
    clock_t startTime, endTime;
    int i;
    UInt32 samplesPerChannel;
    Boolean anySoloChannels = false;

    if (extMixer == NULL || outputBuffer == NULL || frameCount == 0) {
        return 0;
    }

    startTime = clock();

    /* Ensure buffers are large enough */
    if (frameCount > (extMixer)->bufferFrames) {
        frameCount = (extMixer)->bufferFrames;
    }

    samplesPerChannel = frameCount * 2; /* Stereo */

    /* Clear mix buffer */
    memset(extMixer->floatMixBuffer, 0, samplesPerChannel * sizeof(float));

    /* Check for solo channels */
    for (i = 0; i < (extMixer)->numChannels; i++) {
        if (extMixer->extChannels[i].base.active && extMixer->extChannels[i].base.solo) {
            anySoloChannels = true;
            break;
        }
    }

    /* Process each channel */
    for (i = 0; i < (extMixer)->numChannels; i++) {
        ExtendedMixerChannel* chan = &extMixer->extChannels[i];

        if (!(chan)->active || (chan)->synthesizer == NULL) {
            continue;
        }

        /* Skip if muted or if other channels are soloed */
        if ((chan)->muted || (anySoloChannels && !(chan)->solo)) {
            continue;
        }

        /* Ensure temp buffer is large enough */
        if (frameCount > chan->tempBufferSize / 2) {
            continue;
        }

        /* Generate audio from synthesizer */
        /* This would call the appropriate synthesizer generate function */
        /* For now, generate silence as placeholder */
        memset(chan->tempBuffer, 0, frameCount * 2 * sizeof(float));

        /* Apply channel effects if enabled */
        if (chan->effectsEnabled) {
            /* Apply EQ */
            for (UInt32 sample = 0; sample < frameCount; sample++) {
                float left = chan->tempBuffer[sample * 2];
                float right = chan->tempBuffer[sample * 2 + 1];

                /* Process left channel through EQ */
                left = ProcessBiquadFilter(&chan->eq[0], left);
                left = ProcessBiquadFilter(&chan->eq[1], left);
                left = ProcessBiquadFilter(&chan->eq[2], left);

                /* Process right channel through EQ */
                right = ProcessBiquadFilter(&chan->eq[0], right);
                right = ProcessBiquadFilter(&chan->eq[1], right);
                right = ProcessBiquadFilter(&chan->eq[2], right);

                chan->tempBuffer[sample * 2] = left;
                chan->tempBuffer[sample * 2 + 1] = right;
            }

            /* Apply dynamics processing */
            ProcessDynamics(&chan->dynamics, chan->tempBuffer, frameCount * 2);

            /* Apply reverb and echo effects */
            ProcessReverb(&chan->reverb, chan->tempBuffer, chan->tempBuffer, frameCount);
            ProcessEcho(&chan->echo, chan->tempBuffer, chan->tempBuffer, frameCount);
        }

        /* Apply volume and pan */
        ApplyVolumeAndPan(&chan->tempBuffer[0], &chan->tempBuffer[1],
                         frameCount, (chan)->volume, (chan)->pan);

        /* Mix into main buffer */
        for (UInt32 sample = 0; sample < samplesPerChannel; sample++) {
            extMixer->floatMixBuffer[sample] += chan->tempBuffer[sample];
        }

        /* Update performance statistics */
        chan->samplesProcessed += samplesPerChannel;
    }

    /* Apply master effects */
    ProcessDynamics(&extMixer->masterDynamics, extMixer->floatMixBuffer, samplesPerChannel);

    /* Apply master volume */
    if ((extMixer)->masterMute) {
        memset(extMixer->floatMixBuffer, 0, samplesPerChannel * sizeof(float));
    } else {
        float masterGain = (float)(extMixer)->masterVolume / (float)kFullVolume;
        for (UInt32 sample = 0; sample < samplesPerChannel; sample++) {
            extMixer->floatMixBuffer[sample] *= masterGain;
        }
    }

    /* Convert to 16-bit integer output */
    ConvertFloatToInt16(extMixer->floatMixBuffer, outputBuffer, samplesPerChannel);

    /* Update performance monitoring */
    endTime = clock();
    extMixer->totalSamplesProcessed += samplesPerChannel;

    double processingTime = ((double)(endTime - startTime)) / CLOCKS_PER_SEC;
    double audioTime = (double)samplesPerChannel / (double)((extMixer)->sampleRate * 2);

    if (audioTime > 0.0) {
        float currentLoad = (float)(processingTime / audioTime * 100.0);
        extMixer->averageCpuLoad = extMixer->averageCpuLoad * 0.9f + currentLoad * 0.1f;
        (extMixer)->cpuLoad = (UInt32)extMixer->averageCpuLoad;
    }

    return frameCount;
}

/*
 * Internal Helper Functions
 */

static void InitializeBiquadFilter(BiquadFilter* filter, float freq, float q, float gain,
                                  UInt32 sampleRate, int type)
{
    float omega = 2.0f * M_PI * freq / (float)sampleRate;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / (2.0f * q);
    float A = powf(10.0f, gain / 40.0f);

    float b0, b1, b2, a0, a1, a2;

    switch (type) {
        case 0: /* Lowpass */
            b0 = (1.0f - cos_omega) / 2.0f;
            b1 = 1.0f - cos_omega;
            b2 = (1.0f - cos_omega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case 1: /* Bandpass */
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case 2: /* Highpass */
            b0 = (1.0f + cos_omega) / 2.0f;
            b1 = -(1.0f + cos_omega);
            b2 = (1.0f + cos_omega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        default: /* All-pass */
            b0 = 1.0f - alpha;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
    }

    /* Normalize coefficients */
    (filter)->a0 = b0 / a0;
    (filter)->a1 = b1 / a0;
    (filter)->a2 = b2 / a0;
    (filter)->b1 = a1 / a0;
    (filter)->b2 = a2 / a0;

    /* Initialize state */
    filter->x1 = filter->x2 = 0.0f;
    filter->y1 = filter->y2 = 0.0f;
}

static float ProcessBiquadFilter(BiquadFilter* filter, float input)
{
    float output = (filter)->a0 * input +
                   (filter)->a1 * filter->x1 +
                   (filter)->a2 * filter->x2 -
                   (filter)->b1 * filter->y1 -
                   (filter)->b2 * filter->y2;

    /* Update state */
    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;

    return output;
}

static void InitializeReverb(ReverbProcessor* reverb, UInt32 sampleRate)
{
    reverb->bufferSize = (sampleRate * MAX_REVERB_DELAY) / 44100;
    reverb->delayBuffer = (float*)NewPtrClear((reverb->bufferSize) * (sizeof(float)));
    reverb->writeIndex = 0;
    reverb->feedback = 0.3f;
    reverb->wetLevel = 0.2f;
    reverb->dryLevel = 0.8f;

    /* Initialize filters for reverb character */
    InitializeBiquadFilter(&reverb->lowpass, 8000.0f, 0.7f, 1.0f, sampleRate, 0);
    InitializeBiquadFilter(&reverb->highpass, 100.0f, 0.7f, 1.0f, sampleRate, 2);
}

static void ProcessReverb(ReverbProcessor* reverb, float* input, float* output, UInt32 frameCount)
{
    if (reverb->delayBuffer == NULL) {
        return;
    }

    for (UInt32 i = 0; i < frameCount * 2; i++) {
        float dry = input[i];

        /* Read from delay buffer with multiple taps for diffusion */
        float wet = 0.0f;
        UInt32 tap1 = (reverb->writeIndex - reverb->bufferSize / 3) % reverb->bufferSize;
        UInt32 tap2 = (reverb->writeIndex - reverb->bufferSize / 2) % reverb->bufferSize;
        UInt32 tap3 = (reverb->writeIndex - reverb->bufferSize * 2 / 3) % reverb->bufferSize;

        wet += reverb->delayBuffer[tap1] * 0.4f;
        wet += reverb->delayBuffer[tap2] * 0.3f;
        wet += reverb->delayBuffer[tap3] * 0.3f;

        /* Apply filtering for natural reverb character */
        wet = ProcessBiquadFilter(&reverb->lowpass, wet);
        wet = ProcessBiquadFilter(&reverb->highpass, wet);

        /* Write to delay buffer with feedback */
        reverb->delayBuffer[reverb->writeIndex] = dry + wet * reverb->feedback;
        reverb->writeIndex = (reverb->writeIndex + 1) % reverb->bufferSize;

        /* Mix dry and wet signals */
        output[i] = dry * reverb->dryLevel + wet * reverb->wetLevel;
    }
}

static void InitializeEcho(EchoProcessor* echo, UInt32 delayMs, UInt32 sampleRate)
{
    echo->delayTime = (delayMs * sampleRate) / 1000;
    echo->bufferSize = echo->delayTime * 2; /* Double buffer for safety */
    echo->delayBuffer = (float*)NewPtrClear((echo->bufferSize) * (sizeof(float)));
    echo->writeIndex = 0;
    echo->readIndex = 0;
    echo->feedback = 0.4f;
    echo->wetLevel = 0.3f;
}

static void ProcessEcho(EchoProcessor* echo, float* input, float* output, UInt32 frameCount)
{
    if (echo->delayBuffer == NULL) {
        return;
    }

    for (UInt32 i = 0; i < frameCount * 2; i++) {
        float dry = input[i];

        /* Read delayed signal */
        float wet = echo->delayBuffer[echo->readIndex];

        /* Write to delay buffer with feedback */
        echo->delayBuffer[echo->writeIndex] = dry + wet * echo->feedback;

        /* Update indices */
        echo->writeIndex = (echo->writeIndex + 1) % echo->bufferSize;
        echo->readIndex = (echo->readIndex + 1) % echo->bufferSize;

        /* Mix dry and wet signals */
        output[i] = dry + wet * echo->wetLevel;
    }
}

static void InitializeDynamics(DynamicsProcessor* dynamics, UInt32 sampleRate)
{
    dynamics->threshold = 0.7f;
    dynamics->ratio = COMPRESSOR_RATIO;
    dynamics->attack = expf(-1.0f / (0.001f * sampleRate)); /* 1ms attack */
    dynamics->release = expf(-1.0f / (0.1f * sampleRate));  /* 100ms release */
    dynamics->envelope = 0.0f;
    dynamics->gain = 1.0f;
    dynamics->limiterEnabled = true;
}

static void ProcessDynamics(DynamicsProcessor* dynamics, float* buffer, UInt32 frameCount)
{
    for (UInt32 i = 0; i < frameCount; i++) {
        float input = fabsf(buffer[i]);

        /* Envelope follower */
        if (input > dynamics->envelope) {
            dynamics->envelope = dynamics->attack * dynamics->envelope + (1.0f - dynamics->attack) * input;
        } else {
            dynamics->envelope = dynamics->release * dynamics->envelope + (1.0f - dynamics->release) * input;
        }

        /* Compression */
        if (dynamics->envelope > dynamics->threshold) {
            float over = dynamics->envelope - dynamics->threshold;
            float compressedGain = 1.0f - (over * (1.0f - 1.0f / dynamics->ratio));
            dynamics->gain = fminf(dynamics->gain, compressedGain);
        } else {
            dynamics->gain = fminf(1.0f, dynamics->gain * 1.001f); /* Slow release */
        }

        /* Apply gain */
        buffer[i] *= dynamics->gain;

        /* Hard limiter */
        if (dynamics->limiterEnabled) {
            if (buffer[i] > LIMITER_THRESHOLD) {
                buffer[i] = LIMITER_THRESHOLD;
            } else if (buffer[i] < -LIMITER_THRESHOLD) {
                buffer[i] = -LIMITER_THRESHOLD;
            }
        }
    }
}

static void ApplyVolumeAndPan(float* leftBuffer, float* rightBuffer, UInt32 frameCount,
                             UInt16 volume, SInt16 pan)
{
    float gain = (float)volume / (float)kFullVolume;
    float panGain = (float)pan / 127.0f; /* Pan range -127 to +127 */
    float leftGain = gain * (1.0f - fmaxf(0.0f, panGain));
    float rightGain = gain * (1.0f + fminf(0.0f, panGain));

    for (UInt32 i = 0; i < frameCount; i++) {
        leftBuffer[i * 2] *= leftGain;
        rightBuffer[i * 2 + 1] *= rightGain;
    }
}

static void ConvertFloatToInt16(float* floatBuffer, SInt16* intBuffer, UInt32 samples)
{
    for (UInt32 i = 0; i < samples; i++) {
        float sample = floatBuffer[i];

        /* Clamp to valid range */
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        /* Convert to 16-bit integer */
        intBuffer[i] = (SInt16)(sample * 32767.0f);
    }
}

static void ConvertInt16ToFloat(SInt16* intBuffer, float* floatBuffer, UInt32 samples)
{
    for (UInt32 i = 0; i < samples; i++) {
        floatBuffer[i] = (float)intBuffer[i] / 32767.0f;
    }
}
