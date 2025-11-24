/*
 * WindowGeometry.h - Unified Window Coordinate System Abstraction
 *
 * This header defines a unified abstraction for window geometry that eliminates
 * the fragmentation between portRect (LOCAL), portBits.bounds (GLOBAL), and
 * strucRgn/contRgn (GLOBAL) coordinate systems.
 *
 * PROBLEM SOLVED:
 * Previously, updating window geometry required manually synchronizing three
 * separate representations:
 * - portRect: LOCAL coordinates (0,0,w,h)
 * - portBits.bounds: GLOBAL screen coordinates for content area
 * - strucRgn/contRgn: GLOBAL screen coordinates for frame and content
 *
 * Any missed synchronization caused rendering corruption. This abstraction
 * ensures ALL coordinate representations are updated atomically.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef __WINDOW_GEOMETRY_H__
#define __WINDOW_GEOMETRY_H__

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Window Chrome Constants
 * ============================================================================ */

/* Standard window chrome dimensions (centralized from scattered magic numbers) */
typedef struct WindowChrome {
    short leftBorder;       /* Left border width (typically 1) */
    short topBorder;        /* Top border above title (typically 1) */
    short rightBorder;      /* Right border width (typically 2: 1px + highlight) */
    short bottomBorder;     /* Bottom border height (typically 2: 1px + padding) */
    short titleBarHeight;   /* Title bar height (typically 20) */
    short titleSeparator;   /* Separator between title and content (typically 1) */
} WindowChrome;

/* Standard window chrome for document windows */
static const WindowChrome kStandardWindowChrome = {
    1,   /* leftBorder */
    1,   /* topBorder */
    2,   /* rightBorder */
    2,   /* bottomBorder */
    20,  /* titleBarHeight */
    1    /* titleSeparator */
};

/* Dialog window chrome (typically no grow box, simpler borders) */
static const WindowChrome kDialogWindowChrome = {
    1,   /* leftBorder */
    1,   /* topBorder */
    1,   /* rightBorder */
    1,   /* bottomBorder */
    20,  /* titleBarHeight */
    1    /* titleSeparator */
};

/* ============================================================================
 * Window Geometry Structure
 * ============================================================================ */

/*
 * WindowGeometry - Complete window coordinate information
 *
 * This structure maintains ALL coordinate representations needed for proper
 * window rendering. It enforces the invariant that:
 * - localContent is always (0, 0, width, height)
 * - All global rectangles are consistent with globalOrigin
 * - Chrome offsets are applied correctly
 */
typedef struct WindowGeometry {
    /* LOCAL coordinates (for QuickDraw operations within window) */
    Rect localContent;      /* Always (0, 0, width, height) */

    /* GLOBAL screen position */
    Point globalOrigin;     /* Top-left of window FRAME (strucRgn) */

    /* GLOBAL rectangles (for screen rendering) */
    Rect globalFrame;       /* Complete window including chrome (strucRgn) */
    Rect globalContent;     /* Content area only (contRgn, portBits.bounds) */

    /* Content dimensions */
    short contentWidth;     /* Width of content area */
    short contentHeight;    /* Height of content area */

    /* Chrome information */
    WindowChrome chrome;    /* Chrome dimensions used for this window */
} WindowGeometry;

/* ============================================================================
 * Geometry Calculation Functions
 * ============================================================================ */

/*
 * WM_CalculateWindowGeometry - Calculate complete window geometry
 *
 * Given a global origin and content size, calculates ALL coordinate
 * representations atomically. This is the ONLY function that should
 * compute window rectangles from scratch.
 *
 * Parameters:
 *   globalOrigin - Top-left position of window FRAME in screen coordinates
 *   contentWidth - Width of content area (LOCAL coordinates)
 *   contentHeight - Height of content area (LOCAL coordinates)
 *   chrome - Chrome dimensions to use (pass kStandardWindowChrome for normal windows)
 *   outGeometry - Returns complete geometry structure
 */
void WM_CalculateWindowGeometry(Point globalOrigin,
                                 short contentWidth,
                                 short contentHeight,
                                 const WindowChrome* chrome,
                                 WindowGeometry* outGeometry);

/*
 * WM_ApplyWindowGeometry - Apply geometry to window structure
 *
 * Updates ALL coordinate representations in the window structure atomically.
 * This ensures portRect, portBits.bounds, strucRgn, and contRgn are
 * synchronized correctly.
 *
 * Parameters:
 *   window - Window to update
 *   geometry - Complete geometry to apply
 *
 * IMPORTANT: This function assumes window->strucRgn and window->contRgn
 * have already been allocated. It will update their contents but not
 * create new regions.
 */
void WM_ApplyWindowGeometry(WindowPtr window, const WindowGeometry* geometry);

/*
 * WM_GetWindowGeometry - Extract current geometry from window
 *
 * Reads the current window geometry from strucRgn and contRgn.
 * Useful for preserving geometry before modifications.
 *
 * Parameters:
 *   window - Window to read
 *   outGeometry - Returns current geometry
 *
 * Returns: true if geometry extracted successfully, false if window invalid
 */
Boolean WM_GetWindowGeometry(WindowPtr window, WindowGeometry* outGeometry);

/* ============================================================================
 * Geometry Transformation Functions
 * ============================================================================ */

/*
 * WM_MoveWindowGeometry - Calculate geometry after moving window
 *
 * Computes new geometry with updated global origin while preserving size.
 *
 * Parameters:
 *   currentGeometry - Current window geometry
 *   newGlobalOrigin - New top-left position for window frame
 *   outGeometry - Returns updated geometry
 */
void WM_MoveWindowGeometry(const WindowGeometry* currentGeometry,
                            Point newGlobalOrigin,
                            WindowGeometry* outGeometry);

/*
 * WM_ResizeWindowGeometry - Calculate geometry after resizing window
 *
 * Computes new geometry with updated content size while preserving position.
 *
 * Parameters:
 *   currentGeometry - Current window geometry
 *   newContentWidth - New content width
 *   newContentHeight - New content height
 *   outGeometry - Returns updated geometry
 */
void WM_ResizeWindowGeometry(const WindowGeometry* currentGeometry,
                              short newContentWidth,
                              short newContentHeight,
                              WindowGeometry* outGeometry);

/* ============================================================================
 * Convenience Functions
 * ============================================================================ */

/*
 * WM_GetChromeForWindow - Determine appropriate chrome for window type
 *
 * Returns the correct chrome dimensions based on window kind and procID.
 *
 * Parameters:
 *   window - Window to examine
 *
 * Returns: Pointer to appropriate chrome structure (static, don't free)
 */
const WindowChrome* WM_GetChromeForWindow(WindowPtr window);

/*
 * WM_ValidateWindowGeometry - Validate geometry is reasonable
 *
 * Checks that geometry has valid dimensions and positions.
 *
 * Parameters:
 *   geometry - Geometry to validate
 *
 * Returns: true if geometry is valid
 */
Boolean WM_ValidateWindowGeometry(const WindowGeometry* geometry);

#ifdef __cplusplus
}
#endif

#endif /* __WINDOW_GEOMETRY_H__ */
