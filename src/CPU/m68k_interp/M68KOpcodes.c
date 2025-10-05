/*
 * M68KOpcodes.c - 68K Instruction Handlers (Phase-1 MVP)
 *
 * Implements opcode handlers for the MVP instruction set:
 * MOVE, MOVEA, LEA, PEA, CLR, NOT, ADD, SUB, CMP, LINK, UNLK,
 * JSR, JMP, BRA, Bcc, BSR, RTS, TRAP
 */

#include "CPU/M68KInterp.h"
#include "CPU/M68KOpcodes.h"
#include "System71StdLib.h"
#include <string.h>

/*
 * External declarations from M68KDecode.c
 */
extern UInt16 M68K_Fetch16(M68KAddressSpace* as);
extern UInt32 M68K_Fetch32(M68KAddressSpace* as);
extern UInt8 M68K_Read8(M68KAddressSpace* as, UInt32 addr);
extern UInt16 M68K_Read16(M68KAddressSpace* as, UInt32 addr);
extern UInt32 M68K_Read32(M68KAddressSpace* as, UInt32 addr);
extern void M68K_Write8(M68KAddressSpace* as, UInt32 addr, UInt8 value);
extern void M68K_Write16(M68KAddressSpace* as, UInt32 addr, UInt16 value);
extern void M68K_Write32(M68KAddressSpace* as, UInt32 addr, UInt32 value);
extern UInt32 M68K_EA_ComputeAddress(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size);
extern UInt32 M68K_EA_Read(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size);
extern void M68K_EA_Write(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size, UInt32 value);

/*
 * Exception Handler - Raise 68K exception
 */
static void M68K_RaiseException(M68KAddressSpace* as, UInt16 vector, const char* reason)
{
    UInt32 vectorAddr;
    UInt32 handlerPC;
    const char* vecName;

    /* Map vector number to name */
    switch (vector) {
        case M68K_VEC_BUS_ERROR:      vecName = "BUS ERROR"; break;
        case M68K_VEC_ADDRESS_ERROR:  vecName = "ADDRESS ERROR"; break;
        case M68K_VEC_ILLEGAL:        vecName = "ILLEGAL"; break;
        case M68K_VEC_DIVIDE_ZERO:    vecName = "DIVIDE_ZERO"; break;
        case M68K_VEC_CHK:            vecName = "CHK"; break;
        case M68K_VEC_TRAPV:          vecName = "TRAPV"; break;
        case M68K_VEC_PRIVILEGE:      vecName = "PRIVILEGE"; break;
        case M68K_VEC_TRACE:          vecName = "TRACE"; break;
        case M68K_VEC_LINE_A:         vecName = "LINE_A"; break;
        case M68K_VEC_LINE_F:         vecName = "LINE_F"; break;
        default:                      vecName = "UNKNOWN"; break;
    }

    serial_printf("[M68K] EXCEPTION vec=%d (%s) at PC=0x%08X: %s\n",
                 vector, vecName, as->regs.pc, reason);

    as->lastException = vector;

    /* Read exception vector from memory (vectors at 0x0000 + vec*4) */
    vectorAddr = vector * 4;
    if (vectorAddr + 3 < as->memorySize) {
        UInt8* mem = (UInt8*)as->memory;
        handlerPC = (mem[vectorAddr] << 24) |
                   (mem[vectorAddr + 1] << 16) |
                   (mem[vectorAddr + 2] << 8) |
                   mem[vectorAddr + 3];

        /* If handler is NULL or invalid, halt */
        if (handlerPC == 0 || handlerPC >= as->memorySize) {
            serial_printf("[M68K] Exception handler NULL or invalid (0x%08X), halting\n", handlerPC);
            as->halted = true;
        } else {
            /* For now, just log and halt (RTE stub not yet implemented) */
            serial_printf("[M68K] Exception handler at 0x%08X (not invoking yet, halting)\n", handlerPC);
            as->halted = true;
        }
    } else {
        serial_printf("[M68K] Exception vector table not initialized, halting\n");
        as->halted = true;
    }
}

/*
 * Fault Handler (legacy wrapper)
 */
void M68K_Fault(M68KAddressSpace* as, const char* reason)
{
    /* Classify fault and raise appropriate exception */
    if (strstr(reason, "Address error") || strstr(reason, "odd address")) {
        M68K_RaiseException(as, M68K_VEC_ADDRESS_ERROR, reason);
    } else if (strstr(reason, "out of bounds") || strstr(reason, "unmapped")) {
        M68K_RaiseException(as, M68K_VEC_BUS_ERROR, reason);
    } else if (strstr(reason, "Illegal") || strstr(reason, "ILLEGAL")) {
        M68K_RaiseException(as, M68K_VEC_ILLEGAL, reason);
    } else {
        /* Generic fault - use ILLEGAL */
        M68K_RaiseException(as, M68K_VEC_ILLEGAL, reason);
    }
}

/*
 * CCR Flag Helpers
 */
static inline void M68K_SetFlag(M68KAddressSpace* as, UInt16 flag)
{
    as->regs.sr |= flag;
}

static inline void M68K_ClearFlag(M68KAddressSpace* as, UInt16 flag)
{
    as->regs.sr &= ~flag;
}

static inline Boolean M68K_TestFlag(M68KAddressSpace* as, UInt16 flag)
{
    return (as->regs.sr & flag) != 0;
}

static void M68K_SetNZ(M68KAddressSpace* as, UInt32 value, M68KSize size)
{
    /* Set Z if value is zero */
    if ((value & SIZE_MASK(size)) == 0) {
        M68K_SetFlag(as, CCR_Z);
    } else {
        M68K_ClearFlag(as, CCR_Z);
    }

    /* Set N if MSB is set */
    if (value & SIZE_SIGN_BIT(size)) {
        M68K_SetFlag(as, CCR_N);
    } else {
        M68K_ClearFlag(as, CCR_N);
    }
}

/*
 * Condition Code Testing
 */
Boolean M68K_TestCondition(UInt16 sr, M68KCondition cc)
{
    Boolean c = (sr & CCR_C) != 0;
    Boolean v = (sr & CCR_V) != 0;
    Boolean z = (sr & CCR_Z) != 0;
    Boolean n = (sr & CCR_N) != 0;

    switch (cc) {
        case CC_T:  return true;
        case CC_F:  return false;
        case CC_HI: return !c && !z;
        case CC_LS: return c || z;
        case CC_CC: return !c;
        case CC_CS: return c;
        case CC_NE: return !z;
        case CC_EQ: return z;
        case CC_VC: return !v;
        case CC_VS: return v;
        case CC_PL: return !n;
        case CC_MI: return n;
        case CC_GE: return (n && v) || (!n && !v);
        case CC_LT: return (n && !v) || (!n && v);
        case CC_GT: return !z && ((n && v) || (!n && !v));
        case CC_LE: return z || (n && !v) || (!n && v);
        default:    return false;
    }
}

/*
 * Stack Push/Pop Helpers
 */
static void M68K_Push32(M68KAddressSpace* as, UInt32 value)
{
    as->regs.a[7] -= 4;
    M68K_Write32(as, as->regs.a[7], value);
}

static UInt32 M68K_Pop32(M68KAddressSpace* as)
{
    UInt32 value = M68K_Read32(as, as->regs.a[7]);
    as->regs.a[7] += 4;
    return value;
}

static void M68K_Push16(M68KAddressSpace* as, UInt16 value)
{
    as->regs.a[7] -= 2;
    M68K_Write16(as, as->regs.a[7], value);
}

static UInt16 M68K_Pop16(M68KAddressSpace* as)
{
    UInt16 value = M68K_Read16(as, as->regs.a[7]);
    as->regs.a[7] += 2;
    return value;
}

/*
 * MOVE - Move data
 */
void M68K_Op_MOVE(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 size_bits = (opcode >> 12) & 3;
    M68KSize size;
    UInt8 dst_reg = (opcode >> 9) & 7;
    UInt8 dst_mode = (opcode >> 6) & 7;
    UInt8 src_mode = (opcode >> 3) & 7;
    UInt8 src_reg = opcode & 7;
    UInt32 value;

    /* Decode size: 01=byte, 11=word, 10=long */
    if (size_bits == 1) size = SIZE_BYTE;
    else if (size_bits == 3) size = SIZE_WORD;
    else if (size_bits == 2) size = SIZE_LONG;
    else {
        M68K_Fault(as, "Invalid MOVE size");
        return;
    }

    /* Read source */
    value = M68K_EA_Read(as, src_mode, src_reg, size);

    /* Write destination */
    M68K_EA_Write(as, dst_mode, dst_reg, size, value);

    /* Set flags */
    M68K_SetNZ(as, value, size);
    M68K_ClearFlag(as, CCR_V | CCR_C);
}

/*
 * MOVEA - Move to address register
 */
void M68K_Op_MOVEA(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 size_bits = (opcode >> 12) & 3;
    M68KSize size = (size_bits == 3) ? SIZE_WORD : SIZE_LONG;
    UInt8 dst_reg = (opcode >> 9) & 7;
    UInt8 src_mode = (opcode >> 3) & 7;
    UInt8 src_reg = opcode & 7;
    UInt32 value;

    /* Read source */
    value = M68K_EA_Read(as, src_mode, src_reg, size);

    /* Sign-extend if word */
    if (size == SIZE_WORD) {
        value = SIGN_EXTEND_WORD(value);
    }

    /* Write to address register */
    as->regs.a[dst_reg] = value;

    /* MOVEA does not affect flags */
}

/*
 * LEA - Load Effective Address
 */
void M68K_Op_LEA(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 dst_reg = (opcode >> 9) & 7;
    UInt8 src_mode = (opcode >> 3) & 7;
    UInt8 src_reg = opcode & 7;
    UInt32 addr;

    /* Compute effective address */
    addr = M68K_EA_ComputeAddress(as, src_mode, src_reg, SIZE_LONG);

    /* Write to address register */
    as->regs.a[dst_reg] = addr;

    /* LEA does not affect flags */
}

/*
 * PEA - Push Effective Address
 */
void M68K_Op_PEA(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 src_mode = (opcode >> 3) & 7;
    UInt8 src_reg = opcode & 7;
    UInt32 addr;

    /* Compute effective address */
    addr = M68K_EA_ComputeAddress(as, src_mode, src_reg, SIZE_LONG);

    /* Push to stack */
    M68K_Push32(as, addr);

    /* PEA does not affect flags */
}

/*
 * CLR - Clear operand
 */
void M68K_Op_CLR(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 size = (opcode >> 6) & 3;
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;

    /* Write zero */
    M68K_EA_Write(as, mode, reg, size, 0);

    /* Set flags: Z=1, N=V=C=0 */
    M68K_SetFlag(as, CCR_Z);
    M68K_ClearFlag(as, CCR_N | CCR_V | CCR_C);
}

/*
 * NOT - Logical complement
 */
void M68K_Op_NOT(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 size = (opcode >> 6) & 3;
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;
    UInt32 value;

    /* Read value */
    value = M68K_EA_Read(as, mode, reg, size);

    /* Complement */
    value = (~value) & SIZE_MASK(size);

    /* Write back */
    M68K_EA_Write(as, mode, reg, size, value);

    /* Set flags */
    M68K_SetNZ(as, value, size);
    M68K_ClearFlag(as, CCR_V | CCR_C);
}

/*
 * ADD - Add to data register
 */
void M68K_Op_ADD(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 reg = (opcode >> 9) & 7;
    UInt8 dir = (opcode >> 8) & 1;  /* 0=EA+Dn->EA, 1=Dn+EA->Dn */
    UInt8 size = (opcode >> 6) & 3;
    UInt8 ea_mode = (opcode >> 3) & 7;
    UInt8 ea_reg = opcode & 7;
    UInt32 src, dst, result;
    UInt32 mask = SIZE_MASK(size);

    if (dir == 1) {
        /* Dn + EA -> Dn */
        src = M68K_EA_Read(as, ea_mode, ea_reg, size);
        dst = as->regs.d[reg] & mask;
        result = (dst + src) & mask;
        as->regs.d[reg] = (as->regs.d[reg] & ~mask) | result;
    } else {
        /* EA + Dn -> EA */
        dst = M68K_EA_Read(as, ea_mode, ea_reg, size);
        src = as->regs.d[reg] & mask;
        result = (dst + src) & mask;
        M68K_EA_Write(as, ea_mode, ea_reg, size, result);
    }

    /* Set flags */
    M68K_SetNZ(as, result, size);

    /* Set C and X if carry occurred */
    if ((dst + src) > mask) {
        M68K_SetFlag(as, CCR_C | CCR_X);
    } else {
        M68K_ClearFlag(as, CCR_C | CCR_X);
    }

    /* Set V if signed overflow occurred (src and dst same sign, result different) */
    {
        UInt32 sign_bit = SIZE_SIGN_BIT(size);
        Boolean src_neg = (src & sign_bit) != 0;
        Boolean dst_neg = (dst & sign_bit) != 0;
        Boolean res_neg = (result & sign_bit) != 0;

        if (src_neg == dst_neg && res_neg != dst_neg) {
            M68K_SetFlag(as, CCR_V);
        } else {
            M68K_ClearFlag(as, CCR_V);
        }
    }
}

/*
 * SUB - Subtract from data register
 */
void M68K_Op_SUB(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 reg = (opcode >> 9) & 7;
    UInt8 dir = (opcode >> 8) & 1;  /* 0=Dn-EA->EA, 1=Dn-EA->Dn */
    UInt8 size = (opcode >> 6) & 3;
    UInt8 ea_mode = (opcode >> 3) & 7;
    UInt8 ea_reg = opcode & 7;
    UInt32 src, dst, result;
    UInt32 mask = SIZE_MASK(size);

    if (dir == 1) {
        /* Dn - EA -> Dn */
        dst = as->regs.d[reg] & mask;
        src = M68K_EA_Read(as, ea_mode, ea_reg, size);
        result = (dst - src) & mask;
        as->regs.d[reg] = (as->regs.d[reg] & ~mask) | result;
    } else {
        /* EA - Dn -> EA */
        dst = M68K_EA_Read(as, ea_mode, ea_reg, size);
        src = as->regs.d[reg] & mask;
        result = (dst - src) & mask;
        M68K_EA_Write(as, ea_mode, ea_reg, size, result);
    }

    /* Set flags */
    M68K_SetNZ(as, result, size);

    /* Set C and X if borrow occurred */
    if (src > dst) {
        M68K_SetFlag(as, CCR_C | CCR_X);
    } else {
        M68K_ClearFlag(as, CCR_C | CCR_X);
    }

    /* Set V if signed overflow occurred (src and dst different sign, result different from dst) */
    {
        UInt32 sign_bit = SIZE_SIGN_BIT(size);
        Boolean src_neg = (src & sign_bit) != 0;
        Boolean dst_neg = (dst & sign_bit) != 0;
        Boolean res_neg = (result & sign_bit) != 0;

        if (src_neg != dst_neg && res_neg != dst_neg) {
            M68K_SetFlag(as, CCR_V);
        } else {
            M68K_ClearFlag(as, CCR_V);
        }
    }
}

/*
 * CMP - Compare
 */
void M68K_Op_CMP(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 reg = (opcode >> 9) & 7;
    UInt8 size = (opcode >> 6) & 3;
    UInt8 ea_mode = (opcode >> 3) & 7;
    UInt8 ea_reg = opcode & 7;
    UInt32 src, dst, result;
    UInt32 mask = SIZE_MASK(size);

    /* Dn - EA */
    dst = as->regs.d[reg] & mask;
    src = M68K_EA_Read(as, ea_mode, ea_reg, size);
    result = (dst - src) & mask;

    /* Set flags (don't write result) */
    M68K_SetNZ(as, result, size);

    /* Set C if borrow occurred */
    if (src > dst) {
        M68K_SetFlag(as, CCR_C);
    } else {
        M68K_ClearFlag(as, CCR_C);
    }

    /* Clear V for MVP */
    M68K_ClearFlag(as, CCR_V);
}

/*
 * LINK - Link and allocate
 */
void M68K_Op_LINK(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 reg = opcode & 7;
    SInt16 disp;

    /* Push An */
    M68K_Push32(as, as->regs.a[reg]);

    /* An = SP */
    as->regs.a[reg] = as->regs.a[7];

    /* SP = SP + displacement */
    disp = (SInt16)M68K_Fetch16(as);
    as->regs.a[7] += disp;

    /* LINK does not affect flags */
}

/*
 * UNLK - Unlink
 */
void M68K_Op_UNLK(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 reg = opcode & 7;

    /* SP = An */
    as->regs.a[7] = as->regs.a[reg];

    /* Pop An */
    as->regs.a[reg] = M68K_Pop32(as);

    /* UNLK does not affect flags */
}

/*
 * JSR - Jump to subroutine
 */
void M68K_Op_JSR(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;
    UInt32 target;

    /* Compute target address */
    target = M68K_EA_ComputeAddress(as, mode, reg, SIZE_LONG);

    /* Push return address */
    M68K_Push32(as, as->regs.pc);

    /* Jump - log with A5-relative info if using d16(A5) */
    if (mode == MODE_An_DISP && reg == 5) {
        /* JSR d16(A5) - log A5-relative offset */
        SInt32 offsetFromA5 = (SInt32)target - (SInt32)as->regs.a[5];
        serial_printf("[M68K] JSR (A5%+d) -> 0x%08X\n", offsetFromA5, target);
    } else {
        serial_printf("[M68K] JSR 0x%08X -> 0x%08X\n", as->regs.pc - 2, target);
    }
    as->regs.pc = target;

    /* JSR does not affect flags */
}

/*
 * JMP - Jump
 */
void M68K_Op_JMP(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;
    UInt32 target;

    /* Compute target address */
    target = M68K_EA_ComputeAddress(as, mode, reg, SIZE_LONG);

    /* Jump */
    serial_printf("[M68K] JMP 0x%08X -> 0x%08X\n", as->regs.pc - 2, target);
    as->regs.pc = target;

    /* JMP does not affect flags */
}

/*
 * BRA - Branch always
 */
void M68K_Op_BRA(M68KAddressSpace* as, UInt16 opcode)
{
    SInt32 disp;
    UInt32 target;

    disp = (SInt8)(opcode & 0xFF);
    if (disp == 0) {
        /* 16-bit displacement */
        disp = SIGN_EXTEND_WORD(M68K_Fetch16(as));
    }

    target = as->regs.pc + disp;
    serial_printf("[M68K] BRA 0x%08X -> 0x%08X (disp=%d)\n", as->regs.pc - 2, target, disp);
    as->regs.pc = target;

    /* BRA does not affect flags */
}

/*
 * BSR - Branch to subroutine
 */
void M68K_Op_BSR(M68KAddressSpace* as, UInt16 opcode)
{
    SInt32 disp;
    UInt32 target;

    disp = (SInt8)(opcode & 0xFF);
    if (disp == 0) {
        /* 16-bit displacement */
        disp = SIGN_EXTEND_WORD(M68K_Fetch16(as));
    }

    target = as->regs.pc + disp;

    /* Push return address */
    M68K_Push32(as, as->regs.pc);

    /* Jump */
    serial_printf("[M68K] BSR 0x%08X -> 0x%08X (disp=%d)\n", as->regs.pc - 2, target, disp);
    as->regs.pc = target;

    /* BSR does not affect flags */
}

/*
 * Bcc - Branch conditionally
 */
void M68K_Op_Bcc(M68KAddressSpace* as, UInt16 opcode)
{
    M68KCondition cc = (opcode >> 8) & 0xF;
    SInt32 disp;
    UInt32 target;

    disp = (SInt8)(opcode & 0xFF);
    if (disp == 0) {
        /* 16-bit displacement */
        disp = SIGN_EXTEND_WORD(M68K_Fetch16(as));
    }

    if (M68K_TestCondition(as->regs.sr, cc)) {
        target = as->regs.pc + disp;
        serial_printf("[M68K] Bcc (cc=%d) taken: 0x%08X -> 0x%08X\n", cc, as->regs.pc - 2, target);
        as->regs.pc = target;
    } else {
        serial_printf("[M68K] Bcc (cc=%d) not taken\n", cc);
    }

    /* Bcc does not affect flags */
}

/*
 * RTS - Return from subroutine
 */
void M68K_Op_RTS(M68KAddressSpace* as, UInt16 opcode)
{
    UInt32 return_addr;

    (void)opcode;

    /* Pop return address */
    return_addr = M68K_Pop32(as);

    /* Jump */
    serial_printf("[M68K] RTS to 0x%08X\n", return_addr);
    as->regs.pc = return_addr;

    /* RTS does not affect flags */
}

/*
 * RTE - Return from exception (stub)
 * Encoding: 0100 1110 0111 0011 (0x4E73)
 */
void M68K_Op_RTE(M68KAddressSpace* as, UInt16 opcode)
{
    (void)opcode;

    serial_printf("[M68K] RTE (stub) at PC=0x%08X - halting\n", as->regs.pc - 2);

    /* For now, RTE is a stub that just halts */
    /* Full implementation would pop SR and PC from supervisor stack */
    as->halted = true;
}

/*
 * STOP - Load status register and stop (stub)
 * Encoding: 0100 1110 0111 0010 (0x4E72) + immediate SR value
 */
void M68K_Op_STOP(M68KAddressSpace* as, UInt16 opcode)
{
    UInt16 sr_value;

    (void)opcode;

    /* Fetch immediate SR value */
    sr_value = M68K_Fetch16(as);

    serial_printf("[M68K] STOP #0x%04X at PC=0x%08X - treated as NOP\n",
                 sr_value, as->regs.pc - 4);

    /* For now, STOP is a NOP (don't actually stop execution) */
    /* Full implementation would halt until interrupt */
}

/*
 * Scc - Set according to condition
 * Encoding: 0101 cccc 11xx xxxx (0x50C0-0x5FFF)
 * If condition true, set EA byte to 0xFF; else set to 0x00
 */
void M68K_Op_Scc(M68KAddressSpace* as, UInt16 opcode)
{
    M68KCondition cc = (M68KCondition)((opcode >> 8) & 0xF);
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;
    UInt8 value;

    /* Test condition */
    if (M68K_TestCondition(as->regs.sr, cc)) {
        value = 0xFF;
        serial_printf("[M68K] Scc (cc=%d) true -> set 0xFF\n", cc);
    } else {
        value = 0x00;
        serial_printf("[M68K] Scc (cc=%d) false -> set 0x00\n", cc);
    }

    /* Write byte to EA */
    M68K_EA_Write(as, mode, reg, SIZE_BYTE, value);

    /* Scc does not affect flags */
}

/*
 * DBcc - Decrement and branch conditionally
 * Encoding: 0101 cccc 1100 1rrr (0x50C8-0x5FC8)
 * If cc false: Dn--, if Dn != -1 then branch; else fall through
 * If cc true: fall through
 */
void M68K_Op_DBcc(M68KAddressSpace* as, UInt16 opcode)
{
    M68KCondition cc = (M68KCondition)((opcode >> 8) & 0xF);
    UInt8 reg = opcode & 7;
    SInt16 disp;
    UInt32 target;

    /* Fetch displacement */
    disp = (SInt16)M68K_Fetch16(as);

    /* Test condition */
    if (!M68K_TestCondition(as->regs.sr, cc)) {
        /* Condition false - decrement and test */
        SInt16 counter = (SInt16)(as->regs.d[reg] & 0xFFFF);
        counter--;
        as->regs.d[reg] = (as->regs.d[reg] & 0xFFFF0000) | (counter & 0xFFFF);

        if (counter != -1) {
            /* Branch */
            target = (as->regs.pc - 2) + disp;  /* PC-2 because we already fetched disp */
            serial_printf("[M68K] DBcc (cc=%d) false, D%d=%d -> branch to 0x%08X\n",
                         cc, reg, counter, target);
            as->regs.pc = target;
        } else {
            /* Counter expired - fall through */
            serial_printf("[M68K] DBcc (cc=%d) false, D%d=-1 -> fall through\n", cc, reg);
        }
    } else {
        /* Condition true - fall through */
        serial_printf("[M68K] DBcc (cc=%d) true -> fall through\n", cc);
    }

    /* DBcc does not affect flags */
}

/*
 * TRAP - A-line trap
 */
void M68K_Op_TRAP(M68KAddressSpace* as, UInt16 opcode)
{
    UInt16 trap_num = opcode & 0x0FFF;
    UInt32 saved_pc = as->regs.pc;
    OSErr err;

    serial_printf("[M68K] TRAP $A%03X at PC=0x%08X\n", trap_num, saved_pc - 2);

    /* Look up trap handler */
    if (as->trapHandlers[trap_num & 0xFF]) {
        /* Call handler */
        err = as->trapHandlers[trap_num & 0xFF](
            as->trapContexts[trap_num & 0xFF],
            &as->regs.pc,
            as->regs.d  /* Pass registers array */
        );

        if (err != noErr) {
            serial_printf("[M68K] TRAP handler returned error %d\n", err);
            M68K_Fault(as, "TRAP handler error");
            return;
        }
    } else {
        serial_printf("[M68K] WARNING: Unhandled TRAP $A%03X\n", trap_num);
    }

    /* PC is now at next instruction (set by handler or unchanged) */
    /* TRAP does not affect flags */
}

/*
 * MOVEQ - Move quick (sign-extended 8-bit immediate to Dn)
 * Encoding: 0111 rrr0 dddd dddd
 * Format: MOVEQ #imm8, Dn
 */
void M68K_Op_MOVEQ(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 dn = (opcode >> 9) & 7;
    SInt8 imm = opcode & 0xFF;
    SInt32 value = imm;  /* Sign-extend */

    /* Write to Dn */
    as->regs.d[dn] = value;

    /* Set flags based on result */
    M68K_ClearFlag(as, CCR_N | CCR_Z | CCR_V | CCR_C);
    if (value == 0) {
        M68K_SetFlag(as, CCR_Z);
    }
    if (value < 0) {
        M68K_SetFlag(as, CCR_N);
    }
}

/*
 * TST - Test operand (set flags based on operand value)
 * Encoding: 0100 1010 ssxx xrrr
 * Format: TST.<size> <ea>
 */
void M68K_Op_TST(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 size_bits = (opcode >> 6) & 3;
    UInt8 mode = (opcode >> 3) & 7;
    UInt8 reg = opcode & 7;
    M68KSize size;
    UInt32 value;
    SInt32 signed_value;

    /* Decode size */
    if (size_bits == 0) {
        size = SIZE_BYTE;
    } else if (size_bits == 1) {
        size = SIZE_WORD;
    } else if (size_bits == 2) {
        size = SIZE_LONG;
    } else {
        M68K_Fault(as, "Invalid TST size");
        return;
    }

    /* Read operand */
    value = M68K_EA_Read(as, mode, reg, size);

    /* Sign-extend for flag calculation */
    if (size == SIZE_BYTE) {
        signed_value = SIGN_EXTEND_BYTE(value);
    } else if (size == SIZE_WORD) {
        signed_value = SIGN_EXTEND_WORD(value);
    } else {
        signed_value = value;
    }

    /* Set flags */
    M68K_ClearFlag(as, CCR_N | CCR_Z | CCR_V | CCR_C);
    if (signed_value == 0) {
        M68K_SetFlag(as, CCR_Z);
    }
    if (signed_value < 0) {
        M68K_SetFlag(as, CCR_N);
    }
}

/*
 * EXT - Sign-extend data register
 * Encoding: 0100 1000 1oxx xrrr (o=0: byte->word, o=1: word->long)
 * Format: EXT.W Dn  (byte->word) or EXT.L Dn (word->long)
 */
void M68K_Op_EXT(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 dn = opcode & 7;
    UInt8 opmode = (opcode >> 6) & 7;
    SInt32 value;

    if (opmode == 2) {
        /* EXT.W - byte to word */
        value = SIGN_EXTEND_BYTE(as->regs.d[dn] & 0xFF);
        as->regs.d[dn] = (as->regs.d[dn] & 0xFFFF0000) | (value & 0xFFFF);
    } else if (opmode == 3) {
        /* EXT.L - word to long */
        value = SIGN_EXTEND_WORD(as->regs.d[dn] & 0xFFFF);
        as->regs.d[dn] = value;
    } else {
        M68K_Fault(as, "Invalid EXT opmode");
        return;
    }

    /* Set flags */
    M68K_ClearFlag(as, CCR_N | CCR_Z | CCR_V | CCR_C);
    if (value == 0) {
        M68K_SetFlag(as, CCR_Z);
    }
    if (value < 0) {
        M68K_SetFlag(as, CCR_N);
    }
}

/*
 * SWAP - Swap register halves
 * Encoding: 0100 1000 0100 0rrr
 * Format: SWAP Dn
 */
void M68K_Op_SWAP(M68KAddressSpace* as, UInt16 opcode)
{
    UInt8 dn = opcode & 7;
    UInt32 value = as->regs.d[dn];
    UInt32 swapped;

    /* Swap upper and lower 16 bits */
    swapped = ((value & 0xFFFF) << 16) | ((value >> 16) & 0xFFFF);
    as->regs.d[dn] = swapped;

    /* Set flags based on result */
    M68K_ClearFlag(as, CCR_N | CCR_Z | CCR_V | CCR_C);
    if (swapped == 0) {
        M68K_SetFlag(as, CCR_Z);
    }
    if (swapped & 0x80000000) {
        M68K_SetFlag(as, CCR_N);
    }
}
