/*
 * WindowGeometry.c - Unified Window Coordinate System Implementation
 *
 * This file implements the unified window geometry abstraction that prevents
 * coordinate system fragmentation bugs by ensuring atomic updates.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#include "SystemTypes.h"
#include "WindowManager/WindowManager.h"
#include "WindowManager/WindowManagerInternal.h"
#include "WindowManager/WindowGeometry.h"
#include "WindowManager/WindowKinds.h"
#include "QuickDraw/QuickDraw.h"

/* External region functions */
extern void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);

/* ============================================================================
 * Geometry Calculation Functions
 * ============================================================================ */

void WM_CalculateWindowGeometry(Point globalOrigin,
                                 short contentWidth,
                                 short contentHeight,
                                 const WindowChrome* chrome,
                                 WindowGeometry* outGeometry) {
    if (!chrome || !outGeometry) return;

    /* Store content dimensions */
    outGeometry->contentWidth = contentWidth;
    outGeometry->contentHeight = contentHeight;

    /* Store chrome */
    outGeometry->chrome = *chrome;

    /* Store global origin (top-left of FRAME) */
    outGeometry->globalOrigin = globalOrigin;

    /* LOCAL coordinates are always (0, 0, width, height) */
    outGeometry->localContent.left = 0;
    outGeometry->localContent.top = 0;
    outGeometry->localContent.right = contentWidth;
    outGeometry->localContent.bottom = contentHeight;

    /* Calculate global FRAME rectangle (includes all chrome) */
    outGeometry->globalFrame.left = globalOrigin.h;
    outGeometry->globalFrame.top = globalOrigin.v;
    outGeometry->globalFrame.right = globalOrigin.h +
                                     chrome->leftBorder + contentWidth + chrome->rightBorder;
    outGeometry->globalFrame.bottom = globalOrigin.v +
                                      chrome->topBorder + chrome->titleBarHeight +
                                      chrome->titleSeparator + contentHeight + chrome->bottomBorder;

    /* Calculate global CONTENT rectangle (content area only) */
    outGeometry->globalContent.left = globalOrigin.h + chrome->leftBorder;
    outGeometry->globalContent.top = globalOrigin.v + chrome->topBorder +
                                     chrome->titleBarHeight + chrome->titleSeparator;
    outGeometry->globalContent.right = outGeometry->globalContent.left + contentWidth;
    outGeometry->globalContent.bottom = outGeometry->globalContent.top + contentHeight;
}

void WM_ApplyWindowGeometry(WindowPtr window, const WindowGeometry* geometry) {
    if (!window || !geometry) return;

    /* Update portRect (LOCAL coordinates) */
    window->port.portRect = geometry->localContent;

    /* Update portBits.bounds (GLOBAL content area) */
    window->port.portBits.bounds = geometry->globalContent;

    /* Update structure region (GLOBAL frame) */
    if (window->strucRgn) {
        SetRectRgn(window->strucRgn,
                   geometry->globalFrame.left,
                   geometry->globalFrame.top,
                   geometry->globalFrame.right,
                   geometry->globalFrame.bottom);
    }

    /* Update content region (GLOBAL content area) */
    if (window->contRgn) {
        SetRectRgn(window->contRgn,
                   geometry->globalContent.left,
                   geometry->globalContent.top,
                   geometry->globalContent.right,
                   geometry->globalContent.bottom);
    }
}

Boolean WM_GetWindowGeometry(WindowPtr window, WindowGeometry* outGeometry) {
    if (!window || !outGeometry) return false;

    /* Verify window has required regions */
    if (!window->strucRgn || !*(window->strucRgn)) return false;
    if (!window->contRgn || !*(window->contRgn)) return false;

    /* Extract from strucRgn and contRgn */
    Rect globalFrame = (*(window->strucRgn))->rgnBBox;
    Rect globalContent = (*(window->contRgn))->rgnBBox;

    /* Calculate content dimensions */
    outGeometry->contentWidth = globalContent.right - globalContent.left;
    outGeometry->contentHeight = globalContent.bottom - globalContent.top;

    /* Extract chrome by comparing frame and content */
    outGeometry->chrome.leftBorder = globalContent.left - globalFrame.left;
    outGeometry->chrome.topBorder = globalFrame.top + 20 + 1 - globalContent.top; /* Assuming 20px title + 1px separator */
    outGeometry->chrome.rightBorder = globalFrame.right - globalContent.right;
    outGeometry->chrome.bottomBorder = globalFrame.bottom - globalContent.bottom;
    outGeometry->chrome.titleBarHeight = 20; /* Standard */
    outGeometry->chrome.titleSeparator = 1;  /* Standard */

    /* Store global origin (top-left of frame) */
    outGeometry->globalOrigin.h = globalFrame.left;
    outGeometry->globalOrigin.v = globalFrame.top;

    /* LOCAL coordinates are always (0, 0, width, height) */
    outGeometry->localContent = window->port.portRect;

    /* Store global rectangles */
    outGeometry->globalFrame = globalFrame;
    outGeometry->globalContent = globalContent;

    return true;
}

/* ============================================================================
 * Geometry Transformation Functions
 * ============================================================================ */

void WM_MoveWindowGeometry(const WindowGeometry* currentGeometry,
                            Point newGlobalOrigin,
                            WindowGeometry* outGeometry) {
    if (!currentGeometry || !outGeometry) return;

    /* Recalculate with new origin, same size and chrome */
    WM_CalculateWindowGeometry(newGlobalOrigin,
                                currentGeometry->contentWidth,
                                currentGeometry->contentHeight,
                                &currentGeometry->chrome,
                                outGeometry);
}

void WM_ResizeWindowGeometry(const WindowGeometry* currentGeometry,
                              short newContentWidth,
                              short newContentHeight,
                              WindowGeometry* outGeometry) {
    if (!currentGeometry || !outGeometry) return;

    /* Recalculate with same origin, new size */
    WM_CalculateWindowGeometry(currentGeometry->globalOrigin,
                                newContentWidth,
                                newContentHeight,
                                &currentGeometry->chrome,
                                outGeometry);
}

/* ============================================================================
 * Convenience Functions
 * ============================================================================ */

const WindowChrome* WM_GetChromeForWindow(WindowPtr window) {
    if (!window) return &kStandardWindowChrome;

    /* Determine chrome based on window kind */
    switch (window->windowKind) {
        case dialogKind:
            return &kDialogWindowChrome;

        case userKind:
        case systemKind:
        default:
            return &kStandardWindowChrome;
    }
}

Boolean WM_ValidateWindowGeometry(const WindowGeometry* geometry) {
    if (!geometry) return false;

    /* Check content dimensions are positive */
    if (geometry->contentWidth <= 0 || geometry->contentHeight <= 0) {
        return false;
    }

    /* Check LOCAL coordinates are (0,0,w,h) */
    if (geometry->localContent.left != 0 || geometry->localContent.top != 0) {
        return false;
    }
    if (geometry->localContent.right != geometry->contentWidth ||
        geometry->localContent.bottom != geometry->contentHeight) {
        return false;
    }

    /* Check global rectangles have positive dimensions */
    if (geometry->globalFrame.right <= geometry->globalFrame.left ||
        geometry->globalFrame.bottom <= geometry->globalFrame.top) {
        return false;
    }
    if (geometry->globalContent.right <= geometry->globalContent.left ||
        geometry->globalContent.bottom <= geometry->globalContent.top) {
        return false;
    }

    /* Check content is inside frame */
    if (geometry->globalContent.left < geometry->globalFrame.left ||
        geometry->globalContent.top < geometry->globalFrame.top ||
        geometry->globalContent.right > geometry->globalFrame.right ||
        geometry->globalContent.bottom > geometry->globalFrame.bottom) {
        return false;
    }

    return true;
}
