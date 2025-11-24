/*
 * WindowRegions.c - Safe Region Management Implementation
 *
 * This file implements RAII-style region management that prevents memory leaks.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#include "SystemTypes.h"
#include "WindowManager/WindowRegions.h"
#include "QuickDraw/QuickDraw.h"

/* External QuickDraw region functions */
extern RgnHandle NewRgn(void);
extern void DisposeRgn(RgnHandle rgn);
extern void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn);
extern void RectRgn(RgnHandle rgn, const Rect* r);

/* ============================================================================
 * Auto-Disposing Region Functions
 * ============================================================================ */

AutoRgnHandle WM_NewAutoRgn(void) {
    AutoRgnHandle handle;
    handle.rgn = NewRgn();
    handle.owned = true;
    return handle;
}

AutoRgnHandle WM_WrapRgn(RgnHandle rgn, Boolean takeOwnership) {
    AutoRgnHandle handle;
    handle.rgn = rgn;
    handle.owned = takeOwnership;
    return handle;
}

void WM_DisposeAutoRgn(AutoRgnHandle* handle) {
    if (!handle) return;

    if (handle->owned && handle->rgn) {
        DisposeRgn(handle->rgn);
    }

    handle->rgn = NULL;
    handle->owned = false;
}

RgnHandle WM_ReleaseAutoRgn(AutoRgnHandle* handle) {
    if (!handle) return NULL;

    RgnHandle rgn = handle->rgn;
    handle->owned = false;  /* Transfer ownership to caller */
    return rgn;
}

/* ============================================================================
 * Region Operation Helpers
 * ============================================================================ */

AutoRgnHandle WM_CopyToAutoRgn(RgnHandle srcRgn) {
    AutoRgnHandle handle = WM_NewAutoRgn();

    if (handle.rgn && srcRgn) {
        CopyRgn(srcRgn, handle.rgn);
    }

    return handle;
}

AutoRgnHandle WM_RectToAutoRgn(const Rect* rect) {
    AutoRgnHandle handle = WM_NewAutoRgn();

    if (handle.rgn && rect) {
        RectRgn(handle.rgn, rect);
    }

    return handle;
}
