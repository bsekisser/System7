/*
 * PPCOpcodes.c - PowerPC Instruction Implementations
 *
 * Implements PowerPC instruction handlers for the software interpreter.
 * Covers user-mode PowerPC instructions used in Mac OS applications.
 */

#include "CPU/PPCInterp.h"
#include "CPU/PPCOpcodes.h"
#include "CPU/CPULogging.h"
#include "System71StdLib.h"
#include <string.h>

/*
 * Math functions for freestanding environment
 * (Simple implementations for interpreter use)
 */
static inline double fabs(double x) {
    return (x < 0.0) ? -x : x;
}

static inline float sqrtf(float x) {
    /* Newton-Raphson approximation for square root */
    if (x <= 0.0f) return 0.0f;
    float guess = x;
    float epsilon = 0.00001f;
    for (int i = 0; i < 10; i++) {
        float next = 0.5f * (guess + x / guess);
        if (fabs(next - guess) < epsilon) break;
        guess = next;
    }
    return guess;
}

static inline double sqrt(double x) {
    /* Newton-Raphson approximation for square root */
    if (x <= 0.0) return 0.0;
    double guess = x;
    double epsilon = 0.00000001;
    for (int i = 0; i < 15; i++) {
        double next = 0.5 * (guess + x / guess);
        if (fabs(next - guess) < epsilon) break;
        guess = next;
    }
    return guess;
}

static inline double trunc(double x) {
    /* Truncate towards zero */
    return (double)(SInt32)x;
}

/*
 * Helper Functions
 */

/* Fetch 32-bit instruction at PC and advance */
UInt32 PPC_Fetch32(PPCAddressSpace* as)
{
    UInt32 insn;
    UInt8* page;
    UInt32 pageNum, offset;

    /* Get page */
    pageNum = as->regs.pc >> PPC_PAGE_SHIFT;
    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Instruction fetch from unmapped memory");
        return 0;
    }

    page = (UInt8*)as->pageTable[pageNum];
    offset = as->regs.pc & (PPC_PAGE_SIZE - 1);

    /* Read big-endian 32-bit instruction */
    insn = (page[offset] << 24) |
           (page[offset + 1] << 16) |
           (page[offset + 2] << 8) |
           page[offset + 3];

    as->regs.pc += 4;
    return insn;
}

/* Read memory (big-endian) */
UInt32 PPC_Read32(PPCAddressSpace* as, UInt32 addr)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Read from unmapped memory");
        return 0;
    }

    page = (UInt8*)as->pageTable[pageNum];
    return (page[offset] << 24) | (page[offset + 1] << 16) |
           (page[offset + 2] << 8) | page[offset + 3];
}

UInt16 PPC_Read16(PPCAddressSpace* as, UInt32 addr)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Read from unmapped memory");
        return 0;
    }

    page = (UInt8*)as->pageTable[pageNum];
    return (page[offset] << 8) | page[offset + 1];
}

UInt8 PPC_Read8(PPCAddressSpace* as, UInt32 addr)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Read from unmapped memory");
        return 0;
    }

    page = (UInt8*)as->pageTable[pageNum];
    return page[offset];
}

/* Write memory (big-endian) */
void PPC_Write32(PPCAddressSpace* as, UInt32 addr, UInt32 value)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Write to unmapped memory");
        return;
    }

    page = (UInt8*)as->pageTable[pageNum];
    page[offset] = (value >> 24) & 0xFF;
    page[offset + 1] = (value >> 16) & 0xFF;
    page[offset + 2] = (value >> 8) & 0xFF;
    page[offset + 3] = value & 0xFF;
}

void PPC_Write16(PPCAddressSpace* as, UInt32 addr, UInt16 value)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Write to unmapped memory");
        return;
    }

    page = (UInt8*)as->pageTable[pageNum];
    page[offset] = (value >> 8) & 0xFF;
    page[offset + 1] = value & 0xFF;
}

void PPC_Write8(PPCAddressSpace* as, UInt32 addr, UInt8 value)
{
    UInt32 pageNum = addr >> PPC_PAGE_SHIFT;
    UInt32 offset = addr & (PPC_PAGE_SIZE - 1);
    UInt8* page;

    if (pageNum >= PPC_NUM_PAGES || !as->pageTable[pageNum]) {
        PPC_Fault(as, "Write to unmapped memory");
        return;
    }

    page = (UInt8*)as->pageTable[pageNum];
    page[offset] = value;
}

/* Set CR0 based on signed result */
static void PPC_SetCR0(PPCAddressSpace* as, SInt32 result)
{
    as->regs.cr &= 0x0FFFFFFF;  /* Clear CR0 */

    if (result < 0) {
        as->regs.cr |= PPC_CR_LT(0);
    } else if (result > 0) {
        as->regs.cr |= PPC_CR_GT(0);
    } else {
        as->regs.cr |= PPC_CR_EQ(0);
    }

    /* Copy SO from XER */
    if (as->regs.xer & PPC_XER_SO) {
        as->regs.cr |= PPC_CR_SO(0);
    }
}

/* Fault handler */
void PPC_Fault(PPCAddressSpace* as, const char* reason)
{
    serial_printf("[PPC] FAULT at PC=0x%08X: %s\n", as->regs.pc - 4, reason);
    as->halted = true;
}

/*
 * ============================================================================
 * ARITHMETIC INSTRUCTIONS
 * ============================================================================
 */

/*
 * ADDI - Add Immediate
 * rD = rA + SIMM
 */
void PPC_Op_ADDI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);

    if (ra == 0) {
        /* Special case: rA = 0 means value 0, not GPR0 */
        as->regs.gpr[rd] = simm;
    } else {
        as->regs.gpr[rd] = as->regs.gpr[ra] + simm;
    }
}

/*
 * ADDIS - Add Immediate Shifted
 * rD = rA + (SIMM << 16)
 */
void PPC_Op_ADDIS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn) << 16;

    if (ra == 0) {
        as->regs.gpr[rd] = simm;
    } else {
        as->regs.gpr[rd] = as->regs.gpr[ra] + simm;
    }
}

/*
 * ADD - Add
 * rD = rA + rB
 */
void PPC_Op_ADD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[rd] = as->regs.gpr[ra] + as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * SUBF - Subtract From
 * rD = rB - rA (note: reversed operands!)
 */
void PPC_Op_SUBF(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[rd] = as->regs.gpr[rb] - as->regs.gpr[ra];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * MULLI - Multiply Low Immediate
 * rD = rA * SIMM
 */
void PPC_Op_MULLI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);

    as->regs.gpr[rd] = as->regs.gpr[ra] * simm;
}

/*
 * MULLW - Multiply Low Word
 * rD = rA * rB (low 32 bits)
 */
void PPC_Op_MULLW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[rd] = as->regs.gpr[ra] * as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * DIVW - Divide Word
 * rD = rA / rB (signed)
 */
void PPC_Op_DIVW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 dividend = (SInt32)as->regs.gpr[ra];
    SInt32 divisor = (SInt32)as->regs.gpr[rb];

    if (divisor == 0) {
        /* Division by zero: result undefined, leave register unchanged */
        return;
    }

    as->regs.gpr[rd] = (UInt32)(dividend / divisor);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * ============================================================================
 * LOGICAL INSTRUCTIONS
 * ============================================================================
 */

/*
 * ORI - OR Immediate
 * rA = rS | UIMM
 */
void PPC_Op_ORI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] | uimm;
}

/*
 * ORIS - OR Immediate Shifted
 * rA = rS | (UIMM << 16)
 */
void PPC_Op_ORIS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn) << 16;

    as->regs.gpr[ra] = as->regs.gpr[rs] | uimm;
}

/*
 * XORI - XOR Immediate
 * rA = rS ^ UIMM
 */
void PPC_Op_XORI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] ^ uimm;
}

/*
 * XORIS - XOR Immediate Shifted
 * rA = rS ^ (UIMM << 16)
 */
void PPC_Op_XORIS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn) << 16;

    as->regs.gpr[ra] = as->regs.gpr[rs] ^ uimm;
}

/*
 * ANDI. - AND Immediate (always sets CR0)
 * rA = rS & UIMM
 */
void PPC_Op_ANDI_RC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] & uimm;
    PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
}

/*
 * ANDIS. - AND Immediate Shifted (always sets CR0)
 * rA = rS & (UIMM << 16)
 */
void PPC_Op_ANDIS_RC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn) << 16;

    as->regs.gpr[ra] = as->regs.gpr[rs] & uimm;
    PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
}

/*
 * AND - AND
 * rA = rS & rB
 */
void PPC_Op_AND(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] & as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * OR - OR
 * rA = rS | rB
 * Note: OR rx,rx,rx (same register) is used as mr (move register)
 */
void PPC_Op_OR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] | as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * XOR - XOR
 * rA = rS ^ rB
 */
void PPC_Op_XOR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] ^ as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * ============================================================================
 * COMPARISON INSTRUCTIONS
 * ============================================================================
 */

/*
 * CMPI - Compare Immediate
 * Compare rA with SIMM (signed) and set CR field
 */
void PPC_Op_CMPI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = PPC_CRFD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    UInt32 cr_mask = 0xF0000000 >> (crfd * 4);

    /* Clear target CR field */
    as->regs.cr &= ~cr_mask;

    /* Set comparison bits */
    if (a < simm) {
        as->regs.cr |= PPC_CR_LT(crfd);
    } else if (a > simm) {
        as->regs.cr |= PPC_CR_GT(crfd);
    } else {
        as->regs.cr |= PPC_CR_EQ(crfd);
    }

    /* Copy SO from XER */
    if (as->regs.xer & PPC_XER_SO) {
        as->regs.cr |= PPC_CR_SO(crfd);
    }
}

/*
 * CMP - Compare
 * Compare rA with rB (signed) and set CR field
 */
void PPC_Op_CMP(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = PPC_CRFD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];
    UInt32 cr_mask = 0xF0000000 >> (crfd * 4);

    /* Clear target CR field */
    as->regs.cr &= ~cr_mask;

    /* Set comparison bits */
    if (a < b) {
        as->regs.cr |= PPC_CR_LT(crfd);
    } else if (a > b) {
        as->regs.cr |= PPC_CR_GT(crfd);
    } else {
        as->regs.cr |= PPC_CR_EQ(crfd);
    }

    /* Copy SO from XER */
    if (as->regs.xer & PPC_XER_SO) {
        as->regs.cr |= PPC_CR_SO(crfd);
    }
}

/*
 * CMPLI - Compare Logical Immediate
 * Compare rA with UIMM (unsigned) and set CR field
 */
void PPC_Op_CMPLI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = PPC_CRFD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt32 uimm = PPC_UIMM(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 cr_mask = 0xF0000000 >> (crfd * 4);

    /* Clear target CR field */
    as->regs.cr &= ~cr_mask;

    /* Set comparison bits */
    if (a < uimm) {
        as->regs.cr |= PPC_CR_LT(crfd);
    } else if (a > uimm) {
        as->regs.cr |= PPC_CR_GT(crfd);
    } else {
        as->regs.cr |= PPC_CR_EQ(crfd);
    }

    /* Copy SO from XER */
    if (as->regs.xer & PPC_XER_SO) {
        as->regs.cr |= PPC_CR_SO(crfd);
    }
}

/*
 * CMPL - Compare Logical
 * Compare rA with rB (unsigned) and set CR field
 */
void PPC_Op_CMPL(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = PPC_CRFD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];
    UInt32 cr_mask = 0xF0000000 >> (crfd * 4);

    /* Clear target CR field */
    as->regs.cr &= ~cr_mask;

    /* Set comparison bits */
    if (a < b) {
        as->regs.cr |= PPC_CR_LT(crfd);
    } else if (a > b) {
        as->regs.cr |= PPC_CR_GT(crfd);
    } else {
        as->regs.cr |= PPC_CR_EQ(crfd);
    }

    /* Copy SO from XER */
    if (as->regs.xer & PPC_XER_SO) {
        as->regs.cr |= PPC_CR_SO(crfd);
    }
}

/*
 * ============================================================================
 * BRANCH INSTRUCTIONS
 * ============================================================================
 */

/* Test branch condition */
static Boolean PPC_TestBranchCondition(PPCAddressSpace* as, UInt8 bo, UInt8 bi)
{
    Boolean ctr_ok, cond_ok;

    /* Decrement CTR if bit 2 is clear */
    if (!(bo & 0x04)) {
        as->regs.ctr--;
    }

    /* Check CTR condition */
    if (bo & 0x04) {
        ctr_ok = true;  /* Don't test CTR */
    } else if (bo & 0x02) {
        ctr_ok = (as->regs.ctr == 0);  /* Branch if CTR = 0 */
    } else {
        ctr_ok = (as->regs.ctr != 0);  /* Branch if CTR != 0 */
    }

    /* Check CR condition */
    if (bo & 0x10) {
        cond_ok = true;  /* Don't test condition */
    } else {
        UInt32 bit_value = (as->regs.cr >> (31 - bi)) & 1;
        if (bo & 0x08) {
            cond_ok = (bit_value == 1);  /* Branch if bit set */
        } else {
            cond_ok = (bit_value == 0);  /* Branch if bit clear */
        }
    }

    return ctr_ok && cond_ok;
}

/*
 * B - Branch
 * Unconditional branch (can be absolute or relative)
 */
void PPC_Op_B(PPCAddressSpace* as, UInt32 insn)
{
    SInt32 li = PPC_LI(insn);
    Boolean aa = PPC_AA(insn);
    Boolean lk = PPC_LK(insn);
    UInt32 target;

    /* Save return address if LK=1 (bl instruction) */
    if (lk) {
        as->regs.lr = as->regs.pc;
    }

    /* Calculate target address */
    if (aa) {
        target = li;  /* Absolute */
    } else {
        target = (as->regs.pc - 4) + li;  /* Relative to current instruction */
    }

    as->regs.pc = target;
}

/*
 * BC - Branch Conditional
 * Conditional branch based on CR and CTR
 */
void PPC_Op_BC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 bo = PPC_BO(insn);
    UInt8 bi = PPC_BI(insn);
    SInt32 bd = PPC_BD(insn);
    Boolean aa = PPC_AA(insn);
    Boolean lk = PPC_LK(insn);

    if (PPC_TestBranchCondition(as, bo, bi)) {
        UInt32 target;

        /* Save return address if LK=1 */
        if (lk) {
            as->regs.lr = as->regs.pc;
        }

        /* Calculate target */
        if (aa) {
            target = bd;
        } else {
            target = (as->regs.pc - 4) + bd;
        }

        as->regs.pc = target;
    }
}

/*
 * BCLR - Branch Conditional to Link Register
 * Return from function or conditional branch via LR
 */
void PPC_Op_BCLR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 bo = PPC_BO(insn);
    UInt8 bi = PPC_BI(insn);
    Boolean lk = PPC_LK(insn);

    if (PPC_TestBranchCondition(as, bo, bi)) {
        UInt32 target = as->regs.lr;

        /* Save return address if LK=1 (rare) */
        if (lk) {
            as->regs.lr = as->regs.pc;
        }

        as->regs.pc = target;
    }
}

/*
 * BCCTR - Branch Conditional to Count Register
 * Branch via CTR (used for computed branches)
 */
void PPC_Op_BCCTR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 bo = PPC_BO(insn);
    UInt8 bi = PPC_BI(insn);
    Boolean lk = PPC_LK(insn);

    /* Note: CTR is NOT decremented for bctr */
    Boolean cond_ok;

    /* Check CR condition */
    if (bo & 0x10) {
        cond_ok = true;
    } else {
        UInt32 bit_value = (as->regs.cr >> (31 - bi)) & 1;
        if (bo & 0x08) {
            cond_ok = (bit_value == 1);
        } else {
            cond_ok = (bit_value == 0);
        }
    }

    if (cond_ok) {
        UInt32 target = as->regs.ctr;

        if (lk) {
            as->regs.lr = as->regs.pc;
        }

        as->regs.pc = target;
    }
}

/*
 * ============================================================================
 * LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LWZ - Load Word and Zero
 * rD = MEM(rA + d)
 */
void PPC_Op_LWZ(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    as->regs.gpr[rd] = PPC_Read32(as, ea);
}

/*
 * LBZ - Load Byte and Zero
 * rD = zero_extend(MEM(rA + d))
 */
void PPC_Op_LBZ(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    as->regs.gpr[rd] = PPC_Read8(as, ea);
}

/*
 * LHZ - Load Halfword and Zero
 * rD = zero_extend(MEM(rA + d))
 */
void PPC_Op_LHZ(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    as->regs.gpr[rd] = PPC_Read16(as, ea);
}

/*
 * STW - Store Word
 * MEM(rA + d) = rS
 */
void PPC_Op_STW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    PPC_Write32(as, ea, as->regs.gpr[rs]);
}

/*
 * STB - Store Byte
 * MEM(rA + d) = rS[24:31]
 */
void PPC_Op_STB(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    PPC_Write8(as, ea, (UInt8)(as->regs.gpr[rs] & 0xFF));
}

/*
 * STH - Store Halfword
 * MEM(rA + d) = rS[16:31]
 */
void PPC_Op_STH(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    PPC_Write16(as, ea, (UInt16)(as->regs.gpr[rs] & 0xFFFF));
}

/*
 * ============================================================================
 * ADDITIONAL ARITHMETIC INSTRUCTIONS
 * ============================================================================
 */

/*
 * ADDIC - Add Immediate Carrying
 * rD = rA + SIMM, sets CA
 */
void PPC_Op_ADDIC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 result = a + simm;

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (result < a) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }
}

/*
 * ADDIC. - Add Immediate Carrying and Record
 * rD = rA + SIMM, sets CA and CR0
 */
void PPC_Op_ADDIC_RC(PPCAddressSpace* as, UInt32 insn)
{
    PPC_Op_ADDIC(as, insn);
    PPC_SetCR0(as, (SInt32)as->regs.gpr[PPC_RD(insn)]);
}

/*
 * SUBFIC - Subtract From Immediate Carrying
 * rD = SIMM - rA, sets CA
 */
void PPC_Op_SUBFIC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 result = simm - a;

    as->regs.gpr[rd] = result;

    /* Set CA if no borrow occurred (simm >= a in unsigned) */
    if ((UInt32)simm >= a) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }
}

/*
 * NEG - Negate
 * rD = -rA
 */
void PPC_Op_NEG(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];

    as->regs.gpr[rd] = (UInt32)(-a);

    if (rc) {
        PPC_SetCR0(as, -a);
    }
}

/*
 * ADDC - Add Carrying
 * rD = rA + rB, sets CA
 */
void PPC_Op_ADDC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];
    UInt32 result = a + b;

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (result < a) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * SUBFC - Subtract From Carrying
 * rD = rB - rA, sets CA
 */
void PPC_Op_SUBFC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];
    UInt32 result = b - a;

    as->regs.gpr[rd] = result;

    /* Set CA if no borrow occurred (b >= a in unsigned) */
    if (b >= a) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * ============================================================================
 * ADDITIONAL LOGICAL INSTRUCTIONS
 * ============================================================================
 */

/*
 * NOR - NOR
 * rA = ~(rS | rB)
 */
void PPC_Op_NOR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = ~(as->regs.gpr[rs] | as->regs.gpr[rb]);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * NAND - NAND
 * rA = ~(rS & rB)
 */
void PPC_Op_NAND(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = ~(as->regs.gpr[rs] & as->regs.gpr[rb]);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * EQV - Equivalent
 * rA = ~(rS ^ rB)
 */
void PPC_Op_EQV(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = ~(as->regs.gpr[rs] ^ as->regs.gpr[rb]);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * ANDC - AND with Complement
 * rA = rS & ~rB
 */
void PPC_Op_ANDC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] & ~as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * ORC - OR with Complement
 * rA = rS | ~rB
 */
void PPC_Op_ORC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.gpr[ra] = as->regs.gpr[rs] | ~as->regs.gpr[rb];

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * ============================================================================
 * SHIFT INSTRUCTIONS
 * ============================================================================
 */

/*
 * SLW - Shift Left Word
 * rA = rS << rB[27:31]
 */
void PPC_Op_SLW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 shift = as->regs.gpr[rb] & 0x3F;

    if (shift < 32) {
        as->regs.gpr[ra] = as->regs.gpr[rs] << shift;
    } else {
        as->regs.gpr[ra] = 0;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * SRW - Shift Right Word
 * rA = rS >> rB[27:31] (logical)
 */
void PPC_Op_SRW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 shift = as->regs.gpr[rb] & 0x3F;

    if (shift < 32) {
        as->regs.gpr[ra] = as->regs.gpr[rs] >> shift;
    } else {
        as->regs.gpr[ra] = 0;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * SRAW - Shift Right Algebraic Word
 * rA = rS >> rB[27:31] (arithmetic), sets CA
 */
void PPC_Op_SRAW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 shift = as->regs.gpr[rb] & 0x3F;
    SInt32 value = (SInt32)as->regs.gpr[rs];
    SInt32 result;

    if (shift < 32) {
        result = value >> shift;
        /* Set CA if any 1 bits were shifted out of a negative number */
        if (value < 0 && (value & ((1U << shift) - 1))) {
            as->regs.xer |= PPC_XER_CA;
        } else {
            as->regs.xer &= ~PPC_XER_CA;
        }
    } else {
        result = (value < 0) ? -1 : 0;
        /* Set CA if value was negative */
        if (value < 0) {
            as->regs.xer |= PPC_XER_CA;
        } else {
            as->regs.xer &= ~PPC_XER_CA;
        }
    }

    as->regs.gpr[ra] = (UInt32)result;

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * SRAWI - Shift Right Algebraic Word Immediate
 * rA = rS >> SH (arithmetic), sets CA
 */
void PPC_Op_SRAWI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 sh = PPC_SH(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 value = (SInt32)as->regs.gpr[rs];
    SInt32 result = value >> sh;

    as->regs.gpr[ra] = (UInt32)result;

    /* Set CA if any 1 bits were shifted out of a negative number */
    if (value < 0 && sh > 0 && (value & ((1U << sh) - 1))) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * ============================================================================
 * ROTATE INSTRUCTIONS
 * ============================================================================
 */

/* Helper: Create mask from MB to ME */
static UInt32 PPC_MakeMask(UInt8 mb, UInt8 me)
{
    UInt32 mask;
    if (mb <= me) {
        /* Normal case: MB...ME */
        mask = ((1U << (32 - mb)) - 1) & ~((1U << (31 - me)) - 1);
    } else {
        /* Wrapped case: ME...MB */
        mask = ~(((1U << (32 - me - 1)) - 1) & ~((1U << (31 - mb + 1)) - 1));
    }
    return mask;
}

/*
 * RLWINM - Rotate Left Word Immediate then AND with Mask
 * rA = ROTL(rS, SH) & MASK(MB, ME)
 */
void PPC_Op_RLWINM(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 sh = PPC_SH(insn);
    UInt8 mb = PPC_MB(insn);
    UInt8 me = PPC_ME(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 value = as->regs.gpr[rs];
    UInt32 rotated = (value << sh) | (value >> (32 - sh));
    UInt32 mask = PPC_MakeMask(mb, me);

    as->regs.gpr[ra] = rotated & mask;

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * RLWNM - Rotate Left Word then AND with Mask
 * rA = ROTL(rS, rB[27:31]) & MASK(MB, ME)
 */
void PPC_Op_RLWNM(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt8 mb = PPC_MB(insn);
    UInt8 me = PPC_ME(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 value = as->regs.gpr[rs];
    UInt8 sh = as->regs.gpr[rb] & 0x1F;
    UInt32 rotated = (value << sh) | (value >> (32 - sh));
    UInt32 mask = PPC_MakeMask(mb, me);

    as->regs.gpr[ra] = rotated & mask;

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * RLWIMI - Rotate Left Word Immediate then Mask Insert
 * rA = (ROTL(rS, SH) & MASK(MB, ME)) | (rA & ~MASK(MB, ME))
 */
void PPC_Op_RLWIMI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 sh = PPC_SH(insn);
    UInt8 mb = PPC_MB(insn);
    UInt8 me = PPC_ME(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 value = as->regs.gpr[rs];
    UInt32 rotated = (value << sh) | (value >> (32 - sh));
    UInt32 mask = PPC_MakeMask(mb, me);

    as->regs.gpr[ra] = (rotated & mask) | (as->regs.gpr[ra] & ~mask);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * ============================================================================
 * INDEXED LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LWZX - Load Word and Zero Indexed
 * rD = MEM(rA + rB)
 */
void PPC_Op_LWZX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    as->regs.gpr[rd] = PPC_Read32(as, ea);
}

/*
 * LBZX - Load Byte and Zero Indexed
 * rD = MEM(rA + rB)
 */
void PPC_Op_LBZX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    as->regs.gpr[rd] = PPC_Read8(as, ea);
}

/*
 * LHZX - Load Halfword and Zero Indexed
 * rD = MEM(rA + rB)
 */
void PPC_Op_LHZX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    as->regs.gpr[rd] = PPC_Read16(as, ea);
}

/*
 * LHAX - Load Halfword Algebraic Indexed
 * rD = EXTS(MEM(rA + rB))
 */
void PPC_Op_LHAX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    SInt16 value;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    value = (SInt16)PPC_Read16(as, ea);
    as->regs.gpr[rd] = (UInt32)(SInt32)value; /* Sign-extend */
}

/*
 * STWX - Store Word Indexed
 * MEM(rA + rB) = rS
 */
void PPC_Op_STWX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    PPC_Write32(as, ea, as->regs.gpr[rs]);
}

/*
 * STBX - Store Byte Indexed
 * MEM(rA + rB) = rS[24:31]
 */
void PPC_Op_STBX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    PPC_Write8(as, ea, (UInt8)(as->regs.gpr[rs] & 0xFF));
}

/*
 * STHX - Store Halfword Indexed
 * MEM(rA + rB) = rS[16:31]
 */
void PPC_Op_STHX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    PPC_Write16(as, ea, (UInt16)(as->regs.gpr[rs] & 0xFFFF));
}

/*
 * ============================================================================
 * UPDATE-FORM LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LWZU - Load Word and Zero with Update
 * rD = MEM(rA + d), rA = rA + d
 */
void PPC_Op_LWZU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    as->regs.gpr[rd] = PPC_Read32(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LBZU - Load Byte and Zero with Update
 * rD = MEM(rA + d), rA = rA + d
 */
void PPC_Op_LBZU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    as->regs.gpr[rd] = PPC_Read8(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LHZU - Load Halfword and Zero with Update
 * rD = MEM(rA + d), rA = rA + d
 */
void PPC_Op_LHZU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    as->regs.gpr[rd] = PPC_Read16(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LHAU - Load Halfword Algebraic with Update
 * rD = EXTS(MEM(rA + d)), rA = rA + d
 */
void PPC_Op_LHAU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;
    SInt16 value = (SInt16)PPC_Read16(as, ea);

    as->regs.gpr[rd] = (UInt32)(SInt32)value; /* Sign-extend */
    as->regs.gpr[ra] = ea;
}

/*
 * STWU - Store Word with Update
 * MEM(rA + d) = rS, rA = rA + d
 */
void PPC_Op_STWU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    PPC_Write32(as, ea, as->regs.gpr[rs]);
    as->regs.gpr[ra] = ea;
}

/*
 * STBU - Store Byte with Update
 * MEM(rA + d) = rS[24:31], rA = rA + d
 */
void PPC_Op_STBU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    PPC_Write8(as, ea, (UInt8)(as->regs.gpr[rs] & 0xFF));
    as->regs.gpr[ra] = ea;
}

/*
 * STHU - Store Halfword with Update
 * MEM(rA + d) = rS[16:31], rA = rA + d
 */
void PPC_Op_STHU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea = as->regs.gpr[ra] + d;

    PPC_Write16(as, ea, (UInt16)(as->regs.gpr[rs] & 0xFFFF));
    as->regs.gpr[ra] = ea;
}

/*
 * ============================================================================
 * MULTIPLE LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LMW - Load Multiple Word
 * Load consecutive words from memory into registers rD through r31
 */
void PPC_Op_LMW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;
    UInt8 r;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    for (r = rd; r <= 31; r++) {
        as->regs.gpr[r] = PPC_Read32(as, ea);
        ea += 4;
    }
}

/*
 * STMW - Store Multiple Word
 * Store consecutive words from registers rS through r31 into memory
 */
void PPC_Op_STMW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = PPC_SIMM(insn);
    UInt32 ea;
    UInt8 r;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    for (r = rs; r <= 31; r++) {
        PPC_Write32(as, ea, as->regs.gpr[r]);
        ea += 4;
    }
}

/*
 * ============================================================================
 * CONDITION REGISTER LOGICAL OPERATIONS
 * ============================================================================
 */

/*
 * CRAND - Condition Register AND
 * CR[crbD] = CR[crbA] & CR[crbB]
 */
void PPC_Op_CRAND(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = bit_a & bit_b;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CROR - Condition Register OR
 * CR[crbD] = CR[crbA] | CR[crbB]
 */
void PPC_Op_CROR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = bit_a | bit_b;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CRXOR - Condition Register XOR
 * CR[crbD] = CR[crbA] ^ CR[crbB]
 */
void PPC_Op_CRXOR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = bit_a ^ bit_b;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CRNAND - Condition Register NAND
 * CR[crbD] = ~(CR[crbA] & CR[crbB])
 */
void PPC_Op_CRNAND(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = ~(bit_a & bit_b) & 1;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CRNOR - Condition Register NOR
 * CR[crbD] = ~(CR[crbA] | CR[crbB])
 */
void PPC_Op_CRNOR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = ~(bit_a | bit_b) & 1;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CREQV - Condition Register Equivalent
 * CR[crbD] = ~(CR[crbA] ^ CR[crbB])
 */
void PPC_Op_CREQV(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = ~(bit_a ^ bit_b) & 1;

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CRANDC - Condition Register AND with Complement
 * CR[crbD] = CR[crbA] & ~CR[crbB]
 */
void PPC_Op_CRANDC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = bit_a & (~bit_b & 1);

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * CRORC - Condition Register OR with Complement
 * CR[crbD] = CR[crbA] | ~CR[crbB]
 */
void PPC_Op_CRORC(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = PPC_RD(insn);
    UInt8 crba = PPC_RA(insn);
    UInt8 crbb = PPC_RB(insn);
    UInt32 bit_a = (as->regs.cr >> (31 - crba)) & 1;
    UInt32 bit_b = (as->regs.cr >> (31 - crbb)) & 1;
    UInt32 result = bit_a | (~bit_b & 1);

    /* Clear destination bit */
    as->regs.cr &= ~(1U << (31 - crbd));
    /* Set destination bit if result is 1 */
    if (result) {
        as->regs.cr |= (1U << (31 - crbd));
    }
}

/*
 * ============================================================================
 * SYSTEM INSTRUCTIONS
 * ============================================================================
 */

/*
 * SC - System Call
 * Used for Mac Toolbox traps in Mac OS
 */
void PPC_Op_SC(PPCAddressSpace* as, UInt32 insn)
{
    (void)insn;
    /* For now, treat as trap/halt */
    PPC_Fault(as, "System call (sc) instruction - trap handler needed");
}

/*
 * ============================================================================
 * EXTENDED ARITHMETIC WITH CARRY
 * ============================================================================
 */

/*
 * ADDZE - Add to Zero Extended
 * rD = rA + XER[CA]
 */
void PPC_Op_ADDZE(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = a + ca;

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (result < a) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * ADDME - Add to Minus One Extended
 * rD = rA + XER[CA] - 1
 */
void PPC_Op_ADDME(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = a + ca + 0xFFFFFFFF; /* + ca - 1 */

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (ca == 1 || a != 0) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * ADDE - Add Extended
 * rD = rA + rB + XER[CA]
 */
void PPC_Op_ADDE(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = a + b + ca;

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (result < a || (result == a && ca == 1)) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * SUBFE - Subtract From Extended
 * rD = ~rA + rB + XER[CA]
 */
void PPC_Op_SUBFE(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = ~a + b + ca;

    as->regs.gpr[rd] = result;

    /* Set CA if carry occurred */
    if (result < (~a) || (result == (~a) && ca == 1 && b == 0)) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * SUBFZE - Subtract From Zero Extended
 * rD = ~rA + XER[CA]
 */
void PPC_Op_SUBFZE(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = ~a + ca;

    as->regs.gpr[rd] = result;

    /* Set CA appropriately */
    if (result < (~a)) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * SUBFME - Subtract From Minus One Extended
 * rD = ~rA + XER[CA] - 1
 */
void PPC_Op_SUBFME(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 ca = (as->regs.xer & PPC_XER_CA) ? 1 : 0;
    UInt32 result = ~a + ca + 0xFFFFFFFF; /* + ca - 1 */

    as->regs.gpr[rd] = result;

    /* Set CA appropriately */
    if (ca == 1 || a != 0xFFFFFFFF) {
        as->regs.xer |= PPC_XER_CA;
    } else {
        as->regs.xer &= ~PPC_XER_CA;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)result);
    }
}

/*
 * ============================================================================
 * UNSIGNED MULTIPLY/DIVIDE
 * ============================================================================
 */

/*
 * MULHW - Multiply High Word (signed)
 * rD = (rA * rB)[0:31] (upper 32 bits of 64-bit result)
 */
void PPC_Op_MULHW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt64 a = (SInt32)as->regs.gpr[ra];
    SInt64 b = (SInt32)as->regs.gpr[rb];
    SInt64 result = a * b;

    as->regs.gpr[rd] = (UInt32)(result >> 32);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * MULHWU - Multiply High Word Unsigned
 * rD = (rA * rB)[0:31] (upper 32 bits of 64-bit unsigned result)
 */
void PPC_Op_MULHWU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt64 a = as->regs.gpr[ra];
    UInt64 b = as->regs.gpr[rb];
    UInt64 result = a * b;

    as->regs.gpr[rd] = (UInt32)(result >> 32);

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * DIVWU - Divide Word Unsigned
 * rD = rA / rB (unsigned)
 */
void PPC_Op_DIVWU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 a = as->regs.gpr[ra];
    UInt32 b = as->regs.gpr[rb];

    if (b == 0) {
        /* Divide by zero - result is undefined, set to 0 */
        as->regs.gpr[rd] = 0;
    } else {
        as->regs.gpr[rd] = a / b;
    }

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[rd]);
    }
}

/*
 * ============================================================================
 * SIGN EXTENSION AND BIT OPERATIONS
 * ============================================================================
 */

/*
 * EXTSB - Extend Sign Byte
 * rA = EXTS(rS[24:31])
 */
void PPC_Op_EXTSB(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    SInt8 byte = (SInt8)(as->regs.gpr[rs] & 0xFF);

    as->regs.gpr[ra] = (UInt32)(SInt32)byte;

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * EXTSH - Extend Sign Halfword
 * rA = EXTS(rS[16:31])
 */
void PPC_Op_EXTSH(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    SInt16 halfword = (SInt16)(as->regs.gpr[rs] & 0xFFFF);

    as->regs.gpr[ra] = (UInt32)(SInt32)halfword;

    if (rc) {
        PPC_SetCR0(as, (SInt32)as->regs.gpr[ra]);
    }
}

/*
 * CNTLZW - Count Leading Zeros Word
 * rA = count of consecutive zero bits starting at bit 0 of rS
 */
void PPC_Op_CNTLZW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    UInt32 value = as->regs.gpr[rs];
    UInt32 count = 0;

    if (value == 0) {
        count = 32;
    } else {
        while ((value & 0x80000000) == 0) {
            count++;
            value <<= 1;
        }
    }

    as->regs.gpr[ra] = count;

    if (rc) {
        PPC_SetCR0(as, (SInt32)count);
    }
}

/*
 * ============================================================================
 * SPECIAL REGISTER ACCESS
 * ============================================================================
 */

/*
 * MFCR - Move From Condition Register
 * rD = CR
 */
void PPC_Op_MFCR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    as->regs.gpr[rd] = as->regs.cr;
}

/*
 * MTCRF - Move To Condition Register Fields
 * CR = rS (with field mask)
 */
void PPC_Op_MTCRF(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 crm = (insn >> 12) & 0xFF; /* CRM field */
    UInt32 value = as->regs.gpr[rs];
    UInt32 mask = 0;

    /* Build mask from CRM field */
    for (int i = 0; i < 8; i++) {
        if (crm & (1 << (7 - i))) {
            mask |= 0xF << ((7 - i) * 4);
        }
    }

    as->regs.cr = (as->regs.cr & ~mask) | (value & mask);
}

/*
 * MFSPR - Move From Special Purpose Register
 * rD = SPR[spr]
 */
void PPC_Op_MFSPR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt16 spr = ((insn >> 11) & 0x1F) | (((insn >> 16) & 0x1F) << 5);

    switch (spr) {
        case 1:   /* XER */
            as->regs.gpr[rd] = as->regs.xer;
            break;
        case 8:   /* LR */
            as->regs.gpr[rd] = as->regs.lr;
            break;
        case 9:   /* CTR */
            as->regs.gpr[rd] = as->regs.ctr;
            break;
        case 22:  /* DEC - Decrementer */
            as->regs.gpr[rd] = as->regs.dec;
            break;
        case 268: /* TBL - Time Base Lower (read-only) */
            as->regs.gpr[rd] = as->regs.tbl;
            break;
        case 269: /* TBU - Time Base Upper (read-only) */
            as->regs.gpr[rd] = as->regs.tbu;
            break;
        case 287: /* PVR - Processor Version Register (read-only) */
            as->regs.gpr[rd] = as->regs.pvr;
            break;
        default:
            /* Unsupported SPR - return 0 */
            as->regs.gpr[rd] = 0;
            break;
    }
}

/*
 * MTSPR - Move To Special Purpose Register
 * SPR[spr] = rS
 */
void PPC_Op_MTSPR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt16 spr = ((insn >> 11) & 0x1F) | (((insn >> 16) & 0x1F) << 5);
    UInt32 value = as->regs.gpr[rs];

    switch (spr) {
        case 1:   /* XER */
            as->regs.xer = value;
            break;
        case 8:   /* LR */
            as->regs.lr = value;
            break;
        case 9:   /* CTR */
            as->regs.ctr = value;
            break;
        case 22:  /* DEC - Decrementer */
            as->regs.dec = value;
            break;
        case 268: /* TBL - Time Base Lower (supervisor write-only, ignore in user mode) */
        case 269: /* TBU - Time Base Upper (supervisor write-only, ignore in user mode) */
        case 287: /* PVR - Processor Version Register (read-only, ignore writes) */
        default:
            /* Unsupported or read-only SPR - ignore */
            break;
    }
}

/*
 * ============================================================================
 * TRAP INSTRUCTIONS
 * ============================================================================
 */

/* Helper: Check trap condition */
static Boolean PPC_CheckTrapCondition(UInt32 to, SInt32 a, SInt32 b)
{
    if ((to & 16) && (a < b)) return true;   /* LT */
    if ((to & 8) && (a > b)) return true;    /* GT */
    if ((to & 4) && (a == b)) return true;   /* EQ */
    if ((to & 2) && ((UInt32)a < (UInt32)b)) return true;  /* LTU */
    if ((to & 1) && ((UInt32)a > (UInt32)b)) return true;  /* GTU */
    return false;
}

/*
 * TW - Trap Word
 * Trap if condition is met
 */
void PPC_Op_TW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 to = PPC_RD(insn);  /* TO field */
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];

    if (PPC_CheckTrapCondition(to, a, b)) {
        PPC_Fault(as, "Trap condition met (TW)");
    }
}

/*
 * TWI - Trap Word Immediate
 * Trap if condition is met (immediate operand)
 */
void PPC_Op_TWI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 to = PPC_RD(insn);  /* TO field */
    UInt8 ra = PPC_RA(insn);
    SInt32 simm = PPC_SIMM(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];

    if (PPC_CheckTrapCondition(to, a, simm)) {
        PPC_Fault(as, "Trap condition met (TWI)");
    }
}

/*
 * ============================================================================
 * ATOMIC OPERATIONS
 * ============================================================================
 */

/*
 * LWARX - Load Word and Reserve Indexed
 * rD = MEM(rA + rB), set reservation
 */
void PPC_Op_LWARX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    as->regs.gpr[rd] = PPC_Read32(as, ea);

    /* Set reservation (simplified - just remember address) */
    /* In a real implementation, this would set a reservation bit */
    /* For now, we'll just do the load */
}

/*
 * STWCX - Store Word Conditional Indexed
 * If reservation valid: MEM(rA + rB) = rS, CR0[EQ] = 1
 * Else: CR0[EQ] = 0
 */
void PPC_Op_STWCX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        ea = as->regs.gpr[rb];
    } else {
        ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    }

    /* Simplified: always succeed (reservation always valid) */
    PPC_Write32(as, ea, as->regs.gpr[rs]);

    /* Set CR0[EQ] = 1 to indicate success */
    as->regs.cr = (as->regs.cr & 0x0FFFFFFF) | PPC_CR_EQ(0);
}

/*
 * ============================================================================
 * CACHE MANAGEMENT (implemented as NOPs)
 * ============================================================================
 */

/*
 * DCBZ - Data Cache Block Set to Zero
 * Clear cache block (implemented as NOP for interpreter)
 */
void PPC_Op_DCBZ(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP - cache operations not needed in interpreter */
}

/*
 * DCBST - Data Cache Block Store
 * Flush cache block to memory (implemented as NOP)
 */
void PPC_Op_DCBST(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP */
}

/*
 * DCBF - Data Cache Block Flush
 * Flush cache block (implemented as NOP)
 */
void PPC_Op_DCBF(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP */
}

/*
 * ICBI - Instruction Cache Block Invalidate
 * Invalidate instruction cache block (implemented as NOP)
 */
void PPC_Op_ICBI(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP */
}

/*
 * SYNC - Synchronize
 * Memory synchronization (implemented as NOP)
 */
void PPC_Op_SYNC(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP */
}

/*
 * ISYNC - Instruction Synchronize
 * Instruction synchronization (implemented as NOP)
 */
void PPC_Op_ISYNC(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP */
}

/*
 * ============================================================================
 * INDEXED UPDATE LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LWZUX - Load Word and Zero with Update Indexed
 * EA = rA + rB
 * rD = MEM(EA, 4)
 * rA = EA
 */
void PPC_Op_LWZUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "LWZUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    as->regs.gpr[rd] = PPC_Read32(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LBZUX - Load Byte and Zero with Update Indexed
 * EA = rA + rB
 * rD = MEM(EA, 1)
 * rA = EA
 */
void PPC_Op_LBZUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "LBZUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    as->regs.gpr[rd] = PPC_Read8(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LHZUX - Load Halfword and Zero with Update Indexed
 * EA = rA + rB
 * rD = MEM(EA, 2)
 * rA = EA
 */
void PPC_Op_LHZUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "LHZUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    as->regs.gpr[rd] = PPC_Read16(as, ea);
    as->regs.gpr[ra] = ea;
}

/*
 * LHAUX - Load Halfword Algebraic with Update Indexed
 * EA = rA + rB
 * rD = EXTS(MEM(EA, 2))
 * rA = EA
 */
void PPC_Op_LHAUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt16 value;

    if (ra == 0) {
        PPC_Fault(as, "LHAUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    value = PPC_Read16(as, ea);

    /* Sign extend */
    if (value & 0x8000) {
        as->regs.gpr[rd] = 0xFFFF0000 | value;
    } else {
        as->regs.gpr[rd] = value;
    }

    as->regs.gpr[ra] = ea;
}

/*
 * STWUX - Store Word with Update Indexed
 * EA = rA + rB
 * MEM(EA, 4) = rS
 * rA = EA
 */
void PPC_Op_STWUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "STWUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    PPC_Write32(as, ea, as->regs.gpr[rs]);
    as->regs.gpr[ra] = ea;
}

/*
 * STBUX - Store Byte with Update Indexed
 * EA = rA + rB
 * MEM(EA, 1) = rS[24:31]
 * rA = EA
 */
void PPC_Op_STBUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "STBUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    PPC_Write8(as, ea, (UInt8)(as->regs.gpr[rs] & 0xFF));
    as->regs.gpr[ra] = ea;
}

/*
 * STHUX - Store Halfword with Update Indexed
 * EA = rA + rB
 * MEM(EA, 2) = rS[16:31]
 * rA = EA
 */
void PPC_Op_STHUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;

    if (ra == 0) {
        PPC_Fault(as, "STHUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    PPC_Write16(as, ea, (UInt16)(as->regs.gpr[rs] & 0xFFFF));
    as->regs.gpr[ra] = ea;
}

/*
 * ============================================================================
 * LOAD HALFWORD ALGEBRAIC
 * ============================================================================
 */

/*
 * LHA - Load Halfword Algebraic
 * EA = (rA|0) + EXTS(d)
 * rD = EXTS(MEM(EA, 2))
 */
void PPC_Op_LHA(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt16 value;

    if (ra == 0) {
        ea = d;
    } else {
        ea = as->regs.gpr[ra] + d;
    }

    value = PPC_Read16(as, ea);

    /* Sign extend */
    if (value & 0x8000) {
        as->regs.gpr[rd] = 0xFFFF0000 | value;
    } else {
        as->regs.gpr[rd] = value;
    }
}

/*
 * ============================================================================
 * STRING LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LSWI - Load String Word Immediate
 * Load n bytes from EA into consecutive registers
 */
void PPC_Op_LSWI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 nb = PPC_RB(insn);
    UInt32 ea;
    UInt32 count;
    UInt8 reg;
    UInt32 value;
    int shift;

    if (nb == 0) {
        count = 32;
    } else {
        count = nb;
    }

    ea = (ra == 0) ? 0 : as->regs.gpr[ra];
    reg = rd;
    value = 0;
    shift = 24;

    for (UInt32 i = 0; i < count; i++) {
        value |= ((UInt32)PPC_Read8(as, ea + i)) << shift;
        shift -= 8;

        if (shift < 0 || i == count - 1) {
            as->regs.gpr[reg] = value;
            reg = (reg + 1) & 31;
            value = 0;
            shift = 24;
        }
    }
}

/*
 * LSWX - Load String Word Indexed
 * Load XER[25:31] bytes from EA into consecutive registers
 */
void PPC_Op_LSWX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 count;
    UInt8 reg;
    UInt32 value;
    int shift;

    count = as->regs.xer & 0x7F;

    if (count == 0) {
        return;
    }

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    reg = rd;
    value = 0;
    shift = 24;

    for (UInt32 i = 0; i < count; i++) {
        value |= ((UInt32)PPC_Read8(as, ea + i)) << shift;
        shift -= 8;

        if (shift < 0 || i == count - 1) {
            as->regs.gpr[reg] = value;
            reg = (reg + 1) & 31;
            value = 0;
            shift = 24;
        }
    }
}

/*
 * STSWI - Store String Word Immediate
 * Store n bytes from consecutive registers to memory
 */
void PPC_Op_STSWI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 nb = PPC_RB(insn);
    UInt32 ea;
    UInt32 count;
    UInt8 reg;
    UInt32 value;
    int shift;

    if (nb == 0) {
        count = 32;
    } else {
        count = nb;
    }

    ea = (ra == 0) ? 0 : as->regs.gpr[ra];
    reg = rs;
    value = as->regs.gpr[reg];
    shift = 24;

    for (UInt32 i = 0; i < count; i++) {
        PPC_Write8(as, ea + i, (UInt8)((value >> shift) & 0xFF));
        shift -= 8;

        if (shift < 0) {
            reg = (reg + 1) & 31;
            value = as->regs.gpr[reg];
            shift = 24;
        }
    }
}

/*
 * STSWX - Store String Word Indexed
 * Store XER[25:31] bytes from consecutive registers to memory
 */
void PPC_Op_STSWX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 count;
    UInt8 reg;
    UInt32 value;
    int shift;

    count = as->regs.xer & 0x7F;

    if (count == 0) {
        return;
    }

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    reg = rs;
    value = as->regs.gpr[reg];
    shift = 24;

    for (UInt32 i = 0; i < count; i++) {
        PPC_Write8(as, ea + i, (UInt8)((value >> shift) & 0xFF));
        shift -= 8;

        if (shift < 0) {
            reg = (reg + 1) & 31;
            value = as->regs.gpr[reg];
            shift = 24;
        }
    }
}

/*
 * ============================================================================
 * CONDITION REGISTER FIELD OPERATIONS
 * ============================================================================
 */

/*
 * MCRF - Move Condition Register Field
 * CR[4*crbD:4*crbD+3] = CR[4*crbA:4*crbA+3]
 */
void PPC_Op_MCRF(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbD = (insn >> 23) & 0x7;
    UInt8 crbA = (insn >> 18) & 0x7;
    UInt32 field_mask = 0xF << (28 - (crbA * 4));
    UInt32 field_value = as->regs.cr & field_mask;

    /* Clear destination field */
    as->regs.cr &= ~(0xF << (28 - (crbD * 4)));

    /* Move field */
    if (crbD != crbA) {
        if (crbD < crbA) {
            field_value <<= ((crbA - crbD) * 4);
        } else {
            field_value >>= ((crbD - crbA) * 4);
        }
    }

    as->regs.cr |= field_value;
}

/*
 * ============================================================================
 * OVERFLOW ARITHMETIC
 * ============================================================================
 */

/*
 * ADDO - Add with Overflow
 * rD = rA + rB
 * Sets OV if signed overflow occurs
 */
void PPC_Op_ADDO(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];
    SInt32 result = a + b;

    as->regs.gpr[rd] = (UInt32)result;

    /* Check for signed overflow */
    if ((a > 0 && b > 0 && result < 0) || (a < 0 && b < 0 && result > 0)) {
        as->regs.xer |= PPC_XER_OV | PPC_XER_SO;
    } else {
        as->regs.xer &= ~PPC_XER_OV;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * SUBFO - Subtract From with Overflow
 * rD = rB - rA
 * Sets OV if signed overflow occurs
 */
void PPC_Op_SUBFO(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];
    SInt32 result = b - a;

    as->regs.gpr[rd] = (UInt32)result;

    /* Check for signed overflow */
    if ((b > 0 && a < 0 && result < 0) || (b < 0 && a > 0 && result > 0)) {
        as->regs.xer |= PPC_XER_OV | PPC_XER_SO;
    } else {
        as->regs.xer &= ~PPC_XER_OV;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * NEGO - Negate with Overflow
 * rD = -rA
 * Sets OV if overflow occurs
 */
void PPC_Op_NEGO(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 result = -a;

    as->regs.gpr[rd] = (UInt32)result;

    /* Check for signed overflow (only happens for 0x80000000) */
    if (a == (SInt32)0x80000000) {
        as->regs.xer |= PPC_XER_OV | PPC_XER_SO;
    } else {
        as->regs.xer &= ~PPC_XER_OV;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * MULLWO - Multiply Low Word with Overflow
 * rD = rA * rB (low 32 bits)
 * Sets OV if overflow occurs
 */
void PPC_Op_MULLWO(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];
    SInt64 result64 = (SInt64)a * (SInt64)b;
    SInt32 result = (SInt32)(result64 & 0xFFFFFFFF);

    as->regs.gpr[rd] = (UInt32)result;

    /* Check for overflow */
    if (result64 != (SInt64)result) {
        as->regs.xer |= PPC_XER_OV | PPC_XER_SO;
    } else {
        as->regs.xer &= ~PPC_XER_OV;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * DIVWO - Divide Word with Overflow
 * rD = rA / rB
 * Sets OV if overflow occurs (divide by zero or 0x80000000 / -1)
 */
void PPC_Op_DIVWO(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);
    SInt32 a = (SInt32)as->regs.gpr[ra];
    SInt32 b = (SInt32)as->regs.gpr[rb];
    SInt32 result;

    if (b == 0 || (a == (SInt32)0x80000000 && b == -1)) {
        /* Overflow condition */
        as->regs.gpr[rd] = 0;
        as->regs.xer |= PPC_XER_OV | PPC_XER_SO;
        result = 0;
    } else {
        result = a / b;
        as->regs.gpr[rd] = (UInt32)result;
        as->regs.xer &= ~PPC_XER_OV;
    }

    if (rc) {
        PPC_SetCR0(as, result);
    }
}

/*
 * ============================================================================
 * ADDITIONAL SPR ACCESS
 * ============================================================================
 */

/*
 * Additional SPRs implemented:
 * - TBL/TBU (268/269): Time Base Lower/Upper (read-only in user mode)
 * - PVR (287): Processor Version Register (read-only)
 * - DEC (22): Decrementer (read-write, supervisor)
 *
 * Note: These use the existing MFSPR/MTSPR infrastructure, just need
 * to handle the new SPR numbers in those functions.
 */

/*
 * ============================================================================
 * BYTE-REVERSED LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LWBRX - Load Word Byte-Reverse Indexed
 * Load word with bytes reversed (little-endian load)
 */
void PPC_Op_LWBRX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt8 b0, b1, b2, b3;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];

    /* Read bytes individually */
    b0 = PPC_Read8(as, ea + 0);
    b1 = PPC_Read8(as, ea + 1);
    b2 = PPC_Read8(as, ea + 2);
    b3 = PPC_Read8(as, ea + 3);

    /* Reverse byte order */
    as->regs.gpr[rd] = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

/*
 * LHBRX - Load Halfword Byte-Reverse Indexed
 * Load halfword with bytes reversed
 */
void PPC_Op_LHBRX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt8 b0, b1;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];

    b0 = PPC_Read8(as, ea + 0);
    b1 = PPC_Read8(as, ea + 1);

    as->regs.gpr[rd] = (b1 << 8) | b0;
}

/*
 * STWBRX - Store Word Byte-Reverse Indexed
 * Store word with bytes reversed
 */
void PPC_Op_STWBRX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 value;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    value = as->regs.gpr[rs];

    /* Write bytes in reversed order */
    PPC_Write8(as, ea + 0, (value >> 0) & 0xFF);
    PPC_Write8(as, ea + 1, (value >> 8) & 0xFF);
    PPC_Write8(as, ea + 2, (value >> 16) & 0xFF);
    PPC_Write8(as, ea + 3, (value >> 24) & 0xFF);
}

/*
 * STHBRX - Store Halfword Byte-Reverse Indexed
 * Store halfword with bytes reversed
 */
void PPC_Op_STHBRX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 value;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    value = as->regs.gpr[rs];

    PPC_Write8(as, ea + 0, (value >> 0) & 0xFF);
    PPC_Write8(as, ea + 1, (value >> 8) & 0xFF);
}

/*
 * ============================================================================
 * FLOATING-POINT LOAD/STORE INSTRUCTIONS
 * ============================================================================
 */

/*
 * LFS - Load Floating-Point Single
 * Load 32-bit float and convert to 64-bit double in FPR
 */
void PPC_Op_LFS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt32 value;
    float f;

    ea = (ra == 0) ? d : (as->regs.gpr[ra] + d);
    value = PPC_Read32(as, ea);

    /* Convert bits to float, then to double */
    memcpy(&f, &value, 4);
    as->regs.fpr[frd] = (double)f;
}

/*
 * LFSU - Load Floating-Point Single with Update
 */
void PPC_Op_LFSU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt32 value;
    float f;

    if (ra == 0) {
        PPC_Fault(as, "LFSU with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + d;
    value = PPC_Read32(as, ea);

    memcpy(&f, &value, 4);
    as->regs.fpr[frd] = (double)f;
    as->regs.gpr[ra] = ea;
}

/*
 * LFSX - Load Floating-Point Single Indexed
 */
void PPC_Op_LFSX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 value;
    float f;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    value = PPC_Read32(as, ea);

    memcpy(&f, &value, 4);
    as->regs.fpr[frd] = (double)f;
}

/*
 * LFSUX - Load Floating-Point Single with Update Indexed
 */
void PPC_Op_LFSUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 value;
    float f;

    if (ra == 0) {
        PPC_Fault(as, "LFSUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    value = PPC_Read32(as, ea);

    memcpy(&f, &value, 4);
    as->regs.fpr[frd] = (double)f;
    as->regs.gpr[ra] = ea;
}

/*
 * LFD - Load Floating-Point Double
 */
void PPC_Op_LFD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt32 hi, lo;
    UInt64 value64;

    ea = (ra == 0) ? d : (as->regs.gpr[ra] + d);
    hi = PPC_Read32(as, ea);
    lo = PPC_Read32(as, ea + 4);

    value64 = ((UInt64)hi << 32) | lo;
    memcpy(&as->regs.fpr[frd], &value64, 8);
}

/*
 * LFDU - Load Floating-Point Double with Update
 */
void PPC_Op_LFDU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt32 hi, lo;
    UInt64 value64;

    if (ra == 0) {
        PPC_Fault(as, "LFDU with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + d;
    hi = PPC_Read32(as, ea);
    lo = PPC_Read32(as, ea + 4);

    value64 = ((UInt64)hi << 32) | lo;
    memcpy(&as->regs.fpr[frd], &value64, 8);
    as->regs.gpr[ra] = ea;
}

/*
 * LFDX - Load Floating-Point Double Indexed
 */
void PPC_Op_LFDX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 hi, lo;
    UInt64 value64;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    hi = PPC_Read32(as, ea);
    lo = PPC_Read32(as, ea + 4);

    value64 = ((UInt64)hi << 32) | lo;
    memcpy(&as->regs.fpr[frd], &value64, 8);
}

/*
 * LFDUX - Load Floating-Point Double with Update Indexed
 */
void PPC_Op_LFDUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt32 hi, lo;
    UInt64 value64;

    if (ra == 0) {
        PPC_Fault(as, "LFDUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    hi = PPC_Read32(as, ea);
    lo = PPC_Read32(as, ea + 4);

    value64 = ((UInt64)hi << 32) | lo;
    memcpy(&as->regs.fpr[frd], &value64, 8);
    as->regs.gpr[ra] = ea;
}

/*
 * STFS - Store Floating-Point Single
 */
void PPC_Op_STFS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    float f;
    UInt32 value;

    ea = (ra == 0) ? d : (as->regs.gpr[ra] + d);
    f = (float)as->regs.fpr[frs];
    memcpy(&value, &f, 4);
    PPC_Write32(as, ea, value);
}

/*
 * STFSU - Store Floating-Point Single with Update
 */
void PPC_Op_STFSU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    float f;
    UInt32 value;

    if (ra == 0) {
        PPC_Fault(as, "STFSU with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + d;
    f = (float)as->regs.fpr[frs];
    memcpy(&value, &f, 4);
    PPC_Write32(as, ea, value);
    as->regs.gpr[ra] = ea;
}

/*
 * STFSX - Store Floating-Point Single Indexed
 */
void PPC_Op_STFSX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    float f;
    UInt32 value;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    f = (float)as->regs.fpr[frs];
    memcpy(&value, &f, 4);
    PPC_Write32(as, ea, value);
}

/*
 * STFSUX - Store Floating-Point Single with Update Indexed
 */
void PPC_Op_STFSUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    float f;
    UInt32 value;

    if (ra == 0) {
        PPC_Fault(as, "STFSUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    f = (float)as->regs.fpr[frs];
    memcpy(&value, &f, 4);
    PPC_Write32(as, ea, value);
    as->regs.gpr[ra] = ea;
}

/*
 * STFD - Store Floating-Point Double
 */
void PPC_Op_STFD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt64 value64;

    ea = (ra == 0) ? d : (as->regs.gpr[ra] + d);
    memcpy(&value64, &as->regs.fpr[frs], 8);
    PPC_Write32(as, ea, (value64 >> 32) & 0xFFFFFFFF);
    PPC_Write32(as, ea + 4, value64 & 0xFFFFFFFF);
}

/*
 * STFDU - Store Floating-Point Double with Update
 */
void PPC_Op_STFDU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    SInt16 d = (SInt16)(insn & 0xFFFF);
    UInt32 ea;
    UInt64 value64;

    if (ra == 0) {
        PPC_Fault(as, "STFDU with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + d;
    memcpy(&value64, &as->regs.fpr[frs], 8);
    PPC_Write32(as, ea, (value64 >> 32) & 0xFFFFFFFF);
    PPC_Write32(as, ea + 4, value64 & 0xFFFFFFFF);
    as->regs.gpr[ra] = ea;
}

/*
 * STFDX - Store Floating-Point Double Indexed
 */
void PPC_Op_STFDX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt64 value64;

    ea = ((ra == 0) ? 0 : as->regs.gpr[ra]) + as->regs.gpr[rb];
    memcpy(&value64, &as->regs.fpr[frs], 8);
    PPC_Write32(as, ea, (value64 >> 32) & 0xFFFFFFFF);
    PPC_Write32(as, ea + 4, value64 & 0xFFFFFFFF);
}

/*
 * STFDUX - Store Floating-Point Double with Update Indexed
 */
void PPC_Op_STFDUX(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frs = PPC_RS(insn);
    UInt8 ra = PPC_RA(insn);
    UInt8 rb = PPC_RB(insn);
    UInt32 ea;
    UInt64 value64;

    if (ra == 0) {
        PPC_Fault(as, "STFDUX with rA=0");
        return;
    }

    ea = as->regs.gpr[ra] + as->regs.gpr[rb];
    memcpy(&value64, &as->regs.fpr[frs], 8);
    PPC_Write32(as, ea, (value64 >> 32) & 0xFFFFFFFF);
    PPC_Write32(as, ea + 4, value64 & 0xFFFFFFFF);
    as->regs.gpr[ra] = ea;
}

/*
 * ============================================================================
 * FLOATING-POINT ARITHMETIC INSTRUCTIONS
 * ============================================================================
 */

/* Helper: Update FPSCR flags after FP operation */
static void PPC_UpdateFPSCR(PPCAddressSpace* as, double result)
{
    /* Simplified FPSCR update - just handle basic flags */
    as->regs.fpscr &= ~0xF0000000; /* Clear FPRF */

    if (result < 0.0) {
        as->regs.fpscr |= 0x80000000; /* FL - Less than */
    } else if (result > 0.0) {
        as->regs.fpscr |= 0x40000000; /* FG - Greater than */
    } else if (result == 0.0) {
        as->regs.fpscr |= 0x20000000; /* FE - Equal */
    }
}

/*
 * FADD - Floating Add (Double-Precision)
 */
void PPC_Op_FADD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = as->regs.fpr[fra] + as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FADDS - Floating Add Single
 */
void PPC_Op_FADDS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)((float)as->regs.fpr[fra] + (float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FSUB - Floating Subtract (Double-Precision)
 */
void PPC_Op_FSUB(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = as->regs.fpr[fra] - as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FSUBS - Floating Subtract Single
 */
void PPC_Op_FSUBS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)((float)as->regs.fpr[fra] - (float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMUL - Floating Multiply (Double-Precision)
 */
void PPC_Op_FMUL(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = as->regs.fpr[fra] * as->regs.fpr[frc];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMULS - Floating Multiply Single
 */
void PPC_Op_FMULS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)((float)as->regs.fpr[fra] * (float)as->regs.fpr[frc]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FDIV - Floating Divide (Double-Precision)
 */
void PPC_Op_FDIV(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = as->regs.fpr[fra] / as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FDIVS - Floating Divide Single
 */
void PPC_Op_FDIVS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)((float)as->regs.fpr[fra] / (float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FSQRT - Floating Square Root (Double-Precision)
 */
void PPC_Op_FSQRT(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = sqrt(as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FSQRTS - Floating Square Root Single
 */
void PPC_Op_FSQRTS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)sqrtf((float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FRES - Floating Reciprocal Estimate Single
 */
void PPC_Op_FRES(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)(1.0f / (float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FRSQRTE - Floating Reciprocal Square Root Estimate
 */
void PPC_Op_FRSQRTE(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = 1.0 / sqrt(as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMADD - Floating Multiply-Add (Double-Precision)
 * frD = (frA * frC) + frB
 */
void PPC_Op_FMADD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (as->regs.fpr[fra] * as->regs.fpr[frc]) + as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMADDS - Floating Multiply-Add Single
 */
void PPC_Op_FMADDS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    float result = ((float)as->regs.fpr[fra] * (float)as->regs.fpr[frc]) + (float)as->regs.fpr[frb];
    as->regs.fpr[frd] = (double)result;

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMSUB - Floating Multiply-Subtract (Double-Precision)
 * frD = (frA * frC) - frB
 */
void PPC_Op_FMSUB(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (as->regs.fpr[fra] * as->regs.fpr[frc]) - as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMSUBS - Floating Multiply-Subtract Single
 */
void PPC_Op_FMSUBS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    float result = ((float)as->regs.fpr[fra] * (float)as->regs.fpr[frc]) - (float)as->regs.fpr[frb];
    as->regs.fpr[frd] = (double)result;

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNMADD - Floating Negative Multiply-Add
 * frD = -((frA * frC) + frB)
 */
void PPC_Op_FNMADD(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = -((as->regs.fpr[fra] * as->regs.fpr[frc]) + as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNMADDS - Floating Negative Multiply-Add Single
 */
void PPC_Op_FNMADDS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    float result = -(((float)as->regs.fpr[fra] * (float)as->regs.fpr[frc]) + (float)as->regs.fpr[frb]);
    as->regs.fpr[frd] = (double)result;

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNMSUB - Floating Negative Multiply-Subtract
 * frD = -((frA * frC) - frB)
 */
void PPC_Op_FNMSUB(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = -((as->regs.fpr[fra] * as->regs.fpr[frc]) - as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNMSUBS - Floating Negative Multiply-Subtract Single
 */
void PPC_Op_FNMSUBS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;
    Boolean rc = PPC_RC(insn);

    float result = -(((float)as->regs.fpr[fra] * (float)as->regs.fpr[frc]) - (float)as->regs.fpr[frb]);
    as->regs.fpr[frd] = (double)result;

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FABS - Floating Absolute Value
 */
void PPC_Op_FABS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = fabs(as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNEG - Floating Negate
 */
void PPC_Op_FNEG(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = -as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FNABS - Floating Negative Absolute Value
 */
void PPC_Op_FNABS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = -fabs(as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FMR - Floating Move Register
 */
void PPC_Op_FMR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = as->regs.fpr[frb];

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * ============================================================================
 * FLOATING-POINT COMPARE AND CONVERT INSTRUCTIONS
 * ============================================================================
 */

/*
 * FCMPU - Floating Compare Unordered
 */
void PPC_Op_FCMPU(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = (insn >> 23) & 0x7;
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt32 cr_bits = 0;

    if (as->regs.fpr[fra] < as->regs.fpr[frb]) {
        cr_bits = 0x8; /* LT */
    } else if (as->regs.fpr[fra] > as->regs.fpr[frb]) {
        cr_bits = 0x4; /* GT */
    } else {
        cr_bits = 0x2; /* EQ */
    }

    /* Update CR field */
    as->regs.cr &= ~(0xF << (28 - (crfd * 4)));
    as->regs.cr |= (cr_bits << (28 - (crfd * 4)));
}

/*
 * FCMPO - Floating Compare Ordered
 */
void PPC_Op_FCMPO(PPCAddressSpace* as, UInt32 insn)
{
    /* For now, same as FCMPU (simplified) */
    PPC_Op_FCMPU(as, insn);
}

/*
 * FCTIW - Floating Convert to Integer Word
 */
void PPC_Op_FCTIW(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    SInt32 result;
    UInt64 value64;

    result = (SInt32)as->regs.fpr[frb];
    value64 = (UInt32)result;
    memcpy(&as->regs.fpr[frd], &value64, 8);
}

/*
 * FCTIWZ - Floating Convert to Integer Word with Round toward Zero
 */
void PPC_Op_FCTIWZ(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    SInt32 result;
    UInt64 value64;

    result = (SInt32)trunc(as->regs.fpr[frb]);
    value64 = (UInt32)result;
    memcpy(&as->regs.fpr[frd], &value64, 8);
}

/*
 * FRSP - Floating Round to Single-Precision
 */
void PPC_Op_FRSP(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 frb = PPC_RB(insn);
    Boolean rc = PPC_RC(insn);

    as->regs.fpr[frd] = (double)((float)as->regs.fpr[frb]);

    if (rc) {
        PPC_UpdateFPSCR(as, as->regs.fpr[frd]);
    }
}

/*
 * FSEL - Floating Select
 * If frA >= 0 then frD = frC else frD = frB
 */
void PPC_Op_FSEL(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt8 fra = PPC_RA(insn);
    UInt8 frb = PPC_RB(insn);
    UInt8 frc = (insn >> 6) & 0x1F;

    if (as->regs.fpr[fra] >= 0.0) {
        as->regs.fpr[frd] = as->regs.fpr[frc];
    } else {
        as->regs.fpr[frd] = as->regs.fpr[frb];
    }
}

/*
 * ============================================================================
 * FLOATING-POINT STATUS AND CONTROL REGISTER INSTRUCTIONS
 * ============================================================================
 */

/*
 * MFFS - Move from FPSCR
 */
void PPC_Op_MFFS(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 frd = PPC_RD(insn);
    UInt64 value64 = as->regs.fpscr;

    memcpy(&as->regs.fpr[frd], &value64, 8);
}

/*
 * MTFSF - Move to FPSCR Fields
 */
void PPC_Op_MTFSF(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 fm = (insn >> 17) & 0xFF;
    UInt8 frb = PPC_RB(insn);
    UInt64 value64;
    UInt32 mask = 0;

    memcpy(&value64, &as->regs.fpr[frb], 8);

    /* Build mask from FM field */
    for (int i = 0; i < 8; i++) {
        if (fm & (1 << (7 - i))) {
            mask |= (0xF << (28 - (i * 4)));
        }
    }

    as->regs.fpscr = (as->regs.fpscr & ~mask) | ((UInt32)value64 & mask);
}

/*
 * MTFSFI - Move to FPSCR Field Immediate
 */
void PPC_Op_MTFSFI(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crfd = (insn >> 23) & 0x7;
    UInt8 imm = (insn >> 12) & 0xF;
    UInt32 mask = 0xF << (28 - (crfd * 4));

    as->regs.fpscr = (as->regs.fpscr & ~mask) | (imm << (28 - (crfd * 4)));
}

/*
 * MTFSB0 - Move to FPSCR Bit 0
 */
void PPC_Op_MTFSB0(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = (insn >> 21) & 0x1F;

    as->regs.fpscr &= ~(1 << (31 - crbd));
}

/*
 * MTFSB1 - Move to FPSCR Bit 1
 */
void PPC_Op_MTFSB1(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 crbd = (insn >> 21) & 0x1F;

    as->regs.fpscr |= (1 << (31 - crbd));
}

/*
 * ============================================================================
 * SYSTEM AND PRIVILEGED INSTRUCTIONS
 * ============================================================================
 */

/*
 * EIEIO - Enforce In-Order Execution of I/O
 * (Implemented as NOP in interpreter)
 */
void PPC_Op_EIEIO(PPCAddressSpace* as, UInt32 insn)
{
    (void)as;
    (void)insn;
    /* NOP - memory ordering not needed in interpreter */
}

/*
 * MFMSR - Move from Machine State Register
 */
void PPC_Op_MFMSR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rd = PPC_RD(insn);

    as->regs.gpr[rd] = as->regs.msr;
}

/*
 * MTMSR - Move to Machine State Register
 */
void PPC_Op_MTMSR(PPCAddressSpace* as, UInt32 insn)
{
    UInt8 rs = PPC_RS(insn);

    as->regs.msr = as->regs.gpr[rs];
}

/*
 * RFI - Return from Interrupt
 * Simplified implementation
 */
void PPC_Op_RFI(PPCAddressSpace* as, UInt32 insn)
{
    (void)insn;

    /* Restore PC from SRR0 and MSR from SRR1 */
    /* For now, just a placeholder */
    as->halted = true;
}

/*
 * ==================================================
 * COMPREHENSIVE IMPLEMENTATION
 * Total: 206 instructions (119 previous + 87 new)
 * ==================================================
 */

