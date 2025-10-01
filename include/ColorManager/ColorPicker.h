/*
 * ColorPicker.h - Color Selection Interfaces
 *
 * Color picker interfaces and selection dialogs providing HSV, RGB, CMYK,
 * and other color space selection modes compatible with Mac OS color picker.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Color Picker
 */

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ColorManager.h"
#include "ColorSpaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * COLOR PICKER CONSTANTS
 * ================================================================ */

/* Color picker modes */

/* Color picker flags */

/* Color picker result codes */

/* ================================================================
 * COLOR PICKER STRUCTURES
 * ================================================================ */

/* Color picker dialog configuration */

/* Color picker callback functions */

/* Color swatch definition */

/* Color palette definition */

/* ================================================================
 * BASIC COLOR PICKER FUNCTIONS
 * ================================================================ */

/* Simple color picker dialog */
Boolean CMGetColor(SInt16 where_h, SInt16 where_v, const char *prompt,
               const CMRGBColor *inColor, CMRGBColor *outColor);

/* Extended color picker dialog */
CMPickerResult CMShowColorPicker(const CMPickerConfig *config, CMRGBColor *selectedColor);

/* Color picker with callback */
CMPickerResult CMShowColorPickerWithCallback(const CMPickerConfig *config,
                                            CMPickerUpdateCallback updateCallback,
                                            CMPickerValidateCallback validateCallback,
                                            CMRGBColor *selectedColor);

/* ================================================================
 * COLOR SPACE CONVERSIONS IN PICKER
 * ================================================================ */

/* Convert between picker formats */
CMError CMConvertToPickerRGB(const CMColor *input, CMRGBColor *output);
CMError CMConvertFromPickerRGB(const CMRGBColor *input, CMColor *output, CMColorSpace space);

/* Legacy picker compatibility */
UInt16 CMFix2SmallFract(SInt32 fixed);
SInt32 CMSmallFract2Fix(UInt16 fract);

/* Color space conversions for picker */
CMError CMCMY2RGB(const CMCMYColor *cColor, CMRGBColor *rColor);
CMError CMRGB2CMY(const CMRGBColor *rColor, CMCMYColor *cColor);
CMError CMHSL2RGB(const CMHLSColor *hColor, CMRGBColor *rColor);
CMError CMRGB2HSL(const CMRGBColor *rColor, CMHLSColor *hColor);
CMError CMHSV2RGB(const CMHSVColor *hColor, CMRGBColor *rColor);
CMError CMRGB2HSV(const CMRGBColor *rColor, CMHSVColor *hColor);

/* ================================================================
 * COLOR SWATCHES AND PALETTES
 * ================================================================ */

/* Create color palette */
CMError CMCreateColorPalette(const char *name, CMColorPalette **palette);

/* Load palette from file */
CMError CMLoadColorPalette(const char *filename, CMColorPalette **palette);

/* Save palette to file */
CMError CMSaveColorPalette(const CMColorPalette *palette, const char *filename);

/* Dispose color palette */
void CMDisposeColorPalette(CMColorPalette *palette);

/* Add swatch to palette */
CMError CMAddSwatchToPalette(CMColorPalette *palette, const CMRGBColor *color, const char *name);

/* Remove swatch from palette */
CMError CMRemoveSwatchFromPalette(CMColorPalette *palette, UInt32 index);

/* Get swatch from palette */
CMError CMGetSwatchFromPalette(const CMColorPalette *palette, UInt32 index, CMColorSwatch *swatch);

/* Find swatch by color */
CMError CMFindSwatchByColor(const CMColorPalette *palette, const CMRGBColor *color,
                           float tolerance, UInt32 *index);

/* Find swatch by name */
CMError CMFindSwatchByName(const CMColorPalette *palette, const char *name, UInt32 *index);

/* ================================================================
 * STANDARD COLOR PALETTES
 * ================================================================ */

/* Get system color palette */
CMError CMGetSystemColorPalette(CMColorPalette **palette);

/* Get web-safe color palette */
CMError CMGetWebSafeColorPalette(CMColorPalette **palette);

/* Get crayon color palette */
CMError CMGetCrayonColorPalette(CMColorPalette **palette);

/* Get grayscale palette */
CMError CMGetGrayscalePalette(UInt32 steps, CMColorPalette **palette);

/* Create rainbow palette */
CMError CMCreateRainbowPalette(UInt32 steps, CMColorPalette **palette);

/* ================================================================
 * CUSTOM COLOR MANAGEMENT
 * ================================================================ */

/* Custom color storage */

/* Get custom colors */
CMError CMGetCustomColors(CMCustomColors *customColors);

/* Set custom colors */
CMError CMSetCustomColors(const CMCustomColors *customColors);

/* Add custom color */
CMError CMAddCustomColor(const CMRGBColor *color, const char *name, UInt32 *slot);

/* Remove custom color */
CMError CMRemoveCustomColor(UInt32 slot);

/* ================================================================
 * COLOR HARMONY AND GENERATION
 * ================================================================ */

/* Color harmony types */

/* Generate color harmony */
CMError CMGenerateColorHarmony(const CMRGBColor *baseColor, CMColorHarmony harmony,
                              CMRGBColor *harmonicColors, UInt32 *colorCount);

/* Generate color tints */
CMError CMGenerateColorTints(const CMRGBColor *baseColor, UInt32 steps,
                            CMRGBColor *tints);

/* Generate color shades */
CMError CMGenerateColorShades(const CMRGBColor *baseColor, UInt32 steps,
                             CMRGBColor *shades);

/* Generate color tones */
CMError CMGenerateColorTones(const CMRGBColor *baseColor, UInt32 steps,
                            CMRGBColor *tones);

/* ================================================================
 * COLOR ACCESSIBILITY
 * ================================================================ */

/* Color vision deficiency types */

/* Simulate color vision deficiency */
CMError CMSimulateColorVision(const CMRGBColor *original, CMVisionType visionType,
                             CMRGBColor *simulated);

/* Check color contrast */
CMError CMCheckColorContrast(const CMRGBColor *foreground, const CMRGBColor *background,
                            float *contrastRatio, Boolean *isAccessible);

/* Suggest accessible colors */
CMError CMSuggestAccessibleColors(const CMRGBColor *baseColor, Boolean isDarkBackground,
                                 CMRGBColor *suggestions, UInt32 *suggestionCount);

/* ================================================================
 * EYEDROPPER TOOL
 * ================================================================ */

/* Eyedropper configuration */

/* Eyedropper callback */

/* Start eyedropper tool */
CMError CMStartEyedropper(const CMEyedropperConfig *config, CMEyedropperCallback callback, void *userData);

/* Stop eyedropper tool */
void CMStopEyedropper(void);

/* Get pixel color at coordinates */
CMError CMGetPixelColor(SInt16 x, SInt16 y, CMRGBColor *color);

/* Get average color in area */
CMError CMGetAverageColor(SInt16 x, SInt16 y, SInt16 width, SInt16 height, CMRGBColor *color);

/* ================================================================
 * COLOR PICKER EXTENSIONS
 * ================================================================ */

/* Color picker plugin interface */

/* Register color picker plugin */
CMError CMRegisterColorPickerPlugin(const CMColorPickerPlugin *plugin);

/* Unregister color picker plugin */
CMError CMUnregisterColorPickerPlugin(const char *name);

/* Get available picker modes */
CMError CMGetAvailablePickerModes(CMPickerMode *modes, UInt32 *count);

/* ================================================================
 * PLATFORM INTEGRATION
 * ================================================================ */

/* Platform-specific color picker */
CMPickerResult CMShowNativeColorPicker(const CMPickerConfig *config, CMRGBColor *selectedColor);

/* Set platform color picker preferences */
CMError CMSetPlatformPickerPreferences(const CMPickerConfig *config);

/* Get platform color picker capabilities */
CMError CMGetPlatformPickerCapabilities(UInt32 *capabilities);

#ifdef __cplusplus
}
#endif

#endif /* COLORPICKER_H */