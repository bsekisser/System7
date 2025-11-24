/*
 * WindowRegions.h - Safe Region Management with Auto-Disposal
 *
 * This header defines RAII-style wrappers for QuickDraw regions that prevent
 * memory leaks by ensuring regions are always disposed properly.
 *
 * PROBLEM SOLVED:
 * Previously, NewRgn() allocations scattered throughout window management code
 * could leak if early returns or error paths skipped the corresponding
 * DisposeRgn() call. This was particularly problematic in complex functions
 * like MoveWindow() and DragWindow() with multiple exit points.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef __WINDOW_REGIONS_H__
#define __WINDOW_REGIONS_H__

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Auto-Disposing Region Handle
 * ============================================================================ */

/*
 * AutoRgnHandle - RAII-style region handle with automatic disposal
 *
 * This structure tracks whether a region handle is "owned" and should be
 * disposed when no longer needed. Use WM_DisposeAutoRgn() to clean up,
 * even on early returns.
 *
 * Example usage:
 *   void MyFunction(void) {
 *       AutoRgnHandle tempRgn = WM_NewAutoRgn();
 *       if (!tempRgn.rgn) {
 *           return;  // No leak - rgn is NULL
 *       }
 *
 *       // Use tempRgn.rgn for work...
 *       RectRgn(tempRgn.rgn, &someRect);
 *
 *       // Early return is safe
 *       if (someError) {
 *           WM_DisposeAutoRgn(&tempRgn);
 *           return;  // No leak
 *       }
 *
 *       // Normal cleanup
 *       WM_DisposeAutoRgn(&tempRgn);
 *   }
 */
typedef struct AutoRgnHandle {
    RgnHandle rgn;      /* The actual region handle (may be NULL) */
    Boolean owned;      /* True if we should dispose this region */
} AutoRgnHandle;

/* ============================================================================
 * Auto-Disposing Region Functions
 * ============================================================================ */

/*
 * WM_NewAutoRgn - Create new auto-disposing region
 *
 * Allocates a new region that will be tracked for disposal.
 *
 * Returns: AutoRgnHandle with owned=true and rgn=NewRgn() result
 *          If allocation fails, rgn will be NULL but structure is still valid
 *
 * IMPORTANT: Always call WM_DisposeAutoRgn() when done, even if rgn is NULL
 */
AutoRgnHandle WM_NewAutoRgn(void);

/*
 * WM_WrapRgn - Wrap existing region for auto-disposal
 *
 * Wraps an existing region handle so it will be auto-disposed.
 * Useful when taking ownership of a region from another function.
 *
 * Parameters:
 *   rgn - Existing region handle to wrap
 *   takeOwnership - If true, rgn will be disposed by WM_DisposeAutoRgn()
 *
 * Returns: AutoRgnHandle wrapping the provided region
 */
AutoRgnHandle WM_WrapRgn(RgnHandle rgn, Boolean takeOwnership);

/*
 * WM_DisposeAutoRgn - Dispose auto-disposing region
 *
 * Disposes the region if owned, then marks as no longer owned.
 * Safe to call multiple times on the same AutoRgnHandle.
 * Safe to call with NULL rgn.
 *
 * Parameters:
 *   handle - AutoRgnHandle to dispose (will be modified to owned=false)
 */
void WM_DisposeAutoRgn(AutoRgnHandle* handle);

/*
 * WM_ReleaseAutoRgn - Release ownership without disposing
 *
 * Marks the region as no longer owned, so WM_DisposeAutoRgn() will not
 * dispose it. Useful when transferring ownership to another function.
 *
 * Parameters:
 *   handle - AutoRgnHandle to release (will be modified to owned=false)
 *
 * Returns: The region handle (caller now responsible for disposal)
 */
RgnHandle WM_ReleaseAutoRgn(AutoRgnHandle* handle);

/* ============================================================================
 * Convenience Macros for Common Patterns
 * ============================================================================ */

/*
 * WM_WITH_AUTO_RGN - Declare and auto-dispose region in function scope
 *
 * Declares an AutoRgnHandle that will be automatically disposed when
 * the function returns (requires manual cleanup still, but easier to track).
 *
 * Example:
 *   void MyFunction(void) {
 *       WM_WITH_AUTO_RGN(tempRgn);
 *       if (!tempRgn.rgn) return;
 *
 *       // Use tempRgn.rgn...
 *
 *       WM_CLEANUP_AUTO_RGN(tempRgn);  // Explicit cleanup
 *   }
 */
#define WM_WITH_AUTO_RGN(name) AutoRgnHandle name = WM_NewAutoRgn()

/*
 * WM_CLEANUP_AUTO_RGN - Clean up auto region
 *
 * Disposes an auto region created with WM_WITH_AUTO_RGN.
 */
#define WM_CLEANUP_AUTO_RGN(name) WM_DisposeAutoRgn(&(name))

/* ============================================================================
 * Region Operation Helpers
 * ============================================================================ */

/*
 * WM_CopyToAutoRgn - Copy region into auto-disposing handle
 *
 * Creates a new auto-disposing region and copies source region into it.
 * Handles allocation failure gracefully.
 *
 * Parameters:
 *   srcRgn - Source region to copy from
 *
 * Returns: AutoRgnHandle containing copy (rgn may be NULL if allocation failed)
 */
AutoRgnHandle WM_CopyToAutoRgn(RgnHandle srcRgn);

/*
 * WM_RectToAutoRgn - Create rectangular auto-disposing region
 *
 * Creates a new auto-disposing region from a rectangle.
 *
 * Parameters:
 *   rect - Rectangle to convert to region
 *
 * Returns: AutoRgnHandle containing rectangle region
 */
AutoRgnHandle WM_RectToAutoRgn(const Rect* rect);

#ifdef __cplusplus
}
#endif

#endif /* __WINDOW_REGIONS_H__ */
