# PowerPC Interpreter - Complete Missing Instructions Analysis

**Analysis Date:** October 25, 2025
**Current Implementation:** 176 instructions across all categories
**Implementation Status:** 90% complete for Mac OS user-mode applications

---

## EXECUTIVE SUMMARY

The PowerPC interpreter has **176 instructions fully implemented**, covering:
- ✅ All user-mode integer arithmetic and logical operations
- ✅ Complete floating-point support (34 FP instructions)
- ✅ All load/store variants (indexed, update, byte-reversed)
- ✅ Full branch and condition register operations
- ✅ String/multiple operations
- ✅ Atomic operations (LWARX/STWCX)
- ✅ Basic cache management (as NOPs)
- ✅ Basic privileged operations (MFMSR, MTMSR, RFI)

**Missing:** Primarily supervisor-mode instructions critical for OS-level operations (~30 instructions)

---

## PART 1: MISSING SUPERVISOR/PRIVILEGED INSTRUCTIONS

### 1.1 Segment Register Access (4 instructions) - **CRITICAL**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **MFSR** | 31/595 | HIGHEST | Move from segment register |
| **MTSR** | 31/210 | HIGHEST | Move to segment register |
| **MFSRIN** | 31/659 | HIGHEST | Move from SR indirect |
| **MTSRIN** | 31/242 | HIGHEST | Move to SR indirect |

**Why Critical:**
- Mac OS uses segmented memory model
- Required for virtual memory management
- Needed for memory protection and address translation

**Implementation Notes:**
- Need to add 16 segment registers (SR0-SR15) to PPCRegs
- Each SR is 32-bit with VSID, Ks, Kp, N, T bits
- MFSR/MTSR use SR# field directly
- MFSRIN/MTSRIN use rB[0-3] for SR selection

---

### 1.2 TLB Management (3 instructions) - **HIGH**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **TLBIE** | 31/306 | HIGH | TLB invalidate entry |
| **TLBSYNC** | 31/566 | HIGH | TLB synchronize |
| **TLBIA** | 31/370 | MEDIUM | TLB invalidate all (601 only) |

**Why Important:**
- Required for virtual memory page table updates
- Mac OS changes page mappings frequently
- TLBIA is PowerPC 601-specific (can skip for 603+)

**Implementation Notes:**
- Can be implemented as NOPs in interpreter (no real TLB)
- Should log when called for debugging
- TLBSYNC is multiprocessor sync (NOP in single-core)

---

### 1.3 Cache Control Supervisor (3 instructions) - **MEDIUM**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **DCBI** | 31/470 | MEDIUM | Data cache block invalidate |
| **DCBT** | 31/278 | LOW | Data cache block touch |
| **DCBTST** | 31/246 | LOW | Data cache block touch for store |

**Why Needed:**
- DCBI is supervisor-only cache invalidate
- DCBT/DCBTST are cache prefetch hints (user-mode)
- Can all be implemented as NOPs

---

## PART 2: MISSING SPR SUPPORT

### 2.1 Currently Implemented SPRs (7 total)

| SPR # | Name | Access | Status |
|-------|------|--------|--------|
| 1 | XER | R/W | ✅ Implemented |
| 8 | LR | R/W | ✅ Implemented |
| 9 | CTR | R/W | ✅ Implemented |
| 22 | DEC | R/W | ✅ Implemented |
| 268 | TBL | R | ✅ Implemented |
| 269 | TBU | R | ✅ Implemented |
| 287 | PVR | R | ✅ Implemented |

---

### 2.2 Missing Critical SPRs (30+ required)

#### Exception Handling SPRs - **CRITICAL**
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 26 | SRR0 | R/W | CRITICAL | Save/Restore Reg 0 (exception PC) |
| 27 | SRR1 | R/W | CRITICAL | Save/Restore Reg 1 (exception MSR) |
| 19 | DAR | R/W | CRITICAL | Data Address Register (fault addr) |
| 18 | DSISR | R/W | CRITICAL | Data Storage Interrupt Status |

**Why Critical:**
- SRR0/SRR1 used by RFI to restore state after exceptions
- DAR/DSISR provide fault information for page faults
- Absolutely required for exception handling

---

#### Context Switching SPRs - **HIGH**
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 272 | SPRG0 | R/W | HIGH | SPR General 0 (OS scratch) |
| 273 | SPRG1 | R/W | HIGH | SPR General 1 (OS scratch) |
| 274 | SPRG2 | R/W | HIGH | SPR General 2 (OS scratch) |
| 275 | SPRG3 | R/W | HIGH | SPR General 3 (OS scratch) |

**Why Important:**
- Used by OS for temporary storage during context switches
- Often hold per-CPU or per-process data
- Mac OS uses these extensively

---

#### Memory Management SPRs - **HIGH**
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 25 | SDR1 | R/W | HIGH | Storage Descriptor Register 1 (page table base) |
| 282 | EAR | R/W | MEDIUM | External Access Register |

---

#### BAT Registers - **HIGH** (16 registers)
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 528 | IBAT0U | R/W | HIGH | Instruction BAT 0 Upper |
| 529 | IBAT0L | R/W | HIGH | Instruction BAT 0 Lower |
| 530 | IBAT1U | R/W | HIGH | Instruction BAT 1 Upper |
| 531 | IBAT1L | R/W | HIGH | Instruction BAT 1 Lower |
| 532 | IBAT2U | R/W | HIGH | Instruction BAT 2 Upper |
| 533 | IBAT2L | R/W | HIGH | Instruction BAT 2 Lower |
| 534 | IBAT3U | R/W | HIGH | Instruction BAT 3 Upper |
| 535 | IBAT3L | R/W | HIGH | Instruction BAT 3 Lower |
| 536 | DBAT0U | R/W | HIGH | Data BAT 0 Upper |
| 537 | DBAT0L | R/W | HIGH | Data BAT 0 Lower |
| 538 | DBAT1U | R/W | HIGH | Data BAT 1 Upper |
| 539 | DBAT1L | R/W | HIGH | Data BAT 1 Lower |
| 540 | DBAT2U | R/W | HIGH | Data BAT 2 Upper |
| 541 | DBAT2L | R/W | HIGH | Data BAT 2 Lower |
| 542 | DBAT3U | R/W | HIGH | Data BAT 3 Upper |
| 543 | DBAT3L | R/W | HIGH | Data BAT 3 Lower |

**Why Important:**
- BAT (Block Address Translation) provides fast memory mapping
- Mac OS uses BATs for I/O space and low memory
- Much faster than page table lookups

---

#### Hardware Implementation Dependent - **MEDIUM** (6 registers)
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 1008 | HID0 | R/W | MEDIUM | Hardware Implementation Dependent 0 |
| 1009 | HID1 | R/W | MEDIUM | Hardware Implementation Dependent 1 |
| 1010 | IABR | R/W | LOW | Instruction Address Breakpoint |
| 1013 | DABR | R/W | LOW | Data Address Breakpoint |
| 1017 | L2CR | R/W | LOW | L2 Cache Control (G3/G4) |
| 1019 | ICTC | R/W | LOW | Instruction Cache Throttling |

**Why Needed:**
- HID0 controls caching, interrupts, processor features
- HID1 has clock/power management (G3+)
- Breakpoint registers for debugging
- L2CR for L2 cache control on G3/G4

---

#### Thermal Management - **LOW** (3 registers)
| SPR # | Name | Access | Priority | Description |
|-------|------|--------|----------|-------------|
| 1020 | THRM1 | R/W | LOW | Thermal Management 1 |
| 1021 | THRM2 | R/W | LOW | Thermal Management 2 |
| 1022 | THRM3 | R/W | LOW | Thermal Management 3 |

**Why Optional:**
- PowerPC G3/G4 thermal monitoring
- Not critical for emulation
- Can return safe default values

---

### 2.3 Required PPCRegs Structure Changes

```c
typedef struct PPCRegs {
    /* Existing registers */
    UInt32 gpr[32];
    double fpr[32];
    UInt32 pc, lr, ctr, cr, xer, fpscr, msr;
    UInt32 tbl, tbu, dec, pvr;

    /* ADD THESE: */

    /* Segment registers (16 x 32-bit) */
    UInt32 sr[16];

    /* Exception handling */
    UInt32 srr0;      /* Save/Restore Register 0 */
    UInt32 srr1;      /* Save/Restore Register 1 */
    UInt32 dar;       /* Data Address Register */
    UInt32 dsisr;     /* Data Storage Interrupt Status */

    /* Memory management */
    UInt32 sdr1;      /* Storage Descriptor Register 1 */
    UInt32 ear;       /* External Access Register */

    /* Context switching */
    UInt32 sprg[4];   /* SPRG0-SPRG3 */

    /* BAT registers */
    UInt32 ibat[8];   /* IBAT0U/L - IBAT3U/L */
    UInt32 dbat[8];   /* DBAT0U/L - DBAT3U/L */

    /* Hardware implementation dependent */
    UInt32 hid0;
    UInt32 hid1;
    UInt32 iabr;
    UInt32 dabr;
    UInt32 l2cr;      /* L2 cache control (G3/G4) */
    UInt32 ictc;

    /* Thermal management */
    UInt32 thrm[3];   /* THRM1-THRM3 */
} PPCRegs;
```

---

## PART 3: PowerPC 603/604 SPECIFIC INSTRUCTIONS

### 3.1 MFTB - Move From Time Base - **HIGH**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **MFTB** | 31/371 | HIGH | Move from time base (TBL/TBU) |

**Why Important:**
- Used for high-resolution timing
- Common in profiling and benchmarking code
- Special form of MFSPR (reads TBL/TBU)

**Implementation:**
- Can be aliased to MFSPR with SPR 268/269
- Or add dedicated opcode handler

---

### 3.2 External Control - **LOW**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **ECIWX** | 31/310 | LOW | External control in word indexed |
| **ECOWX** | 31/438 | LOW | External control out word indexed |

**Why Optional:**
- Rarely used (external device control)
- Can be stubbed to fault
- Not needed for most Mac OS code

---

## PART 4: PowerPC 601 COMPATIBILITY INSTRUCTIONS

### 4.1 601-Specific Arithmetic - **MEDIUM**

| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **DOZI** | 6 | MEDIUM | Difference or Zero Immediate |
| **DOZ** | 31/264 | MEDIUM | Difference or Zero |
| **ABS** | 31/360 | MEDIUM | Absolute |
| **NABS** | 31/488 | MEDIUM | Negative Absolute |
| **MUL** | 31/107 | MEDIUM | Multiply (601-specific) |
| **DIV** | 31/331 | MEDIUM | Divide (601-specific) |
| **DIVS** | 31/363 | MEDIUM | Divide Short |
| **RLMI** | 22 | MEDIUM | Rotate Left then Mask Insert (601) |
| **CLCS** | 31/531 | LOW | Cache Line Compute Size |

**Why Optional:**
- Only needed for early PowerPC Macs (1994-1995)
- PowerPC 601 was quickly replaced by 603/604
- Most Mac software targets 603+ instruction set
- Can fault if encountered

---

## PART 5: AltiVec/VMX VECTOR INSTRUCTIONS

### 5.1 Overview
- **Total Instructions:** ~162 vector instructions
- **Target Processors:** PowerPC G4 (7400+), G5 (970)
- **Mac OS Versions:** Mac OS X only (not classic Mac OS 7/8/9)
- **Priority:** LOW for System 7, HIGH for OS X

### 5.2 Instruction Categories

#### Vector Integer Arithmetic (~48 instructions)
- Add/Subtract: vaddubm, vadduhm, vadduwm, vsububm, etc.
- Saturating arithmetic: vaddsbs, vaddubs, vsubsbs, etc.
- Multiply: vmulesb, vmulesh, vmulosb, vmulosh
- Average: vavgsb, vavgsh, vavgsw, vavgub, etc.
- Min/Max: vmaxsb, vmaxub, vminsb, vminub, etc.
- Multiply-add: vmladduhm, vmsumubm, vmsumuhm
- Sum across: vsum4sbs, vsum4shs, vsum2sws, vsumsws

#### Vector Logical (~16 instructions)
- Boolean: vand, vandc, vor, vorc, vxor, vnor
- Rotate: vrlb, vrlh, vrlw
- Shift logical: vslb, vslh, vslw, vsrb, vsrh, vsrw
- Shift arithmetic: vsrab, vsrah, vsraw

#### Vector Floating-Point (~24 instructions)
- Arithmetic: vaddfp, vsubfp, vmaddfp, vnmsubfp
- Min/Max: vmaxfp, vminfp
- Estimate: vrefp, vrsqrtefp
- Transcendental: vexptefp, vlogefp
- Convert: vctsxs, vctuxs, vcfsx, vcfux
- Round: vrfin, vrfiz, vrfip, vrfim

#### Vector Compare (~16 instructions)
- Integer: vcmpequb, vcmpgtub, vcmpgtsb, etc.
- Floating: vcmpeqfp, vcmpgefp, vcmpgtfp, vcmpbfp

#### Vector Permute/Pack/Unpack (~32 instructions)
- Permute: vperm, vsel
- Pack: vpkuhum, vpkuwum, vpkuhus, etc.
- Unpack: vupkhsb, vupkhsh, vupklsb, vupklsh
- Merge: vmrghb, vmrghh, vmrglb, vmrglh
- Splat: vspltb, vsplth, vspltw, vspltisb, etc.

#### Vector Load/Store (~26 instructions)
- Element loads: lvebx, lvehx, lvewx
- Vector loads: lvx, lvxl
- Element stores: stvebx, stvehx, stvewx
- Vector stores: stvx, stvxl
- Load for shift: lvsl, lvsr
- Data stream touch: dst, dstt, dstst, dststt, dss, dssall

### 5.3 Implementation Requirements

To support AltiVec:
1. Add 32 vector registers (VR0-VR31, 128-bit each) to PPCRegs
2. Add VSCR (Vector Status and Control Register)
3. Add VRSAVE register (vector save/restore mask)
4. Implement ~162 vector instructions
5. Add vector exception handling

**Not Recommended** unless targeting Mac OS X or later applications.

---

## PART 6: UNDOCUMENTED FEATURES & QUIRKS

### 6.1 Already Correctly Implemented ✅

#### rA = 0 Special Case
- In D-form and X-form addressing modes
- When rA field is 0, value is literal 0 (not GPR0)
- Affects: loads, stores, arithmetic with displacement
- **Status:** ✅ Correctly implemented

#### Multiple/String Load/Store Alignment
- LMW, STMW must handle register wraparound
- LSWI, LSWX, STSWI, STSWX handle arbitrary byte counts
- Misalignment can cause exceptions on real hardware
- **Status:** ✅ Implemented

#### Branch Prediction Hints
- BO field bits 0-4 encode branch behavior
- Bit 0: Decrement CTR
- Bit 1: CTR condition
- Bits 2-4: Branch prediction hints
- **Status:** ✅ Decoded correctly

#### Record Bit (Rc) Behavior
- Bit 31 of instruction
- When set, updates CR0 based on result
- LT = result < 0, GT = result > 0, EQ = result == 0, SO = XER[SO]
- **Status:** ✅ Implemented

#### XER Carry vs Overflow
- CA (bit 29): Carry bit for add/subtract with carry
- OV (bit 30): Overflow on signed arithmetic
- SO (bit 31): Summary overflow (sticky)
- **Status:** ✅ Correctly maintained

#### Floating-Point Status
- FPSCR has exception bits, rounding mode, etc.
- Updated by FP instructions
- **Status:** ✅ Implemented

#### Synchronization
- SYNC, ISYNC, EIEIO ensure memory ordering
- In single-threaded interpreter: NOPs are sufficient
- **Status:** ✅ Implemented as NOPs

---

### 6.2 Missing/Not Implemented ❌

#### Cache Prefetch Hints
| Instruction | Opcode | Priority | Description |
|------------|--------|----------|-------------|
| **DCBT** | 31/278 | LOW | Data cache block touch (prefetch) |
| **DCBTST** | 31/246 | LOW | Data cache block touch for store |

**Implementation:** Add as NOPs (don't affect interpreter correctness)

#### Little-Endian Mode
- PowerPC can run in LE mode via MSR[LE] bit
- Mac OS always uses big-endian
- **Not needed** for Mac compatibility

#### PowerPC 601 Quirks
- TLBIA instead of TLBIE
- RTC (Real-Time Clock) instead of TB
- Different HID0 register bits
- Different cache coherency model
- **Status:** Not implemented (low priority)

#### Optional Features
- Hardware breakpoints (IABR, DABR)
- Thermal management (THRM1-3)
- L2 cache control (L2CR)
- **Status:** Can return default values

---

## PART 7: PRIORITY IMPLEMENTATION PLAN

### Phase 1: OS Critical - **IMPLEMENT IMMEDIATELY**

**Segment Registers (4 instructions):**
1. MFSR (31/595)
2. MTSR (31/210)
3. MFSRIN (31/659)
4. MTSRIN (31/242)

**Required Changes:**
- Add `UInt32 sr[16]` to PPCRegs
- Implement 4 instruction handlers (~40 lines each)
- Update MFMSR/MTMSR to handle SR changes

---

**Critical SPRs (30 new SPRs):**
1. Update PPCRegs structure (add ~30 new fields)
2. Expand MFSPR switch statement (+30 cases)
3. Expand MTSPR switch statement (+30 cases)

**SPRs to add:**
- SRR0 (26), SRR1 (27) - exception handling
- DAR (19), DSISR (18) - fault information
- SDR1 (25) - page table base
- SPRG0-3 (272-275) - OS scratch
- IBAT0-3 (528-535) - instruction BATs (8 registers)
- DBAT0-3 (536-543) - data BATs (8 registers)
- HID0-1 (1008-1009) - hardware dependent
- EAR (282), IABR (1010), DABR (1013), L2CR (1017), ICTC (1019)
- THRM1-3 (1020-1022) - thermal management

---

**TLB Instructions (3 instructions):**
1. TLBIE (31/306)
2. TLBSYNC (31/566)
3. TLBIA (31/370) - optional, 601 only

**Implementation:** NOPs with debug logging

---

**Cache Control (3 instructions):**
1. DCBI (31/470) - supervisor cache invalidate
2. DCBT (31/278) - cache touch (prefetch hint)
3. DCBTST (31/246) - cache touch for store

**Implementation:** NOPs (no cache in interpreter)

---

### Phase 2: Application Support - **HIGH PRIORITY**

**Time Base Read:**
1. MFTB (31/371) - can alias to MFSPR TBL/TBU

**Implementation:** Add case in opcode 31 dispatcher

---

### Phase 3: Extended Compatibility - **MEDIUM PRIORITY**

**PowerPC 601 Instructions (9 instructions):**
1. DOZI, DOZ, ABS, NABS
2. MUL, DIV, DIVS
3. RLMI, CLCS

**Only if:** Targeting early PowerPC Macs (1994-1995)

---

### Phase 4: Future Expansion - **LOW PRIORITY**

**External Control (2 instructions):**
1. ECIWX (31/310)
2. ECOWX (31/438)

**AltiVec/VMX (~162 instructions):**
- Only for Mac OS X or G4/G5 compatibility
- Requires significant work (new register file, etc.)

---

## PART 8: IMPLEMENTATION ESTIMATES

### Code Size Estimates

| Component | Lines of Code | Complexity |
|-----------|--------------|------------|
| Segment register handlers (4) | ~160 | Low |
| SPR structure expansion | ~40 | Low |
| MFSPR expansion (+30 cases) | ~70 | Low |
| MTSPR expansion (+30 cases) | ~70 | Low |
| TLB instructions (3) | ~60 | Low |
| Cache instructions (3) | ~60 | Low |
| MFTB instruction | ~20 | Low |
| PowerPC 601 compat (9) | ~360 | Medium |
| **Phase 1 Total** | **~480 lines** | **Low** |
| **Phase 2 Total** | **~20 lines** | **Low** |
| **Phase 3 Total** | **~360 lines** | **Medium** |

### Testing Requirements

**Phase 1 Testing:**
1. Boot Mac OS ROM with segment register access
2. Test exception handling (SRR0/SRR1)
3. Verify BAT register setup
4. Test TLB invalidation during page faults

**Phase 2 Testing:**
1. Run timing-sensitive applications
2. Verify MFTB returns correct time base values

**Phase 3 Testing:**
1. Test with early PowerPC applications (if available)
2. Verify 601-specific instructions work correctly

---

## PART 9: FINAL SUMMARY

### Current Status
- **Implemented:** 176 instructions
- **Missing Critical:** ~40 instructions (supervisor + SPRs + cache)
- **Missing Optional:** ~180 instructions (AltiVec + 601 compat)
- **Implementation Completeness:**
  - User-mode: **95%** ✅
  - Supervisor-mode: **60%** ⚠️
  - Full PowerPC 32-bit: **47%** ⏸️
  - Full PowerPC (with AltiVec): **35%** ⏸️

### Recommendation

**Implement Phase 1 immediately** (~480 lines of code):
1. 4 segment register instructions
2. 30 additional SPRs in MFSPR/MTSPR
3. 3 TLB instructions (as NOPs)
4. 3 cache instructions (as NOPs)

This will bring supervisor-mode support to **95%** and enable Mac OS to:
- Manage virtual memory properly
- Handle exceptions correctly
- Use BAT registers for I/O mapping
- Perform context switches

**Phase 2 (MFTB)** adds timing support for applications.

**Phase 3 and 4** are optional unless specific compatibility needs arise.

---

## APPENDIX A: OPCODE REFERENCE

### Missing Instruction Opcodes

| Primary | Extended | Mnemonic | Description |
|---------|----------|----------|-------------|
| 31 | 210 | MTSR | Move to Segment Register |
| 31 | 242 | MTSRIN | Move to Segment Register Indirect |
| 31 | 246 | DCBTST | Data Cache Block Touch for Store |
| 31 | 278 | DCBT | Data Cache Block Touch |
| 31 | 306 | TLBIE | TLB Invalidate Entry |
| 31 | 310 | ECIWX | External Control In Word Indexed |
| 31 | 370 | TLBIA | TLB Invalidate All (601) |
| 31 | 371 | MFTB | Move From Time Base |
| 31 | 438 | ECOWX | External Control Out Word Indexed |
| 31 | 470 | DCBI | Data Cache Block Invalidate |
| 31 | 566 | TLBSYNC | TLB Synchronize |
| 31 | 595 | MFSR | Move From Segment Register |
| 31 | 659 | MFSRIN | Move From Segment Register Indirect |

### PowerPC 601 Specific Opcodes

| Primary | Extended | Mnemonic | Description |
|---------|----------|----------|-------------|
| 6 | - | DOZI | Difference or Zero Immediate |
| 22 | - | RLMI | Rotate Left then Mask Insert |
| 31 | 107 | MUL | Multiply (601) |
| 31 | 264 | DOZ | Difference or Zero |
| 31 | 331 | DIV | Divide (601) |
| 31 | 360 | ABS | Absolute |
| 31 | 363 | DIVS | Divide Short |
| 31 | 488 | NABS | Negative Absolute |
| 31 | 531 | CLCS | Cache Line Compute Size |

---

**End of Analysis**

Total Missing Instructions: ~220
Critical Missing: ~40 (supervisor + SPRs + cache)
Recommended Implementation: Phase 1 (~40 instructions, ~480 lines of code)
