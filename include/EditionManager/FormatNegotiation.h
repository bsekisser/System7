/*
 * FormatNegotiation.h
 *
 * Format Negotiation and Conversion API for Edition Manager
 * Handles data format conversion, negotiation, and compatibility
 */

#ifndef __FORMAT_NEGOTIATION_H__
#define __FORMAT_NEGOTIATION_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Format Converter Function Type
 */

/*
 * Format Quality Levels
 */

/*
 * Format Conversion Information
 */

/*
 * Format Negotiation Result
 */

/*
 * Core Format Negotiation Functions
 */

/* Initialize format negotiation system */
OSErr InitializeFormatNegotiation(void);

/* Clean up format negotiation system */
void CleanupFormatNegotiation(void);

/* Register a format converter */
OSErr RegisterFormatConverter(FormatType fromFormat,
                             FormatType toFormat,
                             FormatConverterProc converter,
                             SInt32 priority);

/* Unregister a format converter */
OSErr UnregisterFormatConverter(FormatType fromFormat, FormatType toFormat);

/* Convert data between formats */
OSErr ConvertFormat(FormatType fromFormat,
                   FormatType toFormat,
                   const void* inputData,
                   Size inputSize,
                   void** outputData,
                   Size* outputSize);

/*
 * Format Negotiation
 */

/* Negotiate best format between publisher and subscriber */
OSErr NegotiateBestFormat(const FormatType* publisherFormats,
                         SInt32 publisherCount,
                         const FormatType* subscriberFormats,
                         SInt32 subscriberCount,
                         FormatType* bestFormat,
                         float* compatibility);

/* Get detailed negotiation result */
OSErr NegotiateFormats(const FormatType* publisherFormats,
                      SInt32 publisherCount,
                      const FormatType* subscriberFormats,
                      SInt32 subscriberCount,
                      FormatNegotiationResult* result);

/* Find optimal format for multiple subscribers */
OSErr NegotiateForMultipleSubscribers(const FormatType* publisherFormats,
                                     SInt32 publisherCount,
                                     const FormatType** subscriberFormats,
                                     const SInt32* subscriberCounts,
                                     SInt32 subscriberCount,
                                     FormatType* optimalFormat);

/*
 * Format Discovery and Capabilities
 */

/* Get formats that can be converted to target format */
OSErr GetSupportedFormats(FormatType targetFormat,
                         FormatType** supportedFormats,
                         SInt32* formatCount);

/* Get formats that source format can be converted to */
OSErr GetConvertibleFormats(FormatType sourceFormat,
                           FormatType** convertibleFormats,
                           SInt32* formatCount);

/* Check if conversion is possible */
OSErr CanConvertFormat(FormatType fromFormat,
                      FormatType toFormat,
                      Boolean* canConvert);

/* Get conversion information */
OSErr GetConversionInfo(FormatType fromFormat,
                       FormatType toFormat,
                       FormatConversionInfo* info);

/* Get list of all registered converters */
OSErr GetAllConverters(FormatConversionInfo** converters,
                      SInt32* converterCount);

/*
 * Format Compatibility
 */

/* Set compatibility between two formats */
OSErr SetFormatCompatibility(FormatType format1,
                           FormatType format2,
                           float compatibility,
                           UInt32 conversionCost);

/* Get compatibility between two formats */
OSErr GetFormatCompatibility(FormatType format1,
                           FormatType format2,
                           float* compatibility);

/* Build compatibility matrix for formats */
OSErr BuildCompatibilityMatrix(const FormatType* formats,
                              SInt32 formatCount,
                              float** matrix);

/*
 * Advanced Format Conversion
 */

/* Multi-step format conversion */
OSErr ConvertFormatWithPath(FormatType fromFormat,
                           FormatType toFormat,
                           const void* inputData,
                           Size inputSize,
                           void** outputData,
                           Size* outputSize,
                           FormatType* conversionPath,
                           SInt32* pathLength);

/* Batch format conversion */
OSErr ConvertMultipleFormats(const FormatType* fromFormats,
                            const FormatType* toFormats,
                            void** inputData,
                            Size* inputSizes,
                            SInt32 formatCount,
                            void*** outputData,
                            Size** outputSizes);

/* Conditional format conversion */

OSErr ConvertFormatWithFilter(FormatType fromFormat,
                             FormatType toFormat,
                             const void* inputData,
                             Size inputSize,
                             void** outputData,
                             Size* outputSize,
                             ConversionFilterProc filter,
                             void* userData);

/*
 * Format Validation and Quality
 */

/* Validate format data */
OSErr ValidateFormatData(FormatType format,
                        const void* data,
                        Size dataSize,
                        Boolean* isValid);

/* Get format data quality metrics */

OSErr GetFormatQuality(FormatType format,
                      const void* data,
                      Size dataSize,
                      FormatQualityMetrics* metrics);

/* Estimate conversion quality */
OSErr EstimateConversionQuality(FormatType fromFormat,
                               FormatType toFormat,
                               const void* inputData,
                               Size inputSize,
                               FormatQuality* estimatedQuality);

/*
 * Format Metadata and Description
 */

/* Format description structure */

/* Register format description */
OSErr RegisterFormatDescription(const FormatDescription* description);

/* Get format description */
OSErr GetFormatDescription(FormatType format, FormatDescription* description);

/* Get all format descriptions */
OSErr GetAllFormatDescriptions(FormatDescription** descriptions,
                              SInt32* descriptionCount);

/*
 * Converter Management
 */

/* Converter priority levels */

/* Converter flags */

/* Set converter priority */
OSErr SetConverterPriority(FormatType fromFormat,
                          FormatType toFormat,
                          SInt32 priority);

/* Get converter priority */
OSErr GetConverterPriority(FormatType fromFormat,
                          FormatType toFormat,
                          SInt32* priority);

/* Enable/disable converter */
OSErr SetConverterEnabled(FormatType fromFormat,
                         FormatType toFormat,
                         Boolean enabled);

/* Check if converter is enabled */
OSErr IsConverterEnabled(FormatType fromFormat,
                        FormatType toFormat,
                        Boolean* enabled);

/*
 * Performance and Optimization
 */

/* Conversion statistics */

/* Get conversion statistics */
OSErr GetConversionStatistics(FormatType fromFormat,
                             FormatType toFormat,
                             ConversionStatistics* stats);

/* Reset conversion statistics */
OSErr ResetConversionStatistics(FormatType fromFormat, FormatType toFormat);

/* Set conversion cache size */
OSErr SetConversionCacheSize(Size cacheSize);

/* Clear conversion cache */
OSErr ClearConversionCache(void);

/*
 * Standard Format Support
 */

/* Standard format types */
#define kFormatTypeText         'TEXT'
#define kFormatTypePICT         'PICT'
#define kFormatTypeSound        'snd '
#define kFormatTypeRTF          'RTF '
#define kFormatTypeHTML         'HTML'
#define kFormatTypeXML          'XML '
#define kFormatTypeJSON         'JSON'
#define kFormatTypePDF          'PDF '
#define kFormatTypeJPEG         'JPEG'
#define kFormatTypePNG          'PNG '
#define kFormatTypeGIF          'GIF '
#define kFormatTypeAIFF         'AIFF'
#define kFormatTypeWAV          'WAV '
#define kFormatTypeMP3          'MP3 '

/* Initialize standard format converters */
OSErr InitializeStandardConverters(void);

/* Register platform-specific converters */
OSErr RegisterPlatformConverters(void);

/*
 * Error Codes
 */

/*
 * Constants
 */
#define kMaxConversionPath 8            /* Maximum conversion steps */
#define kMaxFormatCompatibilities 256   /* Maximum compatibility entries */
#define kDefaultConversionCost 100      /* Default conversion cost */
#define kDefaultCompatibility 0.5f     /* Default compatibility score */

#ifdef __cplusplus
}
#endif

#endif /* __FORMAT_NEGOTIATION_H__ */
