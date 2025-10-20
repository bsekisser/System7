/**
 * @file AppSwitcher.h
 * @brief Application Switcher (Command-Tab) Interface
 *
 * Provides the classic Mac OS application switcher functionality,
 * allowing users to cycle through open applications with Command-Tab.
 *
 * Copyright (c) 2025 System 7.1 Portable Project
 */

#pragma once
#ifndef __APPSWITCHER_H__
#define __APPSWITCHER_H__

#include "SystemTypes.h"
#include "Finder/Icon/icon_types.h"

/* Forward declaration to avoid circular includes */
typedef struct ProcessControlBlock ProcessControlBlock;

/*---------------------------------------------------------------------------
 * Data Structures
 *---------------------------------------------------------------------------*/

/** Represents a single switchable application */
typedef struct {
    ProcessSerialNumber psn;           /**< Process serial number */
    Str255 appName;                    /**< Application name */
    OSType signature;                  /**< Application signature */
    SInt16 iconID;                     /**< Icon resource ID (loaded on-demand) */
    Boolean isHidden;                  /**< True if app is hidden */
    Boolean isSystemProcess;           /**< True if system process */
} SwitchableApp;

/** Application switcher state */
typedef struct {
    SwitchableApp* apps;               /**< Array of switchable apps */
    SInt16 appCount;                   /**< Number of apps */
    SInt16 currentIndex;               /**< Currently selected app index */
    Boolean isActive;                  /**< True if switcher is visible */
    Point windowPos;                   /**< Switcher window position */
    Rect windowBounds;                 /**< Switcher window bounds */
    GrafPtr switcherPort;              /**< Drawing port for switcher */
    UInt32 activationTime;             /**< Time switcher was activated */
} AppSwitcherState;

/*---------------------------------------------------------------------------
 * Public API
 *---------------------------------------------------------------------------*/

/**
 * Initialize the application switcher module
 * Must be called once during system startup
 *
 * @return OSErr - noErr if successful
 */
OSErr AppSwitcher_Init(void);

/**
 * Terminate the application switcher module
 * Releases all allocated resources
 */
void AppSwitcher_Shutdown(void);

/**
 * Display the application switcher window
 * Fetches current app list and shows visual switcher UI
 *
 * @return Boolean - true if switcher shown successfully
 */
Boolean AppSwitcher_ShowWindow(void);

/**
 * Hide the application switcher window
 * Cleans up switcher UI resources
 */
void AppSwitcher_HideWindow(void);

/**
 * Check if switcher window is currently visible
 *
 * @return Boolean - true if switcher is active/visible
 */
Boolean AppSwitcher_IsActive(void);

/**
 * Cycle to next application (forward)
 * Called when user presses Tab while holding Command
 * Shows switcher window if not already visible
 */
void AppSwitcher_CycleForward(void);

/**
 * Cycle to previous application (backward)
 * Called when user presses Shift-Command-Tab
 */
void AppSwitcher_CycleBackward(void);

/**
 * Switch to the currently selected application
 * Brings the selected app to front and hides switcher
 */
void AppSwitcher_SelectCurrent(void);

/**
 * Handle Key Up event (typically Tab release)
 * Finalizes the application switch and hides switcher
 */
void AppSwitcher_HandleKeyUp(void);

/**
 * Redraw the application switcher window
 * Called during idle time or on demand
 */
void AppSwitcher_Draw(void);

/**
 * Update the application list from ProcessManager
 * Fetches current list of running applications
 *
 * @return OSErr - noErr if successful
 */
OSErr AppSwitcher_UpdateAppList(void);

/**
 * Get the currently selected application
 *
 * @param outApp - Pointer to receive selected app info (can be NULL)
 * @return ProcessSerialNumber - PSN of selected app
 */
ProcessSerialNumber AppSwitcher_GetSelectedApp(SwitchableApp* outApp);

/**
 * Get total number of switchable applications
 *
 * @return SInt16 - Number of applications in switcher
 */
SInt16 AppSwitcher_GetAppCount(void);

/**
 * Get application at specified index
 *
 * @param index - Application index (0-based)
 * @param outApp - Pointer to receive app info
 * @return Boolean - true if index valid and app retrieved
 */
Boolean AppSwitcher_GetAppAt(SInt16 index, SwitchableApp* outApp);

/**
 * Request switcher to hide after timeout
 * Used to auto-hide switcher if user stops cycling
 *
 * @param timeoutMs - Timeout in milliseconds
 */
void AppSwitcher_StartHideTimeout(UInt32 timeoutMs);

/**
 * Cancel any pending hide timeout
 */
void AppSwitcher_CancelHideTimeout(void);

/*---------------------------------------------------------------------------
 * Configuration
 *---------------------------------------------------------------------------*/

/** Maximum number of applications that can be in switcher */
#define kAppSwitcher_MaxApps 32

/** Size of app icons in switcher window */
#define kAppSwitcher_IconSize 32

/** Switcher window width (approx) */
#define kAppSwitcher_WindowWidth 400

/** Switcher window height (approx) */
#define kAppSwitcher_WindowHeight 300

/** Hide timeout when user stops cycling (milliseconds) */
#define kAppSwitcher_HideTimeout 1000

#endif /* AppSwitcher_h */
