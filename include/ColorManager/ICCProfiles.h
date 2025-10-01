/*
 * ICCProfiles.h - ICC Profile Management
 *
 * ICC profile loading, creation, validation, and management for professional
 * color workflows compatible with ICC v2 and v4 specifications.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager and ColorSync
 */

#ifndef ICCPROFILES_H
#define ICCPROFILES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * ICC PROFILE CONSTANTS
 * ================================================================ */

/* ICC signature constants */
#define kICCProfileSignature    'acsp'
#define kICCVersionMajor        2
#define kICCVersionMinor        0

/* ICC tag signatures */
#define kICCRedColorantTag      'rXYZ'
#define kICCGreenColorantTag    'gXYZ'
#define kICCBlueColorantTag     'bXYZ'
#define kICCWhitePointTag       'wtpt'
#define kICCRedTRCTag           'rTRC'
#define kICCGreenTRCTag         'gTRC'
#define kICCBlueTRCTag          'bTRC'
#define kICCGrayTRCTag          'kTRC'
#define kICCDescriptionTag      'desc'
#define kICCCopyrightTag        'cprt'
#define kICCMediaWhitePointTag  'wtpt'
#define kICCChromaticityTag     'chrm'
#define kICCLuminanceTag        'lumi'
#define kICCMeasurementTag      'meas'
#define kICCTechnologyTag       'tech'
#define kICCViewingConditionsTag 'view'
#define kICCAToB0Tag            'A2B0'
#define kICCAToB1Tag            'A2B1'
#define kICCAToB2Tag            'A2B2'
#define kICCBToA0Tag            'B2A0'
#define kICCBToA1Tag            'B2A1'
#define kICCBToA2Tag            'B2A2'
#define kICCGamutTag            'gamt'
#define kICCPreview0Tag         'pre0'
#define kICCPreview1Tag         'pre1'
#define kICCPreview2Tag         'pre2'
#define kICCNamedColorTag       'ncol'
#define kICCNamedColor2Tag      'ncl2'

/* Type signatures */
#define kICCCurveType           'curv'
#define kICCXYZType             'XYZ '
#define kICCTextType            'text'
#define kICCDescriptionType     'desc'
#define kICCChromaticityType    'chrm'
#define kICCLut8Type            'mft1'
#define kICCLut16Type           'mft2'
#define kICCLutAToBType         'mAB '
#define kICCLutBToAType         'mBA '
#define kICCMeasurementType     'meas'
#define kICCNamedColorType      'ncol'
#define kICCNamedColor2Type     'ncl2'
#define kICCParametricCurveType 'para'
#define kICCSignatureType       'sig '
#define kICCViewingConditionsType 'view'

/* Profile flags */
#define kICCEmbeddedProfile     0x00000001
#define kICCIndependentProfile  0x00000002

/* Device attributes */
#define kICCReflectiveDevice    0x00000000
#define kICCTransparencyDevice  0x00000001
#define kICCGlossyDevice        0x00000000
#define kICCMatteDevice         0x00000002

/* ================================================================
 * ICC PROFILE STRUCTURES
 * ================================================================ */

/* ICC profile header (128 bytes) */

/* Tag table entry */

/* Tag table */

/* XYZ data */

/* Curve data */

/* Text data */

/* Description data */

/* Named color data */

/* ================================================================
 * ICC PROFILE MANAGEMENT
 * ================================================================ */

/* Initialize ICC profile system */
CMError CMInitICCProfiles(void);

/* Create ICC profile structure */
CMError CMCreateICCProfile(CMProfileRef prof, CMProfileClass profileClass,
                          CMColorSpace dataSpace, CMColorSpace pcs);

/* Create default ICC profile */
CMError CMCreateDefaultICCProfile(CMProfileRef prof);

/* Load ICC profile from data */
CMError CMLoadICCProfileFromData(CMProfileRef prof, const void *data, UInt32 size);

/* Save ICC profile to data */
CMError CMSaveICCProfileToData(CMProfileRef prof, void **data, UInt32 *size);

/* Validate ICC profile */
CMError CMValidateICCProfile(const void *data, UInt32 size, Boolean *isValid);

/* ================================================================
 * ICC HEADER MANAGEMENT
 * ================================================================ */

/* Get ICC header */
CMError CMGetICCHeader(CMProfileRef prof, CMICCHeader *header);

/* Set ICC header */
CMError CMSetICCHeader(CMProfileRef prof, const CMICCHeader *header);

/* Update ICC header fields */
CMError CMUpdateICCHeader(CMProfileRef prof);

/* Get profile creation date */
CMError CMGetProfileCreationDate(CMProfileRef prof, void *dateTime);

/* Set profile creation date */
CMError CMSetProfileCreationDate(CMProfileRef prof, const void *dateTime);

/* ================================================================
 * ICC TAG MANAGEMENT
 * ================================================================ */

/* Get tag count */
CMError CMGetICCTagCount(CMProfileRef prof, UInt32 *count);

/* Get tag info by index */
CMError CMGetICCTagInfo(CMProfileRef prof, UInt32 index,
                       UInt32 *signature, UInt32 *size);

/* Get tag data */
CMError CMGetICCTagData(CMProfileRef prof, UInt32 signature,
                       void *data, UInt32 *size);

/* Set tag data */
CMError CMSetICCTagData(CMProfileRef prof, UInt32 signature,
                       const void *data, UInt32 size);

/* Remove tag */
CMError CMRemoveICCTag(CMProfileRef prof, UInt32 signature);

/* Check if tag exists */
Boolean CMICCTagExists(CMProfileRef prof, UInt32 signature);

/* ================================================================
 * STANDARD ICC TAGS
 * ================================================================ */

/* RGB colorant tags */
CMError CMGetRedColorant(CMProfileRef prof, CMXYZColor *xyz);
CMError CMSetRedColorant(CMProfileRef prof, const CMXYZColor *xyz);
CMError CMGetGreenColorant(CMProfileRef prof, CMXYZColor *xyz);
CMError CMSetGreenColorant(CMProfileRef prof, const CMXYZColor *xyz);
CMError CMGetBlueColorant(CMProfileRef prof, CMXYZColor *xyz);
CMError CMSetBlueColorant(CMProfileRef prof, const CMXYZColor *xyz);

/* White point */
CMError CMGetWhitePoint(CMProfileRef prof, CMXYZColor *whitePoint);
CMError CMSetWhitePoint(CMProfileRef prof, const CMXYZColor *whitePoint);

/* Tone reproduction curves */
CMError CMGetRedTRC(CMProfileRef prof, UInt16 *curve, UInt32 *count);
CMError CMSetRedTRC(CMProfileRef prof, const UInt16 *curve, UInt32 count);
CMError CMGetGreenTRC(CMProfileRef prof, UInt16 *curve, UInt32 *count);
CMError CMSetGreenTRC(CMProfileRef prof, const UInt16 *curve, UInt32 count);
CMError CMGetBlueTRC(CMProfileRef prof, UInt16 *curve, UInt32 *count);
CMError CMSetBlueTRC(CMProfileRef prof, const UInt16 *curve, UInt32 count);
CMError CMGetGrayTRC(CMProfileRef prof, UInt16 *curve, UInt32 *count);
CMError CMSetGrayTRC(CMProfileRef prof, const UInt16 *curve, UInt32 count);

/* Profile description */
CMError CMGetProfileDescription(CMProfileRef prof, char *description, UInt32 *size);
CMError CMSetProfileDescription(CMProfileRef prof, const char *description);

/* Copyright */
CMError CMGetProfileCopyright(CMProfileRef prof, char *copyright, UInt32 *size);
CMError CMSetProfileCopyright(CMProfileRef prof, const char *copyright);

/* ================================================================
 * ICC CURVE UTILITIES
 * ================================================================ */

/* Create gamma curve */
CMError CMCreateGammaCurve(float gamma, UInt16 **curve, UInt32 *count);

/* Create linear curve */
CMError CMCreateLinearCurve(UInt16 **curve, UInt32 *count);

/* Create sRGB curve */
CMError CMCreateSRGBCurve(UInt16 **curve, UInt32 *count);

/* Apply curve to value */
UInt16 CMApplyCurve(const UInt16 *curve, UInt32 count, UInt16 input);

/* Invert curve */
CMError CMInvertCurve(const UInt16 *inputCurve, UInt32 inputCount,
                     UInt16 **outputCurve, UInt32 *outputCount);

/* ================================================================
 * ICC PROFILE UTILITIES
 * ================================================================ */

/* Calculate profile MD5 */
CMError CMCalculateProfileMD5(CMProfileRef prof, UInt8 *digest);

/* Get profile size */
CMError CMGetICCProfileSize(CMProfileRef prof, UInt32 *size);

/* Clone ICC profile data */
CMError CMCloneICCProfileData(CMProfileRef srcProf, CMProfileRef dstProf);

/* Compare ICC profiles */
Boolean CMCompareICCProfiles(CMProfileRef prof1, CMProfileRef prof2);

/* ================================================================
 * STANDARD PROFILE CREATION
 * ================================================================ */

/* Create sRGB profile */
CMError CMCreateSRGBProfile(CMProfileRef *prof);

/* Create Adobe RGB profile */
CMError CMCreateAdobeRGBProfile(CMProfileRef *prof);

/* Create ProPhoto RGB profile */
CMError CMCreateProPhotoRGBProfile(CMProfileRef *prof);

/* Create Gray gamma profile */
CMError CMCreateGrayProfile(CMProfileRef *prof, float gamma);

/* Create D50 gray profile */
CMError CMCreateD50GrayProfile(CMProfileRef *prof);

/* Create generic CMYK profile */
CMError CMCreateGenericCMYKProfile(CMProfileRef *prof);

/* ================================================================
 * ICC VALIDATION AND REPAIR
 * ================================================================ */

/* Validate profile structure */
CMError CMValidateProfileStructure(CMProfileRef prof, Boolean *isValid);

/* Check profile completeness */
CMError CMCheckProfileCompleteness(CMProfileRef prof, Boolean *isComplete);

/* Repair profile */
CMError CMRepairProfile(CMProfileRef prof);

/* ================================================================
 * ICC VERSION COMPATIBILITY
 * ================================================================ */

/* Get ICC version */
CMError CMGetICCVersion(CMProfileRef prof, UInt32 *version);

/* Convert to ICC v2 */
CMError CMConvertToICCv2(CMProfileRef prof);

/* Convert to ICC v4 */
CMError CMConvertToICCv4(CMProfileRef prof);

/* Check ICC compatibility */
Boolean CMCheckICCCompatibility(CMProfileRef prof, UInt32 version);

/* ================================================================
 * ICC PLATFORM INTEGRATION
 * ================================================================ */

/* Get platform signature */
CMError CMGetPlatformSignature(UInt32 *platform);

/* Set platform signature */
CMError CMSetPlatformSignature(CMProfileRef prof, UInt32 platform);

/* Get CMM preferences */
CMError CMGetCMMPreferences(UInt32 *cmm);

/* Set CMM preferences */
CMError CMSetCMMPreferences(CMProfileRef prof, UInt32 cmm);

#ifdef __cplusplus
}
#endif

#endif /* ICCPROFILES_H */