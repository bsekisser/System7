/*
 * File: TextToSpeech.h
 *
 * Contains: Text-to-speech conversion and processing for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides text-to-speech conversion functionality
 *              including text processing, phoneme conversion, and speech synthesis.
 */

#ifndef _TEXTTOSPEECH_H_
#define _TEXTTOSPEECH_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Text Processing Constants ===== */

/* Text input modes */

/* Text processing flags */

/* Text analysis results */

/* ===== Text Structures ===== */

/* Text processing context */

/* Text segment for analysis */

/* Text analysis result */

/* Phoneme conversion result */

/* ===== Text Processing Functions ===== */

/* Context management */
OSErr CreateTextProcessingContext(TextProcessingContext **context);
OSErr DisposeTextProcessingContext(TextProcessingContext *context);
OSErr SetTextProcessingMode(TextProcessingContext *context, TextInputMode mode);
OSErr SetTextProcessingFlags(TextProcessingContext *context, TextProcessingFlags flags);

/* Text analysis */
OSErr AnalyzeText(const char *text, long textLength, TextProcessingContext *context,
                  TextAnalysisResult **result);
OSErr DisposeTextAnalysisResult(TextAnalysisResult *result);

/* Text normalization */
OSErr NormalizeText(const char *inputText, long inputLength, TextProcessingContext *context,
                    char **outputText, long *outputLength);
OSErr ExpandAbbreviations(const char *inputText, long inputLength, TextProcessingContext *context,
                          char **outputText, long *outputLength);
OSErr ProcessNumbers(const char *inputText, long inputLength, TextProcessingContext *context,
                     char **outputText, long *outputLength);

/* Phoneme conversion */
OSErr ConvertTextToPhonemes(const char *text, long textLength, TextProcessingContext *context,
                            PhonemeConversionResult **result);
OSErr DisposePhonemeConversionResult(PhonemeConversionResult *result);

/* Dictionary support */
OSErr LoadTextDictionary(const char *dictionaryPath, void **dictionary);
OSErr UnloadTextDictionary(void *dictionary);
OSErr LookupWord(void *dictionary, const char *word, char **pronunciation);
OSErr AddWordToDictionary(void *dictionary, const char *word, const char *pronunciation);

/* Markup processing */
OSErr ProcessSSMLMarkup(const char *ssmlText, long textLength, TextProcessingContext *context,
                        char **processedText, long *processedLength);
OSErr ExtractPlainText(const char *markupText, long textLength,
                       char **plainText, long *plainLength);

/* Text segmentation */
OSErr SegmentTextIntoSentences(const char *text, long textLength,
                               TextSegment **sentences, long *sentenceCount);
OSErr SegmentTextIntoWords(const char *text, long textLength,
                           TextSegment **words, long *wordCount);
OSErr SegmentTextIntoPhrases(const char *text, long textLength, TextProcessingContext *context,
                             TextSegment **phrases, long *phraseCount);

/* ===== Speech Synthesis Text Interface ===== */

/* Text-to-speech conversion */
OSErr SpeakProcessedText(SpeechChannel chan, const char *text, long textLength,
                         TextProcessingContext *context);
OSErr SpeakTextWithCallback(SpeechChannel chan, const char *text, long textLength,
                            TextProcessingContext *context, SpeechTextDoneProcPtr callback,
                            void *userData);

/* Buffered text processing */
OSErr BeginTextProcessing(SpeechChannel chan, TextProcessingContext *context);
OSErr ProcessTextBuffer(SpeechChannel chan, const char *textBuffer, long bufferLength,
                        Boolean isLastBuffer);
OSErr EndTextProcessing(SpeechChannel chan);

/* Text streaming */

OSErr CreateTextStream(SpeechChannel chan, TextProcessingContext *context,
                       TextStreamContext **stream);
OSErr WriteToTextStream(TextStreamContext *stream, const char *text, long textLength);
OSErr FlushTextStream(TextStreamContext *stream);
OSErr CloseTextStream(TextStreamContext *stream);

/* ===== Text Processing Utilities ===== */

/* Language detection */
OSErr DetectTextLanguage(const char *text, long textLength, short *language, short *confidence);
OSErr IsTextInLanguage(const char *text, long textLength, short language, Boolean *isMatch);

/* Text validation */
Boolean IsValidTextForSpeech(const char *text, long textLength);
OSErr ValidateTextEncoding(const char *text, long textLength, long *encoding);
OSErr ConvertTextEncoding(const char *inputText, long inputLength, long inputEncoding,
                          long outputEncoding, char **outputText, long *outputLength);

/* Text statistics */
OSErr GetTextStatistics(const char *text, long textLength, long *wordCount, long *sentenceCount,
                        long *characterCount, long *estimatedSpeechTime);

/* Pronunciation hints */
OSErr SetPronunciationHint(TextProcessingContext *context, const char *word,
                           const char *pronunciation);
OSErr RemovePronunciationHint(TextProcessingContext *context, const char *word);
OSErr GetPronunciationHint(TextProcessingContext *context, const char *word,
                           char **pronunciation);

/* Text emphasis and prosody */
OSErr SetTextEmphasis(TextProcessingContext *context, long startPos, long endPos,
                      short emphasisLevel);
OSErr SetTextProsody(TextProcessingContext *context, long startPos, long endPos,
                     Fixed rate, Fixed pitch, Fixed volume);
OSErr ClearTextAttributes(TextProcessingContext *context, long startPos, long endPos);

/* Text caching */
OSErr CacheProcessedText(const char *originalText, long textLength,
                         const char *processedText, long processedLength);
OSErr LookupCachedText(const char *originalText, long textLength,
                       char **processedText, long *processedLength);
OSErr ClearTextCache(void);

/* ===== Text Processing Callbacks ===== */

/* Text processing progress callback */

/* Text analysis callback */

/* Pronunciation callback */

/* Callback registration */
OSErr SetTextProcessingProgressCallback(TextProcessingContext *context,
                                        TextProcessingProgressProc callback, void *userData);
OSErr SetTextAnalysisCallback(TextProcessingContext *context,
                              TextAnalysisProc callback, void *userData);
OSErr SetPronunciationCallback(TextProcessingContext *context,
                               PronunciationProc callback, void *userData);

/* ===== Advanced Text Features ===== */

/* Text bookmarks */
OSErr SetTextBookmark(TextProcessingContext *context, long position, const char *name);
OSErr GetTextBookmark(TextProcessingContext *context, const char *name, long *position);
OSErr RemoveTextBookmark(TextProcessingContext *context, const char *name);

/* Text variables */
OSErr SetTextVariable(TextProcessingContext *context, const char *name, const char *value);
OSErr GetTextVariable(TextProcessingContext *context, const char *name, char **value);
OSErr ExpandTextVariables(const char *inputText, long inputLength,
                          TextProcessingContext *context, char **outputText, long *outputLength);

/* Conditional text */
OSErr ProcessConditionalText(const char *inputText, long inputLength,
                             TextProcessingContext *context, char **outputText, long *outputLength);

#ifdef __cplusplus
}
#endif

#endif /* _TEXTTOSPEECH_H_ */