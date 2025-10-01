/*
 * ColorSpaces.h - Color Space Definitions and Conversions
 *
 * Color space management and conversion functions for RGB, CMYK, HSV, HSL,
 * XYZ, Lab, and other color spaces used in professional color management.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager
 */

#ifndef COLORSPACES_H
#define COLORSPACES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * COLOR SPACE CONSTANTS
 * ================================================================ */

/* Color temperature constants */
#define kColorTemp2856K     2856    /* Illuminant A */
#define kColorTemp5000K     5000    /* Illuminant D50 */
#define kColorTemp6500K     6500    /* Illuminant D65 */
#define kColorTemp9300K     9300    /* Illuminant D93 */

/* Gamma values */
#define kGamma18            1.8f    /* Mac gamma */
#define kGamma22            2.2f    /* PC/sRGB gamma */
#define kGamma24            2.4f    /* Adobe RGB gamma */

/* Color precision constants */
#define kColorPrecision8Bit     8
#define kColorPrecision16Bit    16
#define kColorPrecision32Bit    32

/* ================================================================
 * COLOR SPACE STRUCTURES
 * ================================================================ */

/* Extended RGB color with alpha */

/* Gray color */

/* YIQ color space */

/* YUV color space */

/* YCbCr color space */

/* LUV color space */

/* Yxy color space */

/* HiFi color space (6+ channels) */

/* Named color */

/* Color space conversion matrix */

/* Illuminant data */

/* ================================================================
 * COLOR SPACE INITIALIZATION
 * ================================================================ */

/* Initialize color space system */
CMError CMInitColorSpaces(void);

/* Get supported color spaces */
CMError CMGetSupportedColorSpaces(CMColorSpace *spaces, UInt32 *count);

/* Check if color space is supported */
Boolean CMIsColorSpaceSupported(CMColorSpace space);

/* Get color space info */
CMError CMGetColorSpaceInfo(CMColorSpace space, char *name, UInt32 *channels,
                           UInt32 *precision);

/* ================================================================
 * RGB COLOR SPACE CONVERSIONS
 * ================================================================ */

/* RGB to other color spaces */
CMError CMConvertRGBToHSV(const CMRGBColor *rgb, CMHSVColor *hsv);
CMError CMConvertRGBToHSL(const CMRGBColor *rgb, CMHLSColor *hsl);
CMError CMConvertRGBToCMYK(const CMRGBColor *rgb, CMCMYKColor *cmyk);
CMError CMConvertRGBToGray(const CMRGBColor *rgb, CMGrayColor *gray);
CMError CMConvertRGBToYIQ(const CMRGBColor *rgb, CMYIQColor *yiq);
CMError CMConvertRGBToYUV(const CMRGBColor *rgb, CMYUVColor *yuv);
CMError CMConvertRGBToYCbCr(const CMRGBColor *rgb, CMYCbCrColor *ycbcr);

/* Other color spaces to RGB */
CMError CMConvertHSVToRGB(const CMHSVColor *hsv, CMRGBColor *rgb);
CMError CMConvertHSLToRGB(const CMHLSColor *hsl, CMRGBColor *rgb);
CMError CMConvertCMYKToRGB(const CMCMYKColor *cmyk, CMRGBColor *rgb);
CMError CMConvertGrayToRGB(const CMGrayColor *gray, CMRGBColor *rgb);
CMError CMConvertYIQToRGB(const CMYIQColor *yiq, CMRGBColor *rgb);
CMError CMConvertYUVToRGB(const CMYUVColor *yuv, CMRGBColor *rgb);
CMError CMConvertYCbCrToRGB(const CMYCbCrColor *ycbcr, CMRGBColor *rgb);

/* ================================================================
 * XYZ COLOR SPACE CONVERSIONS
 * ================================================================ */

/* XYZ to other color spaces */
CMError CMConvertXYZToRGB(const CMXYZColor *xyz, CMRGBColor *rgb,
                         const CMColorMatrix *matrix);
CMError CMConvertXYZToLUV(const CMXYZColor *xyz, CMLUVColor *luv,
                         const CMXYZColor *whitePoint);
CMError CMConvertXYZToYxy(const CMXYZColor *xyz, CMYxyColor *yxy);

/* Other color spaces to XYZ */
CMError CMConvertRGBToXYZ(const CMRGBColor *rgb, CMXYZColor *xyz,
                         const CMColorMatrix *matrix);
CMError CMConvertLUVToXYZ(const CMLUVColor *luv, CMXYZColor *xyz,
                         const CMXYZColor *whitePoint);
CMError CMConvertYxyToXYZ(const CMYxyColor *yxy, CMXYZColor *xyz);

/* ================================================================
 * COLOR SPACE UTILITIES
 * ================================================================ */

/* Color space validation */
Boolean CMIsValidRGBColor(const CMRGBColor *color);
Boolean CMIsValidCMYKColor(const CMCMYKColor *color);
Boolean CMIsValidHSVColor(const CMHSVColor *color);
Boolean CMIsValidXYZColor(const CMXYZColor *color);
Boolean CMIsValidLabColor(const CMLABColor *color);

/* Color clamping */
void CMClampRGBColor(CMRGBColor *color);
void CMClampCMYKColor(CMCMYKColor *color);
void CMClampHSVColor(CMHSVColor *color);

/* Color interpolation */
CMError CMInterpolateRGB(const CMRGBColor *color1, const CMRGBColor *color2,
                        float t, CMRGBColor *result);
CMError CMInterpolateHSV(const CMHSVColor *color1, const CMHSVColor *color2,
                        float t, CMHSVColor *result);
CMError CMInterpolateXYZ(const CMXYZColor *color1, const CMXYZColor *color2,
                        float t, CMXYZColor *result);

/* ================================================================
 * COLOR MATRICES AND TRANSFORMS
 * ================================================================ */

/* Get standard color matrices */
CMError CMGetSRGBToXYZMatrix(CMColorMatrix *matrix);
CMError CMGetAdobeRGBToXYZMatrix(CMColorMatrix *matrix);
CMError CMGetProPhotoRGBToXYZMatrix(CMColorMatrix *matrix);

/* Matrix operations */
CMError CMMultiplyColorMatrix(const CMColorMatrix *m1, const CMColorMatrix *m2,
                             CMColorMatrix *result);
CMError CMInvertColorMatrix(const CMColorMatrix *matrix, CMColorMatrix *inverse);
CMError CMApplyColorMatrix(const CMColorMatrix *matrix, const CMXYZColor *input,
                          CMXYZColor *output);

/* Create custom matrices */
CMError CMCreateColorMatrix(const CMXYZColor *redPrimary,
                           const CMXYZColor *greenPrimary,
                           const CMXYZColor *bluePrimary,
                           const CMXYZColor *whitePoint,
                           CMColorMatrix *matrix);

/* ================================================================
 * ILLUMINANTS AND WHITE POINTS
 * ================================================================ */

/* Get standard illuminants */
CMError CMGetStandardIlluminant(UInt16 temperature, CMIlluminant *illuminant);
CMError CMGetIlluminantD50(CMXYZColor *whitePoint);
CMError CMGetIlluminantD65(CMXYZColor *whitePoint);
CMError CMGetIlluminantA(CMXYZColor *whitePoint);

/* Color temperature conversions */
CMError CMColorTemperatureToXYZ(UInt16 temperature, CMXYZColor *xyz);
CMError CMXYZToColorTemperature(const CMXYZColor *xyz, UInt16 *temperature);

/* Chromatic adaptation */
CMError CMChromaticAdaptation(const CMXYZColor *color,
                             const CMXYZColor *srcWhitePoint,
                             const CMXYZColor *dstWhitePoint,
                             CMXYZColor *adaptedColor);

/* ================================================================
 * GAMMA CORRECTION
 * ================================================================ */

/* Gamma correction functions */
UInt16 CMApplyGamma(UInt16 value, float gamma);
UInt16 CMRemoveGamma(UInt16 value, float gamma);

/* Apply gamma to colors */
CMError CMApplyGammaToRGB(CMRGBColor *color, float gamma);
CMError CMRemoveGammaFromRGB(CMRGBColor *color, float gamma);

/* sRGB gamma functions */
UInt16 CMApplySRGBGamma(UInt16 value);
UInt16 CMRemoveSRGBGamma(UInt16 value);

/* ================================================================
 * COLOR DIFFERENCE CALCULATIONS
 * ================================================================ */

/* Calculate color differences */
float CMCalculateDeltaE76(const CMLABColor *lab1, const CMLABColor *lab2);
float CMCalculateDeltaE94(const CMLABColor *lab1, const CMLABColor *lab2);
float CMCalculateDeltaE2000(const CMLABColor *lab1, const CMLABColor *lab2);

/* RGB color difference */
float CMCalculateRGBDistance(const CMRGBColor *rgb1, const CMRGBColor *rgb2);

/* ================================================================
 * NAMED COLORS
 * ================================================================ */

/* Named color management */
CMError CMRegisterNamedColor(const char *name, const CMColor *color);
CMError CMLookupNamedColor(const char *name, CMColor *color);
CMError CMGetNamedColorList(CMNamedColor *colors, UInt32 *count);

/* Standard color constants */
extern const CMRGBColor kStandardRed;
extern const CMRGBColor kStandardGreen;
extern const CMRGBColor kStandardBlue;
extern const CMRGBColor kStandardCyan;
extern const CMRGBColor kStandardMagenta;
extern const CMRGBColor kStandardYellow;
extern const CMRGBColor kStandardBlack;
extern const CMRGBColor kStandardWhite;

/* ================================================================
 * HIGH PRECISION COLOR OPERATIONS
 * ================================================================ */

/* Extended precision color types */

/* High precision conversions */
CMError CMConvertFloatRGBToXYZ(const CMFloatRGBAColor *rgb, CMFloatXYZColor *xyz,
                              const CMColorMatrix *matrix);
CMError CMConvertFloatXYZToLab(const CMFloatXYZColor *xyz, CMFloatLABColor *lab,
                              const CMFloatXYZColor *whitePoint);

/* Precision conversion helpers */
CMError CMConvert16BitToFloat(const CMRGBColor *rgb16, CMFloatRGBAColor *rgbFloat);
CMError CMConvertFloatTo16Bit(const CMFloatRGBAColor *rgbFloat, CMRGBColor *rgb16);

#ifdef __cplusplus
}
#endif

#endif /* COLORSPACES_H */