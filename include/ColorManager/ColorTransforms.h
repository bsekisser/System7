/*
 * ColorTransforms.h - Color Space Transformations
 *
 * Advanced color space transformations, lookup tables, and optimization
 * for high-performance color processing in professional workflows.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager
 */

#ifndef COLORTRANSFORMS_H
#define COLORTRANSFORMS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * TRANSFORM CONSTANTS
 * ================================================================ */

/* Transform optimization levels */

/* Interpolation methods */

/* Cache levels */

/* ================================================================
 * TRANSFORM STRUCTURES
 * ================================================================ */

/* Transform configuration */

/* Transform statistics */

/* Lookup table descriptor */

/* ================================================================
 * TRANSFORM SYSTEM MANAGEMENT
 * ================================================================ */

/* Initialize transform system */
CMError CMInitTransformSystem(void);

/* Shutdown transform system */
void CMShutdownTransformSystem(void);

/* Set global transform configuration */
CMError CMSetGlobalTransformConfig(const CMTransformConfig *config);

/* Get global transform configuration */
CMError CMGetGlobalTransformConfig(CMTransformConfig *config);

/* Get transform capabilities */
CMError CMGetTransformCapabilities(UInt32 *capabilities);

/* ================================================================
 * HIGH-LEVEL TRANSFORM OPERATIONS
 * ================================================================ */

/* Transform single color with caching */
CMError CMTransformColorCached(CMTransformRef transform, const CMColor *input, CMColor *output);

/* Transform color array with optimization */
CMError CMTransformColorArray(CMTransformRef transform, const CMColor *input,
                             CMColor *output, UInt32 count);

/* Transform image data */
CMError CMTransformImageData(CMTransformRef transform, const void *inputData,
                            void *outputData, UInt32 width, UInt32 height,
                            UInt32 inputStride, UInt32 outputStride,
                            CMColorSpace inputFormat, CMColorSpace outputFormat);

/* Transform with region of interest */
CMError CMTransformImageRegion(CMTransformRef transform, const void *inputData,
                              void *outputData, UInt32 imageWidth, UInt32 imageHeight,
                              UInt32 roiX, UInt32 roiY, UInt32 roiWidth, UInt32 roiHeight,
                              UInt32 inputStride, UInt32 outputStride,
                              CMColorSpace inputFormat, CMColorSpace outputFormat);

/* ================================================================
 * LOOKUP TABLE OPERATIONS
 * ================================================================ */

/* Create 3D lookup table */
CMError CMCreate3DLUT(CMTransformRef transform, UInt32 gridPoints,
                     UInt32 precision, CMLUTDescriptor **lutDesc);

/* Create 1D lookup table */
CMError CMCreate1DLUT(CMTransformRef transform, UInt32 entries,
                     UInt32 precision, CMLUTDescriptor **lutDesc);

/* Load LUT from file */
CMError CMLoadLUTFromFile(const char *filename, CMLUTDescriptor **lutDesc);

/* Save LUT to file */
CMError CMSaveLUTToFile(const CMLUTDescriptor *lutDesc, const char *filename);

/* Dispose LUT descriptor */
void CMDisposeLUTDescriptor(CMLUTDescriptor *lutDesc);

/* Apply LUT to color */
CMError CMApplyLUTToColor(const CMLUTDescriptor *lutDesc, const CMColor *input, CMColor *output);

/* Apply LUT to color array */
CMError CMApplyLUTToColorArray(const CMLUTDescriptor *lutDesc, const CMColor *input,
                              CMColor *output, UInt32 count);

/* Optimize LUT for hardware */
CMError CMOptimizeLUTForHardware(CMLUTDescriptor *lutDesc);

/* ================================================================
 * MATRIX TRANSFORMS
 * ================================================================ */

/* Create matrix transform */
CMError CMCreateMatrixTransform(const CMColorMatrix *matrix, CMTransformRef *transform);

/* Apply matrix to color */
CMError CMApplyMatrixToColor(const CMColorMatrix *matrix, const CMXYZColor *input, CMXYZColor *output);

/* Apply matrix to color array */
CMError CMApplyMatrixToColorArray(const CMColorMatrix *matrix, const CMXYZColor *input,
                                 CMXYZColor *output, UInt32 count);

/* Concatenate matrices */
CMError CMConcatenateMatrices(const CMColorMatrix *m1, const CMColorMatrix *m2, CMColorMatrix *result);

/* Invert matrix */
CMError CMInvertMatrix(const CMColorMatrix *input, CMColorMatrix *output);

/* ================================================================
 * CURVE TRANSFORMS
 * ================================================================ */

/* Curve descriptor */

/* Create curve transform */
CMError CMCreateCurveTransform(const CMCurveDescriptor *curves, UInt32 channelCount,
                              CMTransformRef *transform);

/* Apply curve to value */
UInt16 CMApplyCurveToValue(const CMCurveDescriptor *curve, UInt16 input);

/* Apply curves to color */
CMError CMApplyCurvesToColor(const CMCurveDescriptor *curves, UInt32 channelCount,
                            const CMColor *input, CMColor *output);

/* Create gamma curve */
CMError CMCreateGammaCurveDescriptor(float gamma, UInt32 points, CMCurveDescriptor **curveDesc);

/* Dispose curve descriptor */
void CMDisposeCurveDescriptor(CMCurveDescriptor *curveDesc);

/* ================================================================
 * TRANSFORM OPTIMIZATION
 * ================================================================ */

/* Optimize transform for repeated use */
CMError CMOptimizeTransform(CMTransformRef transform);

/* Create optimized transform chain */
CMError CMCreateOptimizedTransformChain(CMTransformRef *transforms, UInt32 count,
                                       CMTransformRef *optimizedTransform);

/* Analyze transform performance */
CMError CMAnalyzeTransformPerformance(CMTransformRef transform, UInt32 testColors,
                                     CMTransformStatistics *stats);

/* Benchmark transform */
CMError CMBenchmarkTransform(CMTransformRef transform, UInt32 iterations,
                           UInt32 colorCount, float *colorsPerSecond);

/* ================================================================
 * MULTITHREADED TRANSFORMS
 * ================================================================ */

/* Transform color array with multiple threads */
CMError CMTransformColorArrayMT(CMTransformRef transform, const CMColor *input,
                               CMColor *output, UInt32 count, UInt32 threadCount);

/* Transform image with multiple threads */
CMError CMTransformImageDataMT(CMTransformRef transform, const void *inputData,
                              void *outputData, UInt32 width, UInt32 height,
                              UInt32 inputStride, UInt32 outputStride,
                              CMColorSpace inputFormat, CMColorSpace outputFormat,
                              UInt32 threadCount);

/* Set thread pool size */
CMError CMSetTransformThreadPoolSize(UInt32 threadCount);

/* Get optimal thread count */
UInt32 CMGetOptimalThreadCount(void);

/* ================================================================
 * GPU ACCELERATION
 * ================================================================ */

/* GPU transform context */

/* Initialize GPU acceleration */
CMError CMInitializeGPUAcceleration(CMGPUContextRef *context);

/* Create GPU transform */
CMError CMCreateGPUTransform(CMGPUContextRef context, CMTransformRef cpuTransform,
                           CMTransformRef *gpuTransform);

/* Upload LUT to GPU */
CMError CMUploadLUTToGPU(CMGPUContextRef context, const CMLUTDescriptor *lutDesc);

/* Transform on GPU */
CMError CMTransformOnGPU(CMTransformRef gpuTransform, const void *inputData,
                        void *outputData, UInt32 pixelCount);

/* Synchronize GPU operations */
CMError CMSynchronizeGPU(CMGPUContextRef context);

/* Cleanup GPU context */
void CMCleanupGPUAcceleration(CMGPUContextRef context);

/* ================================================================
 * ADVANCED INTERPOLATION
 * ================================================================ */

/* Tetrahedral interpolation */
CMError CMTetrahedralInterpolation(const CMLUTDescriptor *lutDesc, const float *input,
                                  float *output);

/* Trilinear interpolation */
CMError CMTrilinearInterpolation(const CMLUTDescriptor *lutDesc, const float *input,
                                float *output);

/* Cubic interpolation */
CMError CMCubicInterpolation(const CMLUTDescriptor *lutDesc, const float *input,
                           float *output);

/* Adaptive interpolation (chooses best method) */
CMError CMAdaptiveInterpolation(const CMLUTDescriptor *lutDesc, const float *input,
                               float *output);

/* ================================================================
 * TRANSFORM VALIDATION
 * ================================================================ */

/* Validate transform accuracy */
CMError CMValidateTransformAccuracy(CMTransformRef transform, const CMColor *testColors,
                                   UInt32 testCount, float *maxDeltaE, float *averageDeltaE);

/* Compare transforms */
CMError CMCompareTransforms(CMTransformRef transform1, CMTransformRef transform2,
                           const CMColor *testColors, UInt32 testCount,
                           float *maxDifference, float *averageDifference);

/* Test transform roundtrip accuracy */
CMError CMTestTransformRoundtrip(CMTransformRef forwardTransform, CMTransformRef reverseTransform,
                                const CMColor *testColors, UInt32 testCount,
                                float *maxError, float *averageError);

/* ================================================================
 * TRANSFORM SERIALIZATION
 * ================================================================ */

/* Serialize transform to data */
CMError CMSerializeTransform(CMTransformRef transform, void **data, UInt32 *size);

/* Deserialize transform from data */
CMError CMDeserializeTransform(const void *data, UInt32 size, CMTransformRef *transform);

/* Save transform to file */
CMError CMSaveTransformToFile(CMTransformRef transform, const char *filename);

/* Load transform from file */
CMError CMLoadTransformFromFile(const char *filename, CMTransformRef *transform);

/* ================================================================
 * SPECIALIZED TRANSFORMS
 * ================================================================ */

/* White point adaptation transform */
CMError CMCreateWhitePointAdaptationTransform(const CMXYZColor *srcWhitePoint,
                                             const CMXYZColor *dstWhitePoint,
                                             CMAdaptationMethod method,
                                             CMTransformRef *transform);

/* Gamut mapping transform */
CMError CMCreateGamutMappingTransform(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                     CMGamutMethod method, CMTransformRef *transform);

/* Tone mapping transform */
CMError CMCreateToneMappingTransform(float inputRange, float outputRange,
                                    float gamma, CMTransformRef *transform);

/* Color space conversion transform */
CMError CMCreateColorSpaceTransform(CMColorSpace srcSpace, CMColorSpace dstSpace,
                                   const CMXYZColor *whitePoint, CMTransformRef *transform);

/* ================================================================
 * CACHING SYSTEM
 * ================================================================ */

/* Cache configuration */

/* Initialize transform cache */
CMError CMInitializeTransformCache(const CMCacheConfig *config);

/* Clear transform cache */
CMError CMClearTransformCache(void);

/* Get cache statistics */
CMError CMGetCacheStatistics(UInt32 *entries, UInt32 *memoryUsed, UInt32 *hits, UInt32 *misses);

/* Set cache size */
CMError CMSetCacheSize(UInt32 maxEntries, UInt32 maxMemoryKB);

/* ================================================================
 * PERFORMANCE PROFILING
 * ================================================================ */

/* Profile descriptor */

/* Start performance profiling */
CMError CMStartProfiling(void);

/* Stop performance profiling */
CMError CMStopProfiling(void);

/* Get profiling results */
CMError CMGetProfilingResults(CMProfileDescriptor *profiles, UInt32 *count);

/* Reset profiling data */
CMError CMResetProfilingData(void);

#ifdef __cplusplus
}
#endif

#endif /* COLORTRANSFORMS_H */