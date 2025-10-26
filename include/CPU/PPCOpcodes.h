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
 * Special Purpose Register (SPR) Numbers
 * Used by MFSPR/MTSPR instructions
 */
#define SPR_XER         1    /* Fixed-point exception register */
#define SPR_LR          8    /* Link register */
#define SPR_CTR         9    /* Count register */
#define SPR_DSISR       18   /* DSI exception register */
#define SPR_DAR         19   /* Data address register */
#define SPR_DEC         22   /* Decrementer */
#define SPR_SDR1        25   /* Page table base register */
#define SPR_SRR0        26   /* Save/restore register 0 */
#define SPR_SRR1        27   /* Save/restore register 1 */
#define SPR_SPRG0       272  /* SPR general 0 */
#define SPR_SPRG1       273  /* SPR general 1 */
#define SPR_SPRG2       274  /* SPR general 2 */
#define SPR_SPRG3       275  /* SPR general 3 */
#define SPR_EAR         282  /* External access register */
#define SPR_TBL_READ    268  /* Time base lower (read) */
#define SPR_TBU_READ    269  /* Time base upper (read) */
#define SPR_TBL_WRITE   284  /* Time base lower (write) */
#define SPR_TBU_WRITE   285  /* Time base upper (write) */
#define SPR_PVR         287  /* Processor version register */
#define SPR_IBAT0U      528  /* Instruction BAT 0 upper */
#define SPR_IBAT0L      529  /* Instruction BAT 0 lower */
#define SPR_IBAT1U      530  /* Instruction BAT 1 upper */
#define SPR_IBAT1L      531  /* Instruction BAT 1 lower */
#define SPR_IBAT2U      532  /* Instruction BAT 2 upper */
#define SPR_IBAT2L      533  /* Instruction BAT 2 lower */
#define SPR_IBAT3U      534  /* Instruction BAT 3 upper */
#define SPR_IBAT3L      535  /* Instruction BAT 3 lower */
#define SPR_DBAT0U      536  /* Data BAT 0 upper */
#define SPR_DBAT0L      537  /* Data BAT 0 lower */
#define SPR_DBAT1U      538  /* Data BAT 1 upper */
#define SPR_DBAT1L      539  /* Data BAT 1 lower */
#define SPR_DBAT2U      540  /* Data BAT 2 upper */
#define SPR_DBAT2L      541  /* Data BAT 2 lower */
#define SPR_DBAT3U      542  /* Data BAT 3 upper */
#define SPR_DBAT3L      543  /* Data BAT 3 lower */
#define SPR_HID0        1008 /* Hardware implementation register 0 */
#define SPR_HID1        1009 /* Hardware implementation register 1 */
#define SPR_IABR        1010 /* Instruction address breakpoint */
#define SPR_DABR        1013 /* Data address breakpoint */

/*
 * Primary Opcodes (bits 0-5)
 */
#define PPC_OP_TWI          3   /* Trap word immediate */
#define PPC_OP_EXT4         4   /* Extended opcodes (AltiVec/VMX) */
#define PPC_OP_MULLI        7   /* Multiply low immediate */
#define PPC_OP_SUBFIC       8   /* Subtract from immediate carrying */
#define PPC_OP_DOZI         9   /* Difference or zero immediate (601) */
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
#define PPC_OP_RLMI         22  /* Rotate left then mask insert (601) */
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
#define PPC_XOP_DCBI        470 /* Data cache block invalidate */
#define PPC_XOP_DCBT        278 /* Data cache block touch (prefetch) */
#define PPC_XOP_DCBTST      246 /* Data cache block touch for store */

/*
 * TLB Management (Opcode 31)
 */
#define PPC_XOP_TLBIE       306 /* TLB invalidate entry */
#define PPC_XOP_TLBSYNC     566 /* TLB synchronize */
#define PPC_XOP_TLBIA       370 /* TLB invalidate all (601 only) */

/*
 * PowerPC 601 Compatibility Instructions (Opcode 31)
 */
#define PPC_XOP_ABS         360 /* Absolute (601) */
#define PPC_XOP_NABS        488 /* Negative absolute (601) */
#define PPC_XOP_DIV         331 /* Divide (601) */
#define PPC_XOP_DIVS        363 /* Divide short (601) */
#define PPC_XOP_DOZ         264 /* Difference or zero (601) */
#define PPC_XOP_MUL         107 /* Multiply (601) */
#define PPC_XOP_CLCS        531 /* Cache line compute size (601) */

/*
 * Segment Register Operations (Opcode 31)
 */
#define PPC_XOP_MFSR        595 /* Move from segment register */
#define PPC_XOP_MTSR        210 /* Move to segment register */
#define PPC_XOP_MFSRIN      659 /* Move from segment register indirect */
#define PPC_XOP_MTSRIN      242 /* Move to segment register indirect */
#define PPC_XOP_MFTB        371 /* Move from time base */

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
 * Extended Opcode 4 Instructions (AltiVec/VMX Vector Operations)
 */
/* Vector arithmetic */
#define PPC_VXO_VADDUBM     0   /* Vector add unsigned byte modulo */
#define PPC_VXO_VADDUHM     64  /* Vector add unsigned halfword modulo */
#define PPC_VXO_VADDUWM     128 /* Vector add unsigned word modulo */
#define PPC_VXO_VSUBUBM     1024 /* Vector subtract unsigned byte modulo */
#define PPC_VXO_VSUBUHM     1088 /* Vector subtract unsigned halfword modulo */
#define PPC_VXO_VSUBUWM     1152 /* Vector subtract unsigned word modulo */
#define PPC_VXO_VMUL        /* Multiple variants with different encodings */

/* Vector logical */
#define PPC_VXO_VAND        1028 /* Vector AND */
#define PPC_VXO_VOR         1156 /* Vector OR */
#define PPC_VXO_VXOR        1220 /* Vector XOR */
#define PPC_VXO_VANDC       1092 /* Vector AND with complement */
#define PPC_VXO_VNOR        1284 /* Vector NOR */

/* Vector compare */
#define PPC_VXO_VCMPEQUB    6   /* Vector compare equal unsigned byte */
#define PPC_VXO_VCMPEQUH    70  /* Vector compare equal unsigned halfword */
#define PPC_VXO_VCMPEQUW    134 /* Vector compare equal unsigned word */
#define PPC_VXO_VCMPGTUB    518 /* Vector compare greater than unsigned byte */
#define PPC_VXO_VCMPGTSB    774 /* Vector compare greater than signed byte */

/* Vector permute/select */
#define PPC_VXO_VPERM       43  /* Vector permute */
#define PPC_VXO_VSEL        42  /* Vector select */
#define PPC_VXO_VSLDOI      44  /* Vector shift left double by octet immediate */

/* Vector splat */
#define PPC_VXO_VSPLTB      524 /* Vector splat byte */
#define PPC_VXO_VSPLTH      588 /* Vector splat halfword */
#define PPC_VXO_VSPLTW      652 /* Vector splat word */
#define PPC_VXO_VSPLTISB    780 /* Vector splat immediate signed byte */
#define PPC_VXO_VSPLTISH    844 /* Vector splat immediate signed halfword */
#define PPC_VXO_VSPLTISW    908 /* Vector splat immediate signed word */

/* Vector saturating arithmetic */
#define PPC_VXO_VADDSBS     768 /* Vector add signed byte saturate */
#define PPC_VXO_VADDUBS     512 /* Vector add unsigned byte saturate */
#define PPC_VXO_VADDSHS     832 /* Vector add signed halfword saturate */
#define PPC_VXO_VADDUHS     576 /* Vector add unsigned halfword saturate */
#define PPC_VXO_VSUBSBS     1792 /* Vector subtract signed byte saturate */
#define PPC_VXO_VSUBUBS     1536 /* Vector subtract unsigned byte saturate */
#define PPC_VXO_VSUBSHS     1856 /* Vector subtract signed halfword saturate */
#define PPC_VXO_VSUBUHS     1600 /* Vector subtract unsigned halfword saturate */

/* Vector shift */
#define PPC_VXO_VSLB        260 /* Vector shift left byte */
#define PPC_VXO_VSRB        516 /* Vector shift right byte */
#define PPC_VXO_VSRAB       772 /* Vector shift right algebraic byte */
#define PPC_VXO_VSLH        324 /* Vector shift left halfword */
#define PPC_VXO_VSRH        580 /* Vector shift right halfword */
#define PPC_VXO_VSRAW       836 /* Vector shift right algebraic word */

/* Vector pack/unpack */
#define PPC_VXO_VPKUHUM     14  /* Vector pack unsigned halfword unsigned modulo */
#define PPC_VXO_VPKUWUM     78  /* Vector pack unsigned word unsigned modulo */
#define PPC_VXO_VUPKHSB     526 /* Vector unpack high signed byte */
#define PPC_VXO_VUPKLSB     590 /* Vector unpack low signed byte */
#define PPC_VXO_VUPKHSH     654 /* Vector unpack high signed halfword */
#define PPC_VXO_VUPKLSH     718 /* Vector unpack low signed halfword */

/* Vector merge */
#define PPC_VXO_VMRGHB      12  /* Vector merge high byte */
#define PPC_VXO_VMRGLB      268 /* Vector merge low byte */
#define PPC_VXO_VMRGHH      76  /* Vector merge high halfword */
#define PPC_VXO_VMRGLH      332 /* Vector merge low halfword */
#define PPC_VXO_VMRGHW      140 /* Vector merge high word */
#define PPC_VXO_VMRGLW      396 /* Vector merge low word */

/* Vector multiply */
#define PPC_VXO_VMULESB     776 /* Vector multiply even signed byte */
#define PPC_VXO_VMULOSB     264 /* Vector multiply odd signed byte */
#define PPC_VXO_VMULEUB     520 /* Vector multiply even unsigned byte */
#define PPC_VXO_VMULOUB     8   /* Vector multiply odd unsigned byte */
#define PPC_VXO_VMULESH     840 /* Vector multiply even signed halfword */
#define PPC_VXO_VMULOSH     328 /* Vector multiply odd signed halfword */
#define PPC_VXO_VMULEUH     584 /* Vector multiply even unsigned halfword */
#define PPC_VXO_VMULOUH     72  /* Vector multiply odd unsigned halfword */

/* Vector min/max/average */
#define PPC_VXO_VMAXSB      258 /* Vector maximum signed byte */
#define PPC_VXO_VMAXUB      2   /* Vector maximum unsigned byte */
#define PPC_VXO_VMINSB      770 /* Vector minimum signed byte */
#define PPC_VXO_VMINUB      514 /* Vector minimum unsigned byte */
#define PPC_VXO_VMAXSH      322 /* Vector maximum signed halfword */
#define PPC_VXO_VMINSH      834 /* Vector minimum signed halfword */
#define PPC_VXO_VAVGSB      1282 /* Vector average signed byte */
#define PPC_VXO_VAVGUB      1026 /* Vector average unsigned byte */

/* Vector rotate */
#define PPC_VXO_VRLB        4   /* Vector rotate left byte */
#define PPC_VXO_VRLH        68  /* Vector rotate left halfword */
#define PPC_VXO_VRLW        132 /* Vector rotate left word */

/* Vector word shift */
#define PPC_VXO_VSLW        388 /* Vector shift left word */
#define PPC_VXO_VSRW        644 /* Vector shift right word */

/* Additional vector compare */
#define PPC_VXO_VCMPGTUH    582 /* Vector compare greater than unsigned halfword */
#define PPC_VXO_VCMPGTSH    838 /* Vector compare greater than signed halfword */
#define PPC_VXO_VCMPGTUW    646 /* Vector compare greater than unsigned word */
#define PPC_VXO_VCMPGTSW    902 /* Vector compare greater than signed word */

/* Additional vector pack */
#define PPC_VXO_VPKUHUS     142 /* Vector pack unsigned halfword unsigned saturate */
#define PPC_VXO_VPKUWUS     206 /* Vector pack unsigned word unsigned saturate */

/* Vector sum */
#define PPC_VXO_VSUM4UBS    1544 /* Vector sum across quarter unsigned byte saturate */
#define PPC_VXO_VSUM4SBS    1800 /* Vector sum across quarter signed byte saturate */

/* Vector load/store (use primary opcodes 7, 39, etc.) */
#define PPC_OP_LVX          103 /* Load vector indexed (actually opcode 31/103) */
#define PPC_OP_STVX         231 /* Store vector indexed (actually opcode 31/231) */
#define PPC_OP_LVEBX        7   /* Load vector element byte indexed (opcode 31/7) */
#define PPC_OP_LVEHX        39  /* Load vector element halfword indexed (opcode 31/39) */
#define PPC_OP_LVEWX        71  /* Load vector element word indexed (opcode 31/71) */
#define PPC_OP_STVEBX       135 /* Store vector element byte indexed (opcode 31/135) */
#define PPC_OP_STVEHX       167 /* Store vector element halfword indexed (opcode 31/167) */

/*
 * System Instructions (Opcode 31)
 */
#define PPC_XOP_MFMSR       83  /* Move from machine state register */
#define PPC_XOP_MTMSR       146 /* Move to machine state register */
#define PPC_XOP_MFVSCR      1540 /* Move from vector status and control register */
#define PPC_XOP_MTVSCR      1604 /* Move to vector status and control register */

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

/* Segment register operations */
extern void PPC_Op_MFSR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTSR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MFSRIN(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MTSRIN(PPCAddressSpace* as, UInt32 insn);

/* TLB management operations */
extern void PPC_Op_TLBIE(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_TLBSYNC(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_TLBIA(PPCAddressSpace* as, UInt32 insn);

/* Cache control operations */
extern void PPC_Op_DCBI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DCBT(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DCBTST(PPCAddressSpace* as, UInt32 insn);

/* Time base access */
extern void PPC_Op_MFTB(PPCAddressSpace* as, UInt32 insn);

/* PowerPC 601 compatibility instructions */
extern void PPC_Op_DOZI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DOZ(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_ABS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_NABS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_MUL(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DIV(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_DIVS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_RLMI(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_CLCS(PPCAddressSpace* as, UInt32 insn);

/* AltiVec/VMX vector instructions */
extern void PPC_Op_VADDUBM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VADDUHM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VADDUWM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUBUBM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VAND(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VXOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VNOR(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSPLTISB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSPLTISH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSPLTISW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LVX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STVX(PPCAddressSpace* as, UInt32 insn);

/* Vector saturating arithmetic */
extern void PPC_Op_VADDSBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VADDUBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VADDSHS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VADDUHS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUBSBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUBUBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUBSHS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUBUHS(PPCAddressSpace* as, UInt32 insn);

/* Vector shift */
extern void PPC_Op_VSLB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSRB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSRAB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSLH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSRH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSRAW(PPCAddressSpace* as, UInt32 insn);

/* Vector pack/unpack */
extern void PPC_Op_VPKUHUM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VPKUWUM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VUPKHSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VUPKLSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VUPKHSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VUPKLSH(PPCAddressSpace* as, UInt32 insn);

/* Vector merge */
extern void PPC_Op_VMRGHB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMRGLB(PPCAddressSpace* as, UInt32 insn);

/* Vector permute/select */
extern void PPC_Op_VPERM(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSEL(PPCAddressSpace* as, UInt32 insn);

/* Vector compare */
extern void PPC_Op_VCMPEQUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPGTUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPGTSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPEQUH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPEQUW(PPCAddressSpace* as, UInt32 insn);

/* Vector splat */
extern void PPC_Op_VSPLTB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSPLTH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSPLTW(PPCAddressSpace* as, UInt32 insn);

/* Vector element load/store */
extern void PPC_Op_LVEBX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_LVEHX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STVEBX(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_STVEHX(PPCAddressSpace* as, UInt32 insn);

/* Vector multiply */
extern void PPC_Op_VMULESB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULOSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULEUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULOUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULESH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULOSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULEUH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMULOUH(PPCAddressSpace* as, UInt32 insn);

/* Vector min/max/average */
extern void PPC_Op_VMAXSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMAXUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMINSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMINUB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMAXSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMINSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VAVGSB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VAVGUB(PPCAddressSpace* as, UInt32 insn);

/* Vector rotate */
extern void PPC_Op_VRLB(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VRLH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VRLW(PPCAddressSpace* as, UInt32 insn);

/* Vector word shift */
extern void PPC_Op_VSLW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSRW(PPCAddressSpace* as, UInt32 insn);

/* Vector merge halfword/word */
extern void PPC_Op_VMRGHH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMRGLH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMRGHW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VMRGLW(PPCAddressSpace* as, UInt32 insn);

/* Additional vector compare */
extern void PPC_Op_VCMPGTUH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPGTSH(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPGTUW(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VCMPGTSW(PPCAddressSpace* as, UInt32 insn);

/* Additional vector pack */
extern void PPC_Op_VPKUHUS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VPKUWUS(PPCAddressSpace* as, UInt32 insn);

/* Vector sum */
extern void PPC_Op_VSUM4UBS(PPCAddressSpace* as, UInt32 insn);
extern void PPC_Op_VSUM4SBS(PPCAddressSpace* as, UInt32 insn);

#ifdef __cplusplus
}
#endif

#endif /* PPC_OPCODES_H */
