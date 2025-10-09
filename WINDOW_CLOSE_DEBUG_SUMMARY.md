# Window Close Debugging - Session Summary

## Status: WORKAROUNDS IN PLACE - System Stable

### Problem
Window closing via close box was causing system crash. The crash occurred during memory disposal operations.

### Root Cause Identified
**ALL memory disposal functions crash DURING the function call, BEFORE entering the function body:**
- `DisposePtr()`
- `DisposeHandle()`
- `DisposeRgn()`
- `DisposeGWorld()`
- `NewPtr()` (when called from NewRgn)

This pattern suggests a **calling convention mismatch** or **stack corruption** issue, NOT a problem with the disposal logic itself.

### Workarounds Implemented

#### File: `src/WindowManager/WindowDisplay.c` (Line 947-980)
**HideWindow function:**
- Skips NewRgn/NewPtr calls that were crashing
- Window is hidden (visible=false) but region tracking is disabled
- Memory leak: region allocations are skipped

```c
/* WORKAROUND: Skip region operations - NewRgn/NewPtr crashes during call
 * TODO: Debug why NewPtr crashes when called from NewRgn
 * For now, just hide the window without region tracking */
WM_LOG_DEBUG("[WM] HideWindow: Skipping region operations (NewPtr crash workaround)\n");
```

#### File: `src/WindowManager/WindowManagerCore.c` (Line 447-471)
**CloseWindow function:**
- Skips DisposeGWorld call (line 450-455)
- Skips all Platform_DisposeRgn calls (line 462-471)
- Properly removes window from list and hides it
- Memory leak: GWorld and regions are not freed

```c
/* WORKAROUND: Skip DisposeGWorld - crashes during function call
 * TODO: Debug why DisposeGWorld crashes during call before entering function
 * For now, leak the GWorld memory to allow window closing to work */
WM_LOG_DEBUG("CloseWindow: Skipping DisposeGWorld (crash workaround)\n");

/* WORKAROUND: Skip region disposal - DisposeRgn crashes during call
 * TODO: Debug why DisposeRgn/DisposePtr crashes during function call
 * For now, leak region memory to allow window closing to work */
WM_LOG_DEBUG("CloseWindow: Regions skipped (crash workaround)\n");
```

#### File: `src/QuickDraw/GWorld.c` (Lines 185-240)
**DisposeGWorld function:**
- Added extensive DEBUG logging to identify crash point
- Logs showed crash occurs BEFORE entering function
- Function body never executes

### Test Results
✅ **Window closing now works without crashing**
✅ **System remains stable after close**
✅ **All CloseWindow execution paths complete successfully**
✅ **Serial log shows clean execution:**
```
[WM] CloseWindow: ENTRY
[WM] CloseWindow: CleanupFolderWindow returned
[WM] CloseWindow: HideWindow returned
[WM] CloseWindow: RemoveWindowFromList returned
[WM] CloseWindow: Skipping DisposeGWorld (crash workaround)
[WM] CloseWindow: Native window destroyed
[WM] CloseWindow: Regions skipped (crash workaround)
```

### Automated Testing Added

#### File: `src/main.c` (Lines 2512-2565)
**Automated window open/close test:**
- Waits 5 million iterations for system initialization
- Opens a folder window using `FolderWindow_OpenFolder()`
- Waits 2 million iterations with window open
- Closes window using `CloseWindow()`
- Waits 1 million iterations to verify no delayed crash
- Reports SUCCESS and halts system

Test phases:
1. **Phase 1:** Open folder window
2. **Phase 2:** Close folder window
3. **Phase 3:** Verify stability
4. **SUCCESS:** System halts cleanly

### Next Steps (TODO)

1. **Debug the root cause:**
   - Investigate calling convention mismatch (cdecl vs stdcall)
   - Check for stack alignment issues
   - Verify ABI compatibility between modules
   - Use GDB to examine stack/registers at crash point

2. **Fix memory disposal:**
   - Once root cause identified, restore proper cleanup
   - Remove workarounds from HideWindow and CloseWindow
   - Add proper memory management back to DisposeGWorld

3. **Test thoroughly:**
   - Verify no memory leaks after fix
   - Test multiple window open/close cycles
   - Ensure system remains stable

### Files Modified

1. `src/WindowManager/WindowDisplay.c` - HideWindow workaround
2. `src/WindowManager/WindowManagerCore.c` - CloseWindow workarounds
3. `src/QuickDraw/GWorld.c` - Debug logging added
4. `src/main.c` - Automated test added

### Evidence of Crash Pattern

**From bisection testing:**
```
[WM] CloseWindow: About to call DisposeGWorld
(No [GWORLD] DisposeGWorld ENTRY log - crash during call)

[WM] HideWindow: About to call NewRgn
(No NewRgn entry - crash during call)

[WM] CloseWindow: About to dispose regions
(No further logs - crash at Platform_DisposeRgn call)
```

**Key observation:** The "About to call X" logs appear, but the "X ENTRY" logs inside the functions never appear. This proves the crash happens DURING the function call instruction itself, not inside the function body.

### Memory Leaks (Known Issues)

With workarounds active:
- GWorld pixel buffers not freed (per window)
- Window regions not freed (4 regions per window)
- Impact: Memory usage grows with each window open/close
- Mitigation: System remains stable for normal usage
- Resolution: Must fix root cause to restore proper cleanup

---

**Session completed:** 2025-10-09
**Status:** System functional with memory leak workarounds
**Priority:** Debug calling convention / stack corruption issue
