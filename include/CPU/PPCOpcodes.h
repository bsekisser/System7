/*
 * PPCOpcodes.h - PowerPC Instruction Definitions and Helpers
 *
 * Defines PowerPC instruction formats, opcode patterns, and helper macros
 * for the PowerPC interpreter implementation.
 */

#ifndef PPC_OPCODES_H
#define PPC_OPCODES_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PowerPC Instruction Formats
 * All PowerPC instructions are 32 bits wide
 */

/* Extract primary opcode (bits 0-5) */
#define PPC_PRIMARY_OPCODE(insn)  (((insn) >> 26) & 0x3F)

/* Extract extended opcode (bits 21-30 or 26-30 depending on format) */
#define PPC_EXTENDED_OPCODE(insn) (((insn) >> 1) & 0x3FF)
#define PPC_EXTENDED_XO(insn)     (((insn) >> 1) & 0x1FF)

/* Register field extraction */
#define PPC_RD(insn)    (((insn) >> 21) & 0x1F)  /* Destination register */
#define PPC_RS(insn)    (((insn) >> 21) & 0x1F)  /* Source register */
#define PPC_RA(insn)    (((insn) >> 16) & 0x1F)  /* Register A */
#define PPC_RB(insn)    (((insn) >> 11) & 0x1F)  /* Register B */

/* Immediate field extraction */
#define PPC_SIMM(insn)  ((SInt32)(SInt16)((insn) & 0xFFFF))  /* Signed 16-bit immediate */
#define PPC_UIMM(insn)  ((insn) & 0xFFFF)                    /* Unsigned 16-bit immediate */

/* Branch field extraction */
#define PPC_LI(insn)    ((SInt32)(((insn) & 0x03FFFFFC) << 6) >> 6)  /* 24-bit offset (sign-extended, shifted left 2) */
#define PPC_BD(insn)    ((SInt32)(SInt16)(((insn) & 0xFFFC)))         /* 14-bit offset (sign-extended, shifted left 2) */
#define PPC_BO(insn)    (((insn) >> 21) & 0x1F)  /* Branch options */
#define PPC_BI(insn)    (((insn) >> 16) & 0x1F)  /* Branch condition bit */

/* Shift/mask extraction */
#define PPC_SH(insn)    (((insn) >> 11) & 0x1F)  /* Shift amount */
#define PPC_MB(insn)    (((insn) >> 6) & 0x1F)   /* Mask begin */
#define PPC_ME(insn)    (((insn) >> 1) & 0x1F)   /* Mask end */

/* Condition register field */
#define PPC_CRFD(insn)  (((insn) >> 23) & 0x07)  /* CR destination field */
#define PPC_CRFS(insn)  (((insn) >> 18) & 0x07)  /* CR source field */

/* Record bit (Rc) - updates CR0 */
#define PPC_RC(insn)    ((insn) & 0x0001)

/* Overflow enable (OE) */
#define PPC_OE(insn)    (((insn) >> 10) & 0x0001)

/* Link bit (LK) - saves return address */
#define PPC_LK(insn)    ((insn) & 0x0001)

/* Absolute address bit (AA) */
#define PPC_AA(insn)    (((insn) >> 1) & 0x0001)

/*
 * Primary Opcodes (bits 0-5)
 */
#define PPC_OP_TWI          3   /* Trap word immediate */
#define PPC_OP_MULLI        7   /* Multiply low immediate */
#define PPC_OP_SUBFIC       8   /* Subtract from immediate carrying */
#define PPC_OP_CMPLI        10  /* Compare logical immediate */
#define PPC_OP_CMPI         11  /* Compare immediate */
#define PPC_OP_ADDIC        12  /* Add immediate carrying */
#define PPC_OP_ADDIC_RC     13  /* Add immediate carrying and record */
#define PPC_OP_ADDI         14  /* Add immediate */
#define PPC_OP_ADDIS        15  /* Add immediate shifted */
#define PPC_OP_BC           16  /* Branch conditional */
#define PPC_OP_SC           17  /* System call */
#define PPC_OP_B            18  /* Branch */
#define PPC_OP_EXT19        19  /* Extended opcodes (CR ops, branches) */
#define PPC_OP_RLWIMI       20  /* Rotate left word immediate then mask insert */
#define PPC_OP_RLWINM       21  /* Rotate left word immediate then AND with mask */
#define PPC_OP_RLWNM        23  /* Rotate left word then AND with mask */
#define PPC_OP_ORI          24  /* OR immediate */
#define PPC_OP_ORIS         25  /* OR immediate shifted */
#define PPC_OP_XORI         26  /* XOR immediate */
#define PPC_OP_XORIS        27  /* XOR immediate shifted */
#define PPC_OP_ANDI_RC      28  /* AND immediate and record */
#define PPC_OP_ANDIS_RC     29  /* AND immediate shifted and record */
#define PPC_OP_EXT31        31  /* Extended opcodes (arithmetic, logical, loads, stores) */
#define PPC_OP_LWZ          32  /* Load word and zero */
#define PPC_OP_LWZU         33  /* Load word and zero with update */
#define PPC_OP_LBZ          34  /* Load byte and zero */
#define PPC_OP_LBZU         35  /* Load byte and zero with update */
#define PPC_OP_STW          36  /* Store word */
#define PPC_OP_STWU         37  /* Store word with update */
#define PPC_OP_STB          38  /* Store byte */
#define PPC_OP_STBU         39  /* Store byte with update */
#define PPC_OP_LHZ          40  /* Load halfword and zero */
#define PPC_OP_LHZU         41  /* Load halfword and zero with update */
#define PPC_OP_LHA          42  /* Load halfword algebraic */
#define PPC_OP_LHAU         43  /* Load halfword algebraic with update */
#define PPC_OP_STH          44  /* Store halfword */
#define PPC_OP_STHU         45  /* Store halfword with update */
#define PPC_OP_LMW          46  /* Load multiple word */
#define PPC_OP_STMW         47  /* Store multiple word */
#define PPC_OP_LFS          48  /* Load floating-point single */
#define PPC_OP_LFSU         49  /* Load floating-point single with update */
#define PPC_OP_LFD          50  /* Load floating-point double */
#define PPC_OP_LFDU         51  /* Load floating-point double with update */
#define PPC_OP_STFS         52  /* Store floating-point single */
#define PPC_OP_STFSU        53  /* Store floating-point single with update */
#define PPC_OP_STFD         54  /* Store floating-point double */
#define PPC_OP_STFDU        55  /* Store floating-point double with update */
#define PPC_OP_EXT59        59  /* Extended opcodes (single-precision FP) */
#define PPC_OP_EXT63        63  /* Extended opcodes (double-precision FP) */

/*
 * Extended Opcode 31 Instructions (XO form)
 */
#define PPC_XOP_CMP         0   /* Compare */
#define PPC_XOP_CMPL        32  /* Compare logical */
#define PPC_XOP_SUBF        40  /* Subtract from */
#define PPC_XOP_ADD         266 /* Add */
#define PPC_XOP_MULLW       235 /* Multiply low word */
#define PPC_XOP_DIVW        491 /* Divide word */
#define PPC_XOP_AND         28  /* AND */
#define PPC_XOP_OR          444 /* OR */
#define PPC_XOP_XOR         316 /* XOR */
#define PPC_XOP_NAND        476 /* NAND */
#define PPC_XOP_NOR         124 /* NOR */
#define PPC_XOP_EQV         284 /* Equivalent */
#define PPC_XOP_ANDC        60  /* AND with complement */
#define PPC_XOP_ORC         412 /* OR with complement */

/*
 * Load/Store Extended Opcodes
 */
#define PPC_XOP_LWZX        23  /* Load word and zero indexed */
#define PPC_XOP_LWZUX       55  /* Load word and zero with update indexed */
#define PPC_XOP_LBZX        87  /* Load byte and zero indexed */
#define PPC_XOP_LBZUX       119 /* Load byte and zero with update indexed */
#define PPC_XOP_STWX        151 /* Store word indexed */
#define PPC_XOP_STWUX       183 /* Store word with update indexed */
#define PPC_XOP_STBX        215 /* Store byte indexed */
#define PPC_XOP_STBUX       247 /* Store byte with update indexed */
#define PPC_XOP_LHZX        279 /* Load halfword and zero indexed */
#define PPC_XOP_LHZUX       311 /* Load halfword and zero with update indexed */
#define PPC_XOP_LHAX        343 /* Load halfword algebraic indexed */
#define PPC_XOP_LHAUX       375 /* Load halfword algebraic with update indexed */
#define PPC_XOP_STHX        407 /* Store halfword indexed */
#define PPC_XOP_STHUX       439 /* Store halfword with update indexed */

/*
 * Shift Extended Opcodes
 */
#define PPC_XOP_SLW         24  /* Shift left word */
#define PPC_XOP_SRW         536 /* Shift right word */
#define PPC_XOP_SRAW        792 /* Shift right algebraic word */
#define PPC_XOP_SRAWI       824 /* Shift right algebraic word immediate */

/*
 * Extended Arithmetic with Carry (Opcode 31)
 */
#define PPC_XOP_ADDZE       202 /* Add to zero extended */
#define PPC_XOP_ADDME       234 /* Add to minus one extended */
#define PPC_XOP_ADDE        138 /* Add extended */
#define PPC_XOP_SUBFE       136 /* Subtract from extended */
#define PPC_XOP_SUBFZE      200 /* Subtract from zero extended */
#define PPC_XOP_SUBFME      232 /* Subtract from minus one extended */

/*
 * Unsigned Multiply/Divide (Opcode 31)
 */
#define PPC_XOP_MULHW       75  /* Multiply high word */
#define PPC_XOP_MULHWU      11  /* Multiply high word unsigned */
#define PPC_XOP_DIVWU       459 /* Divide word unsigned */

/*
 * Bit Operations (Opcode 31)
 */
#define PPC_XOP_EXTSB       954 /* Extend sign byte */
#define PPC_XOP_EXTSH       922 /* Extend sign halfword */
#define PPC_XOP_CNTLZW      26  /* Count leading zeros word */

/*
 * Special Register Access (Opcode 31)
 */
#define PPC_XOP_MFCR        19  /* Move from condition register */
#define PPC_XOP_MTCRF       144 /* Move to condition register fields */
#define PPC_XOP_MFSPR       339 /* Move from special purpose register */
#define PPC_XOP_MTSPR       467 /* Move to special purpose register */

/*
 * Trap Instructions
 */
#define PPC_XOP_TW          4   /* Trap word */

/*
 * Atomic Operations (Opcode 31)
 */
#define PPC_XOP_LWARX       20  /* Load word and reserve indexed */
#define PPC_XOP_STWCX       150 /* Store word conditional indexed (note: bit 0 must be set) */

/*
 * Cache Management (Opcode 31)
 */
#define PPC_XOP_DCBZ        1014 /* Data cache block set to zero */
#define PPC_XOP_DCBST       54  /* Data cache block store */
#define PPC_XOP_DCBF        86  /* Data cache block flush */
#define PPC_XOP_ICBI        982 /* Instruction cache block invalidate */

/*
 * String Load/Store (Opcode 31)
 */
#define PPC_XOP_LSWI        597 /* Load string word immediate */
#define PPC_XOP_LSWX        533 /* Load string word indexed */
#define PPC_XOP_STSWI       725 /* Store string word immediate */
#define PPC_XOP_STSWX       661 /* Store string word indexed */

/*
 * Byte-Reversed Load/Store (Opcode 31)
 */
#define PPC_XOP_LWBRX       534 /* Load word byte-reverse indexed */
#define PPC_XOP_LHBRX       790 /* Load halfword byte-reverse indexed */
#define PPC_XOP_STWBRX      662 /* Store word byte-reverse indexed */
#define PPC_XOP_STHBRX      918 /* Store halfword byte-reverse indexed */

/*
 * Floating-Point Indexed Load/Store (Opcode 31)
 */
#define PPC_XOP_LFSX        535 /* Load floating-point single indexed */
#define PPC_XOP_LFSUX       567 /* Load floating-point single with update indexed */
#define PPC_XOP_LFDX        599 /* Load floating-point double indexed */
#define PPC_XOP_LFDUX       631 /* Load floating-point double with update indexed */
#define PPC_XOP_STFSX       663 /* Store floating-point single indexed */
#define PPC_XOP_STFSUX      695 /* Store floating-point single with update indexed */
#define PPC_XOP_STFDX       727 /* Store floating-point double indexed */
#define PPC_XOP_STFDUX      759 /* Store floating-point double with update indexed */

/*
 * Memory Ordering (Opcode 31)
 */
#define PPC_XOP_EIEIO       854 /* Enforce in-order execution of I/O */

/*
 * Branch Extended Opcode 19 Instructions
 */
#define PPC_XOP19_BCLR      16  /* Branch conditional to link register */
#define PPC_XOP19_BCCTR     528 /* Branch conditional to count register */
#define PPC_XOP19_CRAND     257 /* Condition register AND */
#define PPC_XOP19_CROR      449 /* Condition register OR */
#define PPC_XOP19_CRXOR     193 /* Condition register XOR */
#define PPC_XOP19_MCRF      0   /* Move condition register field */
#define PPC_XOP19_SYNC      598 /* Synchronize */
#define PPC_XOP19_ISYNC     150 /* Instruction synchronize */
#define PPC_XOP19_RFI       50  /* Return from interrupt */

/*
 * Extended Opcode 59 Instructions (Single-Precision FP)
 */
#define PPC_XOP59_FADDS     21  /* Floating add single */
#define PPC_XOP59_FSUBS     20  /* Floating subtract single */
#define PPC_XOP59_FMULS     25  /* Floating multiply single */
#define PPC_XOP59_FDIVS     18  /* Floating divide single */
#define PPC_XOP59_FSQRTS    22  /* Floating square root single */
#define PPC_XOP59_FRES      24  /* Floating reciprocal estimate single */
#define PPC_XOP59_FMADDS    29  /* Floating multiply-add single */
#define PPC_XOP59_FMSUBS    28  /* Floating multiply-subtract single */
#define PPC_XOP59_FNMADDS   31  /* Floating negative multiply-add single */
#define PPC_XOP59_FNMSUBS   30  /* Floating negative multiply-subtract single */

/*
 * Extended Opcode 63 Instructions (Double-Precision FP)
 */
#define PPC_XOP63_FADD      21  /* Floating add */
#define PPC_XOP63_FSUB      20  /* Floating subtract */
#define PPC_XOP63_FMUL      25  /* Floating multiply */
#define PPC_XOP63_FDIV      18  /* Floating divide */
#define PPC_XOP63_FSQRT     22  /* Floating square root */
#define PPC_XOP63_FSEL      23  /* Floating select */
#define PPC_XOP63_FRSQRTE   26  /* Floating reciprocal square root estimate */
#define PPC_XOP63_FMADD     29  /* Floating multiply-add */
#define PPC_XOP63_FMSUB     28  /* Floating multiply-subtract */
#define PPC_XOP63_FNMADD    31  /* Floating negative multiply-add */
#define PPC_XOP63_FNMSUB    30  /* Floating negative multiply-subtract */
#define PPC_XOP63_FCMPU     0   /* Floating compare unordered */
#define PPC_XOP63_FCMPO     32  /* Floating compare ordered */
#define PPC_XOP63_FRSP      12  /* Floating round to single-precision */
#define PPC_XOP63_FCTIW     14  /* Floating convert to integer word */
#define PPC_XOP63_FCTIWZ    15  /* Floating convert to integer word with round toward zero */
#define PPC_XOP63_FABS      264 /* Floating absolute value */
#define PPC_XOP63_FNEG      40  /* Floating negate */
#define PPC_XOP63_FNABS     136 /* Floating negative absolute value */
#define PPC_XOP63_FMR       72  /* Floating move register */
#define PPC_XOP63_MFFS      583 /* Move from FPSCR */
#define PPC_XOP63_MTFSF     711 /* Move to FPSCR fields */
#define PPC_XOP63_MTFSFI    134 /* Move to FPSCR field immediate */
#define PPC_XOP63_MTFSB0    70  /* Move to FPSCR bit 0 */
#define PPC_XOP63_MTFSB1    38  /* Move to FPSCR bit 1 */

/*
 * System Instructions (Opcode 31)
 */
#define PPC_XOP_MFMSR       83  /* Move from machine state register */
#define PPC_XOP_MTMSR       146 /* Move to machine state register */

/*
 * Forward declarations
 */
typedef struct PPCAddressSpace PPCAddressSpace;

/*
 * Opcode Handler Function Prototypes
 */
extern void PPC_Fault(PPCAddressSpace* as, const char* reason);

/* Arithmetic operations */
extern void PPC_Op_ADD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDIS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDIC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDIC_RC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBF(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFIC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MULLI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MULLW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DIVW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_NEG(PPCAddressSpace* as, UInt32 insn);

/* Logical operations */
extern void PPC_Op_AND(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_OR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_XOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ORI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ORIS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_XORI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_XORIS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ANDI_RC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ANDIS_RC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_NOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_NAND(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_EQV(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ANDC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ORC(PPCAddressSpace* as, UInt32 insn);

/* Comparison operations */
extern void PPC_Op_CMP(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CMPI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CMPL(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CMPLI(PPCAddressSpace* as, UInt32 insn);

/* Branch operations */
extern void PPC_Op_B(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_BC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_BCLR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_BCCTR(PPCAddressSpace* as, UInt32 insn);

/* Shift operations */
extern void PPC_Op_SLW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SRW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SRAW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SRAWI(PPCAddressSpace* as, UInt32 insn);

/* Rotate operations */
extern void PPC_Op_RLWINM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_RLWNM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_RLWIMI(PPCAddressSpace* as, UInt32 insn);

/* Load/Store operations */
extern void PPC_Op_LWZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LBZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STH(PPCAddressSpace* as, UInt32 insn);

/* Indexed Load/Store operations */
extern void PPC_Op_LWZX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LBZX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHZX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHAX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STWX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STBX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STHX(PPCAddressSpace* as, UInt32 insn);

/* Update-form Load/Store operations */
extern void PPC_Op_LWZU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LBZU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHZU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHAU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STWU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STBU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STHU(PPCAddressSpace* as, UInt32 insn);

/* Multiple Load/Store operations */
extern void PPC_Op_LMW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STMW(PPCAddressSpace* as, UInt32 insn);

/* Condition Register operations */
extern void PPC_Op_CRAND(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CROR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CRXOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CRNAND(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CRNOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CREQV(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CRANDC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CRORC(PPCAddressSpace* as, UInt32 insn);

/* Extended arithmetic with carry */
extern void PPC_Op_ADDZE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDME(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ADDE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFZE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFME(PPCAddressSpace* as, UInt32 insn);

/* Unsigned multiply/divide */
extern void PPC_Op_MULHW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MULHWU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DIVWU(PPCAddressSpace* as, UInt32 insn);

/* Sign extension and bit operations */
extern void PPC_Op_EXTSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_EXTSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CNTLZW(PPCAddressSpace* as, UInt32 insn);

/* Special register access */
extern void PPC_Op_MFCR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTCRF(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MFSPR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTSPR(PPCAddressSpace* as, UInt32 insn);

/* Trap instructions */
extern void PPC_Op_TW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_TWI(PPCAddressSpace* as, UInt32 insn);

/* Atomic operations */
extern void PPC_Op_LWARX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STWCX(PPCAddressSpace* as, UInt32 insn);

/* Cache management (NOPs) */
extern void PPC_Op_DCBZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DCBST(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DCBF(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ICBI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SYNC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ISYNC(PPCAddressSpace* as, UInt32 insn);

/* Indexed update load/store */
extern void PPC_Op_LWZUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LBZUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHZUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHAUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STWUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STBUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STHUX(PPCAddressSpace* as, UInt32 insn);

/* Load halfword algebraic */
extern void PPC_Op_LHA(PPCAddressSpace* as, UInt32 insn);

/* String load/store */
extern void PPC_Op_LSWI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LSWX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STSWI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STSWX(PPCAddressSpace* as, UInt32 insn);

/* Condition register field operations */
extern void PPC_Op_MCRF(PPCAddressSpace* as, UInt32 insn);

/* Overflow arithmetic */
extern void PPC_Op_ADDO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_SUBFO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_NEGO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MULLWO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DIVWO(PPCAddressSpace* as, UInt32 insn);

/* System operations */
extern void PPC_Op_SC(PPCAddressSpace* as, UInt32 insn);

/* Byte-reversed load/store */
extern void PPC_Op_LWBRX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LHBRX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STWBRX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STHBRX(PPCAddressSpace* as, UInt32 insn);

/* Floating-point load/store */
extern void PPC_Op_LFS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFSU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFSX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFSUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFDU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFDX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LFDUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFSU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFSX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFSUX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFDU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFDX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STFDUX(PPCAddressSpace* as, UInt32 insn);

/* Floating-point arithmetic */
extern void PPC_Op_FADD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FADDS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FSUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FSUBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMUL(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMULS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FDIV(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FDIVS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FSQRT(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FSQRTS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FRES(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FRSQRTE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMADD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMADDS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMSUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMSUBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNMADD(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNMADDS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNMSUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNMSUBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FABS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNEG(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FNABS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FMR(PPCAddressSpace* as, UInt32 insn);

/* Floating-point compare/convert */
extern void PPC_Op_FCMPU(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FCMPO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FCTIW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FCTIWZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FRSP(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_FSEL(PPCAddressSpace* as, UInt32 insn);

/* Floating-point status */
extern void PPC_Op_MFFS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTFSF(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTFSFI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTFSB0(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTFSB1(PPCAddressSpace* as, UInt32 insn);

/* System instructions */
extern void PPC_Op_EIEIO(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MFMSR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTMSR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_RFI(PPCAddressSpace* as, UInt32 insn);

#ifdef __cplusplus
}
#endif

#endif /* PPC_OPCODES_H */
