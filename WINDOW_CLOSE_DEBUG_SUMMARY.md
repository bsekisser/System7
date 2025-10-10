# Window Close Fix

## Problem
Window closes but won't reopen - `NewWindow()` returns NULL after close.

## Root Cause
`CloseFinderWindow()` was calling `CloseWindow()` then `DisposeWindow()`, but `DisposeWindow()` already calls `CloseWindow()` internally. This was redundant.

More importantly, the close box handler in EventDispatcher calls `CloseWindow()` which doesn't free the WindowRecord memory.

## Fix Applied
Simplified `CloseFinderWindow()` to just call `DisposeWindow()`:
- Before: CloseWindow → DisposeWindow (double-close)  
- After: DisposeWindow only (does everything)

## Still TODO
EventDispatcher.c:311 still calls `CloseWindow()` instead of `DisposeWindow()`, so window memory isn't freed when using close box.
