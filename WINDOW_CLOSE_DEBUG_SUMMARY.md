# Window Close Memory Leak Investigation

## Problem
Windows close but won't reopen - `NewWindow()` returns NULL after closing a window, indicating memory allocation failure.

## Initial Fixes Applied

### 1. Fixed CloseFinderWindow
Simplified `CloseFinderWindow()` to just call `DisposeWindow()`:
- **Before**: CloseWindow → DisposeWindow (double-close)
- **After**: DisposeWindow only (correct sequence)

### 2. Fixed EventDispatcher Close Box
Changed EventDispatcher.c:311 to call `DisposeWindow()` instead of `CloseWindow()`:
- **Before**: CloseWindow (only hides/cleans up, doesn't free memory)
- **After**: DisposeWindow (calls CloseWindow internally + frees memory)

### 3. Fixed Serial Logging Format
Changed all logging from `%p` to `0x%08x` with `(unsigned int)P2UL()` casting for bare-metal serial compatibility.

### 4. Added Extensive Logging
Added detailed logging to:
- DisposeWindow: Track disposal sequence
- AllocateWindowRecord: Monitor calloc() behavior
- DeallocateWindowRecord: Monitor DisposePtr() execution

### 5. Changed DeallocateWindowRecord
Modified to call `DisposePtr()` directly instead of `free()` to bypass libc wrapper.

## Current Status: UNRESOLVED

### The Mystery
Despite all fixes, windows still fail to reopen after closing. Serial logs show:

1. ✅ DisposeWindow is called and completes successfully
2. ✅ DeallocateWindowRecord calls DisposePtr() and returns
3. ❌ **NO [DISPOSE] logs from DisposePtr() internal code appear**
4. ❌ calloc() returns NULL on next window allocation

### Critical Finding
The DisposePtr() function is being called and returns, BUT its internal logging code never executes:
- Expected: `[DISPOSE] ptr=... block=... size=...`
- Actual: Complete silence from DisposePtr internals

This suggests either:
1. DisposePtr() is returning early before any logging (but why?)
2. serial_printf() fails specifically within DisposePtr()
3. The pointer passed is invalid, triggering early return

### Next Steps
1. Add logging before ALL early returns in DisposePtr() to identify execution path
2. Verify pointer validity before calling DisposePtr()
3. Use GDB to set breakpoint in DisposePtr() and inspect actual execution
4. Check memory zone state and free list integrity
