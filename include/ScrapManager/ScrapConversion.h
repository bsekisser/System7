/*
 * ScrapConversion.h - Data Transformation APIs
 * System 7.1 Portable - Scrap Manager Component
 *
 * Defines data format conversion, transformation, and coercion APIs
 * for seamless clipboard data exchange between different formats.
 */

#ifndef SCRAP_CONVERSION_H
#define SCRAP_CONVERSION_H

#include "SystemTypes.h"

/* Forward declarations */

#include "ScrapTypes.h"
#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Core Conversion Functions
 */

/* Initialize conversion system */
OSErr InitScrapConversion(void);

/* Cleanup conversion system */
void CleanupScrapConversion(void);

/* Convert data between formats */
OSErr ConvertScrapData(Handle sourceData, ResType sourceType,
                      Handle *destData, ResType destType,
                      UInt16 conversionFlags);

/* Check if conversion is possible */
Boolean CanConvertScrapData(ResType sourceType, ResType destType);

/* Get conversion cost estimate */
OSErr GetConversionCost(ResType sourceType, ResType destType,
                       SInt32 sourceSize, SInt32 *conversionTime,
                       SInt32 *memoryNeeded, SInt16 *qualityLoss);

/*
 * Converter Registration and Management
 */

/* Register a data converter */
OSErr RegisterDataConverter(ResType sourceType, ResType destType,
                           ScrapConverterProc converter, void *refCon,
                           UInt16 priority, UInt16 flags);

/* Unregister a data converter */
OSErr UnregisterDataConverter(ResType sourceType, ResType destType,
                             ScrapConverterProc converter);

/* Find best converter for types */
ScrapConverterProc FindBestConverter(ResType sourceType, ResType destType,
                                    void **refCon, UInt16 *priority);

/* Enumerate available converters */
OSErr EnumerateConverters(ResType sourceType, ResType destType,
                         ScrapConverter *converters, SInt16 *count,
                         SInt16 maxConverters);

/*
 * Conversion Chain Management
 */

/* Build conversion chain between formats */
OSErr BuildConversionChain(ResType sourceType, ResType destType,
                          ResType *chainTypes, SInt16 *chainLength,
                          SInt16 maxSteps);

/* Execute conversion chain */
OSErr ExecuteConversionChain(Handle sourceData, const ResType *chainTypes,
                            SInt16 chainLength, Handle *destData);

/* Optimize conversion chain for quality */
OSErr OptimizeConversionChain(const ResType *inputChain, SInt16 inputLength,
                             ResType *optimizedChain, SInt16 *optimizedLength);

/* Get chain conversion quality */
SInt16 GetChainQuality(const ResType *chainTypes, SInt16 chainLength);

/*
 * Text Conversion Functions
 */

/* Convert between text encodings */
OSErr ConvertTextEncoding(Handle sourceText, UInt32 sourceEncoding,
                         Handle *destText, UInt32 destEncoding);

/* Convert plain text to styled text */
OSErr PlainToStyledText(Handle plainText, Handle *styledText,
                       Handle *styleInfo, const TextStyle *defaultStyle);

/* Convert styled text to plain text */
OSErr StyledToPlainText(Handle styledText, Handle styleInfo,
                       Handle *plainText);

/* Convert text to uppercase/lowercase */
OSErr ConvertTextCase(Handle textData, SInt16 caseConversion);

/* Normalize text formatting */
OSErr NormalizeTextFormat(Handle textData, ResType textType,
                         Handle *normalizedText);

/* Convert line endings */
OSErr ConvertTextLineEndings(Handle textData, SInt16 sourceFormat,
                            SInt16 destFormat);

/*
 * Image Conversion Functions
 */

/* Convert between image formats */
OSErr ConvertImageFormat(Handle sourceImage, ResType sourceType,
                        Handle *destImage, ResType destType,
                        const ImageConversionOptions *options);

/* Scale image during conversion */
OSErr ConvertAndScaleImage(Handle sourceImage, ResType sourceType,
                          Handle *destImage, ResType destType,
                          SInt16 newWidth, SInt16 newHeight,
                          SInt16 scalingMode);

/* Convert image color depth */
OSErr ConvertImageColorDepth(Handle imageData, ResType imageType,
                            SInt16 newDepth, Handle *convertedImage);

/* Extract image thumbnail */
OSErr ExtractImageThumbnail(Handle imageData, ResType imageType,
                           SInt16 thumbWidth, SInt16 thumbHeight,
                           Handle *thumbnailData);

/* Convert vector to bitmap */
OSErr VectorToBitmap(Handle vectorData, ResType vectorType,
                    Handle *bitmapData, SInt16 width, SInt16 height,
                    SInt16 resolution);

/* Convert bitmap to vector (if possible) */
OSErr BitmapToVector(Handle bitmapData, ResType bitmapType,
                    Handle *vectorData, ResType vectorType);

/*
 * Sound Conversion Functions
 */

/* Convert between sound formats */
OSErr ConvertSoundFormat(Handle sourceSound, ResType sourceType,
                        Handle *destSound, ResType destType,
                        const SoundConversionOptions *options);

/* Convert sound sample rate */
OSErr ConvertSoundSampleRate(Handle soundData, UInt32 sourceSampleRate,
                            UInt32 destSampleRate, Handle *convertedSound);

/* Convert sound bit depth */
OSErr ConvertSoundBitDepth(Handle soundData, SInt16 sourceBitDepth,
                          SInt16 destBitDepth, Handle *convertedSound);

/* Convert mono to stereo or vice versa */
OSErr ConvertSoundChannels(Handle soundData, SInt16 sourceChannels,
                          SInt16 destChannels, Handle *convertedSound);

/* Compress/decompress sound */
OSErr CompressSoundData(Handle soundData, ResType compressionType,
                       Handle *compressedSound);

/*
 * File Format Conversion Functions
 */

/* Convert file references */
OSErr ConvertFileReference(const FSSpec *sourceFile, ResType sourceType,
                          FSSpec *destFile, ResType destType);

/* Convert file paths between formats */
OSErr ConvertFilePath(const char *sourcePath, SInt16 sourceFormat,
                     char *destPath, SInt16 destFormat, int maxLen);

/* Convert file to data */
OSErr FileToScrapData(const FSSpec *fileSpec, ResType dataType,
                     Handle *fileData);

/* Convert data to file */
OSErr ScrapDataToFile(Handle scrapData, ResType dataType,
                     const FSSpec *fileSpec);

/*
 * Structured Data Conversion
 */

/* Convert between structured data formats */
OSErr ConvertStructuredData(Handle sourceData, ResType sourceType,
                           Handle *destData, ResType destType,
                           const ConversionMapping *fieldMappings,
                           SInt16 mappingCount);

/* Convert record-based data */
OSErr ConvertRecordData(Handle sourceData, const RecordFormat *sourceFormat,
                       Handle *destData, const RecordFormat *destFormat);

/* Convert hierarchical data */
OSErr ConvertHierarchicalData(Handle sourceData, ResType sourceType,
                             Handle *destData, ResType destType,
                             Boolean preserveStructure);

/*
 * Automatic Format Detection and Conversion
 */

/* Auto-detect best conversion target */
ResType AutoDetectBestFormat(Handle sourceData, ResType sourceType,
                            const ResType *candidateTypes, SInt16 typeCount);

/* Auto-convert to most compatible format */
OSErr AutoConvertToCompatible(Handle sourceData, ResType sourceType,
                             Handle *destData, ResType *destType);

/* Convert with quality preferences */
OSErr ConvertWithQualityPrefs(Handle sourceData, ResType sourceType,
                             Handle *destData, ResType destType,
                             SInt16 qualityLevel, Boolean preserveMetadata);

/*
 * Lossy Conversion Management
 */

/* Check if conversion is lossy */
Boolean IsLossyConversion(ResType sourceType, ResType destType);

/* Get quality loss estimate */
SInt16 EstimateQualityLoss(ResType sourceType, ResType destType,
                           Handle sourceData);

/* Convert with quality control */
OSErr ConvertWithQualityControl(Handle sourceData, ResType sourceType,
                               Handle *destData, ResType destType,
                               SInt16 maxQualityLoss);

/* Preview conversion result */
OSErr PreviewConversion(Handle sourceData, ResType sourceType,
                       ResType destType, Handle *previewData,
                       SInt16 *qualityRating);

/*
 * Metadata Preservation
 */

/* Extract metadata from data */
OSErr ExtractDataMetadata(Handle sourceData, ResType dataType,
                         Handle *metadataHandle);

/* Apply metadata to converted data */
OSErr ApplyDataMetadata(Handle destData, ResType dataType,
                       Handle metadataHandle);

/* Convert metadata between formats */
OSErr ConvertMetadata(Handle sourceMetadata, ResType sourceType,
                     Handle *destMetadata, ResType destType);

/*
 * Conversion Options and Configuration
 */

/* Set global conversion preferences */
OSErr SetConversionPreferences(const ConversionPrefs *prefs);

/* Get current conversion preferences */
OSErr GetConversionPreferences(ConversionPrefs *prefs);

/* Set converter-specific options */
OSErr SetConverterOptions(ResType sourceType, ResType destType,
                         const void *options, SInt32 optionsSize);

/* Get converter capabilities */
OSErr GetConverterCapabilities(ResType sourceType, ResType destType,
                              ConverterCapabilities *capabilities);

/*
 * Data Structures for Conversion Options
 */

/*
 * Conversion Constants
 */

/* Conversion flags */
#define CONVERT_FLAG_LOSSY_OK       0x0001  /* Allow lossy conversion */
#define CONVERT_FLAG_PRESERVE_META  0x0002  /* Preserve metadata */
#define CONVERT_FLAG_FAST_MODE      0x0004  /* Fast/low quality mode */
#define CONVERT_FLAG_HIGH_QUALITY   0x0008  /* High quality mode */

#include "SystemTypes.h"
#define CONVERT_FLAG_NO_CHAIN       0x0010  /* Disable chain conversion */
#define CONVERT_FLAG_ASYNC          0x0020  /* Asynchronous conversion */

/* Quality levels */
#define QUALITY_DRAFT               25      /* Draft quality */
#define QUALITY_NORMAL              50      /* Normal quality */
#define QUALITY_HIGH                75      /* High quality */

#include "SystemTypes.h"
#define QUALITY_MAXIMUM             100     /* Maximum quality */

/* Case conversion types */
#define CASE_UPPER                  1       /* Convert to uppercase */
#define CASE_LOWER                  2       /* Convert to lowercase */
#define CASE_TITLE                  3       /* Convert to title case */
#define CASE_SENTENCE               4       /* Convert to sentence case */

/* Scaling modes */
#define SCALE_NEAREST_NEIGHBOR      0       /* Nearest neighbor */
#define SCALE_BILINEAR              1       /* Bilinear interpolation */
#define SCALE_BICUBIC               2       /* Bicubic interpolation */
#define SCALE_LANCZOS               3       /* Lanczos resampling */

#ifdef __cplusplus
}
#endif

#endif /* SCRAP_CONVERSION_H */
