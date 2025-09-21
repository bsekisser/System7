/*
 * WindowPlatform.c - Platform implementation for Window Manager
 * Provides platform-specific windowing functions
 */

#include "MacTypes.h"
#include "WindowManager/WindowManager.h"
#include "QuickDraw/QuickDraw.h"

/* External framebuffer and QuickDraw globals */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern GrafPtr g_currentPort;

/* Initialize windowing system */
void Platform_InitWindowing(void) {
    /* Platform windowing is initialized when QuickDraw is initialized */
}

/* Check for Color QuickDraw */
Boolean Platform_HasColorQuickDraw(void) {
    return false;  /* We're using classic QuickDraw for now */
}

/* Initialize a window's graphics port */
Boolean Platform_InitializeWindowPort(WindowPtr window) {
    if (!window) return false;

    /* Initialize the GrafPort part of the window */
    window->port.portBits.baseAddr = (Ptr)framebuffer;
    window->port.portBits.rowBytes = fb_width * 4;  /* Assuming 32-bit color */
    SetRect(&window->port.portBits.bounds, 0, 0, fb_width, fb_height);

    /* Set port rectangle to window bounds */
    window->port.portRect = window->port.portBits.bounds;

    /* Initialize clipping regions */
    if (!window->port.clipRgn) {
        window->port.clipRgn = NewRgn();
    }
    if (!window->port.visRgn) {
        window->port.visRgn = NewRgn();
    }

    RectRgn(window->port.clipRgn, &window->port.portRect);
    RectRgn(window->port.visRgn, &window->port.portRect);

    return true;
}

/* Calculate window regions (structure, content, etc.) */
void Platform_CalculateWindowRegions(WindowPtr window) {
    if (!window) return;

    /* For now, just use the window's portRect for all regions */
    RectRgn(window->strucRgn, &window->port.portRect);
    RectRgn(window->contRgn, &window->port.portRect);
}

/* Create native window (no-op for our framebuffer implementation) */
void Platform_CreateNativeWindow(WindowPtr window) {
    /* No native window system - we draw directly to framebuffer */
}

/* Initialize color window port */
Boolean Platform_InitializeColorWindowPort(WindowPtr window) {
    /* Just use regular port initialization */
    return Platform_InitializeWindowPort(window);
}

/* Cleanup window port */
void Platform_CleanupWindowPort(WindowPtr window) {
    if (!window) return;

    /* Dispose of regions */
    if (window->port.clipRgn) {
        DisposeRgn(window->port.clipRgn);
        window->port.clipRgn = NULL;
    }
    if (window->port.visRgn) {
        DisposeRgn(window->port.visRgn);
        window->port.visRgn = NULL;
    }
}

/* Destroy native window */
void Platform_DestroyNativeWindow(WindowPtr window) {
    /* No native window system */
}

/* Dispose region */
void Platform_DisposeRgn(RgnHandle rgn) {
    DisposeRgn(rgn);
}

/* Invalidate window content */
void Platform_InvalidateWindowContent(WindowPtr window) {
    /* Mark window as needing redraw */
    if (window && window->updateRgn) {
        RectRgn(window->updateRgn, &window->port.portRect);
    }
}

/* Dispose color table */
void Platform_DisposeCTable(CTabHandle ctab) {
    /* Color tables not implemented yet */
}

/* Update window colors */
void Platform_UpdateWindowColors(WindowPtr window) {
    /* Colors not implemented yet */
}

/* Initialize port */
void Platform_InitializePort(GrafPtr port) {
    if (!port) return;

    /* Basic port initialization */
    port->portBits.baseAddr = (Ptr)framebuffer;
    port->portBits.rowBytes = fb_width * 4;
    SetRect(&port->portBits.bounds, 0, 0, fb_width, fb_height);
    port->portRect = port->portBits.bounds;

    /* Initialize regions */
    if (!port->clipRgn) {
        port->clipRgn = NewRgn();
    }
    if (!port->visRgn) {
        port->visRgn = NewRgn();
    }
    RectRgn(port->clipRgn, &port->portRect);
    RectRgn(port->visRgn, &port->portRect);

    /* Initialize patterns */
    extern QDGlobals qd;
    port->bkPat = qd.white;
    port->fillPat = qd.black;
    port->pnLoc.h = 0;
    port->pnLoc.v = 0;
    port->pnSize.h = 1;
    port->pnSize.v = 1;
    port->pnMode = 8; /* patCopy */
    port->pnPat = qd.black;
    port->pnVis = 0;
}

/* Get screen bounds */
void Platform_GetScreenBounds(RgnHandle rgn) {
    Rect screenBounds;
    SetRect(&screenBounds, 0, 0, fb_width, fb_height);
    RectRgn(rgn, &screenBounds);
}

/* Initialize color port */
void Platform_InitializeColorPort(CGrafPtr port) {
    /* Just use regular port initialization */
    Platform_InitializePort((GrafPtr)port);
}

/* Create standard gray pattern */
PixPatHandle Platform_CreateStandardGrayPixPat(void) {
    return NULL;  /* Pixel patterns not implemented yet */
}

/* Create new region */
RgnHandle Platform_NewRgn(void) {
    return NewRgn();
}

/* Get window definition procedure */
Handle Platform_GetWindowDefProc(short procID) {
    /* Window definition procedures not implemented yet */
    static char dummy = 0;
    return (Handle)&dummy;
}

/* WM_ScheduleWindowUpdate is now in WindowDisplay.c */

/* Debug/error logging stubs */
void WM_DEBUG(const char* fmt, ...) { }
void WM_ERROR(const char* fmt, ...) { }

/* CopyPascalString is defined in WindowManagerCore.c */

/* Window drawing functions */
void Platform_SetNativeWindowTitle(WindowPtr window, ConstStr255Param title) {
    /* No native window system - titles drawn manually */
}

void Platform_BeginWindowDraw(WindowPtr window) {
    /* Set port for drawing */
    if (window) {
        SetPort(&window->port);
    }
}

void Platform_EndWindowDraw(WindowPtr window) {
    /* Nothing to do in our implementation */
}

void Platform_PostWindowEvent(WindowPtr window, short eventType) {
    /* Event posting not implemented yet */
}

void Platform_InvalidateWindowFrame(WindowPtr window) {
    /* Mark window as needing redraw */
    if (window && window->updateRgn) {
        RectRgn(window->updateRgn, &window->port.portRect);
    }
}

/* More platform stubs */
void Platform_SendNativeWindowBehind(WindowPtr window, WindowPtr behindWindow) {
    /* No native window ordering */
}

void Platform_GetWindowGrowBoxRect(WindowPtr window, Rect* rect) {
    if (!window || !rect) return;
    /* Standard grow box in bottom right */
    rect->right = window->port.portRect.right;
    rect->bottom = window->port.portRect.bottom;
    rect->left = rect->right - 15;
    rect->top = rect->bottom - 15;
}

void Platform_IntersectRgn(RgnHandle src1, RgnHandle src2, RgnHandle dst) {
    /* Simple implementation - just copy src1 for now */
    if (dst && src1 && *src1) {
        CopyRgn(src1, dst);
    }
}

Boolean Platform_EmptyRgn(RgnHandle rgn) {
    if (!rgn || !*rgn) return true;
    return ((*rgn)->rgnBBox.left >= (*rgn)->rgnBBox.right ||
            (*rgn)->rgnBBox.top >= (*rgn)->rgnBBox.bottom);
}

Boolean Platform_PtInRgn(Point pt, RgnHandle rgn) {
    if (!rgn || !*rgn) return false;
    return (pt.h >= (*rgn)->rgnBBox.left && pt.h < (*rgn)->rgnBBox.right &&
            pt.v >= (*rgn)->rgnBBox.top && pt.v < (*rgn)->rgnBBox.bottom);
}

void Platform_ShowNativeWindow(WindowPtr window) {
    /* No native window system - visibility handled by Window Manager */
}

void WM_InvalidateWindowsBelow(WindowPtr window) {
    /* Mark windows below as needing redraw */
    WindowPtr w = window ? window->nextWindow : NULL;
    while (w) {
        if (w->visible && w->updateRgn) {
            RectRgn(w->updateRgn, &w->port.portRect);
        }
        w = w->nextWindow;
    }
}

void Platform_BringNativeWindowToFront(WindowPtr window) {
    /* No native window system - ordering handled by Window Manager */
}