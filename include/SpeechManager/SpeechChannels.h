/*
 * File: SpeechChannels.h
 *
 * Contains: Speech channel management and control for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides speech channel functionality
 *              including channel lifecycle, properties, and control.
 */

#ifndef _SPEECHCHANNELS_H_
#define _SPEECHCHANNELS_H_

#include "SystemTypes.h"
#include <stdio.h>

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Speech Channel Constants ===== */

/* Channel states */
#define kChannelStateUnitialized 0
#define kChannelStateClosed      1
#define kChannelStateOpen        2
#define kChannelStateActive      3
#define kChannelStatePaused      4

/* Channel types */
#define kChannelTypeMonophonic   0
#define kChannelTypePolyphonic   1

/* Channel priorities */
#define kChannelPriorityMin      0
#define kChannelPriorityNormal   128
#define kChannelPriorityMax      255

/* Channel flags */
#define kChannelFlagNone         0x0000
#define kChannelFlagAsync        0x0001
#define kChannelFlagDoNotInterrupt 0x0002
#define kChannelFlagPreemptible  0x0004

/* ===== Speech Channel Type Definitions ===== */

/* Channel priority type */
typedef unsigned char SpeechChannelPriority;

/* Channel flags type */
typedef unsigned short SpeechChannelFlags;

/* Channel state structure */
typedef struct {
    short state;
    short type;
    SpeechChannelPriority priority;
    SpeechChannelFlags flags;
    Boolean busy;
    Boolean paused;
    long bytesProcessed;
    long bytesRemaining;
} SpeechChannelState;

/* Channel configuration structure */
typedef struct {
    VoiceSpec voice;
    Fixed rate;
    Fixed pitch;
    Fixed volume;
    SpeechChannelPriority priority;
    SpeechChannelFlags flags;
    short inputMode;
    short outputMode;
} SpeechChannelConfig;

/* Channel information structure */
typedef struct {
    SpeechChannelConfig config;
    SpeechChannelState state;
    long totalBytesProcessed;
    long totalBytesRemaining;
    long creationTime;
    long lastActivityTime;
} SpeechChannelInfo;

/* Channel statistics structure */
typedef struct {
    long bytesProcessed;
    long bytesRemaining;
    long timeSinceCreation;
    long timeActive;
    long timePaused;
    long synthesisCount;
    long errorCount;
} SpeechChannelStats;

/* Channel event callback */
typedef void (*SpeechChannelEventProc)(SpeechChannel chan, long eventType, void *eventData, void *userData);

/* ===== Speech Channel Structures ===== */

/* Channel information already defined above */

/* Channel configuration already defined above */

/* Channel statistics already defined above */

/* ===== Channel Management ===== */

/* Channel creation and disposal */
OSErr NewSpeechChannelWithConfig(const SpeechChannelConfig *config, SpeechChannel *chan);
OSErr CloneSpeechChannel(SpeechChannel sourceChannel, SpeechChannel *newChannel);

/* Channel state management */
OSErr GetSpeechChannelState(SpeechChannel chan, SpeechChannelState *state);
OSErr OpenSpeechChannel(SpeechChannel chan);
OSErr CloseSpeechChannel(SpeechChannel chan);
OSErr ResetSpeechChannel(SpeechChannel chan);

/* ===== Channel Properties ===== */

/* Voice management for channels */
OSErr SetSpeechChannelVoice(SpeechChannel chan, const VoiceSpec *voice);
OSErr GetSpeechChannelVoice(SpeechChannel chan, VoiceSpec *voice);

/* Rate control */
OSErr SetSpeechChannelRate(SpeechChannel chan, Fixed rate);
OSErr GetSpeechChannelRate(SpeechChannel chan, Fixed *rate);

/* Pitch control */
OSErr SetSpeechChannelPitch(SpeechChannel chan, Fixed pitch);
OSErr GetSpeechChannelPitch(SpeechChannel chan, Fixed *pitch);

/* Volume control */
OSErr SetSpeechChannelVolume(SpeechChannel chan, Fixed volume);
OSErr GetSpeechChannelVolume(SpeechChannel chan, Fixed *volume);

/* Channel priority */
OSErr SetSpeechChannelPriority(SpeechChannel chan, SpeechChannelPriority priority);
OSErr GetSpeechChannelPriority(SpeechChannel chan, SpeechChannelPriority *priority);

/* Channel flags */
OSErr SetSpeechChannelFlags(SpeechChannel chan, SpeechChannelFlags flags);
OSErr GetSpeechChannelFlags(SpeechChannel chan, SpeechChannelFlags *flags);

/* ===== Channel Information and Control ===== */

/* Generic channel information */
OSErr SetSpeechChannelInfo(SpeechChannel chan, OSType selector, void *speechInfo);
OSErr GetSpeechChannelInfo(SpeechChannel chan, OSType selector, void *speechInfo);

/* Channel status */
Boolean IsSpeechChannelBusy(SpeechChannel chan);
Boolean IsSpeechChannelPaused(SpeechChannel chan);
OSErr GetSpeechChannelBytesLeft(SpeechChannel chan, long *bytesLeft);

/* Channel dictionary support */
OSErr SetSpeechChannelDictionary(SpeechChannel chan, void *dictionary);
OSErr GetSpeechChannelDictionary(SpeechChannel chan, void **dictionary);

/* ===== Channel Text Processing ===== */

/* Text speaking */
OSErr SpeakText(SpeechChannel chan, void *textBuf, long textBytes);
OSErr SpeakBuffer(SpeechChannel chan, void *textBuf, long textBytes, long controlFlags);

/* Text queueing */
OSErr QueueSpeechText(SpeechChannel chan, void *textBuf, long textBytes);
OSErr ClearSpeechQueue(SpeechChannel chan);
OSErr GetSpeechQueueLength(SpeechChannel chan, long *queueLength);

/* Text streaming */
OSErr BeginSpeechStream(SpeechChannel chan);
OSErr WriteSpeechStream(SpeechChannel chan, void *textBuf, long textBytes);
OSErr EndSpeechStream(SpeechChannel chan);

/* ===== Channel Control ===== */

/* Speech control */
OSErr StopSpeech(SpeechChannel chan);
OSErr StopSpeechAt(SpeechChannel chan, long whereToStop);
OSErr PauseSpeechAt(SpeechChannel chan, long whereToPause);
OSErr ContinueSpeech(SpeechChannel chan);

/* Channel synchronization */
OSErr WaitForSpeechCompletion(SpeechChannel chan, long timeoutMs);
OSErr FlushSpeechChannel(SpeechChannel chan);

/* ===== Channel Monitoring ===== */

/* Channel statistics */
OSErr GetSpeechChannelStats(SpeechChannel chan, SpeechChannelStats *stats);
OSErr ResetSpeechChannelStats(SpeechChannel chan);

/* Performance monitoring */
OSErr EnableSpeechChannelMonitoring(SpeechChannel chan, Boolean enable);
OSErr GetSpeechChannelPerformance(SpeechChannel chan, double *cpuUsage, long *memoryUsage);

/* Event monitoring */

/* Event callback */

OSErr SetSpeechChannelEventCallback(SpeechChannel chan, SpeechChannelEventProc callback,
                                    void *userData);

/* ===== System Channel Management ===== */

/* System-wide channel operations */
OSErr CountActiveSpeechChannels(short *channelCount);
OSErr GetActiveSpeechChannels(SpeechChannel **channels, short *channelCount);
Boolean IsSpeechSystemBusy(void);

/* Channel enumeration */
OSErr EnumerateSpeechChannels(Boolean (*callback)(SpeechChannel chan, void *userData), void *userData);

/* System channel control */
OSErr StopAllSpeechChannels(void);
OSErr PauseAllSpeechChannels(void);
OSErr ResumeAllSpeechChannels(void);

/* Channel priority management */
OSErr SetSystemChannelPriorities(Boolean enablePriorities);
OSErr InterruptLowerPriorityChannels(SpeechChannelPriority minimumPriority);

/* ===== Channel Resource Management ===== */

/* Memory management */
OSErr SetSpeechChannelMemoryLimit(SpeechChannel chan, long memoryLimit);
OSErr GetSpeechChannelMemoryUsage(SpeechChannel chan, long *memoryUsage);

/* Buffer management */
OSErr SetSpeechChannelBufferSize(SpeechChannel chan, long bufferSize);
OSErr GetSpeechChannelBufferSize(SpeechChannel chan, long *bufferSize);
OSErr FlushSpeechChannelBuffers(SpeechChannel chan);

/* Resource cleanup */
OSErr CleanupIdleChannels(long maxIdleTime);
OSErr OptimizeSpeechChannelMemory(SpeechChannel chan);

/* ===== Channel Threading ===== */

/* Thread safety */
OSErr LockSpeechChannel(SpeechChannel chan);
OSErr UnlockSpeechChannel(SpeechChannel chan);
OSErr SetSpeechChannelThreadSafe(SpeechChannel chan, Boolean threadSafe);

/* Background processing */
OSErr SetSpeechChannelBackgroundMode(SpeechChannel chan, Boolean backgroundMode);
OSErr GetSpeechChannelBackgroundMode(SpeechChannel chan, Boolean *backgroundMode);

/* ===== Channel Configuration ===== */

/* Default configuration */
OSErr GetDefaultChannelConfig(SpeechChannelConfig *config);
OSErr SetDefaultChannelConfig(const SpeechChannelConfig *config);

/* Channel presets */
OSErr SaveChannelPreset(SpeechChannel chan, const char *presetName);
OSErr LoadChannelPreset(SpeechChannel chan, const char *presetName);
OSErr DeleteChannelPreset(const char *presetName);

/* Configuration validation */
OSErr ValidateChannelConfig(const SpeechChannelConfig *config, Boolean *isValid,
                            char **errorMessage);

/* ===== Channel Debugging ===== */

/* Debug information */
OSErr GetSpeechChannelDebugInfo(SpeechChannel chan, char **debugInfo);
OSErr DumpSpeechChannelState(SpeechChannel chan, FILE *output);

/* Channel tracing */
OSErr EnableSpeechChannelTracing(SpeechChannel chan, Boolean enable);
OSErr GetSpeechChannelTrace(SpeechChannel chan, char **traceData, long *traceSize);

/* Error reporting */
OSErr GetSpeechChannelLastError(SpeechChannel chan, OSErr *error, char **errorMessage);
OSErr ClearSpeechChannelErrors(SpeechChannel chan);

/* ===== Channel Utilities ===== */

/* Channel comparison */
Boolean AreSpeechChannelsEqual(SpeechChannel chan1, SpeechChannel chan2);
int CompareSpeechChannels(SpeechChannel chan1, SpeechChannel chan2);

/* Channel validation */
Boolean IsValidSpeechChannel(SpeechChannel chan);
OSErr ValidateSpeechChannel(SpeechChannel chan);

/* Channel conversion */
OSErr SpeechChannelToString(SpeechChannel chan, char *string, long stringSize);
OSErr StringToSpeechChannel(const char *string, SpeechChannel *chan);

#ifdef __cplusplus
}
#endif

#endif /* _SPEECHCHANNELS_H_ */
