/**
 * @file ColorManager.c
 * @brief Minimal color state tracker backed by QuickDraw.
 *
 * The real classic Color Manager is deeply intertwined with Color QuickDraw
 * and ColorSync. For the portable kernel we just keep track of the colors
 * the user expects and provide a single place to push them to the drawing
 * globals. This keeps control panels decoupled from QuickDraw internals
 * while we build out the rest of the toolbox.
 */

#include "SystemTypes.h"
#include "ColorManager.h"
#include "QuickDraw/ColorQuickDraw.h"

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

static ColorManagerState gColorState = {
    .foreground = { 0x0000, 0x0000, 0x0000 },
    .background = { 0xFFFF, 0xFFFF, 0xFFFF },
    .dirty = false
};

static Boolean gColorManagerInitialized = false;

/* -------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

static void copy_color(RGBColor *dst, const RGBColor *src) {
    if (!dst || !src) {
        return;
    }
    dst->red = src->red;
    dst->green = src->green;
    dst->blue = src->blue;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

OSErr ColorManager_Init(void) {
    if (gColorManagerInitialized) {
        return noErr;
    }

    /* Ensure QuickDraw has the default colors the first time around */
    ColorManager_CommitQuickDraw();
    gColorManagerInitialized = true;
    return noErr;
}

void ColorManager_Shutdown(void) {
    gColorManagerInitialized = false;
}

Boolean ColorManager_IsAvailable(void) {
    return gColorManagerInitialized;
}

void ColorManager_SetForeground(const RGBColor *color) {
    if (!color) {
        return;
    }
    copy_color(&gColorState.foreground, color);
    gColorState.dirty = true;
}

void ColorManager_SetBackground(const RGBColor *color) {
    if (!color) {
        return;
    }
    copy_color(&gColorState.background, color);
    gColorState.dirty = true;
}

void ColorManager_GetForeground(RGBColor *outColor) {
    copy_color(outColor, &gColorState.foreground);
}

void ColorManager_GetBackground(RGBColor *outColor) {
    copy_color(outColor, &gColorState.background);
}

const ColorManagerState *ColorManager_GetState(void) {
    return &gColorState;
}

void ColorManager_CommitQuickDraw(void) {
    /* We never want to call into QuickDraw with NULL pointers */
    RGBForeColor(&gColorState.foreground);
    RGBBackColor(&gColorState.background);
    gColorState.dirty = false;
}
