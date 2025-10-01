/*
 * DisplayCalibration.h - Display Calibration and Gamma Correction
 *
 * Display calibration, gamma correction, and monitor profiling for accurate
 * color reproduction and professional color management workflows.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager
 */

#ifndef DISPLAYCALIBRATION_H
#define DISPLAYCALIBRATION_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * DISPLAY CALIBRATION CONSTANTS
 * ================================================================ */

/* Calibration states */

/* Display types */

/* Gamma correction methods */

/* ================================================================
 * DISPLAY CALIBRATION STRUCTURES
 * ================================================================ */

/* Display information */

/* Gamma curve definition */

/* Calibration target */

/* Measurement data */

/* Calibration results */

/* ================================================================
 * DISPLAY ENUMERATION
 * ================================================================ */

/* Get number of displays */
UInt32 CMGetDisplayCount(void);

/* Get display information */
CMError CMGetDisplayInfo(UInt32 displayIndex, CMDisplayInfo *displayInfo);

/* Get primary display */
CMError CMGetPrimaryDisplay(UInt32 *displayIndex);

/* Set primary display */
CMError CMSetPrimaryDisplay(UInt32 displayIndex);

/* Get display by name */
CMError CMGetDisplayByName(const char *name, UInt32 *displayIndex);

/* ================================================================
 * DISPLAY CALIBRATION
 * ================================================================ */

/* Start display calibration */
CMError CMStartDisplayCalibration(UInt32 displayIndex, const CMCalibrationTarget *target);

/* Stop display calibration */
CMError CMStopDisplayCalibration(UInt32 displayIndex);

/* Get calibration state */
CMError CMGetCalibrationState(UInt32 displayIndex, CMCalibrationState *state);

/* Add measurement */
CMError CMAddMeasurement(UInt32 displayIndex, const CMMeasurementData *measurement);

/* Complete calibration */
CMError CMCompleteCalibration(UInt32 displayIndex, CMCalibrationResults *results);

/* Apply calibration */
CMError CMApplyCalibration(UInt32 displayIndex, const CMCalibrationResults *calibration);

/* Remove calibration */
CMError CMRemoveCalibration(UInt32 displayIndex);

/* ================================================================
 * GAMMA CORRECTION
 * ================================================================ */

/* Create gamma curve */
CMError CMCreateGammaCurve(CMGammaMethod method, float gamma, UInt32 lutSize, CMGammaCurve *curve);

/* Apply gamma correction */
CMError CMApplyGammaCorrection(UInt32 displayIndex, const CMGammaCurve *curve);

/* Get current gamma curves */
CMError CMGetDisplayGammaCurves(UInt32 displayIndex, CMGammaCurve *redCurve,
                               CMGammaCurve *greenCurve, CMGammaCurve *blueCurve);

/* Reset gamma curves */
CMError CMResetDisplayGamma(UInt32 displayIndex);

/* Validate gamma curve */
Boolean CMValidateGammaCurve(const CMGammaCurve *curve);

/* ================================================================
 * WHITE POINT ADJUSTMENT
 * ================================================================ */

/* Set display white point */
CMError CMSetDisplayWhitePoint(UInt32 displayIndex, const CMXYZColor *whitePoint);

/* Get display white point */
CMError CMGetDisplayWhitePoint(UInt32 displayIndex, CMXYZColor *whitePoint);

/* Set color temperature */
CMError CMSetDisplayColorTemperature(UInt32 displayIndex, UInt16 temperature);

/* Get color temperature */
CMError CMGetDisplayColorTemperature(UInt32 displayIndex, UInt16 *temperature);

/* Adjust display brightness */
CMError CMSetDisplayBrightness(UInt32 displayIndex, float brightness);

/* Get display brightness */
CMError CMGetDisplayBrightness(UInt32 displayIndex, float *brightness);

/* ================================================================
 * MEASUREMENT INTEGRATION
 * ================================================================ */

/* Colorimeter interface */

/* Enumerate colorimeters */
CMError CMEnumerateColorimeters(CMColorimeter *colorimeters, UInt32 *count);

/* Connect to colorimeter */
CMError CMConnectColorimeter(UInt32 colorimeterIndex);

/* Disconnect colorimeter */
CMError CMDisconnectColorimeter(UInt32 colorimeterIndex);

/* Measure color patch */
CMError CMeasureColorPatch(UInt32 colorimeterIndex, const CMRGBColor *rgb,
                          CMMeasurementData *measurement);

/* Calibrate colorimeter */
CMError CMCalibrateColorimeter(UInt32 colorimeterIndex);

/* ================================================================
 * DISPLAY PROFILING
 * ================================================================ */

/* Create display profile */
CMError CMCreateDisplayProfile(UInt32 displayIndex, const CMCalibrationResults *calibration,
                              CMProfileRef *profile);

/* Install display profile */
CMError CMInstallDisplayProfile(UInt32 displayIndex, CMProfileRef profile);

/* Get current display profile */
CMError CMGetDisplayProfile(UInt32 displayIndex, CMProfileRef *profile);

/* Remove display profile */
CMError CMRemoveDisplayProfile(UInt32 displayIndex);

/* ================================================================
 * CALIBRATION VALIDATION
 * ================================================================ */

/* Validate display calibration */
CMError CMValidateDisplayCalibration(UInt32 displayIndex, const CMRGBColor *testColors,
                                    UInt32 testCount, float *averageDeltaE, float *maxDeltaE);

/* Generate test patterns */
CMError CMGenerateTestPatterns(CMRGBColor *testColors, UInt32 *count, UInt32 maxColors);

/* Analyze calibration quality */
CMError CMAnalyzeCalibrationQuality(const CMCalibrationResults *calibration,
                                   float *uniformity, float *accuracy, float *repeatability);

/* ================================================================
 * AMBIENT LIGHT COMPENSATION
 * ================================================================ */

/* Ambient light sensor */

/* Measure ambient light */
CMError CMMeasureAmbientLight(CMAmbientLight *ambientLight);

/* Apply ambient compensation */
CMError CMApplyAmbientCompensation(UInt32 displayIndex, const CMAmbientLight *ambient);

/* Set ambient compensation mode */
CMError CMSetAmbientCompensationMode(UInt32 displayIndex, Boolean enabled);

/* ================================================================
 * HDR DISPLAY SUPPORT
 * ================================================================ */

/* HDR metadata */

/* Configure HDR display */
CMError CMConfigureHDRDisplay(UInt32 displayIndex, const CMHDRMetadata *hdrMetadata);

/* Get HDR capabilities */
CMError CMGetHDRCapabilities(UInt32 displayIndex, CMHDRMetadata *capabilities);

/* Set HDR tone mapping */
CMError CMSetHDRToneMapping(UInt32 displayIndex, float gamma, float maxLuminance);

/* ================================================================
 * CALIBRATION PERSISTENCE
 * ================================================================ */

/* Save calibration to file */
CMError CMSaveCalibrationToFile(const CMCalibrationResults *calibration, const char *filename);

/* Load calibration from file */
CMError CMLoadCalibrationFromFile(const char *filename, CMCalibrationResults *calibration);

/* Get calibration directory */
CMError CMGetCalibrationDirectory(char *directory, UInt32 maxLength);

/* Set calibration directory */
CMError CMSetCalibrationDirectory(const char *directory);

/* ================================================================
 * AUTOMATIC CALIBRATION
 * ================================================================ */

/* Automatic calibration configuration */

/* Start automatic calibration */
CMError CMStartAutoCalibration(UInt32 displayIndex, UInt32 colorimeterIndex,
                              const CMCalibrationTarget *target,
                              const CMAutoCalibrationConfig *config);

/* Get auto calibration progress */
CMError CMGetAutoCalibrationProgress(UInt32 displayIndex, float *progress, char *status);

/* ================================================================
 * DISPLAY UNIFORMITY
 * ================================================================ */

/* Uniformity measurement */

/* Measure display uniformity */
CMError CMMeasureDisplayUniformity(UInt32 displayIndex, UInt32 colorimeterIndex,
                                  UInt32 gridWidth, UInt32 gridHeight,
                                  CMUniformityResults *results);

/* Apply uniformity correction */
CMError CMApplyUniformityCorrection(UInt32 displayIndex, const CMUniformityResults *uniformity);

/* Dispose uniformity results */
void CMDisposeUniformityResults(CMUniformityResults *results);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAYCALIBRATION_H */