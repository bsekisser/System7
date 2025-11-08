/*
 * Control Manager Stub Functions
 * Provides minimal implementations for missing functions
 */

/* Lock stub switch: single knob to disable all quarantined stubs */
#ifdef SYS71_PROVIDE_FINDER_TOOLBOX
#define SYS71_STUBS_DISABLED 1
#endif
/* Disable quarantined control stubs by default to avoid shadowing real implementations */
#ifndef SYS71_STUBS_DISABLED
#define SYS71_STUBS_DISABLED 1
#endif

#include "SystemTypes.h"
#include "ControlManager/ControlManager.h"
#include "ControlManager/ControlInternal.h"
#include "WindowManager/WindowManager.h"
#include "EventManager/EventManager.h"
#include "PatternMgr/pattern_manager.h"
#include <string.h>
#include <stdint.h>

/* Simple memory management stubs */
#if 0  /* Now provided by Memory Manager */
void HLock(Handle h) {
    /* In a kernel environment, handles don't move, so this is a no-op */
    (void)h;
}

void HUnlock(Handle h) {
    /* In a kernel environment, handles don't move, so this is a no-op */
    (void)h;
}
#endif

#if 0  /* Now provided by Memory Manager */
/* Memory Manager stub */
Handle NewHandleClear(Size byteCount) {
    /* Allocate and clear memory */
    Handle h = NewHandle(byteCount);
    if (h) {
        memset(*h, 0, byteCount);
    }
    return h;
}
#endif

/* StillDown() moved to MouseEvents.c - reads gCurrentButtons */

/* Window Manager minimal helpers needed by Control Manager */
ControlHandle _GetFirstControl(WindowPtr window) {
    if (!window) return NULL;
    return window->controlList;
}

void GetWindowBounds(WindowPtr window, Rect *bounds) {
    if (!window || !bounds) return;

    /* Return the actual window bounds from portRect */
    *bounds = window->portRect;

    /* Convert from port-local to global coordinates */
    /* portRect is in local coordinates, so offset by window position */
    /* For now, portRect should already be in correct coordinates */
}

void _SetFirstControl(WindowPtr window, ControlHandle control) {
    if (!window) return;
    window->controlList = control;
}

/* Internal: attach control to the head of a window's control list */
void _AttachControlToWindow(ControlHandle c, WindowPtr w) {
    if (!w || !c) return;
    /* Ensure owner is set */
    (*c)->contrlOwner = w;
    /* Link at head */
    (*c)->nextControl = _GetFirstControl(w);
    _SetFirstControl(w, c);
}

/* RegisterStandardControlTypes now implemented in StandardControls.c */

/* [WM-053] QuickDraw region functions - real implementations in QuickDraw/Regions.c */
#if !defined(SYS71_STUBS_DISABLED)
void EraseRgn(RgnHandle rgn) {
    if (!rgn) return;

    /* Get the actual EraseRect implementation from quickdraw_impl */
    extern void* framebuffer;
    extern uint32_t fb_width;
    extern uint32_t fb_height;
    extern uint32_t fb_pitch;
    extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

    Region* region = (Region*)*rgn;
    const Rect* r = &region->rgnBBox;

    if (!framebuffer || !r) return;

    /* Check if we have a color pattern from Pattern Manager */
    uint32_t* colorPattern = NULL;
    if (PM_GetColorPattern(&colorPattern)) {
        /* Use color pattern - it's an 8x8 array of 32-bit ARGB values */
        /* Clip to screen bounds */
        int left = (r->left < 0) ? 0 : r->left;
        int top = (r->top < 0) ? 0 : r->top;
        int right = (r->right > fb_width) ? fb_width : r->right;
        int bottom = (r->bottom > fb_height) ? fb_height : r->bottom;

        for (int y = top; y < bottom; y++) {
            for (int x = left; x < right; x++) {
                /* Get pattern pixel (8x8 tile) */
                int patRow = (y - r->top) & 7;  /* Wrap at 8 pixels */
                int patCol = (x - r->left) & 7; /* Wrap at 8 pixels */
                uint32_t patColor = colorPattern[patRow * 8 + patCol];

                /* Extract RGB from ARGB (ignore alpha) */
                uint8_t red = (patColor >> 16) & 0xFF;
                uint8_t green = (patColor >> 8) & 0xFF;
                uint8_t blue = patColor & 0xFF;

                uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y * fb_pitch + x * 4);
                *pixel = pack_color(red, green, blue);
            }
        }
    } else {
        /* Fall back to calling the regular EraseRect */
        extern void EraseRect(const Rect* r);
        EraseRect(r);
    }
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* RGBBackColor moved to ColorQuickDraw.c */
