# 68K Segment Loader - Implementation Summary

## Overview

This is a **portable, ISA-agnostic implementation** of the classic Mac OS System 7.1 segment loader. It faithfully implements CODE resource loading, A5 world construction, jump table management, and lazy segment loading while remaining completely independent of the host CPU architecture.

## Key Features

✅ **Semantic Fidelity**
- Classic CODE resource parsing (CODE 0 metadata + CODE 1..N segments)
- A5 world layout (below/above A5, QuickDraw globals)
- Jump table with lazy-loading stubs
- `_LoadSeg` trap for on-demand segment loading

✅ **Portability**
- No host ISA assumptions (runs on x86, ARM, any architecture)
- Big-endian safe CODE parsing
- Pluggable CPU backend interface
- Abstract relocation records

✅ **Modularity**
- Clean separation: SegmentLoader (logic) ↔ CPUBackend (execution)
- Swappable backends: M68K interpreter, PPC JIT, native modules
- Extensible for CFM, mixed-mode, future formats

## Architecture

```
LaunchApplication()
    ↓
SegmentLoader (ISA-agnostic)
    ├── CodeParser: Big-endian safe CODE 0/N parsing
    ├── A5World: Allocate below/above A5, build jump table
    └── Relocation: Build abstract reloc records
    ↓
ICPUBackend Interface
    ├── CreateAddressSpace / MapExecutable
    ├── SetRegisterA5 / SetStacks
    ├── WriteJumpTableSlot / MakeLazyJTStub
    └── Relocate / EnterAt
    ↓
M68K Interpreter Backend (x86 host)
    ├── 16MB address space
    ├── Software 68K instruction interpretation
    └── Trap handling
```

## Files Delivered

### Headers
```
include/CPU/
  CPUBackend.h         - ICPUBackend interface definition
  M68KInterp.h         - M68K interpreter types

include/SegmentLoader/
  SegmentLoader.h      - Public API, data structures
  CodeParser.h         - CODE parsing, endian helpers
```

### Implementation
```
src/CPU/
  CPUBackend.c                - Backend registry
  m68k_interp/M68KBackend.c   - M68K interpreter backend

src/SegmentLoader/
  SegmentLoader.c      - Core loader (load/unload/resolve)
  CodeParser.c         - CODE 0/N parsing, relocation table builder
  A5World.c            - A5 world construction, jump table setup
```

### Integration
```
src/ProcessMgr/
  ProcessManager.c     - Wired into LaunchApplication()
```

### Documentation
```
docs/
  SegmentLoader_DESIGN.md  - Comprehensive design document
  SegmentLoader_README.md  - This file
```

## API Usage

### Initialize Segment Loader
```c
SegmentLoaderContext* ctx;
OSErr err = SegmentLoader_Initialize(pcb, "m68k_interp", &ctx);
```

### Load CODE 0 and CODE 1
```c
err = EnsureEntrySegmentsLoaded(ctx);
// → Loads CODE 0, constructs A5 world, builds jump table
// → Loads CODE 1, applies relocations, prepares entry point
```

### Get Entry Point
```c
CPUAddr entry;
err = GetSegmentEntryPoint(ctx, 1, &entry);
```

### Launch Application
```c
err = ctx->cpuBackend->EnterAt(ctx->cpuAS, entry, kEnterApp);
// → Transfers control to CODE 1 entry (typically doesn't return)
```

### Lazy Segment Load (on-demand)
```c
err = LoadSegment(ctx, segID);
// → Called automatically by _LoadSeg trap on JT miss
```

## Build Integration

The segment loader is integrated into the main Makefile:

```makefile
C_SOURCES = ...
            src/CPU/CPUBackend.c \
            src/CPU/m68k_interp/M68KBackend.c \
            src/SegmentLoader/SegmentLoader.c \
            src/SegmentLoader/CodeParser.c \
            src/SegmentLoader/A5World.c \
            ...
```

Build with:
```bash
make clean
make
```

## Testing

### Unit Tests (Recommended)
- `ParseCODE0`: Test with fixture CODE 0 blobs
- `ParseCODEN`: Test entry offset, prologue skip
- `BuildRelocationTable`: Verify pattern detection
- `BE_Read16/32`: Test on little/big-endian hosts

### Integration Tests (Recommended)
- **2-Segment App**: CODE 1 calls CODE 2 via JT → lazy load
- **A5 World**: Verify register set, memory layout
- **Relocation**: Check patched addresses after load

### Backend Conformance
- Exercise all ICPUBackend methods
- Verify trap installation/invocation
- Test JT stub → resolved address transition

## Current Status

### ✅ Implemented
- Complete ISA-agnostic segment loader
- Big-endian safe CODE parsing
- A5 world construction
- Jump table with lazy stubs
- Abstract relocation system
- M68K interpreter backend (structure)
- Integration with Process Manager

### 🚧 MVP Limitations
- **M68K_Execute**: Stub only (no actual instruction interpretation yet)
- **Simple Relocations**: Heuristic-based; not all CODE formats
- **No Segment Purging**: Purgeable state exists but no reclaim
- **No CFM Support**: Classic CODE only

### 🔮 Future Work
- Complete M68K interpreter execution loop
- PPC backend (JIT or interpreter)
- CFM-68K support (mixed-mode apps)
- Native module loading ("NATV" pseudo-segments)
- Advanced relocation parsing
- Segment purging policy (LRU)

## CODE Resource Format Reference

### CODE 0 Structure
```
Offset  Size  Field
+0      4     Above A5 size
+4      4     Below A5 size
+8      4     JT size
+12     4     JT offset from A5
+16     ...   Jump table entries (8 bytes each)
```

### Jump Table Entry
```
+0  2   Offset within segment
+2  2   Instruction (0x4EF9 = JMP)
+4  4   Target address
```

### CODE N Structure
```
+0  2   Entry offset
+2  2   Flags
+4  ... Code bytes
```

## Lazy Loading Flow

1. **Initial**: JT entry = `0x3F3C <segID> 0xA9F0 0x4E75` (stub)
2. **Call**: CPU executes stub → `_LoadSeg` trap
3. **Load**: `LoadSegment(segID)` → map code, relocate
4. **Patch**: `WriteJumpTableSlot` → `0x4EF9 <entry>` (JMP)
5. **Retry**: Return from trap → re-execute JT → jump to segment

## Error Codes

```c
segmentLoaderErr    = -700  // Generic error
segmentNotFound     = -701  // CODE resource missing
segmentBadFormat    = -702  // Malformed CODE
segmentRelocErr     = -703  // Relocation failed
segmentA5WorldErr   = -704  // A5 world setup failed
segmentJTErr        = -705  // Jump table error
```

## Portability Guarantees

✅ **No Unaligned Access**: Byte-by-byte reads via `BE_Read16/32`
✅ **No Host Endian Assumptions**: Always big-endian parsing
✅ **No ISA Leaks**: CPU operations via `ICPUBackend` only
✅ **No UB**: Explicit bounds checks, no signed overflow
✅ **No Hard-Coded ISA**: 68K specifics in backend only

## Example: Adding a New Backend

To add a PPC backend:

1. Create `src/CPU/ppc_jit/PPCBackend.c`
2. Implement all `ICPUBackend` methods:
   - `PPC_CreateAddressSpace`
   - `PPC_MapExecutable` (translate 68K → PPC)
   - `PPC_WriteJumpTableSlot` (emit PPC branch)
   - `PPC_Relocate` (apply PPC-style fixups)
   - etc.
3. Register: `CPUBackend_Register("ppc_jit", &gPPCBackend)`
4. Use: `SegmentLoader_Initialize(pcb, "ppc_jit", &ctx)`

**No changes needed** to SegmentLoader.c, CodeParser.c, or A5World.c!

## Design Philosophy

> **"The segment loader knows WHAT to load and WHERE to put it;
> the CPU backend knows HOW to execute it."**

This separation enables:
- Same loader logic for 68K, PPC, x86, ARM, RISC-V
- Easy testing (mock backends)
- Future-proof (new backends without touching loader)

## Credits

Implemented by: System 7.1 Portable Kernel Team
Date: 2025-10-05
Version: 1.0 (MVP)

## References

- Inside Macintosh: Segment Manager
- Inside Macintosh: A5 World and Jump Table
- Inside Macintosh: Memory Manager
- Inside Macintosh: Resource Manager
- Classic Mac CODE resource format specifications
