/*
 * PPCInterp.h - PowerPC Interpreter CPU Backend
 *
 * Implements ICPUBackend for PowerPC code execution on any host ISA (x86, ARM, etc).
 * Uses software interpretation of PowerPC instructions with explicit big-endian
 * byte ordering to ensure cross-platform compatibility.
 *
 * PowerPC Architecture:
 * - RISC design with fixed 32-bit instruction width
 * - 32 general-purpose registers (GPR0-GPR31)
 * - 32 floating-point registers (FPR0-FPR31)
 * - Condition register (CR) with 8 4-bit fields
 * - Link register (LR) for function returns
 * - Count register (CTR) for loops
 * - Big-endian byte order (like 68K)
 * - Load/store architecture (only load/store access memory)
 *
 * Supported PowerPC:
 * - PowerPC 601, 603, 604, G3, G4 (32-bit)
 * - User-mode instructions only (no supervisor mode initially)
 */

#ifndef PPC_INTERP_H
#define PPC_INTERP_H

#include "CPU/CPUBackend.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PowerPC Register File
 */
typedef struct PPCRegs {
    UInt32 gpr[32];           /* GPR0-GPR31 general-purpose registers */
    double fpr[32];           /* FPR0-FPR31 floating-point registers */
    UInt32 pc;                /* Program counter (CIA - Current Instruction Address) */
    UInt32 lr;                /* Link register (for function returns) */
    UInt32 ctr;               /* Count register (for loops) */
    UInt32 cr;                /* Condition register (8 4-bit fields) */
    UInt32 xer;               /* Fixed-point exception register */
    UInt32 fpscr;             /* Floating-point status and control register */
    UInt32 msr;               /* Machine state register */
} PPCRegs;

/*
 * PowerPC Condition Register Fields (CR0-CR7)
 * Each field has 4 bits: LT, GT, EQ, SO
 */
#define PPC_CR_LT(n)  (0x80000000 >> (n*4 + 0))  /* Less than */
#define PPC_CR_GT(n)  (0x80000000 >> (n*4 + 1))  /* Greater than */
#define PPC_CR_EQ(n)  (0x80000000 >> (n*4 + 2))  /* Equal */
#define PPC_CR_SO(n)  (0x80000000 >> (n*4 + 3))  /* Summary overflow */

/*
 * PowerPC XER Register Bits
 */
#define PPC_XER_SO    0x80000000  /* Summary overflow */
#define PPC_XER_OV    0x40000000  /* Overflow */
#define PPC_XER_CA    0x20000000  /* Carry */

/*
 * Paged Memory Constants (same as M68K for compatibility)
 */
#define PPC_PAGE_SIZE      4096        /* 4KB pages */
#define PPC_PAGE_SHIFT     12          /* log2(4096) */
#define PPC_MAX_ADDR       0x1000000   /* 16MB virtual address space (for now) */
#define PPC_NUM_PAGES      4096        /* 16MB / 4KB */

/*
 * PowerPC Address Space Implementation
 */
typedef struct PPCAddressSpace {
    void* pageTable[PPC_NUM_PAGES];  /* Sparse page table (NULL = not allocated) */
    UInt32 baseAddr;          /* Base address (typically 0) */

    PPCRegs regs;             /* CPU registers */

    /* Trap table (for Mac Toolbox traps via sc instruction) */
    CPUTrapHandler trapHandlers[256];
    void* trapContexts[256];

    /* Segment tracking */
    void* codeSegments[256];
    UInt32 codeSegBases[256];
    Size codeSegSizes[256];
    int numCodeSegs;

    /* Execution state */
    Boolean halted;           /* CPU halted due to fault or completion */
    UInt16 lastException;     /* Last exception code */
} PPCAddressSpace;

/*
 * PowerPC Code Handle Implementation
 */
typedef struct PPCCodeHandle {
    void* hostMemory;         /* Host-mapped memory */
    UInt32 cpuAddr;           /* CPU address */
    Size size;                /* Size */
    int segIndex;             /* Index in address space segment table */
} PPCCodeHandle;

/*
 * PowerPC Backend Initialization
 */
OSErr PPCBackend_Initialize(void);

/*
 * PowerPC Interpreter Core (exposed for testing)
 */
OSErr PPC_Execute(PPCAddressSpace* as, UInt32 startPC, UInt32 maxInstructions);
OSErr PPC_Step(PPCAddressSpace* as);

#ifdef __cplusplus
}
#endif

#endif /* PPC_INTERP_H */
