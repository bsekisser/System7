/*
 * StartupScreen.h - System 7 Startup Screen Display
 *
 * Provides the classic "Welcome to Macintosh" startup screen with
 * extension loading display and progress indication.
 *
 * Copyright (c) 2024 System7.1-Portable Project
 * MIT License
 */

#ifndef STARTUP_SCREEN_H
#define STARTUP_SCREEN_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"
#include <QuickDraw.h>

/* Startup screen phases */

/* Extension info for display */

/* Startup progress info */

/* Startup screen configuration */

/* Startup screen state */

/* Public API */

/* Initialize startup screen system */
OSErr InitStartupScreen(const StartupScreenConfig* config);

/* Show welcome screen */
OSErr ShowWelcomeScreen(void);

/* Hide startup screen */
void HideStartupScreen(void);

/* Update startup progress */
OSErr UpdateStartupProgress(const StartupProgress* progress);

/* Display loading extension */
OSErr ShowLoadingExtension(const ExtensionInfo* extension);

/* Show startup item with icon */
OSErr ShowStartupItem(ConstStr255Param itemName, UInt16 iconID);

/* Set startup phase */
OSErr SetStartupPhase(StartupPhase phase);

/* Draw progress bar */
void DrawProgressBar(short percent);

/* Play startup sound */
OSErr PlayStartupSound(void);

/* Custom startup screen */
OSErr SetCustomStartupScreen(PicHandle picture);

/* Extension loading */
OSErr BeginExtensionLoading(UInt16 extensionCount);
OSErr ShowExtensionIcon(ConstStr255Param name, Handle iconSuite);
OSErr EndExtensionLoading(void);

/* Error display */
OSErr ShowStartupError(ConstStr255Param errorMessage, OSErr errorCode);

/* Cleanup */
void CleanupStartupScreen(void);

/* Startup screen customization */
OSErr SetStartupBackgroundPattern(const Pattern* pattern);
OSErr SetStartupColors(const RGBColor* background, const RGBColor* text);
OSErr SetStartupLogo(PicHandle logo);

/* Progress callbacks */

OSErr RegisterStartupProgressCallback(StartupProgressProc proc, void* userData);

/* Animation support */
OSErr StartStartupAnimation(void);
OSErr StopStartupAnimation(void);
Boolean IsStartupAnimating(void);

/* Debug mode */
void EnableStartupDebugMode(Boolean enable);
void DumpStartupScreenState(void);

#endif /* STARTUP_SCREEN_H */
