# Segment Loader Design Document

## Overview

This document describes the design and architecture of the **portable 68K Segment Loader** for the System 7.1 kernel. The segment loader implements classic Mac OS semantics for loading and executing CODE resources while maintaining complete ISA independence through a pluggable CPU backend interface.

## Goals

1. **Semantic Fidelity**: Match System 7.1 segment loading behavior (CODE resources, A5 world, jump table, lazy loading)
2. **Portability**: No host ISA assumptions; run on x86, ARM, or any host architecture
3. **Modularity**: Clean separation between segment loading logic and CPU execution
4. **Extensibility**: Support future backends (PPC JIT, native modules, CFM)

## Architecture

### Layered Design

The implementation uses strict architectural layering to achieve ISA independence:

```
┌─────────────────────────────────────────────────────────┐
│  Process Manager (LaunchApplication)                    │
│  - Calls segment loader to load CODE 0/1                │
│  - Transfers control to application entry point         │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│  Segment Loader (ISA-Agnostic)                         │
│  - CODE resource parsing (big-endian safe)             │
│  - A5 world construction (portable layout math)        │
│  - Jump table management                               │
│  - Relocation table building                           │
│                                                         │
│  Files: SegmentLoader.c, CodeParser.c, A5World.c       │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│  CPU Backend Interface (ICPUBackend)                   │
│  - CreateAddressSpace / DestroyAddressSpace            │
│  - MapExecutable / UnmapExecutable                     │
│  - SetRegisterA5 / SetStacks                           │
│  - WriteJumpTableSlot / MakeLazyJTStub                 │
│  - Relocate / EnterAt                                  │
│                                                         │
│  Files: CPUBackend.h, CPUBackend.c                      │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│  M68K Interpreter Backend (x86 host)                   │
│  - Software interpretation of 68K instructions         │
│  - 16MB address space allocation                       │
│  - Trap handling infrastructure                        │
│                                                         │
│  Files: M68KBackend.c, M68KInterp.h                    │
└─────────────────────────────────────────────────────────┘
```

### Key Principles

1. **No ISA Leakage Upward**: Upper layers (SegmentLoader) never directly manipulate CPU registers, instruction encoding, or memory layout. All ISA-specific operations go through ICPUBackend.

2. **Big-Endian Safe Parsing**: CODE resources are always big-endian (68K format) regardless of host architecture. Use `BE_Read16/BE_Read32` helpers.

3. **Opaque Types**: CPU addresses (`CPUAddr`), address spaces (`CPUAddressSpace`), and code handles (`CPUCodeHandle`) are opaque to the segment loader.

4. **Abstract Relocations**: Relocation tables use ISA-neutral records (`RelocTable`, `RelocEntry`) that describe *what* to patch, not *how* to encode the patch.

## CODE Resource Format

### CODE 0: A5 World Metadata

CODE 0 is a special resource containing metadata for the A5 world:

```
Offset  Size  Field
+0      4     Above A5 size (bytes above A5: jump table + params)
+4      4     Below A5 size (bytes below A5: app globals)
+8      4     JT size (jump table size in bytes)
+12     4     JT offset from A5
+16     ...   Jump table data (8 bytes per entry)
```

**Jump Table Entry Format** (8 bytes):

```
Offset  Size  Field
+0      2     Offset within segment (or stub)
+2      2     Instruction word (0x4EF9 = JMP for 68K)
+4      4     Target address (to be patched at load time)
```

### CODE N: Executable Segments

CODE resources 1, 2, ... contain executable code:

```
Offset  Size  Field
+0      2     Entry offset (offset to entry point)
+2      2     Flags/version
+4      ...   Code bytes
```

Some linkers add a prologue:

```
+0      2     0x3F3C (MOVE.W #imm,-(SP))
+2      2     Segment number
+4      2     0xA9F0 (_LoadSeg trap)
```

The loader skips this prologue (detected via pattern matching) before mapping the real executable code.

## A5 World Layout

The A5 world is a classic 68K addressing scheme using the A5 register as a base pointer:

```
Memory Layout:
┌────────────────────────────┐
│  Below A5 Area             │  ← a5BelowBase
│  (Application Globals)     │
│  (QuickDraw Globals)       │
├────────────────────────────┤
│  A5 Register Value         │  ← a5Base = a5BelowBase + a5BelowSize
├────────────────────────────┤
│  Above A5 Area             │  ← a5AboveBase (typically == a5Base)
│  (Jump Table)              │  ← jtBase = a5Base + jtOffsetFromA5
│  (Parameters)              │
└────────────────────────────┘
```

**Construction Steps** (in `InstallA5World`):

1. Allocate `a5BelowSize` bytes via `backend->AllocateMemory` → `a5BelowBase`
2. Calculate `a5Base = a5BelowBase + a5BelowSize`
3. Allocate `a5AboveSize` bytes → `a5AboveBase`
4. Calculate `jtBase = a5Base + jtOffsetFromA5`
5. Call `backend->SetRegisterA5(a5Base)`
6. Zero-initialize both areas
7. Build jump table with lazy stubs

### Why A5?

In classic 68K Macs, the A5 register serves as a global pointer for:
- **Negative offsets**: Application globals (e.g., `-A5[0x100]` = global variable)
- **Positive offsets**: Jump table (e.g., `A5[0x20]` = jump table entry)

This allows position-independent access to globals and inter-segment calls.

## Jump Table and Lazy Loading

### Jump Table Purpose

The jump table enables **inter-segment calls** without hard-coding segment addresses. A call to function `Foo` in segment 5:

```
JSR (A5 + JT_offset_for_Foo)
```

At runtime, the JT entry points to the actual function in segment 5. If segment 5 isn't loaded, the JT entry contains a **lazy stub** that triggers `_LoadSeg`.

### Lazy Stub Format (68K)

```
+0  0x3F3C  MOVE.W #segID,-(SP)   ; Push segment ID
+2  segID   (16-bit segment number)
+4  0xA9F0  _LoadSeg trap          ; Trigger loader
+6  0x4E75  RTS                    ; Return
```

### Lazy Load Flow

1. **Initial State**: JT entry contains lazy stub (created by `MakeLazyJTStub`)
2. **First Call**: CPU executes stub → `_LoadSeg` trap → `LoadSegment(segID)`
3. **Segment Load**:
   - Load CODE resource
   - Map into address space
   - Apply relocations
   - Get entry address
4. **JT Patch**: Call `WriteJumpTableSlot` to replace stub with `JMP <entry>`
5. **Retry**: Return from trap; CPU re-executes JT entry (now patched) → jumps to segment

## Relocation System

### Portable Relocation Records

Relocations are described abstractly:

```c
typedef enum {
    kRelocAbsSegBase,   // Absolute segment base address
    kRelocA5Relative,   // A5-relative data access
    kRelocJTImport,     // Jump table import
    kRelocPCRel16,      // PC-relative 16-bit
    kRelocPCRel32,      // PC-relative 32-bit
    kRelocSegmentRef    // Reference to another segment
} RelocKind;

typedef struct {
    RelocKind kind;
    UInt32 atOffset;      // Offset within segment to patch
    SInt32 addend;        // Addend to apply
    UInt16 targetSegment; // Target segment (if applicable)
    UInt16 jtIndex;       // JT index (if kRelocJTImport)
} RelocEntry;
```

### Relocation Process

1. **Build Phase** (`BuildRelocationTable` in CodeParser.c):
   - Scan CODE segment for relocation patterns
   - Detect JMP/JSR instructions (0x4EF9/0x4EB9)
   - Heuristically classify as JT import or segment reference
   - Build portable `RelocTable`

2. **Apply Phase** (`backend->Relocate` in M68KBackend.c):
   - For each `RelocEntry`:
     - Calculate target address based on kind
     - Write target to `atOffset` in code (big-endian)
   - Example: `kRelocAbsSegBase` → patch with `segBase + addend`

### Why ISA-Neutral?

Future backends (PPC, native) can have different instruction encodings, but the *semantic* need to patch addresses is the same. The backend's `Relocate` method handles ISA-specific encoding.

## CPU Backend Interface

### Design Rationale

The ICPUBackend interface abstracts:
- **Memory Management**: Address space creation, executable mapping
- **Register State**: A5, stacks (USP/SSP)
- **Code Patching**: Jump table slots, lazy stubs
- **Execution Control**: Entry point invocation
- **Relocations**: ISA-specific fixups

### M68K Interpreter Backend

The reference implementation (`M68KBackend.c`) provides:

- **Address Space**: 16MB flat memory (classic Mac limit)
- **Execution**: Software interpretation of 68K instructions (stub for MVP)
- **Trap Handling**: Infrastructure for `_LoadSeg` and system traps
- **Stub Generation**: Emits 68K instruction sequences for lazy JT stubs

**Key Methods**:

- `M68K_CreateAddressSpace`: Allocate 16MB host memory, initialize registers
- `M68K_MapExecutable`: Copy code into address space, return CPU address
- `M68K_WriteJumpTableSlot`: Emit `0x4EF9 <target>` (JMP absolute.L)
- `M68K_MakeLazyJTStub`: Emit `0x3F3C <segID> 0xA9F0 0x4E75` (stub sequence)
- `M68K_Relocate`: Patch code bytes with big-endian target addresses

### Future Backends

- **PPC JIT**: Translate 68K → PPC at load time; JT entries jump to translated code
- **Native ABI**: Load native x86 plugins; JT entries use host calling convention
- **CFM-68K**: Handle Code Fragment Manager format (different CODE 0 layout)

## Launch Flow

### Complete Application Launch Sequence

1. **Process Manager** calls `LaunchApplication(launchParams)`

2. **Process Creation**:
   - `Process_Create` allocates partition, heap, stack
   - Returns new PCB

3. **Segment Loader Initialization**:
   - `SegmentLoader_Initialize` creates CPU address space
   - Selects backend (default: `"m68k_interp"`)

4. **Load Entry Segments**:
   - `EnsureEntrySegmentsLoaded`:
     - **CODE 0**: Parse A5 world metadata → `InstallA5World` → `BuildJumpTable`
     - **CODE 1**: `LoadSegment(1)` → parse, map, relocate, store entry address

5. **Stack Setup**:
   - Calculate `stackTop = stackBase + stackSize - 16`
   - Call `backend->SetStacks(stackTop, 0)`

6. **Install Traps**:
   - `InstallLoadSegTrap` registers `_LoadSeg` handler for lazy loads

7. **Enter Application**:
   - `backend->EnterAt(entryPoint, kEnterApp)`
   - Transfers control to CODE 1 entry (typically doesn't return)

### Error Handling

All functions return `OSErr`:
- `noErr`: Success
- `segmentNotFound`: CODE resource missing
- `segmentBadFormat`: Malformed CODE data
- `segmentRelocErr`: Relocation failed
- `segmentA5WorldErr`: A5 world setup failed
- `memFullErr`: Allocation failed

On error, cleanup routines (`SegmentLoader_Cleanup`, `Process_Cleanup`) are called to unwind allocations.

## Segment State Machine

Each segment has a lifecycle:

```
kSegmentUnloaded → LoadSegment() → kSegmentLoading → kSegmentLoaded
                                                           ↓
                                     UnloadSegment() → kSegmentPurgeable
```

- **Unloaded**: CODE resource not in memory
- **Loading**: Resource being fetched, relocated, mapped
- **Loaded**: Code mapped, executable, ref-counted
- **Purgeable**: Ref count = 0, eligible for purging (not yet implemented)

## Endianness Handling

### Big-Endian Safe Parsing

All CODE resources are big-endian (68K format). Use helpers:

```c
static inline UInt16 BE_Read16(const void* ptr) {
    const UInt8* p = (const UInt8*)ptr;
    return ((UInt16)p[0] << 8) | (UInt16)p[1];
}

static inline UInt32 BE_Read32(const void* ptr) {
    const UInt8* p = (const UInt8*)ptr;
    return ((UInt32)p[0] << 24) | ((UInt32)p[1] << 16) |
           ((UInt32)p[2] << 8)  | (UInt32)p[3];
}
```

**No Unaligned Access**: Always read byte-by-byte; no UB on hosts with strict alignment.

### Host Endianness

The *host* endianness doesn't matter—CODE parsing is always big-endian, and the CPU backend's memory (`M68KAddressSpace.memory`) stores big-endian instruction bytes as-is.

## Memory Safety

### Bounds Checking

- **Resource Size**: Validate CODE 0/N size before parsing (`ValidateCODE0`, `ValidateCODEN`)
- **Offset Validation**: Check `offset + len` before reading/writing memory
- **Relocation Bounds**: Ensure `atOffset + 4 <= segmentSize`

### No Undefined Behavior

- Explicit byte-by-byte reads (no aliasing violations)
- No unaligned pointer casts
- No signed overflow in relocation math
- No null pointer dereferences (all pointers checked)

## Testing Strategy

### Unit Tests

- **ParseCODE0**: Fixture blobs with known A5 sizes, JT counts
- **ParseCODEN**: Verify entry offset, prologue skip detection
- **BE_Read16/32**: Test on both little-endian and big-endian hosts
- **BuildRelocationTable**: Check pattern detection (JMP/JSR)

### Integration Tests

- **2-Segment App**: CODE 1 calls function in CODE 2 via JT → triggers lazy load
- **A5 World**: Verify A5 register set, below/above areas allocated correctly
- **Relocation**: Load segment with known relocations; verify patched addresses

### Backend Conformance

- **All ICPUBackend Methods**: Exercise each method with test harness
- **Trap Installation**: Verify trap handler invoked on `_LoadSeg`
- **JT Patching**: Check lazy stub → resolved address after load

## Future Work

### MVP Limitations (Current)

- **No Full Interpreter**: `M68K_Execute` is a stub; doesn't actually run 68K code
- **Simple Relocation Heuristics**: Pattern-based scanning; not all CODE formats supported
- **No Segment Purging**: Purgeable state exists but no memory reclaim policy
- **No CFM Support**: Classic CODE only; mixed-mode apps not supported

### Roadmap

1. **Complete M68K Interpreter**: Full instruction decode/execute loop
2. **Advanced Relocation**: Parse linker-specific relocation tables (if embedded in CODE)
3. **PPC Backend**: Add PPC JIT or interpreter for fat binaries
4. **CFM-68K**: Handle Code Fragment Manager (different CODE 0 format)
5. **Native Modules**: Allow host-native plugins via "NATV" pseudo-segments
6. **Segment Purging**: Implement LRU purge policy based on ref counts
7. **Debugging Support**: Add PC → source mapping, breakpoints, stepping

## File Organization

```
include/
  CPU/
    CPUBackend.h        - ICPUBackend interface, opaque types
    M68KInterp.h        - M68K-specific types (regs, address space)
  SegmentLoader/
    SegmentLoader.h     - Public API, data structures
    CodeParser.h        - CODE parsing helpers, big-endian readers

src/
  CPU/
    CPUBackend.c        - Backend registry
    m68k_interp/
      M68KBackend.c     - M68K interpreter implementation
  SegmentLoader/
    SegmentLoader.c     - Core loader (load/unload/resolve)
    CodeParser.c        - CODE 0/N parsing, relocation building
    A5World.c           - A5 world construction, JT building
  ProcessMgr/
    ProcessManager.c    - Wiring into LaunchApplication
```

## Conclusion

This segment loader achieves **semantic fidelity** to System 7.1 while maintaining **complete portability** through clean architectural layering. The CPU backend interface enables future expansion (PPC, native modules, JITs) without changing upper layers. Big-endian safe parsing and abstract relocations ensure correctness across all host architectures.

---

**Author**: System 7.1 Segment Loader Team
**Date**: 2025-10-05
**Version**: 1.0 (MVP)
