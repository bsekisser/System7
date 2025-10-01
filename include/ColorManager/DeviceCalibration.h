/*
 * DeviceCalibration.h - Device Color Calibration and Profiling
 *
 * Device calibration and profiling for scanners, printers, cameras, and other
 * color input/output devices for accurate color reproduction.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager
 */

#ifndef DEVICECALIBRATION_H
#define DEVICECALIBRATION_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * DEVICE CALIBRATION CONSTANTS
 * ================================================================ */

/* Device types */

/* Calibration methods */

/* Paper types for printer calibration */

/* Ink types */

/* ================================================================
 * DEVICE CALIBRATION STRUCTURES
 * ================================================================ */

/* Device information */

/* Scanner calibration settings */

/* Printer calibration settings */

/* Camera calibration settings */

/* Test chart definition */

/* Calibration measurement */

/* Device calibration results */

/* ================================================================
 * DEVICE ENUMERATION
 * ================================================================ */

/* Enumerate color devices */
CMError CMEnumerateColorDevices(CMDeviceInfo *devices, UInt32 *count, CMDeviceType deviceType);

/* Get device information */
CMError CMGetDeviceInfo(const char *deviceName, CMDeviceInfo *deviceInfo);

/* Check device color management support */
Boolean CMDeviceSupportsColorManagement(const char *deviceName);

/* Get device capabilities */
CMError CMGetDeviceCapabilities(const char *deviceName, UInt32 *capabilities);

/* ================================================================
 * SCANNER CALIBRATION
 * ================================================================ */

/* Initialize scanner calibration */
CMError CMInitializeScannerCalibration(const char *scannerName, const CMScannerSettings *settings);

/* Scan calibration target */
CMError CMScanCalibrationTarget(const char *scannerName, const CMTestChart *testChart,
                               void *scannedImage, UInt32 *imageSize);

/* Analyze scanned calibration target */
CMError CMAnalyzeScannerCalibration(const void *scannedImage, const CMTestChart *testChart,
                                   CMDeviceCalibrationResults *results);

/* Apply scanner calibration */
CMError CMApplyScannerCalibration(const char *scannerName, const CMDeviceCalibrationResults *calibration);

/* Create scanner profile */
CMError CMCreateScannerProfile(const CMDeviceCalibrationResults *calibration, CMProfileRef *profile);

/* ================================================================
 * PRINTER CALIBRATION
 * ================================================================ */

/* Initialize printer calibration */
CMError CMInitializePrinterCalibration(const char *printerName, const CMPrinterSettings *settings);

/* Print calibration target */
CMError CMPrintCalibrationTarget(const char *printerName, const CMTestChart *testChart);

/* Measure printed calibration target */
CMError CMeasurePrintedTarget(const CMTestChart *testChart, UInt32 colorimeterIndex,
                             CMDeviceCalibrationResults *results);

/* Create printer profile */
CMError CMCreatePrinterProfile(const CMDeviceCalibrationResults *calibration,
                              const CMPrinterSettings *settings, CMProfileRef *profile);

/* Apply printer calibration */
CMError CMApplyPrinterCalibration(const char *printerName, const CMDeviceCalibrationResults *calibration);

/* ================================================================
 * CAMERA CALIBRATION
 * ================================================================ */

/* Initialize camera calibration */
CMError CMInitializeCameraCalibration(const char *cameraName, const CMCameraSettings *settings);

/* Capture calibration target */
CMError CMCaptureCameraTarget(const char *cameraName, const CMTestChart *testChart,
                             void *capturedImage, UInt32 *imageSize);

/* Analyze camera calibration */
CMError CMAnalyzeCameraCalibration(const void *capturedImage, const CMTestChart *testChart,
                                  CMDeviceCalibrationResults *results);

/* Create camera profile */
CMError CMCreateCameraProfile(const CMDeviceCalibrationResults *calibration,
                             const CMCameraSettings *settings, CMProfileRef *profile);

/* Apply camera calibration */
CMError CMApplyCameraCalibration(const char *cameraName, const CMDeviceCalibrationResults *calibration);

/* ================================================================
 * TEST CHART MANAGEMENT
 * ================================================================ */

/* Create test chart */
CMError CMCreateTestChart(const char *name, const char *reference, UInt32 patchCount,
                         CMTestChart **testChart);

/* Load test chart from file */
CMError CMLoadTestChart(const char *filename, CMTestChart **testChart);

/* Save test chart to file */
CMError CMSaveTestChart(const CMTestChart *testChart, const char *filename);

/* Dispose test chart */
void CMDisposeTestChart(CMTestChart *testChart);

/* Generate standard test charts */
CMError CMGenerateIT8Chart(CMTestChart **testChart);
CMError CMGenerateColorCheckerChart(CMTestChart **testChart);
CMError CMGenerateCustomChart(UInt32 patchCount, CMTestChart **testChart);

/* ================================================================
 * LINEARIZATION
 * ================================================================ */

/* Device linearization */
CMError CMLinearizeDevice(const char *deviceName, CMDeviceType deviceType);

/* Create linearization curves */
CMError CMCreateLinearizationCurves(const CMDeviceCalibrationResults *calibration,
                                   UInt16 **redCurve, UInt16 **greenCurve, UInt16 **blueCurve,
                                   UInt32 *curveSize);

/* Apply linearization */
CMError CMApplyLinearization(const char *deviceName, const UInt16 *redCurve,
                           const UInt16 *greenCurve, const UInt16 *blueCurve, UInt32 curveSize);

/* Verify linearization */
CMError CMVerifyLinearization(const char *deviceName, float *linearity, float *maxError);

/* ================================================================
 * QUALITY ASSESSMENT
 * ================================================================ */

/* Assess calibration quality */
CMError CMAssessCalibrationQuality(const CMDeviceCalibrationResults *calibration,
                                  float *accuracy, float *precision, float *consistency);

/* Compare device calibrations */
CMError CMCompareDeviceCalibrations(const CMDeviceCalibrationResults *calibration1,
                                   const CMDeviceCalibrationResults *calibration2,
                                   float *similarity);

/* Validate device profile */
CMError CMValidateDeviceProfile(CMProfileRef profile, const CMTestChart *testChart,
                               float *averageError, float *maxError);

/* ================================================================
 * SOFT PROOFING FOR DEVICES
 * ================================================================ */

/* Create device soft proof */
CMError CMCreateDeviceSoftProof(CMProfileRef inputProfile, CMProfileRef deviceProfile,
                               CMProfileRef displayProfile, CMTransformRef *proofTransform);

/* Preview device output */
CMError CMPreviewDeviceOutput(CMTransformRef proofTransform, const void *inputImage,
                             void *previewImage, UInt32 width, UInt32 height);

/* Check device gamut */
CMError CMCheckDeviceGamut(CMProfileRef deviceProfile, const CMColor *colors,
                          UInt32 colorCount, Boolean *inGamut);

/* ================================================================
 * DEVICE PROFILE MANAGEMENT
 * ================================================================ */

/* Install device profile */
CMError CMInstallDeviceProfile(const char *deviceName, CMProfileRef profile);

/* Get device profile */
CMError CMGetDeviceProfile(const char *deviceName, CMProfileRef *profile);

/* Remove device profile */
CMError CMRemoveDeviceProfile(const char *deviceName);

/* List device profiles */
CMError CMListDeviceProfiles(const char *deviceName, char **profileNames, UInt32 *count);

/* ================================================================
 * BATCH PROCESSING
 * ================================================================ */

/* Batch calibration configuration */

/* Start batch calibration */
CMError CMStartBatchCalibration(const CMBatchCalibrationConfig *config);

/* Get batch calibration progress */
CMError CMGetBatchCalibrationProgress(float *progress, char *currentDevice);

/* Stop batch calibration */
CMError CMStopBatchCalibration(void);

/* ================================================================
 * MAINTENANCE AND VERIFICATION
 * ================================================================ */

/* Schedule device recalibration */
CMError CMScheduleDeviceRecalibration(const char *deviceName, UInt32 intervalDays);

/* Check calibration expiry */
CMError CMCheckCalibrationExpiry(const char *deviceName, Boolean *isExpired, UInt32 *daysRemaining);

/* Verify device consistency */
CMError CMVerifyDeviceConsistency(const char *deviceName, const CMTestChart *testChart,
                                 float *consistency);

/* Generate calibration report */
CMError CMGenerateCalibrationReport(const CMDeviceCalibrationResults *calibration,
                                   const char *filename);

/* ================================================================
 * SPECTRAL MEASUREMENT SUPPORT
 * ================================================================ */

/* Spectral measurement data */

/* Measure spectral data */
CMError CMMeasureSpectralData(UInt32 spectrometerIndex, const CMRGBColor *stimulusColor,
                             CMSpectralMeasurement *spectralData);

/* Convert spectral to XYZ */
CMError CMConvertSpectralToXYZ(const CMSpectralMeasurement *spectralData,
                              const CMXYZColor *illuminant, CMXYZColor *xyz);

/* Calculate metamerism index */
CMError CMCalculateMetamerismIndex(const CMSpectralMeasurement *sample1,
                                  const CMSpectralMeasurement *sample2,
                                  const CMXYZColor *illuminant1,
                                  const CMXYZColor *illuminant2,
                                  float *metamerismIndex);

#ifdef __cplusplus
}
#endif

#endif /* DEVICECALIBRATION_H */