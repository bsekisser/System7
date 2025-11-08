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

/* [WM-053] QuickDraw region functions
 * EraseRgn removed - implemented in QuickDraw/Regions.c:611
 * Real implementation handles patterns and framebuffer erasing */

/* RGBBackColor moved to ColorQuickDraw.c */
