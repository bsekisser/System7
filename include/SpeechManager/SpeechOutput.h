/*
 * File: SpeechOutput.h
 *
 * Contains: Speech output device control and routing for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides speech audio output functionality
 *              including device management, routing, and audio processing.
 */

#ifndef _SPEECHOUTPUT_H_
#define _SPEECHOUTPUT_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Audio Output Constants ===== */

/* Output device types */

/* Output quality levels */

/* Output routing modes */

/* Output processing flags */

/* ===== Audio Output Structures ===== */

/* Audio device information */

/* Audio format specification */

/* Audio output configuration */

/* Audio output statistics */

/* ===== Audio Output Management ===== */

/* Output system initialization */
OSErr InitializeAudioOutput(void);
void CleanupAudioOutput(void);

/* Device enumeration */
OSErr CountAudioOutputDevices(short *deviceCount);
OSErr GetIndAudioOutputDevice(short index, AudioOutputDevice *device);
OSErr GetDefaultAudioOutputDevice(AudioOutputDevice *device);
OSErr SetDefaultAudioOutputDevice(const char *deviceID);

/* Device information */
OSErr GetAudioOutputDeviceInfo(const char *deviceID, AudioOutputDevice *device);
OSErr GetAudioOutputDeviceCapabilities(const char *deviceID, AudioOutputFormat **formats,
                                       short *formatCount, AudioOutputFlags *supportedFlags);

/* Device status */
Boolean IsAudioOutputDeviceAvailable(const char *deviceID);
OSErr GetAudioOutputDeviceStatus(const char *deviceID, Boolean *isActive, long *currentSampleRate,
                                 short *currentChannels);

/* ===== Audio Output Configuration ===== */

/* Output configuration */
OSErr CreateAudioOutputConfig(AudioOutputConfig **config);
OSErr DisposeAudioOutputConfig(AudioOutputConfig *config);
OSErr SetAudioOutputConfig(const AudioOutputConfig *config);
OSErr GetAudioOutputConfig(AudioOutputConfig *config);
OSErr ValidateAudioOutputConfig(const AudioOutputConfig *config, Boolean *isValid,
                                char **errorMessage);

/* Format management */
OSErr SetAudioOutputFormat(const AudioOutputFormat *format);
OSErr GetAudioOutputFormat(AudioOutputFormat *format);
OSErr GetBestAudioOutputFormat(const char *deviceID, AudioOutputQuality quality,
                               AudioOutputFormat *format);

/* Volume and balance control */
OSErr SetAudioOutputVolume(Fixed volume);
OSErr GetAudioOutputVolume(Fixed *volume);
OSErr SetAudioOutputBalance(Fixed balance);
OSErr GetAudioOutputBalance(Fixed *balance);
OSErr SetChannelVolume(short channel, Fixed volume);
OSErr GetChannelVolume(short channel, Fixed *volume);

/* ===== Audio Streaming ===== */

/* Stream management */

OSErr CreateAudioOutputStream(const AudioOutputConfig *config, AudioOutputStream **stream);
OSErr DisposeAudioOutputStream(AudioOutputStream *stream);
OSErr OpenAudioOutputStream(AudioOutputStream *stream);
OSErr CloseAudioOutputStream(AudioOutputStream *stream);

/* Stream control */
OSErr StartAudioOutputStream(AudioOutputStream *stream);
OSErr StopAudioOutputStream(AudioOutputStream *stream);
OSErr PauseAudioOutputStream(AudioOutputStream *stream);
OSErr ResumeAudioOutputStream(AudioOutputStream *stream);
OSErr FlushAudioOutputStream(AudioOutputStream *stream);

/* Stream data */
OSErr WriteAudioData(AudioOutputStream *stream, const void *audioData, long dataSize,
                     long *framesWritten);
OSErr WriteAudioFrames(AudioOutputStream *stream, const void *audioFrames, long frameCount);
OSErr GetAudioStreamPosition(AudioOutputStream *stream, long *currentFrame, long *totalFrames);

/* Stream properties */
OSErr SetAudioStreamProperty(AudioOutputStream *stream, OSType property, const void *value,
                             long valueSize);
OSErr GetAudioStreamProperty(AudioOutputStream *stream, OSType property, void *value,
                             long *valueSize);

/* ===== Audio Processing ===== */

/* Audio effects and processing */

OSErr CreateAudioProcessor(AudioOutputFlags processingFlags, AudioProcessor **processor);
OSErr DisposeAudioProcessor(AudioProcessor *processor);
OSErr ProcessAudioData(AudioProcessor *processor, void *audioData, long dataSize);

/* Built-in effects */
OSErr ApplyVolumeControl(void *audioData, long dataSize, const AudioOutputFormat *format,
                         Fixed volume);
OSErr ApplyNormalization(void *audioData, long dataSize, const AudioOutputFormat *format);
OSErr ApplyCompression(void *audioData, long dataSize, const AudioOutputFormat *format,
                       Fixed threshold, Fixed ratio);
OSErr ApplyEqualization(void *audioData, long dataSize, const AudioOutputFormat *format,
                        Fixed *bandGains, short bandCount);

/* Custom effects */

OSErr RegisterAudioEffect(OSType effectType, AudioEffectProc effectProc, void *userData);
OSErr ApplyCustomEffect(OSType effectType, void *audioData, long dataSize,
                        const AudioOutputFormat *format, void *effectData);

/* ===== Audio Routing ===== */

/* Routing control */
OSErr SetAudioRoutingMode(AudioRoutingMode mode);
OSErr GetAudioRoutingMode(AudioRoutingMode *mode);
OSErr RouteAudioToDevice(const char *deviceID);
OSErr GetCurrentAudioRoute(char *deviceID, long deviceIDSize);

/* Multi-device routing */
OSErr EnableMultiDeviceOutput(Boolean enable);
OSErr AddOutputDevice(const char *deviceID, Fixed volume);
OSErr RemoveOutputDevice(const char *deviceID);
OSErr GetActiveOutputDevices(char ***deviceIDs, short *deviceCount);

/* Routing policies */
OSErr SetAudioInterruptionPolicy(Boolean allowInterruptions);
OSErr SetAudioDuckingEnabled(Boolean enable);
OSErr SetAudioPriorityLevel(short priority);

/* ===== Audio Monitoring ===== */

/* Level monitoring */
OSErr EnableAudioLevelMonitoring(Boolean enable);
OSErr GetAudioLevels(Fixed *leftLevel, Fixed *rightLevel);
OSErr GetPeakLevels(Fixed *leftPeak, Fixed *rightPeak);
OSErr ResetPeakLevels(void);

/* Spectrum analysis */
OSErr EnableSpectrumAnalysis(Boolean enable);
OSErr GetAudioSpectrum(Fixed *spectrum, short bandCount);
OSErr SetSpectrumAnalysisParameters(short fftSize, short overlap);

/* Audio statistics */
OSErr GetAudioOutputStats(AudioOutputStats *stats);
OSErr ResetAudioOutputStats(void);

/* ===== Audio File Output ===== */

/* File output */

OSErr StartAudioFileRecording(const char *filePath, AudioFileFormat format,
                              const AudioOutputFormat *audioFormat);
OSErr StopAudioFileRecording(void);
OSErr WriteAudioToFile(const char *filePath, const void *audioData, long dataSize,
                       const AudioOutputFormat *format, AudioFileFormat fileFormat);

/* ===== Audio Callbacks ===== */

/* Output callback */

/* Level callback */

/* Device change callback */

/* Buffer callback */

/* Callback registration */
OSErr SetAudioOutputCallback(AudioOutputStream *stream, AudioOutputProc callback, void *userData);
OSErr SetAudioLevelCallback(AudioLevelProc callback, void *userData);
OSErr SetAudioDeviceChangeCallback(AudioDeviceChangeProc callback, void *userData);
OSErr SetAudioBufferCallback(AudioOutputStream *stream, AudioBufferProc callback, void *userData);

/* ===== Platform Integration ===== */

/* Platform-specific audio support */
#ifdef PLATFORM_REMOVED_WIN32
OSErr InitializeDirectSoundOutput(void);
OSErr InitializeWASAPIOutput(void);
OSErr ConfigureWindowsAudioSession(void *sessionConfig);
#endif

#ifdef PLATFORM_REMOVED_APPLE
OSErr InitializeCoreAudioOutput(void);
OSErr ConfigureAudioUnit(void *audioUnitConfig);
OSErr SetAudioSessionCategory(OSType category);
#endif

#ifdef PLATFORM_REMOVED_LINUX
OSErr InitializeALSAOutput(void);
OSErr InitializePulseAudioOutput(void);
OSErr InitializeJACKOutput(void);
OSErr ConfigureLinuxAudioSystem(const char *configFile);
#endif

/* Cross-platform abstraction */
OSErr GetPlatformAudioInfo(char **platformName, char **driverVersion, long *capabilities);
OSErr SetPlatformAudioPreferences(const void *preferences);

/* ===== Audio Utilities ===== */

/* Format conversion */
OSErr ConvertAudioFormat(const void *inputData, long inputSize,
                         const AudioOutputFormat *inputFormat,
                         const AudioOutputFormat *outputFormat,
                         void **outputData, long *outputSize);

/* Sample rate conversion */
OSErr ResampleAudio(const void *inputData, long inputFrames,
                    const AudioOutputFormat *inputFormat, long outputSampleRate,
                    void **outputData, long *outputFrames);

/* Channel mapping */
OSErr MapAudioChannels(const void *inputData, long dataSize,
                       short inputChannels, short outputChannels,
                       void **outputData, long *outputSize);

/* Audio validation */
OSErr ValidateAudioData(const void *audioData, long dataSize,
                        const AudioOutputFormat *format, Boolean *isValid);
OSErr AnalyzeAudioData(const void *audioData, long dataSize,
                       const AudioOutputFormat *format, Fixed *rmsLevel, Fixed *peakLevel);

/* Timing utilities */
OSErr AudioFramesToTime(long frameCount, long sampleRate, long *timeMs);
OSErr AudioTimeToFrames(long timeMs, long sampleRate, long *frameCount);
OSErr GetCurrentAudioTime(long *timeMs);

/* ===== Audio Debugging ===== */

/* Debug information */
OSErr GetAudioOutputDebugInfo(char **debugInfo);
OSErr DumpAudioOutputState(FILE *output);
OSErr LogAudioActivity(const char *message);

/* Performance monitoring */
OSErr EnableAudioPerformanceMonitoring(Boolean enable);
OSErr GetAudioPerformanceData(double *cpuUsage, long *memoryUsage, long *bufferUsage);

/* Audio testing */
OSErr GenerateTestTone(double frequency, long durationMs, const AudioOutputFormat *format,
                       void **audioData, long *dataSize);
OSErr PlayTestTone(double frequency, long durationMs);
OSErr TestAudioOutputDevice(const char *deviceID, Boolean *isWorking, char **errorMessage);

#ifdef __cplusplus
}
#endif

#endif /* _SPEECHOUTPUT_H_ */