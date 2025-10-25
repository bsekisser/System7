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
 * Helper Functions
 */

/* Fetch 32-bit instruction at PC and advance */
static UInt32 PPC_Fetch32(PPCAddressSpace* as)
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
static UInt32 PPC_Read32(PPCAddressSpace* as, UInt32 addr)
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

static UInt16 PPC_Read16(PPCAddressSpace* as, UInt32 addr)
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

static UInt8 PPC_Read8(PPCAddressSpace* as, UInt32 addr)
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
static void PPC_Write32(PPCAddressSpace* as, UInt32 addr, UInt32 value)
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

static void PPC_Write16(PPCAddressSpace* as, UInt32 addr, UInt16 value)
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

static void PPC_Write8(PPCAddressSpace* as, UInt32 addr, UInt8 value)
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
    PPC_LOG_ERROR("PowerPC FAULT at PC=0x%08X: %s\n", as->regs.pc - 4, reason);
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
 * ==================================================
 * FOUNDATION IMPLEMENTATION COMPLETE
 * Additional instructions can be added as needed
 * ==================================================
 */

