/*
 * File: SpeechSynthesis.h
 *
 * Contains: Speech synthesis engine integration for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides speech synthesis engine functionality
 *              including engine management, audio synthesis, and output control.
 */

#ifndef _SPEECHSYNTHESIS_H_
#define _SPEECHSYNTHESIS_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Synthesis Engine Constants ===== */

/* Synthesis engine types */
#define kSynthEngineTypeFestival 0
#define kSynthEngineTypeEspeak   1
#define kSynthEngineTypeVoiceXML 2

/* Synthesis quality levels */
#define kSynthQualityLow    0
#define kSynthQualityMed    1
#define kSynthQualityHigh   2

/* Synthesis engine capabilities */
#define kSynthCapPolyphonic   0x0001
#define kSynthCapStreaming    0x0002
#define kSynthCapRealtime     0x0004
#define kSynthCapPhonetic     0x0008

/* Audio format specifications */
#define kAudioFormatPCM       0
#define kAudioFormatMP3       1
#define kAudioFormatWAV       2

/* ===== Synthesis Engine Type Definitions ===== */

/* Opaque handles for synthesis objects */
typedef long SynthEngineRef;
typedef long SynthEngineType;
typedef long SynthEngineCapabilities;
typedef long SynthesisState;
typedef long SynthesisProgress;
typedef short SynthQuality;
typedef long AudioFormatDescriptor;
typedef long EmotionalState;

/* Forward declarations for opaque structures */
typedef struct SynthEngineInfoStruct *SynthEngineInfo;
typedef struct SynthesisParametersStruct *SynthesisParameters;
typedef struct SynthesisResultStruct *SynthesisResult;

/* Callback types */
typedef void (*SynthesisProgressProc)(SynthEngineRef engine, long bytesProcessed, long totalBytes, void *userData);
typedef void (*SynthesisCompletionProc)(SynthEngineRef engine, void *result, void *userData);
typedef void (*SynthesisErrorProc)(SynthEngineRef engine, OSErr error, void *userData);
typedef void (*SynthesisAudioProc)(SynthEngineRef engine, const void *audioData, long audioLength, void *userData);

/* ===== Synthesis Engine Structures ===== */

/* Audio format descriptor */

/* Synthesis engine information - opaque */

/* Synthesis parameters - opaque */

/* Synthesis progress information - opaque */

/* Synthesis result - opaque */

/* ===== Synthesis Engine Management ===== */

/* Engine opaque handle already defined above */

/* Engine initialization and cleanup */
OSErr InitializeSpeechSynthesis(void);
void CleanupSpeechSynthesis(void);

/* Engine enumeration */
OSErr CountSynthEngines(short *engineCount);
OSErr GetIndSynthEngine(short index, SynthEngineRef *engine);
OSErr GetSynthEngineInfo(SynthEngineRef engine, SynthEngineInfo *info);

/* Engine selection */
OSErr FindBestSynthEngine(SynthEngineCapabilities requiredCaps, SynthEngineRef *engine);
OSErr FindSynthEngineByType(SynthEngineType type, SynthEngineRef *engine);
OSErr FindSynthEngineByName(const char *engineName, SynthEngineRef *engine);

/* Engine lifecycle */
OSErr CreateSynthEngine(SynthEngineType type, SynthEngineRef *engine);
OSErr DisposeSynthEngine(SynthEngineRef engine);
OSErr OpenSynthEngine(SynthEngineRef engine);
OSErr CloseSynthEngine(SynthEngineRef engine);

/* ===== Synthesis Operations ===== */

/* Text synthesis */
OSErr SynthesizeText(SynthEngineRef engine, const char *text, long textLength,
                     const SynthesisParameters *params, SynthesisResult **result);
OSErr SynthesizeTextToFile(SynthEngineRef engine, const char *text, long textLength,
                           const SynthesisParameters *params, const char *outputFile);
OSErr SynthesizeTextStreaming(SynthEngineRef engine, const char *text, long textLength,
                              const SynthesisParameters *params, void *streamContext);

/* Phoneme synthesis */
OSErr SynthesizePhonemes(SynthEngineRef engine, const char *phonemes, long phonemeLength,
                         const SynthesisParameters *params, SynthesisResult **result);

/* SSML synthesis */
OSErr SynthesizeSSML(SynthEngineRef engine, const char *ssmlText, long ssmlLength,
                     const SynthesisParameters *params, SynthesisResult **result);

/* Result management */
OSErr DisposeSynthesisResult(SynthesisResult *result);
OSErr CopySynthesisResult(const SynthesisResult *source, SynthesisResult **dest);

/* ===== Synthesis Control ===== */

/* Synthesis state */

/* Synthesis control */
OSErr StartSynthesis(SynthEngineRef engine);
OSErr PauseSynthesis(SynthEngineRef engine);
OSErr ResumeSynthesis(SynthEngineRef engine);
OSErr StopSynthesis(SynthEngineRef engine);
OSErr GetSynthesisState(SynthEngineRef engine, SynthesisState *state);

/* Synthesis monitoring */
OSErr GetSynthesisProgress(SynthEngineRef engine, SynthesisProgress *progress);
Boolean IsSynthesisActive(SynthEngineRef engine);
OSErr WaitForSynthesisCompletion(SynthEngineRef engine, long timeoutMs);

/* ===== Synthesis Parameters ===== */

/* Parameter management */
OSErr SetSynthesisParameters(SynthEngineRef engine, const SynthesisParameters *params);
OSErr GetSynthesisParameters(SynthEngineRef engine, SynthesisParameters *params);
OSErr ResetSynthesisParameters(SynthEngineRef engine);

/* Individual parameter control */
OSErr SetSynthesisRate(SynthEngineRef engine, Fixed rate);
OSErr GetSynthesisRate(SynthEngineRef engine, Fixed *rate);
OSErr SetSynthesisPitch(SynthEngineRef engine, Fixed pitch);
OSErr GetSynthesisPitch(SynthEngineRef engine, Fixed *pitch);
OSErr SetSynthesisVolume(SynthEngineRef engine, Fixed volume);
OSErr GetSynthesisVolume(SynthEngineRef engine, Fixed *volume);
OSErr SetSynthesisQuality(SynthEngineRef engine, SynthQuality quality);
OSErr GetSynthesisQuality(SynthEngineRef engine, SynthQuality *quality);

/* Audio format control */
OSErr SetSynthesisAudioFormat(SynthEngineRef engine, const AudioFormatDescriptor *format);
OSErr GetSynthesisAudioFormat(SynthEngineRef engine, AudioFormatDescriptor *format);
OSErr GetSupportedAudioFormats(SynthEngineRef engine, AudioFormatDescriptor **formats, short *count);

/* ===== Synthesis Callbacks ===== */

/* Synthesis progress callback */

/* Synthesis completion callback */

/* Synthesis error callback */

/* Audio output callback */

/* Callback registration */
OSErr SetSynthesisProgressCallback(SynthEngineRef engine, SynthesisProgressProc callback, void *userData);
OSErr SetSynthesisCompletionCallback(SynthEngineRef engine, SynthesisCompletionProc callback, void *userData);
OSErr SetSynthesisErrorCallback(SynthEngineRef engine, SynthesisErrorProc callback, void *userData);
OSErr SetSynthesisAudioCallback(SynthEngineRef engine, SynthesisAudioProc callback, void *userData);

/* ===== Voice Engine Integration ===== */

/* Voice loading for engines */
OSErr LoadVoiceForEngine(SynthEngineRef engine, const VoiceSpec *voice);
OSErr UnloadVoiceFromEngine(SynthEngineRef engine, const VoiceSpec *voice);
OSErr SetEngineVoice(SynthEngineRef engine, const VoiceSpec *voice);
OSErr GetEngineVoice(SynthEngineRef engine, VoiceSpec *voice);

/* Voice compatibility */
OSErr CheckVoiceEngineCompatibility(const VoiceSpec *voice, SynthEngineRef engine, Boolean *compatible);
OSErr GetCompatibleEnginesForVoice(const VoiceSpec *voice, SynthEngineRef **engines, short *count);

/* ===== Engine-Specific Features ===== */

/* Engine configuration */
OSErr SetEngineProperty(SynthEngineRef engine, OSType property, const void *value, long valueSize);
OSErr GetEngineProperty(SynthEngineRef engine, OSType property, void *value, long *valueSize);
OSErr GetEnginePropertyInfo(SynthEngineRef engine, OSType property, OSType *dataType, long *dataSize);

/* Engine capabilities testing */
Boolean EngineSupportsCapability(SynthEngineRef engine, SynthEngineCapabilities capability);
OSErr GetEngineCapabilities(SynthEngineRef engine, SynthEngineCapabilities *capabilities);

/* Engine statistics */
OSErr GetEngineStatistics(SynthEngineRef engine, long *totalSyntheses, long *totalBytes,
                          long *averageSpeed, long *errorCount);
OSErr ResetEngineStatistics(SynthEngineRef engine);

/* ===== Advanced Synthesis Features ===== */

/* Emotional synthesis */

OSErr SetSynthesisEmotion(SynthEngineRef engine, EmotionalState emotion, Fixed intensity);
OSErr GetSynthesisEmotion(SynthEngineRef engine, EmotionalState *emotion, Fixed *intensity);

/* Multi-voice synthesis */
OSErr BeginMultiVoiceSynthesis(SynthEngineRef engine);
OSErr AddVoiceToSynthesis(SynthEngineRef engine, const VoiceSpec *voice, const char *text, long textLength);
OSErr EndMultiVoiceSynthesis(SynthEngineRef engine, SynthesisResult **result);

/* Synthesis caching */
OSErr EnableSynthesisCache(SynthEngineRef engine, Boolean enable);
OSErr ClearSynthesisCache(SynthEngineRef engine);
OSErr GetCacheStatistics(SynthEngineRef engine, long *cacheHits, long *cacheMisses, long *cacheSize);

/* ===== Platform Integration ===== */

/* Platform-specific engine support */
#ifdef PLATFORM_REMOVED_WIN32
OSErr InitializeSAPIEngine(SynthEngineRef *engine);
OSErr ConfigureSAPIEngine(SynthEngineRef engine, void *sapiConfig);
#endif

#ifdef PLATFORM_REMOVED_APPLE
OSErr InitializeAVSpeechEngine(SynthEngineRef *engine);
OSErr ConfigureAVSpeechEngine(SynthEngineRef engine, void *avConfig);
#endif

#ifdef PLATFORM_REMOVED_LINUX
OSErr InitializeESpeakEngine(SynthEngineRef *engine);
OSErr InitializeFestivalEngine(SynthEngineRef *engine);
OSErr ConfigureLinuxEngine(SynthEngineRef engine, const char *configFile);
#endif

/* Network engines */
OSErr InitializeCloudEngine(SynthEngineRef *engine, const char *apiKey, const char *endpoint);
OSErr SetCloudEngineCredentials(SynthEngineRef engine, const char *apiKey, const char *secret);

#ifdef __cplusplus
}
#endif

#endif /* _SPEECHSYNTHESIS_H_ */