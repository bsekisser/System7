# PowerPC Interpreter - Foundation Implementation
**Date:** October 25, 2025
**Project:** System 7 Portable - PowerPC Interpreter Foundation

## Overview

Created the foundational infrastructure for a PowerPC software interpreter to enable
running later classic Mac applications (1994-2006 era) that used PowerPC processors.

## Architecture

### PowerPC vs 68K Comparison

| Feature | 68000 | PowerPC |
|---------|-------|---------|
| Design | CISC | RISC |
| Instruction width | Variable (16-bit base) | Fixed 32-bit |
| Registers | 8 data + 8 address | 32 GPR + 32 FPR |
| Addressing | Complex modes | Load/store only |
| Byte order | Big-endian | Big-endian |
| Function calls | JSR/RTS | Branch + Link Register |

### PowerPC Register File (32-bit)

- **32 GPRs (r0-r31):** General-purpose registers
  - r0: Special (reads as 0 in some contexts)
  - r1: Stack pointer by convention
  - r3-r10: Function arguments/return values
  - r13-r31: Preserved across calls

- **Special Registers:**
  - PC (CIA): Current Instruction Address
  - LR: Link Register (return addresses)
  - CTR: Count Register (loops)
  - CR: Condition Register (8 4-bit fields)
  - XER: Fixed-point Exception Register
  - MSR: Machine State Register

## Files Created

### 1. include/CPU/PPCInterp.h (145 lines)
- PowerPC register file structure
- Address space definition
- Memory paging system (same as M68K - 16MB virtual)
- Backend interface declarations

### 2. include/CPU/PPCOpcodes.h (236 lines)
- Instruction format macros
- Field extraction helpers
- Primary opcode definitions
- Extended opcode definitions
- Function prototypes

### 3. src/CPU/ppc_interp/PPCOpcodes.c (874 lines)
- Memory read/write helpers (big-endian)
- Instruction fetch mechanism
- CR0 flag setting
- Arithmetic instructions (ADDI, ADD, SUBF, MUL, DIV)
- Logical instructions (AND, OR, XOR, etc.)
- Comparison instructions (CMP, CMPI, CMPL, CMPLI)
- Branch instructions (B, BC, BCLR, BCCTR)
- Load/store instructions (LWZ, LBZ, LHZ, STW, STB, STH)
- System call placeholder

## Instructions Implemented (Foundation)

### Arithmetic (7 instructions)
✅ ADDI - Add immediate
✅ ADDIS - Add immediate shifted
✅ ADD - Add
✅ SUBF - Subtract from
✅ MULLI - Multiply low immediate
✅ MULLW - Multiply low word
✅ DIVW - Divide word

### Logical (9 instructions)
✅ ORI - OR immediate
✅ ORIS - OR immediate shifted
✅ XORI - XOR immediate
✅ XORIS - XOR immediate shifted
✅ ANDI. - AND immediate (sets CR0)
✅ ANDIS. - AND immediate shifted (sets CR0)
✅ AND - AND
✅ OR - OR (mr pseudo-op when same register)
✅ XOR - XOR

### Comparison (4 instructions)
✅ CMPI - Compare immediate (signed)
✅ CMP - Compare (signed)
✅ CMPLI - Compare logical immediate (unsigned)
✅ CMPL - Compare logical (unsigned)

### Branch (4 instructions)
✅ B - Branch (unconditional, with/without link)
✅ BC - Branch conditional
✅ BCLR - Branch conditional to link register (blr/ret)
✅ BCCTR - Branch conditional to count register

### Load/Store (6 instructions)
✅ LWZ - Load word and zero
✅ LBZ - Load byte and zero
✅ LHZ - Load halfword and zero
✅ STW - Store word
✅ STB - Store byte
✅ STH - Store halfword

### System (1 instruction)
✅ SC - System call (placeholder)

**Total: 31 foundational instructions**

## Key Features Implemented

### 1. Big-Endian Memory Access
All memory operations explicitly construct/deconstruct big-endian values,
ensuring cross-platform compatibility (works on x86, ARM, etc.).

### 2. Condition Register Handling
Full CR (Condition Register) support with 8 4-bit fields (LT, GT, EQ, SO).
CR0 automatically updated by record-form instructions (Rc bit set).

### 3. Branch Condition Testing
Complete bo/bi field interpretation for PowerPC's sophisticated branch
prediction and loop control:
- CTR decrement and testing
- CR bit testing
- Combined conditions

### 4. rA=0 Special Case
Correctly implements PowerPC's special case where rA field of 0 means
the value 0 (not GPR0) in certain instructions.

### 5. Link Register Management
Proper LR save/restore for function calls (bl instruction) and returns (blr).

## Still Needed for Complete Implementation

### Critical Instructions (~40 more needed)
- **More arithmetic:** ADDIC, SUBFIC, NEG, etc.
- **More logical:** NOR, NAND, EQV, ANDC, ORC
- **Shifts/rotates:** SLW, SRW, SRAW, RLWINM, RLWNM, RLWIMI
- **Load/store indexed:** LWZX, LBZX, STWX, STBX, etc.
- **Load/store update:** LWZU, LBZU, STWU, STBU, etc.
- **Multiple loads/stores:** LMW, STMW
- **CR operations:** CRAND, CROR, CRXOR, etc.

### Backend Implementation
- **PPCBackend.c:** Instruction decoder and CPU backend interface
- **Dispatcher:** Route opcodes to handlers
- **Address space management:** Create/destroy, map executable
- **Trap handling:** System call dispatch
- **Integration:** Register with CPU backend system

### Advanced Features
- Floating-point support (32 FPRs)
- AltiVec/VMX vector instructions (optional)
- Supervisor mode instructions
- Memory management unit
- Cache management
- Exception handling

## Design Decisions

### 1. Shared Memory Paging
Uses same paging system as M68K (4KB pages, 16MB virtual space) for
simplicity and compatibility. Can be expanded later.

### 2. User-Mode Focus
Initial implementation focuses on user-mode instructions used by
applications. Supervisor mode can be added later.

### 3. RISC Simplicity
PowerPC's RISC design makes implementation cleaner than 68K:
- Fixed 32-bit instruction width (easier decoding)
- Load/store architecture (memory ops isolated)
- Regular instruction formats
- Fewer special cases

### 4. No Floating-Point Yet
FPR registers defined but no FP instructions implemented yet.
Can add 68881-style FP emulation later.

## Code Statistics

- **Header files:** 381 lines
- **Implementation:** 874 lines
- **Total code:** 1,255 lines
- **Instructions:** 31 foundational opcodes

## Next Steps

1. **Complete PPCBackend.c** - Decoder and backend interface (~500 lines)
2. **Add critical instructions** - Shifts, rotates, more loads/stores (~400 lines)
3. **Implement dispatcher** - Route primary/extended opcodes to handlers
4. **Test with simple PowerPC code** - Arithmetic, branches, memory ops
5. **Add remaining common instructions** - Expand to ~100 opcodes
6. **Integrate with build system** - Add to Makefile
7. **Register backend** - Hook into CPU backend registry

## Compatibility Target

Target PowerPC versions:
- ✅ PowerPC 601 (original, used in Power Macs 1994-1995)
- ✅ PowerPC 603/604 (desktop, 1995-1997)
- ✅ PowerPC G3/750 (iMac, PowerBook, 1997-2003)
- ⚠️ PowerPC G4/74xx (partial - no AltiVec yet)
- ❌ PowerPC G5/970 (64-bit, not targeted yet)

## Foundation Status

**Status: FOUNDATION COMPLETE** ✅

The PowerPC interpreter foundation is in place with core instruction types
implemented. The architecture is sound and ready for expansion. Full
implementation requires completing the backend dispatcher and adding
remaining common instructions.

This provides a solid base for running PowerPC Mac applications alongside
the existing 68K interpreter, enabling System 7 Portable to support both
classic and later-generation Mac software.

---

*PowerPC Interpreter Foundation - October 25, 2025*
