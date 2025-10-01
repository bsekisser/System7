/*
 * PopupMenus.h - Popup Menu Management and Positioning
 *
 * Header for popup menu functions including positioning, display,
 * and context menu support for the Menu Manager.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Menu Manager
 */

#ifndef __POPUP_MENUS_H__
#define __POPUP_MENUS_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "MenuManager.h"
#include "MenuTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Popup Menu Constants
 * ============================================================================ */

/* Popup menu positioning modes */

/* Popup menu animation types */

/* Popup menu constraints */

/* Context menu triggers */

/* ============================================================================
 * Popup Menu Data Structures
 * ============================================================================ */

/* Popup menu positioning information */

/* Context menu information */

/* Popup menu result */

/* ============================================================================
 * Popup Menu Core Functions
 * ============================================================================ */

/*
 * PopUpMenuSelectEx - Extended popup menu selection
 *
 * Enhanced version of PopUpMenuSelect with full positioning and animation options.
 *
 * Parameters:
 *   popupInfo - Popup menu information
 *   result    - Receives selection result
 *
 * Returns: Menu selection result code
 */
short PopUpMenuSelectEx(const PopupMenuInfo* popupInfo, PopupMenuResult* result);

/*
 * ShowPopupMenu - Display popup menu
 *
 * Displays a popup menu at the specified location with the given options.
 *
 * Parameters:
 *   theMenu       - Menu to display
 *   location      - Target location
 *   positionMode  - Positioning mode
 *   alignItem     - Item to align (for centering modes)
 *
 * Returns: TRUE if menu was displayed successfully
 */
Boolean ShowPopupMenu(MenuHandle theMenu, Point location, short positionMode, short alignItem);

/*
 * HidePopupMenu - Hide currently displayed popup menu
 *
 * Hides the currently displayed popup menu and restores the background.
 */
void HidePopupMenu(void);

/*
 * TrackPopupMenuEx - Extended popup menu tracking
 *
 * Tracks user interaction with a popup menu with extended options.
 *
 * Parameters:
 *   theMenu      - Menu to track
 *   menuRect     - Menu rectangle
 *   popupInfo    - Popup information
 *   result       - Receives tracking result
 *
 * Returns: Tracking result code
 */
short TrackPopupMenuEx(MenuHandle theMenu, const Rect* menuRect,
                      const PopupMenuInfo* popupInfo, PopupMenuResult* result);

/* ============================================================================
 * Popup Menu Positioning Functions
 * ============================================================================ */

/*
 * CalcPopupMenuPosition - Calculate popup menu position
 *
 * Calculates the optimal position for a popup menu based on the specified parameters.
 *
 * Parameters:
 *   theMenu       - Menu to position
 *   targetPoint   - Target position
 *   positionMode  - Positioning mode
 *   alignItem     - Item to align with point
 *   constraints   - Positioning constraints
 *   boundingRect  - Bounding rectangle
 *   menuRect      - Receives calculated menu rectangle
 */
void CalcPopupMenuPosition(MenuHandle theMenu, Point targetPoint, short positionMode,
                          short alignItem, short constraints, const Rect* boundingRect,
                          Rect* menuRect);

/*
 * ConstrainPopupToScreen - Constrain popup menu to screen bounds
 *
 * Adjusts popup menu position to ensure it remains on screen.
 *
 * Parameters:
 *   menuRect    - Menu rectangle to adjust
 *   screenRect  - Screen bounds
 *   constraints - Constraint flags
 */
void ConstrainPopupToScreen(Rect* menuRect, const Rect* screenRect, short constraints);

/*
 * GetPopupMenuBounds - Get bounds for popup menu
 *
 * Calculates the bounds rectangle needed for a popup menu.
 *
 * Parameters:
 *   theMenu    - Menu to calculate bounds for
 *   menuBounds - Receives menu bounds
 */
void GetPopupMenuBounds(MenuHandle theMenu, Rect* menuBounds);

/*
 * AdjustPopupForAvoidance - Adjust popup to avoid UI elements
 *
 * Adjusts popup position to avoid menu bar, dock, and other UI elements.
 *
 * Parameters:
 *   menuRect    - Menu rectangle to adjust
 *   avoidAreas  - Array of rectangles to avoid
 *   areaCount   - Number of avoidance areas
 */
void AdjustPopupForAvoidance(Rect* menuRect, const Rect avoidAreas[], short areaCount);

/* ============================================================================
 * Context Menu Functions
 * ============================================================================ */

/*
 * ShowContextMenu - Display context menu
 *
 * Displays a context menu at the specified location.
 *
 * Parameters:
 *   contextInfo - Context menu information
 *   result      - Receives selection result
 *
 * Returns: Context menu result code
 */
short ShowContextMenu(const ContextMenuInfo* contextInfo, PopupMenuResult* result);

/*
 * BuildContextMenu - Build context menu dynamically
 *
 * Builds a context menu based on the current context and available actions.
 *
 * Parameters:
 *   contextData   - Context-specific data
 *   baseMenu      - Base menu to extend (can be NULL)
 *   includeDefaults - Include default context actions
 *
 * Returns: Handle to context menu, or NULL if failed
 */
MenuHandle BuildContextMenu(void* contextData, MenuHandle baseMenu, Boolean includeDefaults);

/*
 * AddContextMenuItem - Add item to context menu
 *
 * Adds a context-specific item to a context menu.
 *
 * Parameters:
 *   contextMenu - Context menu to modify
 *   itemText    - Item text
 *   itemAction  - Action identifier
 *   enabled     - TRUE if item should be enabled
 */
void AddContextMenuItem(MenuHandle contextMenu, ConstStr255Param itemText,
                       short itemAction, Boolean enabled);

/*
 * IsContextMenuTrigger - Check if event triggers context menu
 *
 * Determines if the specified event should trigger a context menu.
 *
 * Parameters:
 *   mousePoint - Mouse position
 *   modifiers  - Modifier keys
 *   clickType  - Type of click
 *
 * Returns: TRUE if context menu should be triggered
 */
Boolean IsContextMenuTrigger(Point mousePoint, unsigned long modifiers, short clickType);

/* ============================================================================
 * Popup Menu Animation Functions
 * ============================================================================ */

/*
 * AnimatePopupShow - Animate popup menu appearance
 *
 * Animates the appearance of a popup menu with the specified effect.
 *
 * Parameters:
 *   theMenu       - Menu to animate
 *   startRect     - Starting rectangle
 *   endRect       - Ending rectangle
 *   animationType - Animation type
 *   duration      - Duration in ticks
 */
void AnimatePopupShow(MenuHandle theMenu, const Rect* startRect, const Rect* endRect,
                     short animationType, short duration);

/*
 * AnimatePopupHide - Animate popup menu disappearance
 *
 * Animates the disappearance of a popup menu with the specified effect.
 *
 * Parameters:
 *   theMenu       - Menu to animate
 *   startRect     - Starting rectangle
 *   endRect       - Ending rectangle
 *   animationType - Animation type
 *   duration      - Duration in ticks
 */
void AnimatePopupHide(MenuHandle theMenu, const Rect* startRect, const Rect* endRect,
                     short animationType, short duration);

/*
 * SetPopupAnimationPrefs - Set popup animation preferences
 *
 * Sets the default animation preferences for popup menus.
 *
 * Parameters:
 *   defaultAnimation - Default animation type
 *   defaultDuration  - Default duration
 *   enableAnimations - TRUE to enable animations
 */
void SetPopupAnimationPrefs(short defaultAnimation, short defaultDuration, Boolean enableAnimations);

/* ============================================================================
 * Popup Menu Utilities
 * ============================================================================ */

/*
 * GetPopupMenuUnderMouse - Get popup menu under mouse
 *
 * Returns the popup menu currently under the mouse cursor, if any.
 *
 * Returns: Handle to popup menu under mouse, or NULL if none
 */
MenuHandle GetPopupMenuUnderMouse(void);

/*
 * IsPopupMenuVisible - Check if popup menu is visible
 *
 * Checks if any popup menu is currently visible.
 *
 * Returns: TRUE if a popup menu is visible
 */
Boolean IsPopupMenuVisible(void);

/*
 * DismissAllPopupMenus - Dismiss all popup menus
 *
 * Dismisses all currently visible popup menus.
 */
void DismissAllPopupMenus(void);

/*
 * GetPopupMenuSelection - Get current popup selection
 *
 * Gets the current selection in the active popup menu.
 *
 * Parameters:
 *   result - Receives current selection
 *
 * Returns: TRUE if there is an active selection
 */
Boolean GetPopupMenuSelection(PopupMenuResult* result);

/* ============================================================================
 * Screen and Window Integration
 * ============================================================================ */

/*
 * GetScreenBounds - Get screen bounds for popup positioning
 *
 * Gets the bounds of the screen for popup menu positioning.
 *
 * Parameters:
 *   screenBounds - Receives screen bounds
 */
void GetScreenBounds(Rect* screenBounds);

/*
 * GetMenuBarBounds - Get menu bar bounds
 *
 * Gets the bounds of the menu bar for avoidance calculations.
 *
 * Parameters:
 *   menuBarBounds - Receives menu bar bounds
 */
void GetMenuBarBounds(Rect* menuBarBounds);

/*
 * GetDockBounds - Get dock bounds
 *
 * Gets the bounds of the dock/taskbar for avoidance calculations.
 *
 * Parameters:
 *   dockBounds - Receives dock bounds
 *   dockSide   - Receives dock side (0=bottom, 1=left, 2=top, 3=right)
 *
 * Returns: TRUE if dock is visible
 */
Boolean GetDockBounds(Rect* dockBounds, short* dockSide);

/*
 * GetAvoidanceAreas - Get all UI areas to avoid
 *
 * Gets rectangles for all UI elements that popup menus should avoid.
 *
 * Parameters:
 *   avoidAreas - Array to receive avoidance rectangles
 *   maxAreas   - Maximum number of areas
 *
 * Returns: Number of avoidance areas returned
 */
short GetAvoidanceAreas(Rect avoidAreas[], short maxAreas);

/* ============================================================================
 * Popup Menu State Management
 * ============================================================================ */

/*
 * BeginPopupMenuSession - Begin popup menu session
 *
 * Begins a popup menu session and initializes tracking state.
 *
 * Parameters:
 *   popupInfo - Popup session information
 */
void BeginPopupMenuSession(const PopupMenuInfo* popupInfo);

/*
 * EndPopupMenuSession - End popup menu session
 *
 * Ends the popup menu session and cleans up resources.
 */
void EndPopupMenuSession(void);

/*
 * UpdatePopupMenuSession - Update popup menu session
 *
 * Updates the popup menu session based on current input state.
 *
 * Parameters:
 *   mousePoint - Current mouse position
 *   mouseDown  - Current mouse button state
 */
void UpdatePopupMenuSession(Point mousePoint, Boolean mouseDown);

/* ============================================================================
 * Platform Integration for Popup Menus
 * ============================================================================ */

/*
 * Platform popup menu functions that must be implemented:
 *
 * Boolean Platform_ShowPopupMenu(const PopupMenuInfo* info);
 * void Platform_HidePopupMenu(void);
 * Boolean Platform_TrackPopupMenu(const PopupMenuInfo* info, PopupMenuResult* result);
 * void Platform_GetScreenBounds(Rect* bounds);
 * void Platform_GetMenuBarBounds(Rect* bounds);
 * Boolean Platform_GetDockBounds(Rect* bounds, short* side);
 * void Platform_AnimatePopup(MenuHandle menu, const Rect* startRect, const Rect* endRect,
 *                            short animationType, short duration);
 */

/* ============================================================================
 * Popup Menu Macros
 * ============================================================================ */

/* Check if popup positioning mode is alignment-based */
#define IsAlignmentMode(mode) \
    ((mode) >= kPopupAlignLeft && (mode) <= kPopupAlignBottom)

/* Check if popup positioning mode is centering-based */
#define IsCenteringMode(mode) \
    ((mode) == kPopupCenterOnItem || (mode) == kPopupCenterOnPoint)

/* Check if constraint flag is set */
#define HasConstraint(constraints, flag) \
    (((constraints) & (flag)) != 0)

/* Create popup menu info with defaults */
#define InitPopupMenuInfo(info, menu, point, mode) \
    do { \
        (info)->menu = (menu); \
        (info)->targetPoint = (point); \
        (info)->positionMode = (mode); \
        (info)->alignItem = 0; \
        (info)->constraints = kPopupConstrainToScreen; \
        (info)->animationType = kPopupAnimateNone; \
        (info)->animationDuration = 0; \
        (info)->modal = false; \
        (info)->dismissOnRelease = true; \
    } while (0)

/* Extract coordinates from point */
#define PointH(pt)  ((pt).h)
#define PointV(pt)  ((pt).v)

/* Create point from coordinates */
#define MakePoint(h, v)  ((Point){(v), (h)})

#ifdef __cplusplus
}
#endif

#endif /* __POPUP_MENUS_H__ */
