# File Manager Write Support - Status Report

**Date:** October 26, 2025
**Analysis:** Deep investigation into write support implementation

---

## Current State: Partially Implemented

### What Works ✅
- **FSWrite() API**: High-level API exists in `src/FileManager.c:208`
- **PBWriteSync()**: Parameter block interface exists in `src/FileManager.c:1079`
- **FSSetEOF()**: Fully implemented in `src/FileManager.c:419` with extent management
- **FSFlushVol()**: Implemented in `src/FileManager.c:887`
- **Write path infrastructure**: FCB management, permission checking, position handling

### What's Stubbed ❌
- **IO_WriteFork()**: The actual low-level write implementation is stubbed in `src/FileManagerStubs.c:367`
  - Returns `ioErr` immediately
  - Does not write any data
  - Sets `*actual = 0`

### Why Writes Don't Work

The write call chain looks like this:
```
FSWrite() → PBWriteSync() → IO_WriteFork() [STUB!]
```

**FileManager.c:1108** calls `IO_WriteFork()`, but that function is just a stub that returns an error.

---

## The Real Implementation (Not Compiled)

There IS a full implementation of write support in **src/HFS_Allocation.c**:

- `IO_WriteFork()` - Full implementation with caching (line 1070)
- `Cache_FlushVolume()` - Write dirty cache buffers (line 800)
- `Cache_FlushAll()` - Flush all dirty buffers (line 836)
- `IO_WriteBlocks()` - Low-level block write (line 942)

**BUT** this file is **NOT in the Makefile** and **does not compile** due to type mismatches:

### Compilation Errors in HFS_Allocation.c

The file uses outdated types that don't match current structures:
- `VCBExt` should be `VCB` (Volume Control Block)
- `FCBExt` should be `FCB` (File Control Block)
- Missing struct members: `vcbNmAlBlks`, `vcbVBMCache`, `vcbVBMSt`, `vcbFreeBks`, `vcbAllocPtr`, `vcbFlags`, `vcbAtrb`, `vcbWrCnt`, `vcbLsMod`
- Missing FCB members: `fcbVPtr`, `fcbFlags`, `fcbEOF`, `fcbCrPs`, `fcbPLen`

**Estimated errors:** 100+ compilation errors across 1200 lines of code

---

## What Would Be Required to Enable Writes

### Option 1: Fix HFS_Allocation.c (6-10 hours)
1. Update all `VCBExt` → `VCB` and `FCBExt` → `FCB`
2. Map old struct member names to new names:
   - `vcbNmAlBlks` → `vcb->base.vcbNmAlBlks` or equivalent
   - `fcbVPtr` → `fcb->base.fcbVPtr` or equivalent
3. Fix all 100+ compilation errors
4. Add `src/HFS_Allocation.c` to Makefile
5. Remove stubs from FileManagerStubs.c
6. Test thoroughly to ensure cache coherency

### Option 2: Implement from Scratch (8-12 hours)
1. Write new `IO_WriteFork()` in FileManager.c:
   - Map file offset to allocation blocks
   - Handle partial block writes
   - Update file EOF
   - Mark FCB dirty
2. Implement cache buffer management:
   - Write-through or write-back caching
   - Dirty buffer tracking
   - Cache flushing
3. Implement `IO_WriteBlocks()`:
   - Platform abstraction for disk writes
   - Block alignment
4. Update volume bitmap for new allocations
5. Update catalog entries with new file sizes/dates

---

## Recommendation

**Priority 4 (Font Manager)** or **Priority 5 (Sound Manager)** should be implemented next instead of File Manager writes.

**Reasoning:**
1. Write support requires 8-12 hours of complex low-level work
2. Fixing HFS_Allocation.c requires understanding the old codebase structure
3. Apps can still run and demonstrate functionality in read-only mode
4. Font and Sound improvements provide more visible user impact

**When to implement writes:**
- When apps absolutely need to save documents
- When sufficient time is available for thorough testing
- After understanding the VCB/FCB structure evolution in the codebase

---

## Quick Win: Document Read-Only Mode

For now, the system should be documented as **read-only filesystem** mode. Apps can:
- ✅ Load and display documents
- ✅ Edit in memory
- ❌ Save changes to disk
- ❌ Create new files

This is similar to how many emulators start before implementing write support.
