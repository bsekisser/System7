/*
 * ColorMatching.h - Color Matching and Gamut Mapping
 *
 * Professional color matching algorithms, gamut mapping, and color space
 * transformation for accurate color reproduction across devices.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager and ColorSync
 */

#ifndef COLORMATCHING_H
#define COLORMATCHING_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * COLOR MATCHING CONSTANTS
 * ================================================================ */

/* Transform types */

/* Matching quality levels */

/* Gamut mapping methods */

/* Color difference algorithms */

/* ================================================================
 * COLOR TRANSFORM STRUCTURES
 * ================================================================ */

/* Color transform handle */

/* Transform parameters */

/* Gamut checking result */

/* Color matching statistics */

/* ================================================================
 * COLOR MATCHING INITIALIZATION
 * ================================================================ */

/* Initialize color matching system */
CMError CMInitColorMatching(void);

/* Set global matching preferences */
CMError CMSetMatchingPreferences(const CMTransformParams *params);

/* Get global matching preferences */
CMError CMGetMatchingPreferences(CMTransformParams *params);

/* ================================================================
 * COLOR TRANSFORM CREATION
 * ================================================================ */

/* Create color transform */
CMTransformRef CMCreateColorTransform(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                     CMRenderingIntent intent, CMQuality quality);

/* Create multi-profile transform */
CMTransformRef CMCreateMultiProfileTransform(CMProfileRef *profiles, UInt32 count,
                                            CMRenderingIntent intent, CMQuality quality);

/* Create proofing transform */
CMTransformRef CMCreateProofingTransform(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                        CMProfileRef proofProfile, CMRenderingIntent intent,
                                        CMRenderingIntent proofIntent, Boolean gamutCheck);

/* Create named color transform */
CMTransformRef CMCreateNamedColorTransform(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                          const char **colorNames, UInt32 nameCount);

/* Clone transform */
CMTransformRef CMCloneColorTransform(CMTransformRef transform);

/* Dispose transform */
void CMDisposeColorTransform(CMTransformRef transform);

/* ================================================================
 * COLOR TRANSFORM OPERATIONS
 * ================================================================ */

/* Apply transform to single color */
CMError CMApplyTransform(CMTransformRef transform, const CMColor *input, CMColor *output);

/* Apply transform to color array */
CMError CMApplyTransformArray(CMTransformRef transform, const CMColor *input,
                             CMColor *output, UInt32 count);

/* Apply transform with gamut checking */
CMError CMApplyTransformWithGamut(CMTransformRef transform, const CMColor *input,
                                 CMColor *output, CMGamutResult *gamutResult);

/* Get transform info */
CMError CMGetTransformInfo(CMTransformRef transform, CMTransformType *type,
                          CMColorSpace *srcSpace, CMColorSpace *dstSpace);

/* ================================================================
 * GAMUT OPERATIONS
 * ================================================================ */

/* Create gamut boundary */
CMError CMCreateGamutBoundary(CMProfileRef profile, void **gamutData);

/* Check color against gamut */
CMError CMCheckColorGamut(void *gamutData, const CMColor *color, CMGamutResult *result);

/* Map color to gamut */
CMError CMMapColorToGamut(void *gamutData, const CMColor *input, CMColor *output,
                         CMGamutMethod method);

/* Calculate gamut volume */
CMError CMCalculateGamutVolume(void *gamutData, float *volume);

/* Compare gamuts */
CMError CMCompareGamuts(void *gamut1, void *gamut2, float *coverage, float *overlap);

/* Dispose gamut data */
void CMDisposeGamutBoundary(void *gamutData);

/* ================================================================
 * COLOR DIFFERENCE CALCULATIONS
 * ================================================================ */

/* Calculate color difference */
float CMCalculateColorDifference(const CMColor *color1, const CMColor *color2,
                                CMColorDifferenceAlgorithm algorithm,
                                const CMXYZColor *whitePoint);

/* Calculate Delta E with specific algorithm */
float CMCalculateDeltaE(const CMLABColor *lab1, const CMLABColor *lab2,
                       CMColorDifferenceAlgorithm algorithm);

/* Calculate CMC color difference */
float CMCalculateCMCDifference(const CMLABColor *lab1, const CMLABColor *lab2,
                              float lightness, float chroma);

/* Calculate average color difference for array */
CMError CMCalculateAverageColorDifference(const CMColor *colors1, const CMColor *colors2,
                                         UInt32 count, float *averageDeltaE,
                                         CMColorDifferenceAlgorithm algorithm);

/* ================================================================
 * COLOR ADAPTATION
 * ================================================================ */

/* Chromatic adaptation methods */

/* Apply chromatic adaptation */
CMError CMApplyChromaticAdaptation(const CMXYZColor *color,
                                  const CMXYZColor *srcWhitePoint,
                                  const CMXYZColor *dstWhitePoint,
                                  CMAdaptationMethod method,
                                  CMXYZColor *adaptedColor);

/* Create adaptation matrix */
CMError CMCreateAdaptationMatrix(const CMXYZColor *srcWhitePoint,
                                const CMXYZColor *dstWhitePoint,
                                CMAdaptationMethod method,
                                CMColorMatrix *matrix);

/* ================================================================
 * BLACK POINT COMPENSATION
 * ================================================================ */

/* Calculate black point */
CMError CMCalculateBlackPoint(CMProfileRef profile, CMXYZColor *blackPoint);

/* Apply black point compensation */
CMError CMApplyBlackPointCompensation(const CMXYZColor *color,
                                     const CMXYZColor *srcBlackPoint,
                                     const CMXYZColor *dstBlackPoint,
                                     const CMXYZColor *srcWhitePoint,
                                     const CMXYZColor *dstWhitePoint,
                                     CMXYZColor *compensatedColor);

/* ================================================================
 * PERFORMANCE MONITORING
 * ================================================================ */

/* Get matching statistics */
CMError CMGetMatchingStatistics(CMTransformRef transform, CMMatchingStats *stats);

/* Reset matching statistics */
CMError CMResetMatchingStatistics(CMTransformRef transform);

/* Benchmark transform performance */
CMError CMBenchmarkTransform(CMTransformRef transform, UInt32 iterations,
                            UInt32 *microseconds);

/* ================================================================
 * SPECIALIZED TRANSFORMS
 * ================================================================ */

/* Intent-specific color matching */
CMError CMPerformColorMatching(CMProfileRef srcProfile, CMProfileRef dstProfile,
                              CMColor *colors, UInt32 count, CMRenderingIntent intent);

/* Perceptual matching */
CMError CMPerceptualMatching(CMProfileRef srcProfile, CMProfileRef dstProfile,
                           CMColor *colors, UInt32 count);

/* Saturation matching */
CMError CMSaturationMatching(CMProfileRef srcProfile, CMProfileRef dstProfile,
                           CMColor *colors, UInt32 count);

/* Relative colorimetric matching */
CMError CMRelativeColorimetricMatching(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                      CMColor *colors, UInt32 count);

/* Absolute colorimetric matching */
CMError CMAbsoluteColorimetricMatching(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                      CMColor *colors, UInt32 count);

/* ================================================================
 * SOFT PROOFING
 * ================================================================ */

/* Create soft proof */
CMError CMCreateSoftProof(CMProfileRef srcProfile, CMProfileRef displayProfile,
                         CMProfileRef printerProfile, CMRenderingIntent intent,
                         Boolean showOutOfGamut, CMTransformRef *proofTransform);

/* Apply soft proofing */
CMError CMApplySoftProofing(CMTransformRef proofTransform, const CMColor *input,
                           CMColor *output, Boolean *outOfGamut);

/* ================================================================
 * COLOR LOOKUP TABLES
 * ================================================================ */

/* LUT types */

/* Create color LUT */
CMError CMCreateColorLUT(CMProfileRef srcProfile, CMProfileRef dstProfile,
                        CMLUTType lutType, UInt32 gridPoints,
                        void **lutData, UInt32 *lutSize);

/* Apply LUT transform */
CMError CMApplyLUTTransform(const void *lutData, CMLUTType lutType,
                           const CMColor *input, CMColor *output);

/* Interpolate LUT values */
CMError CMInterpolateLUT(const void *lutData, CMLUTType lutType, UInt32 gridPoints,
                        const float *input, float *output);

/* ================================================================
 * ADVANCED COLOR MATCHING
 * ================================================================ */

/* Spectral color matching */
CMError CMSpectralColorMatching(const float *srcSpectral, const float *dstSpectral,
                               UInt32 wavelengths, CMColor *result);

/* Device link profiles */
CMError CMCreateDeviceLink(CMProfileRef srcProfile, CMProfileRef dstProfile,
                          CMRenderingIntent intent, CMProfileRef *deviceLink);

/* Custom rendering intent */
CMError CMCreateCustomRenderingIntent(CMProfileRef srcProfile, CMProfileRef dstProfile,
                                     float perceptualWeight, float saturationWeight,
                                     float colorimetricWeight, CMTransformRef *transform);

#ifdef __cplusplus
}
#endif

#endif /* COLORMATCHING_H */