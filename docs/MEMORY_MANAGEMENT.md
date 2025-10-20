# Memory Management Guidelines

## ⚠️ CRITICAL: Do Not Use Standard C Allocators

**NEVER use `malloc()`, `free()`, `calloc()`, or `realloc()` in application code.**

### Why?

Mac OS System 7 uses a proprietary **Memory Manager** with its own heap structures. The Memory Manager maintains:
- Relocatable blocks (Handles)
- Non-relocatable blocks (Pointers)
- Heap compaction and defragmentation
- Master pointer tables
- Circular linked lists for free blocks

Standard C allocators (`malloc`/`free`) use a **completely separate heap**. Mixing them causes:
- ❌ Heap corruption ("Broken circular link" errors)
- ❌ Memory leaks (blocks tracked by wrong manager)
- ❌ Crashes and system instability
- ❌ Unpredictable behavior

### Root Cause

When you call `malloc()`, it allocates from the C runtime heap.
When you call `free()` on a `NewPtr()` allocation, you corrupt the C heap.
When you call `DisposePtr()` on a `malloc()` allocation, you corrupt the Memory Manager heap.

**The two heaps are incompatible and must never be mixed.**

---

## ✅ Correct Usage

### Basic Allocation

| ❌ WRONG (Standard C) | ✅ CORRECT (Memory Manager) |
|----------------------|----------------------------|
| `void* ptr = malloc(1024);` | `Ptr ptr = NewPtr(1024);` |
| `void* ptr = calloc(1, 1024);` | `Ptr ptr = NewPtrClear(1024);` |
| `void* ptr = calloc(10, sizeof(Type));` | `Ptr ptr = NewPtrClear(10 * sizeof(Type));` |
| `free(ptr);` | `DisposePtr(ptr);` |

### Error Checking

```c
// Memory Manager style
Ptr buffer = NewPtr(1024);
if (!buffer) {
    OSErr err = MemError();
    // Handle error: err will be memFullErr (-108)
    return err;
}

// Use buffer...

DisposePtr(buffer);
```

### Realloc Pattern

`realloc()` doesn't exist in Memory Manager. Use this pattern instead:

```c
// OLD (Standard C):
newPtr = realloc(oldPtr, newSize);

// NEW (Memory Manager):
Ptr newPtr = NewPtr(newSize);
if (!newPtr) {
    return MemError();
}

if (oldPtr) {
    Size copySize = (newSize < oldSize) ? newSize : oldSize;
    BlockMove(oldPtr, newPtr, copySize);
    DisposePtr(oldPtr);
}

// Now use newPtr...
```

### Handles (Relocatable Memory)

For larger allocations that can be moved during heap compaction:

```c
Handle h = NewHandle(32768);  // 32KB
if (!h) return MemError();

// Lock before dereferencing
HLock(h);
Ptr data = *h;
// Use data...
HUnlock(h);

// When done
DisposeHandle(h);
```

---

## 🛡️ Protection Mechanisms

This codebase has **four layers of protection** to prevent malloc/free:

### 1. Compile-Time Prevention

Include `no_stdlib_alloc.h` in your source files:

```c
#include "no_stdlib_alloc.h"
```

This header redefines `malloc`, `free`, `calloc`, and `realloc` to produce **compile errors** with helpful messages:

```
error: 'DO_NOT_USE_MALLOC__USE_NewPtr_INSTEAD' undeclared
```

### 2. Pre-Commit Git Hook

Automatically runs when you commit, checking all `.c` files for violations:

```bash
✗ VIOLATION: src/foo.c uses malloc() - use NewPtr() instead
COMMIT REJECTED: Standard C allocators detected!
```

### 3. Build-Time Verification

The build system checks for violations and fails with a clear error message.

### 4. Code Review

All pull requests are checked for Memory Manager compliance.

---

## 🔧 Memory Manager API Reference

### Non-Relocatable Blocks (Pointers)

```c
Ptr     NewPtr(Size byteCount);              // Allocate
Ptr     NewPtrClear(Size byteCount);         // Allocate and zero
void    DisposePtr(Ptr p);                   // Free
Size    GetPtrSize(Ptr p);                   // Get size
void    SetPtrSize(Ptr p, Size newSize);     // Resize (if possible)
OSErr   MemError(void);                      // Get last error
```

### Relocatable Blocks (Handles)

```c
Handle  NewHandle(Size byteCount);           // Allocate relocatable
Handle  NewHandleClear(Size byteCount);      // Allocate and zero
void    DisposeHandle(Handle h);             // Free
Size    GetHandleSize(Handle h);             // Get size
void    SetHandleSize(Handle h, Size newSize); // Resize
void    HLock(Handle h);                     // Lock (prevent relocation)
void    HUnlock(Handle h);                   // Unlock
```

### Utility Functions

```c
void    BlockMove(const void* src, void* dst, Size count);  // Fast copy
Size    FreeMem(void);                       // Available memory
Size    MaxMem(Size* grow);                  // Maximum contiguous block
void    PurgeMem(Size cbNeeded);             // Purge purgeable blocks
Size    CompactMem(Size cbNeeded);           // Compact heap
```

---

## 🚫 Exception: Memory Manager Implementation

**Only** `src/MemoryMgr/MemoryManager.c` is allowed to use `malloc`/`free`.

This file implements the Memory Manager itself and uses standard C allocators for the **underlying heap storage**. It must define:

```c
#define MEMORY_MANAGER_INTERNAL
```

before including any headers. This exempts it from the compile-time restrictions.

**No other files should ever define this macro.**

---

## 📚 Additional Resources

- **Inside Macintosh: Memory** - Original Apple documentation
- `include/MemoryMgr/MemoryManager.h` - API declarations
- `src/MemoryMgr/MemoryManager.c` - Implementation
- Heap corruption audit report: `MALLOC_AUDIT_REPORT.md`

---

## 🐛 Troubleshooting

### "Broken circular link" Error

**Cause:** Mixed malloc/free with NewPtr/DisposePtr
**Solution:** Audit code for standard C allocators and convert to Memory Manager

### Compile Error: "DO_NOT_USE_MALLOC"

**Cause:** You used `malloc()` in a source file
**Solution:** Change to `NewPtr()`

### Pre-commit Hook Rejection

**Cause:** Committed code contains malloc/free/calloc/realloc
**Solution:** Convert to Memory Manager API before committing

### Memory Leak

**Cause:** Allocated with NewPtr but never called DisposePtr
**Solution:** Ensure all NewPtr/NewHandle calls have matching Dispose calls

---

## ✅ Summary

1. **ALWAYS** use Memory Manager (NewPtr/DisposePtr)
2. **NEVER** use standard C allocators (malloc/free)
3. **INCLUDE** `no_stdlib_alloc.h` in all source files
4. **CHECK** that pre-commit hook is installed
5. **REVIEW** this document when in doubt

**Following these guidelines prevents heap corruption and ensures system stability.**
