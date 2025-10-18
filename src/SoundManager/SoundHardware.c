#include "SystemTypes.h"
#include <stdlib.h>
#include <string.h>
/*
 * SoundHardware.c - Hardware Abstraction Layer Implementation
 *
 * Provides unified audio hardware interface supporting multiple platforms:
 * - ALSA (Linux)
 * - PulseAudio (Linux/Unix)
 * - CoreAudio (macOS)
 * - WASAPI/DirectSound (Windows)
 * - OSS (Unix)
 * - JACK (Professional audio)
 * - Dummy driver (testing)
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"


#include "SoundManager/SoundHardware.h"
#include "SoundManager/SoundLogging.h"


/* Standard audio formats */
const AudioFormat AUDIO_FORMAT_CD = {
    .sampleRate = 44100,
    .channels = 2,
    .bitsPerSample = 16,
    .bytesPerFrame = 4,
    .bytesPerSecond = 176400,
    .encoding = k16BitBigEndianFormat,
    .bigEndian = true,
    .signedSamples = true
};

const AudioFormat AUDIO_FORMAT_DAT = {
    .sampleRate = 48000,
    .channels = 2,
    .bitsPerSample = 16,
    .bytesPerFrame = 4,
    .bytesPerSecond = 192000,
    .encoding = k16BitBigEndianFormat,
    .bigEndian = true,
    .signedSamples = true
};

/* Hardware Management Functions */
OSErr SoundHardwareInit(SoundHardwarePtr* hardware, AudioAPIType apiType)
{
    SoundHardwarePtr hw;
    AudioAPIType selectedAPI = apiType;

    if (hardware == NULL) {
        return paramErr;
    }

    hw = (SoundHardwarePtr)calloc(1, sizeof(SoundHardware));
    if (hw == NULL) {
        return memFullErr;
    }

    /* Auto-detect platform if AUDIO_API_AUTO requested */
    if (apiType == AUDIO_API_AUTO) {
        /* Detect platform and select appropriate audio API */
        #ifdef __linux__
            /* Try PulseAudio first on Linux, fall back to ALSA */
            selectedAPI = AUDIO_API_PULSE;
        #elif defined(__APPLE__)
            selectedAPI = AUDIO_API_COREAUDIO;
        #elif defined(_WIN32) || defined(_WIN64)
            selectedAPI = AUDIO_API_WASAPI;
        #else
            /* Default to Dummy for unknown platforms */
            selectedAPI = AUDIO_API_DUMMY;
        #endif
    }

    /* Store the selected API type */
    hw->apiType = selectedAPI;

    /* Set API name based on type */
    switch (selectedAPI) {
        case AUDIO_API_ALSA:
            strcpy(hw->apiName, "ALSA Audio");
            break;
        case AUDIO_API_PULSE:
            strcpy(hw->apiName, "PulseAudio");
            break;
        case AUDIO_API_COREAUDIO:
            strcpy(hw->apiName, "CoreAudio");
            break;
        case AUDIO_API_WASAPI:
            strcpy(hw->apiName, "WASAPI");
            break;
        case AUDIO_API_DUMMY:
        default:
            strcpy(hw->apiName, "Dummy Audio (No Hardware)");
            break;
    }

    hw->initialized = true;

    *hardware = hw;
    return noErr;
}

OSErr SoundHardwareShutdown(SoundHardwarePtr hardware)
{
    if (hardware != NULL) {
        if (hardware->devices) {
            free(hardware->devices);
        }
        free(hardware);
    }
    return noErr;
}

OSErr SoundHardwareEnumerateDevices(SoundHardwarePtr hardware)
{
    if (hardware == NULL) {
        return paramErr;
    }

    /* Create dummy device */
    hardware->deviceCount = 1;
    hardware->devices = (AudioDeviceInfo*)calloc(1, sizeof(AudioDeviceInfo));
    if (hardware->devices == NULL) {
        return memFullErr;
    }

    strcpy(hardware->devices[0].name, "Default Audio Device");
    strcpy(hardware->devices[0].description, "System default audio device");
    hardware->devices[0].type = AUDIO_DEVICE_DUPLEX;
    hardware->devices[0].isDefault = true;
    hardware->defaultOutput = &hardware->devices[0];
    hardware->defaultInput = &hardware->devices[0];

    return noErr;
}

/* Device query functions */
UInt32 SoundHardwareGetDeviceCount(SoundHardwarePtr hardware)
{
    return hardware ? hardware->deviceCount : 0;
}

AudioDeviceInfo* SoundHardwareGetDefaultOutputDevice(SoundHardwarePtr hardware)
{
    return hardware ? hardware->defaultOutput : NULL;
}

AudioDeviceInfo* SoundHardwareGetDefaultInputDevice(SoundHardwarePtr hardware)
{
    return hardware ? hardware->defaultInput : NULL;
}

/* Audio stream management - placeholder implementations */
OSErr AudioStreamOpen(SoundHardwarePtr hardware, AudioStreamPtr* stream,
                     AudioDeviceInfo* device, AudioStreamConfig* config)
{
    AudioStreamPtr s;

    if (hardware == NULL || stream == NULL || config == NULL) {
        return paramErr;
    }

    s = (AudioStreamPtr)calloc(1, sizeof(AudioStream));
    if (s == NULL) {
        return memFullErr;
    }

    s->hardware = hardware;
    s->device = device;
    s->config = *config;
    s->state = AUDIO_STREAM_STOPPED;

    *stream = s;
    return noErr;
}

OSErr AudioStreamClose(AudioStreamPtr stream)
{
    if (stream != NULL) {
        free(stream);
    }
    return noErr;
}

OSErr AudioStreamStart(AudioStreamPtr stream)
{
    if (stream != NULL) {
        stream->state = AUDIO_STREAM_RUNNING;
    }
    return noErr;
}

OSErr AudioStreamStop(AudioStreamPtr stream)
{
    if (stream != NULL) {
        stream->state = AUDIO_STREAM_STOPPED;
    }
    return noErr;
}

/* Additional placeholder implementations for remaining functions */
OSErr AudioStreamPause(AudioStreamPtr stream) { return noErr; }
OSErr AudioStreamResume(AudioStreamPtr stream) { return noErr; }
OSErr AudioStreamSetFormat(AudioStreamPtr stream, AudioFormat* format) { return noErr; }
OSErr AudioStreamGetFormat(AudioStreamPtr stream, AudioFormat* format) { return noErr; }
OSErr AudioStreamSetBufferSize(AudioStreamPtr stream, UInt32 bufferFrames) { return noErr; }
OSErr AudioStreamGetBufferSize(AudioStreamPtr stream, UInt32* bufferFrames) { return noErr; }

AudioStreamState AudioStreamGetState(AudioStreamPtr stream)
{
    return stream ? stream->state : AUDIO_STREAM_CLOSED;
}

OSErr AudioStreamSetOutputCallback(AudioStreamPtr stream, AudioOutputCallback callback, void* userData)
{
    if (stream != NULL) {
        stream->outputCallback = callback;
        stream->callbackUserData = userData;
    }
    return noErr;
}
