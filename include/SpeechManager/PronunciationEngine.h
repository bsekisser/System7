/*
 * File: PronunciationEngine.h
 *
 * Contains: Phoneme processing and pronunciation engine for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides phoneme processing and pronunciation
 *              functionality including phonetic analysis and conversion.
 */

#ifndef _PRONUNCIATIONENGINE_H_
#define _PRONUNCIATIONENGINE_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Phoneme and Pronunciation Constants ===== */

/* Phoneme symbol types */

/* Phoneme categories */

/* Stress levels */

/* Syllable boundaries */

/* ===== Phoneme Structures ===== */

/* Extended phoneme information */

/* Pronunciation entry */

/* Phonetic analysis result */

/* Pronunciation dictionary */

/* ===== Pronunciation Engine Management ===== */

/* Engine initialization */
OSErr InitializePronunciationEngine(void);
void CleanupPronunciationEngine(void);

/* Dictionary management */
OSErr CreatePronunciationDictionary(PronunciationDictionary **dictionary);
OSErr DisposePronunciationDictionary(PronunciationDictionary *dictionary);
OSErr LoadPronunciationDictionary(const char *dictionaryPath, PronunciationDictionary **dictionary);
OSErr SavePronunciationDictionary(PronunciationDictionary *dictionary, const char *dictionaryPath);

/* Dictionary operations */
OSErr AddPronunciation(PronunciationDictionary *dictionary, const char *word,
                       const char *pronunciation, PhonemeSymbolType symbolType);
OSErr RemovePronunciation(PronunciationDictionary *dictionary, const char *word);
OSErr LookupPronunciation(PronunciationDictionary *dictionary, const char *word,
                          PronunciationEntry **entry);
OSErr UpdatePronunciation(PronunciationDictionary *dictionary, const char *word,
                          const PronunciationEntry *newEntry);

/* ===== Text-to-Phoneme Conversion ===== */

/* Basic conversion */
OSErr ConvertTextToPhonemes(const char *text, long textLength, PhonemeSymbolType outputType,
                            char *phonemeBuffer, long bufferSize, long *phonemeLength);
OSErr ConvertTextToPhonemeString(const char *text, long textLength,
                                 char *phonemeString, long stringSize, long *stringLength);

/* Advanced conversion with analysis */
OSErr AnalyzeTextPhonetically(const char *text, long textLength, PhonemeSymbolType outputType,
                              PronunciationDictionary *dictionary, PhoneticAnalysis **analysis);
OSErr DisposePhoneticAnalysis(PhoneticAnalysis *analysis);

/* Word-by-word conversion */
OSErr ConvertWordToPhonemes(const char *word, PhonemeSymbolType outputType,
                            PronunciationDictionary *dictionary,
                            char *phonemeBuffer, long bufferSize);

/* ===== Phoneme-to-Text Conversion ===== */

/* Reverse conversion */
OSErr ConvertPhonemesToText(const char *phonemeString, PhonemeSymbolType inputType,
                            char *textBuffer, long bufferSize, long *textLength);
OSErr TransliteratePhonemesToOrthography(const char *phonemeString, PhonemeSymbolType inputType,
                                         char *orthographicText, long bufferSize);

/* ===== Phoneme Symbol Conversion ===== */

/* Symbol type conversion */
OSErr ConvertPhonemeSymbols(const char *inputPhonemes, PhonemeSymbolType inputType,
                            PhonemeSymbolType outputType, char *outputPhonemes, long bufferSize);

/* Symbol validation */
OSErr ValidatePhonemeString(const char *phonemeString, PhonemeSymbolType symbolType,
                            Boolean *isValid, char **errorMessage);

/* Symbol information */
OSErr GetPhonemeSymbolInfo(const char *symbol, PhonemeSymbolType symbolType,
                           PhonemeInfoExtended *info);

/* ===== Stress and Syllable Analysis ===== */

/* Stress pattern analysis */
OSErr AnalyzeStressPattern(const char *word, PronunciationDictionary *dictionary,
                           StressLevel **stressLevels, short *syllableCount);
OSErr ApplyStressPattern(char *phonemeString, const StressLevel *stressLevels,
                         short syllableCount, PhonemeSymbolType symbolType);

/* Syllable boundary detection */
OSErr DetectSyllableBoundaries(const char *phonemeString, PhonemeSymbolType symbolType,
                               SyllableBoundary **boundaries, short *boundaryCount);
OSErr InsertSyllableBoundaries(char *phonemeString, long bufferSize,
                               const SyllableBoundary *boundaries, short boundaryCount);

/* ===== Pronunciation Rules ===== */

/* Rule types */

/* Pronunciation rule */

/* Rule management */
OSErr AddPronunciationRule(PronunciationDictionary *dictionary, const PronunciationRule *rule);
OSErr RemovePronunciationRule(PronunciationDictionary *dictionary, const char *pattern);
OSErr GetPronunciationRules(PronunciationDictionary *dictionary, PronunciationRule **rules,
                            long *ruleCount);
OSErr ApplyPronunciationRules(const char *text, PronunciationDictionary *dictionary,
                              char *phonemeOutput, long outputSize);

/* ===== Language-Specific Support ===== */

/* Language models */
OSErr LoadLanguageModel(short languageCode, PronunciationDictionary **languageDict);
OSErr SetDefaultLanguage(short languageCode);
OSErr GetDefaultLanguage(short *languageCode);

/* Multi-language support */
OSErr DetectTextLanguage(const char *text, long textLength, short *languageCode,
                         short *confidence);
OSErr ConvertTextWithLanguage(const char *text, long textLength, short languageCode,
                              PhonemeSymbolType outputType, char *phonemeOutput, long outputSize);

/* ===== Acoustic Modeling ===== */

/* Acoustic features */

/* Acoustic modeling */
OSErr GetPhonemeAcousticFeatures(short phonemeOpcode, AcousticFeatures **features);
OSErr DisposeAcousticFeatures(AcousticFeatures *features);
OSErr SynthesizePhonemeAudio(short phonemeOpcode, long duration, void **audioData, long *audioSize);

/* ===== Pronunciation Training ===== */

/* Training data */

/* Training operations */
OSErr TrainPronunciationModel(PronunciationTrainingData *trainingData, long dataCount);
OSErr EvaluatePronunciation(const char *word, const char *actualPhonemes,
                            PronunciationDictionary *dictionary, double *similarity);
OSErr AdaptPronunciationModel(const char *word, const char *correctPhonemes,
                              PronunciationDictionary *dictionary);

/* ===== Pronunciation Callbacks ===== */

/* Word analysis callback */

/* Phoneme conversion callback */

/* Rule application callback */

/* Callback registration */
OSErr SetWordAnalysisCallback(PronunciationDictionary *dictionary, WordAnalysisProc callback,
                              void *userData);
OSErr SetPhonemeConversionCallback(PhonemeConversionProc callback, void *userData);
OSErr SetRuleApplicationCallback(PronunciationDictionary *dictionary, RuleApplicationProc callback,
                                 void *userData);

/* ===== Utilities and Tools ===== */

/* Phoneme utilities */
Boolean IsValidPhonemeOpcode(short opcode);
OSErr GetPhonemeCount(short *phonemeCount);
OSErr GetIndPhoneme(short index, PhonemeInfoExtended *info);

/* Dictionary utilities */
OSErr GetDictionaryWordCount(PronunciationDictionary *dictionary, long *wordCount);
OSErr GetDictionaryWords(PronunciationDictionary *dictionary, char ***words, long *wordCount);
OSErr SearchDictionary(PronunciationDictionary *dictionary, const char *pattern,
                       PronunciationEntry **matches, long *matchCount);

/* Pronunciation comparison */
double ComparePronunciations(const char *pronunciation1, const char *pronunciation2,
                             PhonemeSymbolType symbolType);
OSErr AlignPronunciations(const char *pronunciation1, const char *pronunciation2,
                          PhonemeSymbolType symbolType, char **alignment1, char **alignment2);

/* Text preprocessing */
OSErr PreprocessTextForPronunciation(const char *inputText, long inputLength,
                                     char **preprocessedText, long *outputLength);
OSErr NormalizeOrthography(const char *inputText, long inputLength,
                           char **normalizedText, long *outputLength);

/* ===== Performance and Caching ===== */

/* Pronunciation caching */
OSErr EnablePronunciationCache(Boolean enable);
OSErr ClearPronunciationCache(void);
OSErr GetCacheStatistics(long *cacheSize, long *hitCount, long *missCount);

/* Performance optimization */
OSErr OptimizeDictionary(PronunciationDictionary *dictionary);
OSErr CompressDictionary(PronunciationDictionary *dictionary, Boolean compress);

#ifdef __cplusplus
}
#endif

#endif /* _PRONUNCIATIONENGINE_H_ */