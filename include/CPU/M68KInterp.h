/*
 * M68KInterp.h - 68K Interpreter CPU Backend
 *
 * Implements ICPUBackend for 68K code execution on x86 (or any) host.
 * Uses software interpretation of 68K instructions.
 */

#ifndef M68K_INTERP_H
#define M68K_INTERP_H

#include "CPU/CPUBackend.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * M68K Register File
 */
typedef struct M68KRegs {
    UInt32 d[8];              /* D0-D7 data registers */
    UInt32 a[8];              /* A0-A7 address registers (A7 = SP) */
    UInt32 pc;                /* Program counter */
    UInt16 sr;                /* Status register */
    UInt32 usp;               /* User stack pointer */
    UInt32 ssp;               /* Supervisor stack pointer */
} M68KRegs;

/*
 * 68K Exception Vectors
 */
#define M68K_VEC_RESET_SSP      0   /* Reset: Initial SSP */
#define M68K_VEC_RESET_PC       1   /* Reset: Initial PC */
#define M68K_VEC_BUS_ERROR      2   /* Bus error */
#define M68K_VEC_ADDRESS_ERROR  3   /* Address error */
#define M68K_VEC_ILLEGAL        4   /* Illegal instruction */
#define M68K_VEC_DIVIDE_ZERO    5   /* Integer divide by zero */
#define M68K_VEC_CHK            6   /* CHK instruction */
#define M68K_VEC_TRAPV          7   /* TRAPV instruction */
#define M68K_VEC_PRIVILEGE      8   /* Privilege violation */
#define M68K_VEC_TRACE          9   /* Trace */
#define M68K_VEC_LINE_A         10  /* Line 1010 emulator */
#define M68K_VEC_LINE_F         11  /* Line 1111 emulator */

/*
 * M68K Address Space Implementation
 */
typedef struct M68KAddressSpace {
    void* memory;             /* Host memory backing store */
    Size memorySize;          /* Total memory size */
    UInt32 baseAddr;          /* Base address (typically 0) */

    M68KRegs regs;            /* CPU registers */

    /* Trap table (A-line traps 0xA000-0xAFFF) */
    CPUTrapHandler trapHandlers[256];
    void* trapContexts[256];

    /* Segment tracking */
    void* codeSegments[256];
    UInt32 codeSegBases[256];
    Size codeSegSizes[256];
    int numCodeSegs;

    /* Execution state */
    Boolean halted;           /* CPU halted due to fault or completion */
    UInt16 lastException;     /* Last exception vector number */
} M68KAddressSpace;

/*
 * M68K Code Handle Implementation
 */
typedef struct M68KCodeHandle {
    void* hostMemory;         /* Host-mapped memory */
    UInt32 cpuAddr;           /* CPU address */
    Size size;                /* Size */
    int segIndex;             /* Index in address space segment table */
} M68KCodeHandle;

/*
 * M68K Backend Initialization
 */
OSErr M68KBackend_Initialize(void);

/*
 * M68K Interpreter Core (exposed for testing)
 */
OSErr M68K_Execute(M68KAddressSpace* as, UInt32 startPC, UInt32 maxInstructions);
OSErr M68K_Step(M68KAddressSpace* as);

#ifdef __cplusplus
}
#endif

#endif /* M68K_INTERP_H */
