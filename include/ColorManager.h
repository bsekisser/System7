/**
 * @file ColorManager.h
 * @brief Lightweight color state helper for Control Panels.
 *
 * The classic Color Manager on System 7 is tightly coupled with QuickDraw.
 * The portable kernel keeps things simple: this helper tracks the desired
 * foreground/background colors and can push them to the QuickDraw globals
 * on demand. It provides just enough structure for control panels and
 * future palette code without pretending to be full ColorSync.
 *
 * Copyright (c) 2024
 * Licensed under MIT License
 */

#ifndef COLOR_MANAGER_H
#define COLOR_MANAGER_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ColorManagerState {
    RGBColor foreground;
    RGBColor background;
    Boolean dirty;
} ColorManagerState;

OSErr ColorManager_Init(void);
void ColorManager_Shutdown(void);
Boolean ColorManager_IsAvailable(void);

void ColorManager_SetForeground(const RGBColor *color);
void ColorManager_SetBackground(const RGBColor *color);
void ColorManager_GetForeground(RGBColor *outColor);
void ColorManager_GetBackground(RGBColor *outColor);
const ColorManagerState *ColorManager_GetState(void);

void ColorManager_CommitQuickDraw(void);

#ifdef __cplusplus
}
#endif

#endif /* COLOR_MANAGER_H */
