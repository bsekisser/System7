# Known Issues and Technical Debt

This document tracks known issues, workarounds, and technical debt in the System 7.1 reimplementation codebase.

## Critical Issues

### 1. Mouse Button Tracking May Get Stuck (TIMEOUT-001)

**Location**: `src/WindowManager/WindowDragging.c:307-352`

**Severity**: Medium (Has safety timeout, but UX degradation)

**Description**: The drag loop occasionally gets stuck waiting for button release even though the button was released. The drag loop uses `StillDown()` to detect when the mouse button is released, but in some cases (particularly with rapid clicks or in QEMU), `StillDown()` continues returning true even after the physical button is released.

**Current Workaround**:
- Safety timeout after 100,000 iterations (~27 minutes at 60Hz) - prevents infinite hangs
- Secondary timeout after 100 iterations without movement (~1.6 seconds at 60Hz) - improved responsiveness
- Timeouts force-exit the drag loop when stuck is detected

**Root Cause Analysis**:
```
DragWindow() -> EventPumpYield() -> ProcessModernInput() -> updates gCurrentButtons
            ↓
            StillDown() -> Button() -> reads gCurrentButtons (volatile UInt8)
                                       ↑
                                       Read from same variable
```

**Investigation Findings**:
1. The architecture is sound: `EventPumpYield()` IS being called every loop iteration
2. `ProcessModernInput()` IS updating `gCurrentButtons`
3. PS2 driver IS detecting button state changes correctly (logs show button events)
4. The issue appears to be **timing-related** rather than a missed update:
   - QEMU mouse emulation may have timing quirks
   - Very rapid click sequences (press-release-press) can confuse the loop
   - The `volatile` keyword ensures no compiler optimization issues

**Likely Cause**: QEMU mouse emulation timing or rapid user clicks causing the loop to sample button state at exactly the wrong moment (between release and next press in a rapid click sequence).

**Mitigation Applied**:
- Reduced no-movement timeout from 1000 to 100 iterations for better responsiveness
- Safety timeouts remain as defense-in-depth

**To Fix Properly** (if needed):
1. Add hysteresis/debouncing to button state detection
2. Track button release timestamp and reject drags shorter than minimum threshold
3. Consider event-driven approach instead of polling in tight loop

**Files Involved**:
- `src/WindowManager/WindowDragging.c` (drag loop with timeout)
- `src/EventManager/MouseEvents.c` (`Button()` implementation)
- `src/EventManager/ModernInput.c` (button state updates)
- `src/Platform/x86/ps2.c` (PS2 hardware interface)

**Testing**: Perform rapid drag operations with quick mouse releases to reproduce. More likely to occur in QEMU than on real hardware.

---

### ✅ 2. Update Event Flow Broken (UPDATE-001) - FIXED in Hot Mess 5

**Previously**: After dragging windows, window content did not redraw. Windows showed empty or stale content after being moved.

**Root Cause**: The Finder's `DoUpdate()` function (src/Finder/finder_main.c:853-885) only handled specific window types (About, GetInfo, Find, Folder). Unknown window types fell through to a no-op default case, never calling `BeginUpdate()`/`EndUpdate()` to clear the update region.

**Investigation**:
- `InvalRgn()` correctly posts `updateEvt` (WindowEvents.c:445)
- Event loop correctly receives and dispatches update events (finder_main.c:484-486)
- `DoUpdate()` was called but did nothing for generic windows
- This was NOT an event system bug, but a missing default handler

**Fix**: Added generic update handler in `DoUpdate()` that calls `BeginUpdate()`, `EraseRect()`, and `EndUpdate()` for unknown window types.

**Impact**: ALL windows now redraw their content after drag/resize operations, not just DISK/TRSH/About windows

**Location**: `src/Finder/finder_main.c:884-900`

---

## Medium Priority Issues

### 3. Desktop Background Window Refilling

**Location**: `src/WindowManager/WindowDisplay.c:133-138`

**Severity**: Low (Visual glitch)

**Description**: Desktop background window (refCon=0) should not be filled with white during `PaintOne()`, as this erases desktop icons.

**Current Fix**: Exception check added to skip filling windows with refCon=0

**Potential Issue**: This is a fragile check - better solution would be a window flag like `kSkipContentFill`.

---

### ✅ 4. Region Lifecycle Management (RESOLVED)

**Previously**: Uncertainty about proper region lifecycle and potential memory leaks.

**Audit Completed**: Comprehensive audit of all NewRgn() calls in WindowManager (January 2025).

**Findings**:
- All 6 temporary region allocations properly disposed
- Window structure regions correctly managed by window lifecycle
- Global regions (grayRgn) intentionally never disposed
- **No memory leaks found**

**Files Audited**:
- WindowDisplay.c: 5 temporary regions - all properly disposed
- WindowManagerHelpers.c: 1 temporary region - properly disposed with error handling
- WindowManagerCore.c: 1 global region - intentionally never disposed
- WindowRegions.c: AutoRgnHandle infrastructure - correct implementation

**Resolution**: No changes needed. All region management is correct. WindowRegions.h provides AutoRgnHandle pattern for future code.

**AutoRgnHandle Conversion - FALSE DIAGNOSIS CORRECTED (2025-01-24)**:
- **Initial attempts** (commits f723621, 0f8364d): Converted temporary regions to AutoRgnHandle but encountered regressions (text rendering outside windows, window dragging broken). Reverted.
- **False conclusion**: Initially believed AutoRgnHandle pattern was fundamentally broken.
- **Root cause discovered**: Bugs were **PRE-EXISTING** from commit 4a68085 "Fix window resize and drag coordinate system bugs" which actually BROKE coordinate handling by forcing portRect to LOCAL (0,0,w,h) instead of preserving position offsets.
- **Resolution** (commit a6964a7): Reverted all WindowManager code to commit 7117509 (last known good state). Both bugs fixed - AutoRgnHandle was never the problem!
- **Status**: AutoRgnHandle pattern is CORRECT and ready for use. Converting temporary regions to AutoRgnHandle is safe and will improve code clarity.

---

### 5. EraseRgn Doesn't Work with Direct Framebuffer

**Location**: `src/WindowManager/WindowEvents.c:621`

**Severity**: Low (Feature limitation)

**Description**: `EraseRgn()` doesn't work correctly with the Direct Framebuffer approach.

**Current Workaround**: Using `InvalRgn()` to trigger repaints instead of erasing regions.

**Impact**: Slightly less efficient, may leave artifacts in some cases.

---

## Low Priority / Technical Debt

### ✅ 6. Dead Code: Disabled Drag State System (FIXED in Hot Mess 5)

**Previously**: 423 lines of unused drag state system code commented out with `#if 0` blocks

**Fix**: Removed all dead code blocks from WindowDragging.c, reducing file from 1280 to 857 lines

**Impact**: Cleaner codebase, 33% reduction in file size, easier maintenance

---

### 7. Missing Features

Several features are noted as incomplete:

- **Color QuickDraw**: `Platform_HasColorQuickDraw()` returns false (WindowPlatform.c:32)
- **ARM64 Port**: Exists but incomplete/untested (noted in Hot Mess 4 release)
- **Many Menu Items**: Remain placeholders
- **Graphics Mode**: Stuck in classic VGA mode

---

## Performance Issues

### ✅ 7. Excessive Screen Flushes During Window Drag (FIXED in Hot Mess 5)

**Previously**: `QDPlatform_FlushScreen()` called twice on every mouse move during window drag, causing severe performance degradation

**Problem**: The drag loop called flush after erasing old XOR outline AND after drawing new outline (800x600 framebuffer flush = ~480KB copied twice per pixel movement)

**Fix**: Removed redundant flush calls from drag loop. XOR operations work directly on framebuffer without needing immediate flush. Single flush after erasing final outline ensures clean transition, then final flush happens when window is repainted at end of drag.

**Impact**: Dramatically improved drag performance - smooth, responsive window movement (changed from pixel-by-pixel stuttering to fluid dragging)

**Location**: `src/WindowManager/WindowDragging.c:375-399, 413-421`

---

### 8. O(n×8) Window Snapping Algorithm

**Location**: `src/WindowManager/WindowDragging.c:1226-1328`

**Severity**: Low (Performance degradation with many windows)

**Description**: Snap-to-window iterates all windows testing 8 edge combinations on EVERY mouse move during drag.

**Impact**: Performance degrades linearly with window count.

**Recommendation**: Pre-compute snap edges in spatial index, cache results between small movements.

---

### 9. No Dirty Rectangle Optimization

**Location**: `src/WindowManager/WindowDisplay.c` (PaintOne/PaintBehind)

**Severity**: Low (Performance)

**Description**: Always repaints entire windows/regions rather than tracking minimal dirty rectangles.

**Impact**: Poor performance with many windows or large windows.

**Recommendation**: Implement dirty rectangle tracking for incremental updates.

---

## Fixed Issues

### ✅ 10. Coordinate System Fragmentation (FIXED in Hot Mess 5)

**Previously**: Manual synchronization of portRect (LOCAL), portBits.bounds (GLOBAL), and regions (GLOBAL) caused frequent bugs.

**Fix**: Implemented `WindowGeometry` abstraction providing atomic coordinate updates.

**Files**:
- `include/WindowManager/WindowGeometry.h`
- `src/WindowManager/WindowGeometry.c`

**Impact**: Eliminates entire class of coordinate corruption bugs.

---

### ✅ 11. Region Memory Leaks (FIXED in Hot Mess 5)

**Previously**: Manual `DisposeRgn()` calls were easily forgotten on error paths.

**Fix**: Implemented `AutoRgnHandle` RAII-style pattern with guaranteed cleanup.

**Files**:
- `include/WindowManager/WindowRegions.h`
- `src/WindowManager/WindowRegions.c`

**Impact**: Prevents region leaks even on early returns.

---

### ✅ 12. Window Resize Coordinate Bug (FIXED in Hot Mess 4)

**Previously**: `portRect` preserved offsets instead of using LOCAL coordinates (0,0,w,h).

**Fix**: Changed `SizeWindow()` to use explicit LOCAL coordinates.

**Impact**: Windows render correctly after resize operations.

---

### ✅ 13. Window Drag Coordinate Bug (FIXED in Hot Mess 4)

**Previously**: Redundant `Platform_CalculateWindowRegions()` call overwrote correct values.

**Fix**: Removed redundant call, added explanatory comment.

**Impact**: Windows render correctly after drag operations.

---

## Contributing

When adding workarounds or discovering new issues:

1. Document the issue in this file
2. Add a comment in the code with the issue ID (e.g., `/* KNOWN ISSUE: UPDATE-001 */`)
3. Describe the root cause if known
4. Note files involved for future investigation
5. Update when fixed or if new information discovered

---

*Last Updated: 2025-01-24 (Hot Mess 5 - performance improvements)*
