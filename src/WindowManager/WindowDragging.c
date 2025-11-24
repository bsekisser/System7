#include <string.h>
/*
 * WindowDragging.c - Window Dragging and Positioning Implementation
 *
 * This file implements window dragging, positioning, and movement functions.
 * These functions handle user interaction for moving windows around the screen,
 * including constraint checking, drag feedback, and platform coordination.
 *
 * Key functions implemented:
 * - Window dragging (DragWindow)
 * - Window positioning and movement (MoveWindow)
 * - Drag constraint enforcement
 * - Drag feedback and visual tracking
 * - Multi-monitor and screen boundary handling
 * - Window snapping and alignment
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Window Manager
 */

#include "SystemTypes.h"
#include "System71StdLib.h"
#include "WindowManager/WMLogging.h"
#include "WindowManager/WindowGeometry.h"
#include "WindowManager/WindowRegions.h"
#include "QuickDraw/QuickDraw.h"
#include "MemoryMgr/MemoryManager.h"

#include "WindowManager/WindowManagerInternal.h"
#include <math.h>

/* Forward declarations */
Boolean WM_ValidateWindowPosition(WindowPtr window, const Rect* bounds);
void WM_ConstrainWindowPosition(WindowPtr window, Rect* bounds);
void WM_UpdateWindowVisibility(WindowPtr window);
void WM_OffsetRect(Rect* rect, short deltaH, short deltaV);


/* ============================================================================
 * Dragging Constants and Configuration
 * ============================================================================ */

/* Drag behavior constants */
#define DRAG_THRESHOLD              4    /* Minimum pixels to start drag */
#define SNAP_DISTANCE              8    /* Snap-to-edge distance */
#define SCREEN_EDGE_MARGIN         4    /* Minimum margin from screen edge */
#define TITLE_BAR_HEIGHT           20   /* Height of title bar */
#define TITLE_BAR_DRAG_MARGIN      50   /* Minimum title bar visible */
#define DRAG_UPDATE_INTERVAL       16   /* Update interval in milliseconds */
/* Window size constraints are defined in WindowManagerInternal.h */


static void Local_InvalidateScreenRegion(RgnHandle region);
static Boolean Local_RectsIntersect(const Rect* rect1, const Rect* rect2);

/* Finder About box helpers (avoid direct Finder dependencies elsewhere) */
extern Boolean AboutWindow_IsOurs(WindowPtr w);
extern Boolean AboutWindow_HandleUpdate(WindowPtr w);
static void Local_ApplyWindowSnap(WindowPtr draggedWindow, short* newLeft, short* newTop, short windowWidth, short windowHeight);


/* ============================================================================
 * Window Movement Functions
 * ============================================================================ */

void MoveWindow(WindowPtr theWindow, short hGlobal, short vGlobal, Boolean front) {
    if (theWindow == NULL) return;

    WM_LOG_DEBUG("MoveWindow: Moving window to (%d, %d), front = %s",
             hGlobal, vGlobal, front ? "true" : "false");

    /* CRITICAL: Get current global position from strucRgn, NOT from portRect
     * portRect is ALWAYS in LOCAL coordinates (0,0,width,height)
     * strucRgn contains the GLOBAL screen position */
    Rect currentGlobalBounds;
    if (theWindow->strucRgn && *(theWindow->strucRgn)) {
        currentGlobalBounds = (*(theWindow->strucRgn))->rgnBBox;
    } else {
        /* Fallback: use portRect but this is wrong if already corrupted */
        currentGlobalBounds = theWindow->port.portRect;
    }

    /* Calculate movement offset */
    short deltaH = hGlobal - currentGlobalBounds.left;
    short deltaV = vGlobal - currentGlobalBounds.top;

    /* Check if window actually needs to move */
    if (deltaH == 0 && deltaV == 0) {
        if (front) {
            SelectWindow(theWindow);
        }
        return;
    }

    /* Validate new position */
    Rect newBounds = currentGlobalBounds;
    WM_OffsetRect(&newBounds, deltaH, deltaV);

    if (!WM_ValidateWindowPosition(theWindow, &newBounds)) {
        WM_LOG_DEBUG("MoveWindow: Invalid window position, constraining");
        WM_ConstrainWindowPosition(theWindow, &newBounds);
        deltaH = newBounds.left - currentGlobalBounds.left;
        deltaV = newBounds.top - currentGlobalBounds.top;
    }

    /* Save old structure region for invalidation (using auto-disposal) */
    WM_WITH_AUTO_RGN(oldStrucRgn);
    if (oldStrucRgn.rgn && theWindow->strucRgn) {
        Platform_CopyRgn(theWindow->strucRgn, oldStrucRgn.rgn);
    }

    /* Use WindowGeometry abstraction for atomic coordinate updates
     * This replaces 100+ lines of manual coordinate synchronization */
    WindowGeometry currentGeom, newGeom;
    if (!WM_GetWindowGeometry(theWindow, &currentGeom)) {
        WM_CLEANUP_AUTO_RGN(oldStrucRgn);
        return;  /* Failed to get geometry - window is invalid */
    }

    /* Calculate new global origin from validated newBounds */
    Point newGlobalOrigin;
    newGlobalOrigin.h = newBounds.left;
    newGlobalOrigin.v = newBounds.top;

    /* Calculate new geometry with updated position (size unchanged) */
    WM_MoveWindowGeometry(&currentGeom, newGlobalOrigin, &newGeom);

    /* Validate new geometry */
    if (!WM_ValidateWindowGeometry(&newGeom)) {
        WM_LOG_DEBUG("MoveWindow: New geometry is invalid");
        WM_CLEANUP_AUTO_RGN(oldStrucRgn);
        return;
    }

    /* Apply new geometry atomically - updates ALL coordinate representations */
    WM_ApplyWindowGeometry(theWindow, &newGeom);

    /* Move native platform window using new global position from strucRgn */
    if (theWindow->strucRgn && *(theWindow->strucRgn)) {
        Rect newGlobalBounds = (*(theWindow->strucRgn))->rgnBBox;
        Platform_MoveNativeWindow(theWindow, newGlobalBounds.left, newGlobalBounds.top);
    }

    /* Bring to front if requested */
    if (front) {
        SelectWindow(theWindow);
    }

    /* Invalidate old and new positions */
    if (theWindow->visible) {
        if (oldStrucRgn.rgn) {
            Local_InvalidateScreenRegion(oldStrucRgn.rgn);
        }
        if (theWindow->strucRgn) {
            Local_InvalidateScreenRegion(theWindow->strucRgn);
        }
    }

    /* Clean up (auto-disposal handles region cleanup) */
    WM_CLEANUP_AUTO_RGN(oldStrucRgn);

    /* Update window layering if needed */
    WM_UpdateWindowVisibility(theWindow);

    /* Debug log removed to avoid unused variable when logging is compiled out */
}

/* ============================================================================
 * Window Dragging Implementation
 * ============================================================================ */

void DragWindow(WindowPtr theWindow, Point startPt, const Rect* boundsRect) {
    if (theWindow == NULL) {
        return;
    }

    WM_LOG_DEBUG("DragWindow: Starting drag from (%d, %d)", startPt.h, startPt.v);

    /* Get GLOBAL window bounds from strucRgn - System 7 faithful */
    Rect frameG;
    if (theWindow->strucRgn && *(theWindow->strucRgn)) {
        frameG = (*(theWindow->strucRgn))->rgnBBox;  /* GLOBAL coords */
        WM_LOG_TRACE("DragWindow: frameG=(%d,%d,%d,%d)\n",
                     frameG.top, frameG.left, frameG.bottom, frameG.right);
    } else {
        /* Should never happen if Window Manager initialized properly */
        WM_LOG_ERROR("DragWindow: ERROR - no strucRgn!\n");
        return;
    }

    /* Calculate mouse offset from window origin (GLOBAL coords) */
    Point offset;
    offset.h = startPt.h - frameG.left;
    offset.v = startPt.v - frameG.top;

    const short windowWidth = frameG.right - frameG.left;
    const short windowHeight = frameG.bottom - frameG.top;

    /* Use provided bounds or default to screen minus menubar */
    Rect dragBounds;
    if (boundsRect) {
        dragBounds = *boundsRect;
    } else {
        extern QDGlobals qd;
        dragBounds.top = 20;     /* menubar height */
        dragBounds.left = qd.screenBits.bounds.left;
        dragBounds.bottom = qd.screenBits.bounds.bottom;
        dragBounds.right = qd.screenBits.bounds.right;
    }

    /* Ensure callers cannot allow windows to overlap the menu bar */
    if (dragBounds.top < TITLE_BAR_HEIGHT) {
        dragBounds.top = TITLE_BAR_HEIGHT;
    }

    /* Guard against degenerate rectangles that would reject movement */
    if (dragBounds.bottom <= dragBounds.top) {
        dragBounds.bottom = dragBounds.top + windowHeight;
    }
    if (dragBounds.right <= dragBounds.left) {
        dragBounds.right = dragBounds.left + windowWidth;
    }
    if (dragBounds.bottom - dragBounds.top < windowHeight) {
        dragBounds.bottom = dragBounds.top + windowHeight;
    }
    if (dragBounds.right - dragBounds.left < windowWidth) {
        dragBounds.right = dragBounds.left + windowWidth;
    }

    /* System 7 modal drag loop using StillDown/GetMouse */
    extern Boolean StillDown(void);
    extern void GetMouse(Point* mouseLoc);

    Point ptG;
    Point lastPos = startPt;
    Boolean moved = false;
    Rect oldBounds = frameG;

    /* XOR outline state */
    Rect dragOutline = frameG;
    Boolean outlineDrawn = false;

    /* QuickDraw functions for XOR outline */
    extern void QDPlatform_FlushScreen(void);
    extern void GetWMgrPort(GrafPtr* port);
    extern void SetPort(GrafPtr port);

    /* Set graphics port to Window Manager port for XOR drawing */
    GrafPtr wmPort;
    GetWMgrPort(&wmPort);
    if (wmPort) {
        SetPort(wmPort);
    }

    /* CRITICAL: Erase the window from its current position BEFORE starting XOR drag
     * This prevents leaving white artifacts where the title bar was */
    extern void HideWindow(WindowPtr window);
    extern void ShowWindow(WindowPtr window);
    extern void InvalidateCursor(void);  /* Force cursor redraw */
    Boolean wasVisible = theWindow->visible;
    (void)wasVisible; /* Avoid unused warning; keep window visible during drag */
    /* NOTE: Do not HideWindow/PaintBehind here. Using XOR outline without
     * erasing the window avoids a heap overwrite we are chasing in the
     * hide/paint path. This matches classic XOR-drag behavior visually,
     * and prevents the freeze observed at drag start. */

    /* Invalidate cursor state before drag to prevent stale background artifacts */
    InvalidateCursor();

    /* Main modal drag loop - System 7 idiom with XOR outline feedback
     *
     * KNOWN ISSUE (TIMEOUT-001): Safety timeout to prevent infinite loop if StillDown() gets stuck
     * See docs/KNOWN_ISSUES.md for details.
     *
     * Root cause: Race condition in PS2Controller where button release events may not update
     * gCurrentButtons, causing StillDown() to never return false.
     *
     * TODO: Fix PS2Controller interrupt handling, then remove these timeouts */
    extern void EventPumpYield(void);
    extern void UpdateCursorDisplay(void);
    UInt32 loopCount = 0;
    const UInt32 MAX_DRAG_ITERATIONS = 100000;  /* Safety timeout: ~1666 seconds at 60Hz */

    /* Track iterations without movement to detect stuck loop
     * Reduced threshold from 1000 to 100 to make stuck detection more responsive */
    UInt32 noMovementCount = 0;
    const UInt32 MAX_NO_MOVEMENT_ITERS = 100;  /* ~1.6 seconds at 60Hz */

    while (StillDown() && loopCount < MAX_DRAG_ITERATIONS) {
        loopCount++;
        noMovementCount++;

        /* Poll hardware for new input events (mouse button state) */
        EventPumpYield();

        /* Update cursor display (drag has its own event loop that bypasses main loop) */
        UpdateCursorDisplay();

        GetMouse(&ptG);  /* Returns GLOBAL coords */

        /* Check if stuck in loop without any StillDown() returning false */
        if (noMovementCount > MAX_NO_MOVEMENT_ITERS) {
            /* Force exit if button tracking is completely broken */
            extern Boolean Button(void);
            if (!Button()) {
                /* Button was actually released, break out */
                WM_LOG_WARN("DragWindow: Breaking out of stuck loop - button actually released after %u iterations\n", loopCount);
                break;
            } else {
                /* Still reporting button down after many iterations - might be stuck for real */
                if (noMovementCount > MAX_NO_MOVEMENT_ITERS * 10) {
                    WM_LOG_ERROR("DragWindow: Force exiting stuck loop after %u iterations with no release\n", loopCount);
                    break;  /* Force exit to prevent infinite timeout */
                }
            }
        }

        /* Only process if mouse moved */
        if (ptG.h != lastPos.h || ptG.v != lastPos.v) {
            noMovementCount = 0;  /* Reset counter on movement */

            /* Calculate new window position */
            short newLeft = ptG.h - offset.h;
            short newTop = ptG.v - offset.v;

            /* Constrain to drag bounds */
            if (newLeft < dragBounds.left)
                newLeft = dragBounds.left;
            if (newTop < dragBounds.top)
                newTop = dragBounds.top;
            if (newLeft + windowWidth > dragBounds.right)
                newLeft = dragBounds.right - windowWidth;
            if (newTop + windowHeight > dragBounds.bottom)
                newTop = dragBounds.bottom - windowHeight;

            /* Apply snap to other windows */
            Local_ApplyWindowSnap(theWindow, &newLeft, &newTop, windowWidth, windowHeight);

            /* XOR outline feedback (authentic Mac OS behavior) */
            if (newLeft != dragOutline.left || newTop != dragOutline.top) {
                /* Erase old outline if it exists (XOR erases by redrawing) */
                if (outlineDrawn) {
                    extern void InvertRect(const Rect* rect);
                    WM_LOG_TRACE("DragWindow: Erasing old outline at (%d,%d,%d,%d)\n",
                                 dragOutline.left, dragOutline.top, dragOutline.right, dragOutline.bottom);
                    InvertRect(&dragOutline);  /* Use InvertRect for simple XOR */
                }

                /* Calculate new outline position */
                dragOutline.left = newLeft;
                dragOutline.top = newTop;
                dragOutline.right = newLeft + windowWidth;
                dragOutline.bottom = newTop + windowHeight;

                /* Draw new outline */
                extern void InvertRect(const Rect* rect);
                WM_LOG_TRACE("DragWindow: Drawing new outline at (%d,%d,%d,%d)\n",
                             dragOutline.left, dragOutline.top, dragOutline.right, dragOutline.bottom);
                InvertRect(&dragOutline);  /* Use InvertRect for simple XOR */
                outlineDrawn = true;

                moved = true;
            }

            lastPos = ptG;
        }
    }

    /* Check if we hit the safety timeout */
    if (loopCount >= MAX_DRAG_ITERATIONS) {
        WM_LOG_ERROR("DragWindow: TIMEOUT! Loop iterated %u times, StillDown() never returned false!\n", loopCount);
        WM_LOG_ERROR("DragWindow: This indicates mouse button tracking is broken.\n");
    } else {
        WM_LOG_DEBUG("DragWindow: Exited drag loop normally after %u iterations\n", loopCount);
    }

    /* Erase final outline before moving window (XOR erases by redrawing) */
    if (outlineDrawn) {
        extern void InvertRect(const Rect* rect);
        extern void QDPlatform_FlushScreen(void);
        WM_LOG_TRACE("DragWindow: Erasing final outline\n");
        InvertRect(&dragOutline);
        /* Flush ONCE after erasing final outline to ensure it's cleared before window repaint */
        QDPlatform_FlushScreen();
    }

    /* Invalidate cursor state after drag to force fresh redraw */
    InvalidateCursor();

    if (moved) {
        WM_LOG_DEBUG("DragWindow: Final MoveWindow to (%d,%d)\n", dragOutline.left, dragOutline.top);

        /* Create a region for the old window position to invalidate */
        extern RgnHandle NewRgn(void);
        extern void RectRgn(RgnHandle rgn, const Rect* r);
        extern void DisposeRgn(RgnHandle rgn);
        extern void PaintBehind(WindowPtr startWindow, RgnHandle clobberedRgn);
        extern void PaintOne(WindowPtr window, RgnHandle clobberedRgn);
        extern void CalcVis(WindowPtr window);

        /* Use auto-disposing regions to prevent leaks on early returns */
        WM_WITH_AUTO_RGN(oldRgn);
        if (!oldRgn.rgn) {
            WM_LOG_WARN("DragWindow: Failed to allocate oldRgn\n");
            WM_CLEANUP_AUTO_RGN(oldRgn);
            return;
        }
        RectRgn(oldRgn.rgn, &oldBounds);

        WM_LOG_TRACE("DragWindow: Created oldRgn for bounds (%d,%d,%d,%d)\n",
                     oldBounds.left, oldBounds.top, oldBounds.right, oldBounds.bottom);

        /* Move the window to new position */
        MoveWindow(theWindow, dragOutline.left, dragOutline.top, false);

        /* Recalculate window visibility */
        CalcVis(theWindow);

        /* Calculate the uncovered desktop region: old position minus new position */
        extern void DiffRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
        WM_WITH_AUTO_RGN(uncoveredRgn);
        WM_WITH_AUTO_RGN(newRgn);

        if (!uncoveredRgn.rgn || !newRgn.rgn) {
            WM_LOG_WARN("DragWindow: Failed to allocate regions for uncovered area\n");
            WM_CLEANUP_AUTO_RGN(uncoveredRgn);
            WM_CLEANUP_AUTO_RGN(newRgn);
            WM_CLEANUP_AUTO_RGN(oldRgn);
            return;
        }

        if (theWindow->strucRgn && *theWindow->strucRgn) {
            CopyRgn(theWindow->strucRgn, newRgn.rgn);
        }

        /* Compute: oldRgn - newRgn = region that was uncovered */
        DiffRgn(oldRgn.rgn, newRgn.rgn, uncoveredRgn.rgn);

        WM_LOG_TRACE("DragWindow: Computed uncovered region\n");

        /* Paint the desktop pattern in the uncovered region FIRST */
        extern void GetWMgrPort(GrafPtr* port);
        extern void SetClip(RgnHandle rgn);
        typedef void (*DeskHookProc)(RgnHandle rgn);
        extern DeskHookProc g_deskHook;  /* From sys71_stubs.c */

        GrafPtr savePort;
        GetPort(&savePort);
        GrafPtr wmgrPort;
        GetWMgrPort(&wmgrPort);
        SetPort(wmgrPort);

        /* Set clip to only the uncovered region */
        SetClip(uncoveredRgn.rgn);

        /* Call the desk hook to paint the desktop pattern in the uncovered region */
        if (g_deskHook) {
            WM_LOG_TRACE("DragWindow: Calling DeskHook for uncovered region\n");
            g_deskHook(uncoveredRgn.rgn);
        }

        /* Reset clip */
        extern void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);
        WM_WITH_AUTO_RGN(fullClip);
        if (fullClip.rgn) {
            SetRectRgn(fullClip.rgn, -32768, -32768, 32767, 32767);
            SetClip(fullClip.rgn);
            WM_CLEANUP_AUTO_RGN(fullClip);
        }

        SetPort(savePort);
        WM_LOG_TRACE("DragWindow: Desktop repainted in uncovered region\n");

        /* Repaint windows behind in the uncovered region */
        PaintBehind(theWindow->nextWindow, uncoveredRgn.rgn);
        WM_LOG_TRACE("DragWindow: PaintBehind called for uncovered region\n");

        /* Restore window visibility if it was visible before drag */
        if (wasVisible) {
            theWindow->visible = true;
        }

        /* Repaint the window itself at new position */
        if (theWindow->refCon == 0x54525348 || theWindow->refCon == 0x4449534b) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[DRAGEND] BEFORE PaintOne: portBits.bounds=(%d,%d,%d,%d) portRect=(%d,%d,%d,%d)\n",
                         theWindow->port.portBits.bounds.left, theWindow->port.portBits.bounds.top,
                         theWindow->port.portBits.bounds.right, theWindow->port.portBits.bounds.bottom,
                         theWindow->port.portRect.left, theWindow->port.portRect.top,
                         theWindow->port.portRect.right, theWindow->port.portRect.bottom);
        }

        PaintOne(theWindow, NULL);
        WM_LOG_TRACE("DragWindow: PaintOne called for window at new position\n");

        if (theWindow->refCon == 0x54525348 || theWindow->refCon == 0x4449534b) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[DRAGEND] AFTER PaintOne: portBits.bounds=(%d,%d,%d,%d)\n",
                         theWindow->port.portBits.bounds.left, theWindow->port.portBits.bounds.top,
                         theWindow->port.portBits.bounds.right, theWindow->port.portBits.bounds.bottom);
        }

        /* Immediately draw window content BEFORE screen flush by calling BeginUpdate/app draw/EndUpdate
         * This ensures folder items are visible after drag */
        extern void BeginUpdate(WindowPtr window);
        extern void EndUpdate(WindowPtr window);
        extern Boolean IsFolderWindow(WindowPtr w);
        extern void FolderWindow_Draw(WindowPtr w);
        extern Boolean AboutWindow_IsOurs(WindowPtr w);
        extern Boolean AboutWindow_HandleUpdate(WindowPtr window);

        BeginUpdate(theWindow);
        if (theWindow->refCon == 0x54525348 || theWindow->refCon == 0x4449534b) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[DRAGEND] AFTER BeginUpdate: portBits.bounds=(%d,%d,%d,%d)\n",
                         theWindow->port.portBits.bounds.left, theWindow->port.portBits.bounds.top,
                         theWindow->port.portBits.bounds.right, theWindow->port.portBits.bounds.bottom);
        }

        if (IsFolderWindow(theWindow)) {
            WM_LOG_TRACE("DragWindow: Drawing folder window content\n");
            FolderWindow_Draw(theWindow);
        } else if (AboutWindow_IsOurs(theWindow)) {
            WM_LOG_TRACE("DragWindow: Drawing About window content\n");
            AboutWindow_HandleUpdate(theWindow);
        }

        if (theWindow->refCon == 0x54525348 || theWindow->refCon == 0x4449534b) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[DRAGEND] AFTER FolderWindow_Draw: portBits.bounds=(%d,%d,%d,%d)\n",
                         theWindow->port.portBits.bounds.left, theWindow->port.portBits.bounds.top,
                         theWindow->port.portBits.bounds.right, theWindow->port.portBits.bounds.bottom);
        }

        EndUpdate(theWindow);
        WM_LOG_TRACE("DragWindow: Window content drawn\n");

        /* Clean up regions (auto-disposal) */
        WM_LOG_TRACE("DragWindow: About to cleanup regions\n");
        WM_CLEANUP_AUTO_RGN(uncoveredRgn);
        WM_CLEANUP_AUTO_RGN(newRgn);
        WM_CLEANUP_AUTO_RGN(oldRgn);
        WM_LOG_TRACE("DragWindow: Region cleanup completed\n");

        /* Force screen update */
        extern void QDPlatform_FlushScreen(void);
        QDPlatform_FlushScreen();

        if (wasVisible) {
            theWindow->visible = true;
        }
    }
    else if (wasVisible) {
        /* Restore window visibility even if it never moved */
        theWindow->visible = true;
        PaintOne(theWindow, NULL);
        if (theWindow->contRgn) {
            extern void InvalRgn(RgnHandle badRgn);
            GrafPtr oldPort;
            GetPort(&oldPort);
            SetPort((GrafPtr)theWindow);
            InvalRgn(theWindow->contRgn);
            SetPort(oldPort);
        }
    }

    WM_LOG_TRACE("DragWindow EXIT: moved=%d\n", moved);
}


/* ============================================================================
 * Drag State Management
 * ============================================================================ */



/* ============================================================================
 * Drag Feedback Management
 * ============================================================================ */




/* ============================================================================
 * Position Calculation and Constraints
 * ============================================================================ */






/* ============================================================================
 * Window Position Validation
 * ============================================================================ */

Boolean WM_ValidateWindowPosition(WindowPtr window, const Rect* bounds) {
    if (window == NULL || bounds == NULL) return false;

    /* Check for valid rectangle */
    if (!WM_VALID_RECT(bounds)) {
        WM_DEBUG("WM_ValidateWindowPosition: Invalid rectangle");
        return false;
    }

    /* Check minimum window size */
    short width = bounds->right - bounds->left;
    short height = bounds->bottom - bounds->top;

    if (width < MIN_WINDOW_WIDTH || height < MIN_WINDOW_HEIGHT) {
        WM_DEBUG("WM_ValidateWindowPosition: Window too small (%dx%d)", width, height);
        return false;
    }

    /* Check maximum window size */
    if (width > MAX_WINDOW_WIDTH || height > MAX_WINDOW_HEIGHT) {
        WM_DEBUG("WM_ValidateWindowPosition: Window too large (%dx%d)", width, height);
        return false;
    }

    /* Check screen bounds */
    Rect screenBounds;
    Platform_GetScreenBounds(&screenBounds);

    /* Ensure some part of title bar is visible */
    Rect titleBarRect = *bounds;
    titleBarRect.top -= TITLE_BAR_HEIGHT;
    titleBarRect.bottom = bounds->top;

    if (!Local_RectsIntersect(&titleBarRect, &screenBounds)) {
        WM_DEBUG("WM_ValidateWindowPosition: Title bar not visible");
        return false;
    }

    return true;
}

void WM_ConstrainWindowPosition(WindowPtr window, Rect* bounds) {
    if (window == NULL || bounds == NULL) return;

    WM_DEBUG("WM_ConstrainWindowPosition: Constraining window position");

    /* Constrain size first */
    short width = bounds->right - bounds->left;
    short height = bounds->bottom - bounds->top;

    if (width < MIN_WINDOW_WIDTH) {
        bounds->right = bounds->left + MIN_WINDOW_WIDTH;
    } else if (width > MAX_WINDOW_WIDTH) {
        bounds->right = bounds->left + MAX_WINDOW_WIDTH;
    }

    if (height < MIN_WINDOW_HEIGHT) {
        bounds->bottom = bounds->top + MIN_WINDOW_HEIGHT;
    } else if (height > MAX_WINDOW_HEIGHT) {
        bounds->bottom = bounds->top + MAX_WINDOW_HEIGHT;
    }

    /* Constrain position to screen */
    Rect screenBounds;
    Platform_GetScreenBounds(&screenBounds);

    /* Ensure title bar is visible */
    if (bounds->top > screenBounds.bottom - TITLE_BAR_HEIGHT) {
        short offset = (screenBounds.bottom - TITLE_BAR_HEIGHT) - bounds->top;
        bounds->top += offset;
        bounds->bottom += offset;
    }

    if (bounds->top < screenBounds.top - (height - TITLE_BAR_HEIGHT)) {
        short offset = (screenBounds.top - (height - TITLE_BAR_HEIGHT)) - bounds->top;
        bounds->top += offset;
        bounds->bottom += offset;
    }

    /* Ensure some part of window is horizontally visible */
    if (bounds->right < screenBounds.left + TITLE_BAR_DRAG_MARGIN) {
        short offset = (screenBounds.left + TITLE_BAR_DRAG_MARGIN) - bounds->right;
        bounds->left += offset;
        bounds->right += offset;
    }

    if (bounds->left > screenBounds.right - TITLE_BAR_DRAG_MARGIN) {
        short offset = (screenBounds.right - TITLE_BAR_DRAG_MARGIN) - bounds->left;
        bounds->left += offset;
        bounds->right += offset;
    }

    WM_DEBUG("WM_ConstrainWindowPosition: Constrained to (%d, %d, %d, %d)",
             bounds->left, bounds->top, bounds->right, bounds->bottom);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

static Boolean Local_RectsIntersect(const Rect* rect1, const Rect* rect2) {
    if (rect1 == NULL || rect2 == NULL) return false;

    return !(rect1->right <= rect2->left ||
             rect1->left >= rect2->right ||
             rect1->bottom <= rect2->top ||
             rect1->top >= rect2->bottom);
}

static void Local_InvalidateScreenRegion(RgnHandle rgn) {
    if (rgn == NULL || !*rgn) return;

    WM_DEBUG("Local_InvalidateScreenRegion: Invalidating screen region");

    /* Repaint windows behind this region to erase the old window chrome/content
     * This is critical after a drag operation to remove the ghost image at the old position */
    extern void PaintBehind(WindowPtr startWindow, RgnHandle clobberedRgn);
    extern WindowManagerState* GetWindowManagerState(void);

    WindowManagerState* wmState = GetWindowManagerState();
    if (wmState && wmState->windowList) {
        /* PaintBehind will redraw all windows from the start of the list
         * The region parameter tells PaintBehind what area was affected */
        PaintBehind(wmState->windowList, rgn);
    }
}

/**
 * Local_ApplyWindowSnap - Apply snap-to-window edges during drag
 * Snaps the dragged window to edges of other visible windows if within SNAP_DISTANCE
 */
static void Local_ApplyWindowSnap(WindowPtr draggedWindow, short* newLeft, short* newTop,
                                   short windowWidth, short windowHeight) {
    if (!draggedWindow || !newLeft || !newTop) return;

    extern WindowManagerState* GetWindowManagerState(void);
    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState || !wmState->windowList) return;

    /* Calculate dragged window edges */
    short dragRight = *newLeft + windowWidth;
    short dragBottom = *newTop + windowHeight;

    /* Best snap distance found so far (closest edge) */
    short bestSnapDist = SNAP_DISTANCE + 1;
    short snapDeltaH = 0;
    short snapDeltaV = 0;

    /* Iterate through all windows in the window list */
    WindowPtr otherWindow = wmState->windowList;
    while (otherWindow) {
        /* Skip the dragged window itself and invisible windows */
        if (otherWindow != draggedWindow && otherWindow->visible && otherWindow->strucRgn) {
            RgnHandle strucRgn = otherWindow->strucRgn;
            if (*strucRgn) {
                Rect otherBounds = (*strucRgn)->rgnBBox;

                /* Check horizontal snapping (left/right edges) */
                /* Snap dragged left edge to other right edge */
                short dist = (*newLeft > otherBounds.right) ? (*newLeft - otherBounds.right) : (otherBounds.right - *newLeft);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = otherBounds.right - *newLeft;
                    snapDeltaV = 0;
                }

                /* Snap dragged right edge to other left edge */
                dist = (dragRight > otherBounds.left) ? (dragRight - otherBounds.left) : (otherBounds.left - dragRight);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = otherBounds.left - dragRight;
                    snapDeltaV = 0;
                }

                /* Snap dragged left edge to other left edge (align) */
                dist = (*newLeft > otherBounds.left) ? (*newLeft - otherBounds.left) : (otherBounds.left - *newLeft);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = otherBounds.left - *newLeft;
                    snapDeltaV = 0;
                }

                /* Snap dragged right edge to other right edge (align) */
                dist = (dragRight > otherBounds.right) ? (dragRight - otherBounds.right) : (otherBounds.right - dragRight);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = otherBounds.right - dragRight;
                    snapDeltaV = 0;
                }

                /* Check vertical snapping (top/bottom edges) */
                /* Snap dragged top edge to other bottom edge */
                dist = (*newTop > otherBounds.bottom) ? (*newTop - otherBounds.bottom) : (otherBounds.bottom - *newTop);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = 0;
                    snapDeltaV = otherBounds.bottom - *newTop;
                }

                /* Snap dragged bottom edge to other top edge */
                dist = (dragBottom > otherBounds.top) ? (dragBottom - otherBounds.top) : (otherBounds.top - dragBottom);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = 0;
                    snapDeltaV = otherBounds.top - dragBottom;
                }

                /* Snap dragged top edge to other top edge (align) */
                dist = (*newTop > otherBounds.top) ? (*newTop - otherBounds.top) : (otherBounds.top - *newTop);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = 0;
                    snapDeltaV = otherBounds.top - *newTop;
                }

                /* Snap dragged bottom edge to other bottom edge (align) */
                dist = (dragBottom > otherBounds.bottom) ? (dragBottom - otherBounds.bottom) : (otherBounds.bottom - dragBottom);
                if (dist <= SNAP_DISTANCE && dist < bestSnapDist) {
                    bestSnapDist = dist;
                    snapDeltaH = 0;
                    snapDeltaV = otherBounds.bottom - dragBottom;
                }
            }
        }

        otherWindow = otherWindow->nextWindow;
    }

    /* Apply the best snap found */
    if (bestSnapDist <= SNAP_DISTANCE) {
        *newLeft += snapDeltaH;
        *newTop += snapDeltaV;
    }
}

/* Platform functions are defined in Platform/WindowPlatform.c */
