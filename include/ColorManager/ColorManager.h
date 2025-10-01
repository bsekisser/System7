/*
 * ColorManager.h - Main Color Manager API
 *
 * Professional color management interface providing ICC profiles, color space
 * conversion, color matching, and device calibration for System 7.1 Portable.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Manager and ColorSync
 */

#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * COLOR MANAGER TYPES AND CONSTANTS
 * ================================================================ */

/* Color Manager version */
#define kColorManagerVersion    0x0200

/* Error codes */

/* Profile flags */

/* Color space types */

/* Rendering intents */

/* Quality levels */

/* Profile types */

/* Basic color structures */

/* Generic color value */

    CMCMYKColor     cmyk;
    CMHSVColor      hsv;
    CMHLSColor      hls;
    CMXYZColor      xyz;
    CMLABColor      lab;
    UInt8         gray;
    UInt8         data[16];  /* Generic data for any color space */
} CMColor;

/* Profile and world handles */

/* Iteration callback function */

/* Progress callback function */

/* ================================================================
 * COLOR MANAGER INITIALIZATION
 * ================================================================ */

/* Initialize Color Manager */
CMError CMInitColorManager(void);

/* Get Color Manager version */
UInt32 CMGetColorManagerVersion(void);

/* Check if Color Manager is available */
Boolean CMColorManagerAvailable(void);

/* ================================================================
 * PROFILE MANAGEMENT
 * ================================================================ */

/* Open profile from file */
CMError CMOpenProfile(CMProfileRef *prof, const char *filename);

/* Create new profile */
CMError CMNewProfile(CMProfileRef *prof, CMProfileClass profileClass,
                     CMColorSpace dataSpace, CMColorSpace pcs);

/* Close profile */
CMError CMCloseProfile(CMProfileRef prof);

/* Copy profile */
CMError CMCopyProfile(CMProfileRef *dstProf, CMProfileRef srcProf);

/* Clone profile */
CMError CMCloneProfileRef(CMProfileRef prof);

/* Get profile location */
CMError CMGetProfileLocation(CMProfileRef prof, char *location, UInt32 *size);

/* Flatten profile to memory */
CMError CMFlattenProfile(CMProfileRef prof, UInt32 flags,
                        CMFlattenUPP proc, void *refCon);

/* Update profile */
CMError CMUpdateProfile(CMProfileRef prof);

/* Get profile header */
CMError CMGetProfileHeader(CMProfileRef prof, void *header);

/* Set profile header */
CMError CMSetProfileHeader(CMProfileRef prof, const void *header);

/* Get profile element */
CMError CMGetProfileElement(CMProfileRef prof, UInt32 tag,
                           UInt32 *elementSize, void *elementData);

/* Set profile element */
CMError CMSetProfileElement(CMProfileRef prof, UInt32 tag,
                           UInt32 elementSize, const void *elementData);

/* Count profile elements */
CMError CMCountProfileElements(CMProfileRef prof, UInt32 *elementCount);

/* Get profile element info */
CMError CMGetProfileElementInfo(CMProfileRef prof, UInt32 index,
                               UInt32 *tag, UInt32 *elementSize, Boolean *refs);

/* Remove profile element */
CMError CMRemoveProfileElement(CMProfileRef prof, UInt32 tag);

/* ================================================================
 * COLOR SPACE MANAGEMENT
 * ================================================================ */

/* Get profile color space */
CMError CMGetProfileSpace(CMProfileRef prof, CMColorSpace *space);

/* Get profile connection space */
CMError CMGetProfilePCS(CMProfileRef prof, CMColorSpace *pcs);

/* Get profile class */
CMError CMGetProfileClass(CMProfileRef prof, CMProfileClass *profileClass);

/* Set profile class */
CMError CMSetProfileClass(CMProfileRef prof, CMProfileClass profileClass);

/* Get color space name */
CMError CMGetColorSpaceName(CMColorSpace space, char *name, UInt32 maxLength);

/* ================================================================
 * COLOR MATCHING WORLD
 * ================================================================ */

/* Begin color matching session */
CMError CMBeginMatching(CMProfileRef src, CMProfileRef dst, CMMatchRef *myRef);

/* End color matching session */
CMError CMEndMatching(CMMatchRef myRef);

/* Enable matching */
CMError CMEnableMatching(Boolean enableIt);

/* Create color world */
CMError CMCreateColorWorld(CMWorldRef *cw, CMProfileRef src, CMProfileRef dst,
                          CMProfileRef proof);

/* Dispose color world */
CMError CMDisposeColorWorld(CMWorldRef cw);

/* Clone color world */
CMError CMCloneColorWorld(CMWorldRef *clonedCW, CMWorldRef srcCW);

/* Get color world info */
CMError CMGetColorWorldInfo(CMWorldRef cw, UInt32 *info);

/* Use color world */
CMError CMUseColorWorld(CMWorldRef cw);

/* ================================================================
 * COLOR CONVERSION
 * ================================================================ */

/* Match colors using color world */
CMError CMMatchColors(CMWorldRef cw, CMColor *myColors, UInt32 count);

/* Check colors */
CMError CMCheckColors(CMWorldRef cw, CMColor *myColors, UInt32 count,
                     Boolean *gamutResult);

/* Match single color */
CMError CMMatchColor(CMWorldRef cw, CMColor *color);

/* Check single color gamut */
CMError CMCheckColor(CMWorldRef cw, const CMColor *color, Boolean *inGamut);

/* Convert colors between color spaces */
CMError CMConvertXYZToLab(const CMXYZColor *src, CMLABColor *dst,
                         const CMXYZColor *whitePoint);

CMError CMConvertLabToXYZ(const CMLABColor *src, CMXYZColor *dst,
                         const CMXYZColor *whitePoint);

CMError CMConvertXYZToRGB(const CMXYZColor *src, CMRGBColor *dst,
                         CMProfileRef profile);

CMError CMConvertRGBToXYZ(const CMRGBColor *src, CMXYZColor *dst,
                         CMProfileRef profile);

/* ================================================================
 * DEVICE LINK PROFILES
 * ================================================================ */

/* Concatenate profiles */
CMError CMConcatenateProfiles(CMProfileRef thru, CMProfileRef dst,
                             CMProfileRef *newDst);

/* Create multi-profile link */
CMError CMCreateMultiProfileLink(CMProfileRef *profiles, UInt32 nProfiles,
                                CMProfileRef *multiProf);

/* ================================================================
 * NAMED COLORS
 * ================================================================ */

/* Get named color count */
CMError CMGetNamedColorCount(CMProfileRef prof, UInt32 *count);

/* Get named color info */
CMError CMGetNamedColorInfo(CMProfileRef prof, UInt32 index,
                           CMColor *color, char *name, UInt32 maxNameLength);

/* Get named color value */
CMError CMGetNamedColorValue(CMProfileRef prof, const char *name, CMColor *color);

/* ================================================================
 * COLOR PICKER INTEGRATION
 * ================================================================ */

/* Get color */
Boolean CMGetColor(SInt16 where_h, SInt16 where_v, const char *prompt,
               const CMRGBColor *inColor, CMRGBColor *outColor);

/* Color picker dialog */
CMError CMPickColor(CMRGBColor *color, const char *prompt);

/* ================================================================
 * SYSTEM INTEGRATION
 * ================================================================ */

/* Get system profile */
CMError CMGetSystemProfile(CMProfileRef *prof);

/* Set system profile */
CMError CMSetSystemProfile(const char *profileName);

/* Get default profile for device class */
CMError CMGetDefaultProfileByClass(CMProfileClass profileClass, CMProfileRef *prof);

/* Set default profile for device class */
CMError CMSetDefaultProfileByClass(CMProfileClass profileClass, CMProfileRef prof);

/* Iterate through profiles */
CMError CMIterateColorProfiles(CMProfileIterateUPP proc, UInt32 *seed,
                              UInt32 *count, void *refCon);

/* Search for profiles */
CMError CMSearchForProfiles(void *searchSpec, CMProfileIterateUPP proc,
                           UInt32 *seed, UInt32 *count, void *refCon);

/* Get profile by AID */
CMError CMGetProfileByAID(UInt32 AID, CMProfileRef *prof);

/* Get profile AID */
CMError CMGetProfileAID(CMProfileRef prof, UInt32 *AID);

/* ================================================================
 * RENDERING INTENTS AND QUALITY
 * ================================================================ */

/* Set rendering intent */
CMError CMSetRenderingIntent(CMWorldRef cw, CMRenderingIntent intent);

/* Get rendering intent */
CMError CMGetRenderingIntent(CMWorldRef cw, CMRenderingIntent *intent);

/* Set quality */
CMError CMSetQuality(CMWorldRef cw, CMQuality quality);

/* Get quality */
CMError CMGetQuality(CMWorldRef cw, CMQuality *quality);

/* ================================================================
 * GAMUT CHECKING
 * ================================================================ */

/* Create gamut */
CMError CMCreateGamut(CMProfileRef prof, void **gamut);

/* Check point in gamut */
CMError CMGamutCheckColor(void *gamut, const CMColor *color, Boolean *inGamut);

/* ================================================================
 * VALIDATION
 * ================================================================ */

/* Validate profile */
CMError CMValidateProfile(CMProfileRef prof, Boolean *valid, Boolean *preferred);

/* Get profile description */
CMError CMGetProfileDescription(CMProfileRef prof, char *name, UInt32 *size);

/* Get profile MD5 */
CMError CMGetProfileMD5(CMProfileRef prof, UInt8 *digest);

#ifdef __cplusplus
}
#endif

#endif /* COLORMANAGER_H */