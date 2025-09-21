/*
 * SoundHardware.h - Sound Hardware Abstraction Layer
 *
 * Provides a unified interface for audio hardware on different platforms.
 * Abstracts platform-specific audio APIs (ALSA, PulseAudio, CoreAudio,
 * WASAPI) behind a common interface for the Mac OS Sound Manager.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef _SOUNDHARDWARE_H_
#define _SOUNDHARDWARE_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SoundTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Platform Audio API Types */

/* Audio Device Types */

/* Audio Format Description */

/* Audio Device Information */

/* Audio Stream Configuration */

/* Audio Stream State */

/* Audio Stream Statistics */

/* Forward declarations */

/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* Audio Stream Callbacks */

/* Sound Hardware Structure */

/* Audio Stream Structure */

/* Hardware Management Functions */
OSErr SoundHardwareInit(SoundHardwarePtr* hardware, AudioAPIType apiType);
OSErr SoundHardwareShutdown(SoundHardwarePtr hardware);
OSErr SoundHardwareEnumerateDevices(SoundHardwarePtr hardware);
OSErr SoundHardwareRefreshDevices(SoundHardwarePtr hardware);

/* Device Query Functions */
UInt32 SoundHardwareGetDeviceCount(SoundHardwarePtr hardware);
AudioDeviceInfo* SoundHardwareGetDevice(SoundHardwarePtr hardware, UInt32 index);
AudioDeviceInfo* SoundHardwareGetDefaultOutputDevice(SoundHardwarePtr hardware);
AudioDeviceInfo* SoundHardwareGetDefaultInputDevice(SoundHardwarePtr hardware);
AudioDeviceInfo* SoundHardwareFindDevice(SoundHardwarePtr hardware,
                                         const char* name,
                                         AudioDeviceType type);

/* Audio Stream Management */
OSErr AudioStreamOpen(SoundHardwarePtr hardware,
                     AudioStreamPtr* stream,
                     AudioDeviceInfo* device,
                     AudioStreamConfig* config);

OSErr AudioStreamClose(AudioStreamPtr stream);
OSErr AudioStreamStart(AudioStreamPtr stream);
OSErr AudioStreamStop(AudioStreamPtr stream);
OSErr AudioStreamPause(AudioStreamPtr stream);
OSErr AudioStreamResume(AudioStreamPtr stream);

/* Stream Configuration */
OSErr AudioStreamSetFormat(AudioStreamPtr stream, AudioFormat* format);
OSErr AudioStreamGetFormat(AudioStreamPtr stream, AudioFormat* format);
OSErr AudioStreamSetBufferSize(AudioStreamPtr stream, UInt32 bufferFrames);
OSErr AudioStreamGetBufferSize(AudioStreamPtr stream, UInt32* bufferFrames);

/* Stream Information */
AudioStreamState AudioStreamGetState(AudioStreamPtr stream);
OSErr AudioStreamGetStats(AudioStreamPtr stream, AudioStreamStats* stats);
OSErr AudioStreamGetLatency(AudioStreamPtr stream, UInt32* latencyFrames);

/* Callback Management */
OSErr AudioStreamSetOutputCallback(AudioStreamPtr stream,
                                  AudioOutputCallback callback,
                                  void* userData);

OSErr AudioStreamSetInputCallback(AudioStreamPtr stream,
                                 AudioInputCallback callback,
                                 void* userData);

OSErr AudioStreamSetDuplexCallback(AudioStreamPtr stream,
                                  AudioDuplexCallback callback,
                                  void* userData);

OSErr AudioStreamSetStreamCallback(AudioStreamPtr stream,
                                  AudioStreamCallback callback,
                                  void* userData);

/* Buffer Management */
OSErr AudioStreamGetInputBuffer(AudioStreamPtr stream,
                               SInt16** buffer,
                               UInt32* frameCount);

OSErr AudioStreamGetOutputBuffer(AudioStreamPtr stream,
                                SInt16** buffer,
                                UInt32* frameCount);

OSErr AudioStreamReleaseBuffer(AudioStreamPtr stream);

/* Volume Control */
OSErr AudioStreamSetVolume(AudioStreamPtr stream, float volume);
OSErr AudioStreamGetVolume(AudioStreamPtr stream, float* volume);
OSErr AudioStreamSetMute(AudioStreamPtr stream, Boolean muted);
OSErr AudioStreamGetMute(AudioStreamPtr stream, Boolean* muted);

/* Format Utilities */
Boolean AudioFormatIsSupported(AudioDeviceInfo* device, AudioFormat* format);
OSErr AudioFormatGetBestMatch(AudioDeviceInfo* device,
                             AudioFormat* desired,
                             AudioFormat* best);

UInt32 AudioFormatGetBytesPerFrame(AudioFormat* format);
UInt32 AudioFormatGetBytesPerSecond(AudioFormat* format);
UInt32 AudioFormatFramesToBytes(AudioFormat* format, UInt32 frames);
UInt32 AudioFormatBytesToFrames(AudioFormat* format, UInt32 bytes);

/* Conversion Functions */
void AudioConvertFormat(void* srcBuffer, AudioFormat* srcFormat,
                       void* dstBuffer, AudioFormat* dstFormat,
                       UInt32 frameCount);

void AudioConvertSampleRate(SInt16* srcBuffer, UInt32 srcFrames, UInt32 srcRate,
                           SInt16* dstBuffer, UInt32* dstFrames, UInt32 dstRate);

void AudioConvertChannels(SInt16* srcBuffer, UInt16 srcChannels,
                         SInt16* dstBuffer, UInt16 dstChannels,
                         UInt32 frameCount);

/* Platform-specific Hardware Implementations */
#ifdef PLATFORM_REMOVED_LINUX
OSErr SoundHardwareInitALSA(SoundHardwarePtr hardware);
OSErr SoundHardwareInitPulse(SoundHardwarePtr hardware);
OSErr SoundHardwareInitOSS(SoundHardwarePtr hardware);
OSErr SoundHardwareInitJACK(SoundHardwarePtr hardware);
#endif

#ifdef PLATFORM_REMOVED_APPLE
OSErr SoundHardwareInitCoreAudio(SoundHardwarePtr hardware);
#endif

#ifdef PLATFORM_REMOVED_WIN32
OSErr SoundHardwareInitWASAPI(SoundHardwarePtr hardware);
OSErr SoundHardwareInitDirectSound(SoundHardwarePtr hardware);
#endif

/* Dummy/Null driver for testing */
OSErr SoundHardwareInitDummy(SoundHardwarePtr hardware);

/* Stream Event Types */
#define AUDIO_STREAM_EVENT_STARTED      1
#define AUDIO_STREAM_EVENT_STOPPED      2
#define AUDIO_STREAM_EVENT_PAUSED       3
#define AUDIO_STREAM_EVENT_RESUMED      4
#define AUDIO_STREAM_EVENT_ERROR        5
#define AUDIO_STREAM_EVENT_UNDERRUN     6
#define AUDIO_STREAM_EVENT_OVERRUN      7
#define AUDIO_STREAM_EVENT_DROPOUT      8

/* Error Codes */
#define AUDIO_ERROR_SUCCESS             0
#define AUDIO_ERROR_INVALID_PARAM      -1
#define AUDIO_ERROR_NO_DEVICE          -2
#define AUDIO_ERROR_DEVICE_BUSY        -3
#define AUDIO_ERROR_FORMAT_NOT_SUPPORTED -4
#define AUDIO_ERROR_BUFFER_TOO_SMALL   -5
#define AUDIO_ERROR_BUFFER_TOO_LARGE   -6
#define AUDIO_ERROR_MEMORY_ERROR       -7
#define AUDIO_ERROR_HARDWARE_ERROR     -8

#include "SystemTypes.h"
#define AUDIO_ERROR_NOT_INITIALIZED    -9
#define AUDIO_ERROR_ALREADY_RUNNING    -10
#define AUDIO_ERROR_NOT_RUNNING        -11

/* Standard Audio Formats */
extern const AudioFormat AUDIO_FORMAT_CD;          /* 44.1kHz, 16-bit, stereo */
extern const AudioFormat AUDIO_FORMAT_DAT;         /* 48kHz, 16-bit, stereo */
extern const AudioFormat AUDIO_FORMAT_MAC_22K;     /* 22.254kHz, 16-bit, stereo */
extern const AudioFormat AUDIO_FORMAT_MAC_11K;     /* 11.127kHz, 8-bit, mono */
extern const AudioFormat AUDIO_FORMAT_PHONE;       /* 8kHz, 8-bit, mono */

/* Hardware Capability Flags */
#define AUDIO_CAP_OUTPUT                0x01
#define AUDIO_CAP_INPUT                 0x02
#define AUDIO_CAP_DUPLEX                0x04
#define AUDIO_CAP_EXCLUSIVE             0x08
#define AUDIO_CAP_MMAP                  0x10
#define AUDIO_CAP_REALTIME              0x20
#define AUDIO_CAP_HARDWARE_VOLUME       0x40

#include "SystemTypes.h"
#define AUDIO_CAP_HARDWARE_MUTE         0x80

#include "SystemTypes.h"

/* Recording Device State */

/* Ptr is defined in MacTypes.h */

/* Audio Recording Functions */
OSErr AudioRecorderInit(RecorderPtr* recorder, SoundHardwarePtr hardware);
OSErr AudioRecorderShutdown(RecorderPtr recorder);
OSErr AudioRecorderSetFormat(RecorderPtr recorder, AudioFormat* format);
OSErr AudioRecorderStart(RecorderPtr recorder);
OSErr AudioRecorderStop(RecorderPtr recorder);
OSErr AudioRecorderPause(RecorderPtr recorder);
OSErr AudioRecorderResume(RecorderPtr recorder);
OSErr AudioRecorderGetData(RecorderPtr recorder, SInt16** buffer, UInt32* frameCount);

#ifdef __cplusplus
}
#endif

#endif /* _SOUNDHARDWARE_H_ */