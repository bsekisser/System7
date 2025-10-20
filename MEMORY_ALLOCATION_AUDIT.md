# Memory Allocation Audit Report

**Date**: 2025-10-19
**Issue**: Heap allocations conflicting with region allocations
**Root Cause**: Long-lived structures allocated on heap can be overwritten by regions

## Executive Summary

The System 7 codebase had a critical memory corruption issue where long-lived data structures allocated on the heap were being overwritten by subsequent region allocations at the same memory addresses. This audit documents the issue, fixes applied, and remaining risks.

## Issues Fixed ✓

### 1. Desktop Icons (CRITICAL - FIXED)
**Commit**: 87e036d
**File**: `src/Finder/desktop_manager.c:857`

**Problem**:
```c
gDesktopIcons = (DesktopItem*)NewPtr(sizeof(DesktopItem) * kMaxDesktopIcons);
```
- Desktop icons allocated at heap address 0x008C7340
- Regions later allocated at same address 0x008C7340
- Icon data overwritten with x86 machine code (8B 87 5D 88)
- Result: Icons displayed as white with corrupted names (` and ])

**Evidence**:
```
[DESKTOP_INIT] gDesktopIcons allocated at 0x008C7340
[DESKTOP_INIT] Created Trash icon: name='Trash' pos=(700,520)
...
[REGION] NewRgn region=0x008C7340  ← CONFLICT!
...
[ARRANGE] Icon 0: type=6129803 (corrupted)
[ARRANGE] Icon 0: name bytes: 8B 87 5D (machine code!)
```

**Fix**:
```c
gDesktopIcons = gDesktopIconStatic;  // Static array at 0x002ABCC0
```

**Result**: ✅ Icons display correctly, no corruption

---

### 2. Window Manager Ports (CRITICAL - FIXED)
**Commit**: e777b58
**Files**:
- `include/WindowManager/WindowManagerInternal.h:415`
- `src/WindowManager/WindowManagerCore.c:677,695-697`

**Problem**:
```c
g_wmState.wMgrPort = (WMgrPort*)NewPtrClear(sizeof(WMgrPort));
g_wmState.wMgrCPort = (CGrafPtr)NewPtrClear(sizeof(CGrafPort));
```
- Permanent structures allocated on heap
- Never freed until shutdown
- Can conflict with regions created during window operations
- Same pattern as desktop icons

**Fix**:
```c
// Added to WindowManagerState:
GrafPort  port;   // Embedded (already existed)
CGrafPort cPort;  // Embedded (newly added)

// In InitializeWMgrPort():
g_wmState.wMgrPort = &g_wmState.port;    // Point to embedded
g_wmState.wMgrCPort = &g_wmState.cPort;  // Point to embedded
```

**Result**: ✅ Windows work correctly, no heap allocations

---

## Remaining Risks (Prioritized)

### HIGH PRIORITY

#### 3. Menu SaveBits - Pixel Buffers
**File**: `src/MenuManager/menu_savebits.c:89`
**Risk**: ⚠️ **HIGH** - Very large allocations

```c
savedBits->bitsData = NewPtr(savedBits->dataSize);
```

**Issue**:
- Size: `width * height * 4` bytes (can be 10-50KB!)
- Example: 200×300 menu = 240,000 bytes
- Created every time menu is shown
- Disposed when menu hidden
- High risk of colliding with regions

**Impact**: Menu pixel corruption, visual glitches

**Recommended Fix**:
- Use static buffer pool (e.g., 4×50KB buffers)
- Or use alternative approach (redraw menus instead of saving)

---

#### 4. GWorld Pixel Buffers
**File**: `src/QuickDraw/GWorld.c:111`
**Risk**: ⚠️ **HIGH** - Very large allocations

```c
Ptr pixelBuffer = NewPtr(bufferSize);  // height * rowBytes
```

**Issue**:
- Size: Can be 100KB+ for large GWorlds
- Used for offscreen drawing
- Long-lived (until GWorld disposed)

**Impact**: Offscreen drawing corruption

**Recommended Fix**:
- Track all GWorld allocations
- Ensure proper disposal
- Consider memory pool for common sizes

---

### MEDIUM PRIORITY

#### 5. Menu List (gMenuList)
**File**: `src/MenuManager/MenuManagerCore.c:812,855`
**Risk**: ⚠️ **MEDIUM** - Global, reallocated

```c
gMenuList = NewPtr(menuBarSize);
```

**Issue**:
- Global menu bar structure
- Reallocated when menus added (line 855)
- IS properly disposed before reallocation
- Still on heap, could conflict

**Mitigation**: Already has proper disposal, lower risk

**Recommended Fix**:
- Convert to static array `static MenuBarList gMenuListStatic[MAX_MENUS]`

---

#### 6. Window Records
**File**: `src/WindowManager/WindowManagerCore.c:727`
**Risk**: ⚠️ **MEDIUM** - Multiple long-lived allocations

```c
WindowPtr window = NewPtrClear(recordSize);
```

**Issue**:
- One allocation per window
- Long-lived (until user closes window)
- Multiple windows = fragmentation
- Regions created during window operations

**Impact**: Window structure corruption

**Recommended Fix**:
- Use static window pool (e.g., max 32 windows)
- Track allocated addresses to detect conflicts

---

#### 7. Folder Window Items
**File**: `src/Finder/folder_window.c:352,416,532,1544`
**Risk**: ⚠️ **MEDIUM** - Dynamic arrays, reallocated

```c
state->items = NewPtr(sizeof(FolderItem) * state->itemCount);
```

**Issue**:
- Allocated for each folder window
- Reallocated when items added (line 1544)
- Size varies with folder contents

**Impact**: Folder display corruption

**Recommended Fix**:
- Static pool per window
- Better reallocation strategy (grow by 2×)

---

### MEDIUM-LOW PRIORITY

#### 8. Auxiliary Window Records
**File**: `src/WindowManager/WindowManagerCore.c:936,945`
**Risk**: ⚠️ **MEDIUM-LOW** - Double allocation

```c
auxRec = NewHandle(sizeof(AuxWinRec*));
(*auxRec) = NewPtr(sizeof(AuxWinRec));
```

**Issue**:
- Handle + Ptr allocation per color window
- Linked list in heap
- Long-lived

**Recommended Fix**:
- Static pool or embed in window record

---

#### 9. Dialog Records
**File**: `src/DialogManager/DialogManagerCore.c:501`
**Risk**: ⚠️ **LOW-MEDIUM**

```c
dialog = NewPtr(sizeof(DialogRecord));
```

**Issue**:
- Long-lived (modal dialogs)
- Similar to windows but fewer instances

**Recommended Fix**:
- Static pool (max 8 dialogs)

---

### LOW PRIORITY

#### 10. Region Scan Data
**File**: `src/QuickDraw/Regions.c:294`
**Risk**: ⚠️ **LOW** - Small but persistent

```c
g_regionRecorder.scanData = NewPtr(1024 * sizeof(SInt16));
```

**Issue**:
- Global buffer, 2KB
- Never freed (memory leak)
- Small but persistent

**Recommended Fix**:
- Make static: `static SInt16 g_scanDataStatic[1024]`

---

## Root Cause Analysis

### The Core Problem

The Memory Manager doesn't track "ownership" of allocated memory. When memory is freed or the allocator loses track, it can be reallocated for a different purpose:

```
Time 0: Desktop icons allocated at 0x008C7340
Time 1: Icons used, data correct
Time 2: [Some operation]
Time 3: Region allocated at 0x008C7340  ← CONFLICT!
Time 4: Icon data now contains region data (corruption)
```

### Why This Happens

1. **No ownership tracking**: Allocator doesn't know "this memory belongs to desktop icons"
2. **Heap fragmentation**: Multiple allocations/deallocations create gaps
3. **Address reuse**: Allocator finds "free" space at previously used address
4. **No bounds checking**: Writing past buffer end corrupts adjacent memory

### Why Static Storage Works

```c
static DesktopItem gDesktopIconStatic[256];  // At 0x002ABCC0 (BSS)
gDesktopIcons = gDesktopIconStatic;          // Never on heap
```

Benefits:
- ✅ Fixed address (never reallocated)
- ✅ In BSS section (separate from heap zone)
- ✅ No fragmentation
- ✅ No deallocation bugs
- ✅ Zero runtime overhead

---

## Architectural Guidelines

### When to Use Static Storage

Use static storage for:
- ✅ Global/system structures (desktop icons, window manager state)
- ✅ Permanent allocations (live entire program lifetime)
- ✅ Fixed-size arrays with known maximum (max 256 icons)
- ✅ Structures accessed very frequently (performance)

### When Heap is OK

Heap is acceptable for:
- ✅ Temporary allocations (properly freed quickly)
- ✅ Unknown/variable sizes (file data)
- ✅ Many small instances (list nodes)
- ✅ Short-lived (created and freed in same function)

### Red Flags

Avoid:
- ❌ Global pointers to heap (`Ptr gMenuList`)
- ❌ Never-freed allocations (`NewPtr` at init, no `DisposePtr`)
- ❌ Very large allocations (50KB+ SaveBits)
- ❌ Reallocation without tracking old address

---

## Testing Recommendations

### Memory Corruption Detection

1. **Add canary values**:
```c
#define CANARY 0xDEADBEEF
struct DesktopItem {
    uint32_t canary_start;
    // ... fields ...
    uint32_t canary_end;
};
```

2. **Periodic validation**:
```c
void ValidateDesktopIcons(void) {
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].canary_start != CANARY) {
            PANIC("Icon %d corrupted!", i);
        }
    }
}
```

3. **Address range checking**:
```c
#define HEAP_START 0x008C0000
#define HEAP_END   0x00900000
if ((uintptr_t)ptr >= HEAP_START && (uintptr_t)ptr < HEAP_END) {
    LOG("WARNING: Critical data on heap at %p", ptr);
}
```

---

## Summary Statistics

**Total Issues Identified**: 10

**Fixed**: 2 (Desktop icons, WMgr ports)
**High Priority Remaining**: 2 (SaveBits, GWorlds)
**Medium Priority**: 4 (Menu list, Windows, Folders, AuxWin)
**Low Priority**: 2 (Dialogs, Region scan data)

**Heap Allocations Eliminated**: 3
**Memory Saved from Heap**: ~600 bytes (moved to static storage)
**Corruption Risks Eliminated**: 2 critical paths

---

## Next Steps

1. **Immediate** (High Priority):
   - Fix SaveBits buffer (use static pool)
   - Audit GWorld allocations and lifetimes

2. **Short Term** (Medium Priority):
   - Convert gMenuList to static
   - Implement window pool
   - Fix folder item reallocation

3. **Long Term**:
   - Add memory corruption detection
   - Implement heap validator
   - Consider separate heaps for different object types

---

## References

- Desktop Icon Corruption Fix: Commit 87e036d
- WMgr Port Fix: Commit e777b58
- Original Investigation: Commit 564d546 (diagnostic code)
- Memory Manager: `src/MemoryMgr/MemoryManager.c`
- Region System: `src/QuickDraw/Regions.c`

---

**Report Generated**: 2025-10-19
**Last Updated**: After WMgr port fix (e777b58)
**Status**: 2/10 critical issues fixed, monitoring ongoing
