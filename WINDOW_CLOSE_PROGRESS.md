# Window Close Debugging Progress

## Problem Statement
Windows could not be closed - clicking the close box caused system freezes or crashes.

## Root Causes Identified and Fixed

### 1. ✅ FIXED: PIC/PIE Compilation Issue
- **Problem**: GCC on Ubuntu 24.04 compiles with `--enable-default-pie` by default, causing PLT-based function calls that require a dynamic linker
- **Solution**: Added `-fno-pic -fno-pie` to CFLAGS and `-no-pie` to LDFLAGS in Makefile
- **Status**: RESOLVED - No more PLT crashes

### 2. ✅ FIXED: Stub Function Override
- **Problem**: Stub implementations of malloc/calloc/realloc in the old `all_stubs.c` and `ultra_stubs.c` files were being linked instead of real implementations in `MemoryManager.c`
- **Symptoms**: calloc() always returned NULL, causing all window allocations to fail
- **Solution**: Commented out stub implementations in both stub files
- **Status**: RESOLVED - Real memory manager functions now properly linked

### 3. ✅ FIXED: serial_puts() Clobbering Return Values
- **Problem**: Calling `serial_puts()` immediately before `return result` in NewPtr() was clobbering the %eax register containing the return value
- **Symptoms**: NewPtr() calculated a valid pointer but malloc() received NULL
- **Solution**: Removed serial_puts() call from NewPtr's return path
- **Status**: RESOLVED - NewPtr now properly returns allocated memory

### 4. ❌ ACTIVE: Free List Corruption

**Current State**:
- First window opens successfully ✅
- CloseWindow executes completely ✅
- All disposal steps complete (HideWindow, RemoveWindowFromList, DisposeGWorld, destroy native window, dispose regions) ✅
- BUT: freelist_insert() detects infinite loop during memory deallocation ❌
- Memory is NOT added back to free list ❌
- Subsequent window allocations fail due to insufficient free memory ❌

**Evidence from logs**:
```
[WM] CloseWindow: Regions disposed
freelist_insert: ENTRY
freelist_insert: checking freeHead
freelist_insert: starting loop
freelist_insert: INFINITE LOOP!   <-- Loop counter triggers
freelist_insert: ENTRY             <-- Next allocation attempt
```

**Root Cause**:
The circular doubly-linked free list has become corrupted. The loop in freelist_insert():

```c
while (it->next != z->freeHead && it->next < n) {
    it = it->next;
    loop_count++;
    if (loop_count > 100) {
        serial_puts("freelist_insert: INFINITE LOOP!\n");
        return;  /* Prevent infinite loop */
    }
}
```

Should terminate when wrapping back to `freeHead`, but doesn't, indicating:
- Circular list doesn't include freeHead, OR
- Node pointers are corrupted, OR
- Block sizes/addresses are invalid

**Impact**:
- The infinite loop protection prevents crashes ✅
- BUT also prevents proper memory deallocation ❌
- Memory leaks on every window close ❌
- Eventually exhausts available memory ❌

## Next Steps

1. **Investigate free list structure**:
   - Add detailed logging to freelist_insert to show node addresses
   - Trace prev/next pointers to identify where corruption occurs
   - Check if blocks overlap or have invalid sizes

2. **Check disposal order**:
   - Verify regions are being disposed in correct order
   - Check if double-frees are occurring
   - Ensure block headers aren't corrupted during disposal

3. **Validate heap integrity**:
   - Add heap validation before/after window operations
   - Check block alignment and size consistency
   - Verify prevSize fields match actual previous blocks

## Test Results

**Boot**: ✅ System boots successfully
**First Window Open**: ✅ Opens and displays correctly
**Window Close**: ⚠️ Completes but corrupts free list
**Subsequent Window Open**: ❌ Fails with "allocation failed"

## Files Modified

- `Makefile` - Added PIC/PIE disable flags
- `src/all_stubs.c` - Removed from the tree (legacy stub implementations deleted)
- `src/ultra_stubs.c` - Removed from the tree (legacy stub implementations deleted)
- `src/MemoryMgr/MemoryManager.c` - Removed serial_puts from return path, added debug tracing
- `src/WindowManager/WindowDisplay.c` - Extensive HideWindow debug logging
- `src/WindowManager/WindowManagerCore.c` - CloseWindow debug logging

## Git Commits

1. `45ae75d` - Add extensive debugging for window close freeze investigation
2. `3bbed5b` - Fix calloc stub override and serial_puts return value clobbering
