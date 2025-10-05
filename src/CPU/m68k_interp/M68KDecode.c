/*
 * M68KDecode.c - 68K Instruction Fetch and Effective Address Decoding
 *
 * Provides fetch helpers, EA computation, and EA read/write operations
 * for the Phase-1 MVP 68K interpreter.
 */

#include "CPU/M68KInterp.h"
#include "CPU/M68KOpcodes.h"
#include "System71StdLib.h"
#include <string.h>

/*
 * Forward declarations from M68KOpcodes.c
 */
extern void M68K_Fault(M68KAddressSpace* as, const char* reason);

/*
 * Fetch16 - Fetch next 16-bit word from PC (big-endian)
 */
UInt16 M68K_Fetch16(M68KAddressSpace* as)
{
    UInt16 value;
    UInt8 b0, b1;

    if (as->regs.pc + 1 >= as->memorySize) {
        M68K_Fault(as, "PC out of bounds in Fetch16");
        return 0;
    }

    b0 = ((UInt8*)as->memory)[as->regs.pc];
    b1 = ((UInt8*)as->memory)[as->regs.pc + 1];
    value = (b0 << 8) | b1;

    as->regs.pc += 2;
    return value;
}

/*
 * Fetch32 - Fetch next 32-bit long from PC (big-endian)
 */
UInt32 M68K_Fetch32(M68KAddressSpace* as)
{
    UInt32 hi = M68K_Fetch16(as);
    UInt32 lo = M68K_Fetch16(as);
    return (hi << 16) | lo;
}

/*
 * Read8 - Read byte from address space
 */
UInt8 M68K_Read8(M68KAddressSpace* as, UInt32 addr)
{
    if (addr >= as->memorySize) {
        M68K_Fault(as, "Read8 out of bounds");
        return 0;
    }
    return ((UInt8*)as->memory)[addr];
}

/*
 * Read16 - Read word from address space (big-endian)
 */
UInt16 M68K_Read16(M68KAddressSpace* as, UInt32 addr)
{
    UInt8 b0, b1;

    if (addr + 1 >= as->memorySize) {
        M68K_Fault(as, "Read16 out of bounds");
        return 0;
    }

    b0 = ((UInt8*)as->memory)[addr];
    b1 = ((UInt8*)as->memory)[addr + 1];
    return (b0 << 8) | b1;
}

/*
 * Read32 - Read long from address space (big-endian)
 */
UInt32 M68K_Read32(M68KAddressSpace* as, UInt32 addr)
{
    UInt32 hi = M68K_Read16(as, addr);
    UInt32 lo = M68K_Read16(as, addr + 2);
    return (hi << 16) | lo;
}

/*
 * Write8 - Write byte to address space
 */
void M68K_Write8(M68KAddressSpace* as, UInt32 addr, UInt8 value)
{
    if (addr >= as->memorySize) {
        M68K_Fault(as, "Write8 out of bounds");
        return;
    }
    ((UInt8*)as->memory)[addr] = value;
}

/*
 * Write16 - Write word to address space (big-endian)
 */
void M68K_Write16(M68KAddressSpace* as, UInt32 addr, UInt16 value)
{
    if (addr + 1 >= as->memorySize) {
        M68K_Fault(as, "Write16 out of bounds");
        return;
    }
    ((UInt8*)as->memory)[addr] = (value >> 8) & 0xFF;
    ((UInt8*)as->memory)[addr + 1] = value & 0xFF;
}

/*
 * Write32 - Write long to address space (big-endian)
 */
void M68K_Write32(M68KAddressSpace* as, UInt32 addr, UInt32 value)
{
    M68K_Write16(as, addr, value >> 16);
    M68K_Write16(as, addr + 2, value & 0xFFFF);
}

/*
 * EA_ComputeAddress - Compute effective address without reading
 * Returns address; for register direct modes, returns register number
 */
UInt32 M68K_EA_ComputeAddress(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size)
{
    UInt32 addr;
    UInt16 ext;
    SInt16 disp;
    UInt8 xn_reg, xn_type;
    SInt32 index;

    switch (mode) {
        case MODE_Dn:
            /* Dn - return register number */
            return reg;

        case MODE_An:
            /* An - return register number */
            return reg;

        case MODE_An_IND:
            /* (An) */
            return as->regs.a[reg];

        case MODE_An_POST:
            /* (An)+ - return current An, will increment after */
            return as->regs.a[reg];

        case MODE_An_PRE:
            /* -(An) - decrement An first, then return */
            if (size == SIZE_BYTE && reg == 7) {
                as->regs.a[reg] -= 2;  /* A7 byte operations use word size */
            } else {
                as->regs.a[reg] -= SIZE_BYTES(size);
            }
            return as->regs.a[reg];

        case MODE_An_DISP:
            /* d16(An) */
            disp = (SInt16)M68K_Fetch16(as);
            return as->regs.a[reg] + disp;

        case MODE_An_INDEX:
            /* d8(An,Xn) */
            ext = M68K_Fetch16(as);
            disp = SIGN_EXTEND_BYTE(ext & 0xFF);
            xn_reg = (ext >> 12) & 0xF;
            xn_type = (ext >> 15) & 1;  /* 0=Dn, 1=An */

            if (xn_type) {
                index = as->regs.a[xn_reg & 7];
            } else {
                index = as->regs.d[xn_reg & 7];
            }

            /* Check index size (bit 11: 0=sign-extend word, 1=long) */
            if (!(ext & 0x800)) {
                index = SIGN_EXTEND_WORD(index & 0xFFFF);
            }

            return as->regs.a[reg] + disp + index;

        case MODE_OTHER:
            switch (reg) {
                case OTHER_ABS_W:
                    /* abs.W */
                    return SIGN_EXTEND_WORD(M68K_Fetch16(as));

                case OTHER_ABS_L:
                    /* abs.L */
                    return M68K_Fetch32(as);

                case OTHER_PC_DISP:
                    /* d16(PC) - PC at start of extension word */
                    addr = as->regs.pc;
                    disp = (SInt16)M68K_Fetch16(as);
                    return addr + disp;

                case OTHER_PC_INDEX:
                    /* d8(PC,Xn) */
                    addr = as->regs.pc;
                    ext = M68K_Fetch16(as);
                    disp = SIGN_EXTEND_BYTE(ext & 0xFF);
                    xn_reg = (ext >> 12) & 0xF;
                    xn_type = (ext >> 15) & 1;

                    if (xn_type) {
                        index = as->regs.a[xn_reg & 7];
                    } else {
                        index = as->regs.d[xn_reg & 7];
                    }

                    if (!(ext & 0x800)) {
                        index = SIGN_EXTEND_WORD(index & 0xFFFF);
                    }

                    return addr + disp + index;

                case OTHER_IMMEDIATE:
                    /* #<data> - return PC, caller fetches immediate */
                    return as->regs.pc;

                default:
                    M68K_Fault(as, "Invalid OTHER mode in EA");
                    return 0;
            }

        default:
            M68K_Fault(as, "Invalid addressing mode in EA");
            return 0;
    }
}

/*
 * EA_Read - Read value from effective address
 */
UInt32 M68K_EA_Read(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size)
{
    UInt32 addr;
    UInt32 value;

    /* Handle register direct modes specially */
    if (mode == MODE_Dn) {
        /* Dn - read from data register */
        value = as->regs.d[reg];
        return value & SIZE_MASK(size);
    }

    if (mode == MODE_An) {
        /* An - read from address register (always 32-bit) */
        return as->regs.a[reg];
    }

    /* Handle immediate mode */
    if (mode == MODE_OTHER && reg == OTHER_IMMEDIATE) {
        if (size == SIZE_BYTE || size == SIZE_WORD) {
            return M68K_Fetch16(as) & SIZE_MASK(size);
        } else {
            return M68K_Fetch32(as);
        }
    }

    /* Compute address and read from memory */
    addr = M68K_EA_ComputeAddress(as, mode, reg, size);

    switch (size) {
        case SIZE_BYTE:
            value = M68K_Read8(as, addr);
            break;
        case SIZE_WORD:
            value = M68K_Read16(as, addr);
            break;
        case SIZE_LONG:
            value = M68K_Read32(as, addr);
            break;
        default:
            M68K_Fault(as, "Invalid size in EA_Read");
            return 0;
    }

    /* Handle postincrement */
    if (mode == MODE_An_POST) {
        if (size == SIZE_BYTE && reg == 7) {
            as->regs.a[reg] += 2;  /* A7 byte operations use word size */
        } else {
            as->regs.a[reg] += SIZE_BYTES(size);
        }
    }

    return value;
}

/*
 * EA_Write - Write value to effective address
 */
void M68K_EA_Write(M68KAddressSpace* as, UInt8 mode, UInt8 reg, M68KSize size, UInt32 value)
{
    UInt32 addr;

    /* Handle register direct modes specially */
    if (mode == MODE_Dn) {
        /* Dn - write to data register with proper masking */
        switch (size) {
            case SIZE_BYTE:
                as->regs.d[reg] = (as->regs.d[reg] & 0xFFFFFF00) | (value & 0xFF);
                break;
            case SIZE_WORD:
                as->regs.d[reg] = (as->regs.d[reg] & 0xFFFF0000) | (value & 0xFFFF);
                break;
            case SIZE_LONG:
                as->regs.d[reg] = value;
                break;
        }
        return;
    }

    if (mode == MODE_An) {
        /* An - always 32-bit write */
        as->regs.a[reg] = value;
        return;
    }

    /* Compute address and write to memory */
    addr = M68K_EA_ComputeAddress(as, mode, reg, size);

    switch (size) {
        case SIZE_BYTE:
            M68K_Write8(as, addr, value & 0xFF);
            break;
        case SIZE_WORD:
            M68K_Write16(as, addr, value & 0xFFFF);
            break;
        case SIZE_LONG:
            M68K_Write32(as, addr, value);
            break;
        default:
            M68K_Fault(as, "Invalid size in EA_Write");
            return;
    }

    /* Handle postincrement */
    if (mode == MODE_An_POST) {
        if (size == SIZE_BYTE && reg == 7) {
            as->regs.a[reg] += 2;  /* A7 byte operations use word size */
        } else {
            as->regs.a[reg] += SIZE_BYTES(size);
        }
    }
}
