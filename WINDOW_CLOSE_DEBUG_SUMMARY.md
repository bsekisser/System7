# Window Close/Reopen Bug - IN PROGRESS ⚠️

## Current Problem
Windows still fail to reopen after being closed. The issue has evolved through multiple fixes:
1. **Phase 1 (FIXED)**: NULL pointer returns - caused by variadic logging corruption
2. **Phase 2 (FIXED)**: Freelist corruption from 100-iteration loop limit
3. **Phase 3 (FIXED)**: Infinite loops from missing NULL checks
4. **Phase 4 (CURRENT)**: Freelist still corrupts after disposal, causing freezes

## Root Causes Identified

### PRIMARY ISSUE (FIXED): Variadic Logging Corruption
**serial_printf() with variadic arguments was crashing/hanging when called from inside DisposePtr()**, preventing the memory manager from freeing window memory.

## Investigation Journey

### Phase 1: Disposal Chain Analysis
- Fixed CloseFinderWindow to call DisposeWindow (not CloseWindow + DisposeWindow)
- Fixed EventDispatcher to call DisposeWindow instead of CloseWindow
- Added extensive logging throughout disposal chain

### Phase 2: The Mystery
- Logs showed: DisposeWindow → DeallocateWindowRecord → "Calling DisposePtr directly" → "DisposePtr returned"
- **But ZERO logs appeared from inside DisposePtr!**
- Even with logging at the very first line of DisposePtr: nothing

### Phase 3: Deep Dive
- Checked for symbol conflicts: Only one DisposePtr symbol (no stubs)
- Disassembled DisposePtr: Confirmed it calls serial_printf at entry
- Verified format strings in binary: All correct
- Upgraded to C23: Made no difference (not a nullptr issue)

### Phase 4: The Breakthrough
- Disassembled DeallocateWindowRecord: Confirmed it calls DisposePtr at 0x129989
- DisposePtr exists, is called, and returns
- **But serial_printf inside it never produces output!**

### Phase 5: The Fix
**Replaced serial_printf() with serial_puts() in DisposePtr**

Result: ✅ Immediate success!
- [DISPOSE] logs now appear
- Memory is properly freed
- Windows can be closed and reopened indefinitely

## Technical Details

### The Bug
In bare-metal environment, serial_printf's variadic argument handling has an ABI/calling convention issue when invoked from certain contexts (specifically from DisposePtr). The varargs mechanism corrupts the stack or registers, causing silent failure.

### The Solution
Use serial_puts() (simple, non-variadic) instead of serial_printf() in DisposePtr. This avoids the varargs issue entirely while still providing adequate debugging output.

## Verification
Serial logs confirm:
```
[DISPOSE] ENTRY
[DISPOSE] gCurrentZone read
[DISPOSE] BlockHeader calculated
[DISPOSE] Calling coalesce_forward
...
[DISPOSE] Complete
```

And critically:
```
[WM] AllocateWindowRecord: calloc returned 0x001c4170
[WM] AllocateWindowRecord: calloc returned 0x0071c098
[WM] DisposeWindow: ENTRY window=0x0071c098
[WM] AllocateWindowRecord: calloc returned 0x007b054b ← SUCCESS!
```

Windows now close and reopen perfectly.

## Additional Improvements
- Upgraded build to C23 (-std=c2x) for better pointer safety
- Fixed logging format (0x%08x with P2UL() instead of %p)
- Added comprehensive debug logging for future troubleshooting

## Lessons Learned
1. In bare-metal environments, variadic functions can have subtle ABI issues
2. Serial logging from different contexts may behave differently
3. Disassembly + symbol table analysis is essential for tracking "ghost" bugs
4. Simple is better: serial_puts > serial_printf for critical paths

---

## CURRENT STATUS (Latest Session)

### Secondary Issues Discovered - Freelist Corruption

**Problem**: After fixing variadic logging, windows still fail to reopen due to freelist corruption during disposal.

**Fixes Applied (src/MemoryMgr/MemoryManager.c)**:

1. **freelist_insert() Loop Limit Bug (FIXED)**
   - Removed arbitrary 100-iteration limit that caused silent failures
   - When freelist grew beyond 100 nodes (common after window disposal with ~40+ freed blocks), insertions would silently fail
   - **Fix**: Use proper circular list traversal with start marker

2. **Missing Bounds Validation (FIXED)**
   - Added `is_valid_freenode()` helper to validate pointers are within zone bounds
   - Prevents crashes from dereferencing corrupt pointers outside valid memory

3. **Infinite Loop Protection (FIXED)**
   - Added 10000-iteration safety limits to `find_fit()` and `MaxMem()`
   - Added pointer validation before dereferencing in circular list traversal
   - Clear freelist when corruption detected to prevent crashes

**Test Results**:
- ✅ No more crashes/reboots
- ✅ Allocations succeed initially
- ❌ Still freezes after first window disposal (freeze point varies)
- ❌ Freelist corruption persists despite all protections

**Current Hypothesis**:
The freelist is still being corrupted during disposal operations, but now the corruption is detected and the freelist is cleared (`z->freeHead = NULL`). This prevents crashes but causes subsequent allocations to fail or freeze as the allocator tries to recover with an empty freelist.

### Next Steps Needed
1. Identify the actual source of freelist corruption (buffer overrun, incorrect size calculations, etc.)
2. Add heap validation function to detect corruption earlier
3. Consider alternative freelist structures (size-class segregated lists)
4. Add magic numbers/checksums to FreeNode structures

**Status: PARTIALLY RESOLVED - Core issues fixed, underlying corruption persists** ⚠️
