/*
 * M68KBackend.c - 68K Interpreter CPU Backend Implementation
 *
 * Implements ICPUBackend interface for 68K code execution via interpretation.
 * Runs on any host ISA (x86, ARM, etc.) by interpreting 68K instructions.
 */

#include "CPU/M68KInterp.h"
#include "CPU/CPUBackend.h"
#include "SegmentLoader/SegmentLoader.h"
#include "MemoryMgr/MemoryManager.h"
#include "System71StdLib.h"
#include <string.h>

/* Forward declarations of ICPUBackend methods */
static OSErr M68K_CreateAddressSpace(void* processHandle, CPUAddressSpace* out);
static OSErr M68K_DestroyAddressSpace(CPUAddressSpace as);
static OSErr M68K_MapExecutable(CPUAddressSpace as, const void* image, Size len,
                                CPUMapFlags flags, CPUCodeHandle* outHandle,
                                CPUAddr* outBase);
static OSErr M68K_UnmapExecutable(CPUAddressSpace as, CPUCodeHandle handle);
static OSErr M68K_SetRegisterA5(CPUAddressSpace as, CPUAddr a5);
static OSErr M68K_SetStacks(CPUAddressSpace as, CPUAddr usp, CPUAddr ssp);
static OSErr M68K_InstallTrap(CPUAddressSpace as, TrapNumber trapNum,
                              CPUTrapHandler handler, void* context);
static OSErr M68K_WriteJumpTableSlot(CPUAddressSpace as, CPUAddr slotAddr,
                                     CPUAddr target);
static OSErr M68K_MakeLazyJTStub(CPUAddressSpace as, CPUAddr slotAddr,
                                 SInt16 segID, SInt16 entryIndex);
static OSErr M68K_EnterAt(CPUAddressSpace as, CPUAddr entry, CPUEnterFlags flags);
static OSErr M68K_Relocate(CPUAddressSpace as, CPUCodeHandle code,
                           const RelocTable* relocs, CPUAddr segBase,
                           CPUAddr jtBase, CPUAddr a5Base);
static OSErr M68K_AllocateMemory(CPUAddressSpace as, Size size,
                                 CPUMapFlags flags, CPUAddr* outAddr);
static OSErr M68K_WriteMemory(CPUAddressSpace as, CPUAddr addr,
                              const void* data, Size len);
static OSErr M68K_ReadMemory(CPUAddressSpace as, CPUAddr addr,
                             void* data, Size len);

/*
 * Global M68K Backend Instance
 */
const ICPUBackend gM68KInterpreterBackend = {
    .CreateAddressSpace = M68K_CreateAddressSpace,
    .DestroyAddressSpace = M68K_DestroyAddressSpace,
    .MapExecutable = M68K_MapExecutable,
    .UnmapExecutable = M68K_UnmapExecutable,
    .SetRegisterA5 = M68K_SetRegisterA5,
    .SetStacks = M68K_SetStacks,
    .InstallTrap = M68K_InstallTrap,
    .WriteJumpTableSlot = M68K_WriteJumpTableSlot,
    .MakeLazyJTStub = M68K_MakeLazyJTStub,
    .EnterAt = M68K_EnterAt,
    .Relocate = M68K_Relocate,
    .AllocateMemory = M68K_AllocateMemory,
    .WriteMemory = M68K_WriteMemory,
    .ReadMemory = M68K_ReadMemory
};

/*
 * M68K Backend Initialization
 */
OSErr M68KBackend_Initialize(void)
{
    return CPUBackend_Register("m68k_interp", &gM68KInterpreterBackend);
}

/*
 * CreateAddressSpace - Allocate M68K address space
 */
static OSErr M68K_CreateAddressSpace(void* processHandle, CPUAddressSpace* out)
{
    M68KAddressSpace* as;

    (void)processHandle; /* Unused for now */

    serial_printf("[M68K] CreateAddressSpace: allocating M68KAddressSpace struct size=%u\n",
                  (unsigned)sizeof(M68KAddressSpace));
    as = (M68KAddressSpace*)NewPtr(sizeof(M68KAddressSpace));
    if (!as) {
        serial_printf("[M68K] FAIL: struct allocation memFullErr, MemError=%d\n", MemError());
        return memFullErr;
    }

    memset(as, 0, sizeof(M68KAddressSpace));

    /* Allocate 1MB address space (temporary for MVP triage) */
    as->memorySize = 1 * 1024 * 1024;
    serial_printf("[M68K] CreateAddressSpace: allocating memory buffer size=%u bytes\n",
                  (unsigned)as->memorySize);
    as->memory = NewPtr(as->memorySize);
    if (!as->memory) {
        serial_printf("[M68K] FAIL: memory buffer allocation memFullErr, MemError=%d\n", MemError());
        DisposePtr((Ptr)as);
        return memFullErr;
    }

    memset(as->memory, 0, as->memorySize);
    as->baseAddr = 0;

    /* Initialize registers */
    memset(&as->regs, 0, sizeof(M68KRegs));
    as->regs.sr = 0x2700; /* Supervisor mode, interrupts disabled */

    *out = (CPUAddressSpace)as;
    return noErr;
}

/*
 * DestroyAddressSpace - Free address space
 */
static OSErr M68K_DestroyAddressSpace(CPUAddressSpace as)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas) {
        return paramErr;
    }

    if (mas->memory) {
        DisposePtr((Ptr)mas->memory);
    }

    DisposePtr((Ptr)mas);
    return noErr;
}

/*
 * MapExecutable - Map code into address space
 */
static OSErr M68K_MapExecutable(CPUAddressSpace as, const void* image, Size len,
                                CPUMapFlags flags, CPUCodeHandle* outHandle,
                                CPUAddr* outBase)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    M68KCodeHandle* handle;
    UInt32 addr;

    if (!mas || !image || !outHandle || !outBase) {
        return paramErr;
    }

    /* Allocate code handle */
    handle = (M68KCodeHandle*)NewPtr(sizeof(M68KCodeHandle));
    if (!handle) {
        return memFullErr;
    }

    /* Find free address space (simple bump allocator) */
    addr = 0x1000; /* Start at 4K to avoid null pointers */
    for (int i = 0; i < mas->numCodeSegs; i++) {
        UInt32 end = mas->codeSegBases[i] + mas->codeSegSizes[i];
        if (end > addr) {
            addr = end;
        }
    }

    /* Align to 16-byte boundary */
    addr = (addr + 15) & ~15;

    /* Check bounds */
    if (addr + len > mas->memorySize) {
        DisposePtr((Ptr)handle);
        return memFullErr;
    }

    /* Copy code into address space */
    memcpy((UInt8*)mas->memory + addr, image, len);

    /* Track segment */
    if (mas->numCodeSegs < 256) {
        mas->codeSegments[mas->numCodeSegs] = (UInt8*)mas->memory + addr;
        mas->codeSegBases[mas->numCodeSegs] = addr;
        mas->codeSegSizes[mas->numCodeSegs] = len;
        handle->segIndex = mas->numCodeSegs;
        mas->numCodeSegs++;
    } else {
        DisposePtr((Ptr)handle);
        return memFullErr;
    }

    handle->hostMemory = (UInt8*)mas->memory + addr;
    handle->cpuAddr = addr;
    handle->size = len;

    *outHandle = (CPUCodeHandle)handle;
    *outBase = addr;

    return noErr;
}

/*
 * UnmapExecutable - Unmap code segment
 */
static OSErr M68K_UnmapExecutable(CPUAddressSpace as, CPUCodeHandle handle)
{
    M68KCodeHandle* mhandle = (M68KCodeHandle*)handle;

    (void)as; /* Unused */

    if (!mhandle) {
        return paramErr;
    }

    /* For now, just free the handle (memory stays allocated) */
    DisposePtr((Ptr)mhandle);
    return noErr;
}

/*
 * SetRegisterA5 - Set A5 register
 */
static OSErr M68K_SetRegisterA5(CPUAddressSpace as, CPUAddr a5)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas) {
        return paramErr;
    }

    mas->regs.a[5] = a5;
    return noErr;
}

/*
 * SetStacks - Configure stacks
 */
static OSErr M68K_SetStacks(CPUAddressSpace as, CPUAddr usp, CPUAddr ssp)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas) {
        return paramErr;
    }

    mas->regs.usp = usp;
    mas->regs.ssp = ssp;
    mas->regs.a[7] = usp; /* A7 = USP initially */

    return noErr;
}

/*
 * InstallTrap - Install trap handler
 */
static OSErr M68K_InstallTrap(CPUAddressSpace as, TrapNumber trapNum,
                              CPUTrapHandler handler, void* context)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas || trapNum > 255) {
        return paramErr;
    }

    mas->trapHandlers[trapNum] = handler;
    mas->trapContexts[trapNum] = context;

    return noErr;
}

/*
 * WriteJumpTableSlot - Patch jump table entry
 */
static OSErr M68K_WriteJumpTableSlot(CPUAddressSpace as, CPUAddr slotAddr,
                                     CPUAddr target)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    UInt8* slot;

    if (!mas || slotAddr >= mas->memorySize) {
        return paramErr;
    }

    slot = (UInt8*)mas->memory + slotAddr;

    /*
     * Write 68K JMP instruction: 0x4EF9 followed by 32-bit address
     *
     * Format:
     *   +0: 0x4E 0xF9     (JMP absolute long)
     *   +2: target address (big-endian 32-bit)
     */
    slot[0] = 0x4E;
    slot[1] = 0xF9;
    slot[2] = (target >> 24) & 0xFF;
    slot[3] = (target >> 16) & 0xFF;
    slot[4] = (target >> 8) & 0xFF;
    slot[5] = target & 0xFF;

    return noErr;
}

/*
 * MakeLazyJTStub - Create lazy-loading stub
 */
static OSErr M68K_MakeLazyJTStub(CPUAddressSpace as, CPUAddr slotAddr,
                                 SInt16 segID, SInt16 entryIndex)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    UInt8* slot;

    if (!mas || slotAddr >= mas->memorySize) {
        return paramErr;
    }

    slot = (UInt8*)mas->memory + slotAddr;

    /*
     * Create lazy stub that triggers _LoadSeg:
     *
     *   +0: 0x3F3C  MOVE.W #segID,-(SP)
     *   +2: segID (16-bit)
     *   +4: 0xA9F0  _LoadSeg trap
     *   +6: 0x4E75  RTS (return after load)
     *
     * Note: entryIndex is embedded in the trap context
     */
    slot[0] = 0x3F;
    slot[1] = 0x3C;
    slot[2] = (segID >> 8) & 0xFF;
    slot[3] = segID & 0xFF;
    slot[4] = 0xA9;
    slot[5] = 0xF0;
    slot[6] = 0x4E;
    slot[7] = 0x75;

    (void)entryIndex; /* Stored in trap handler context */

    return noErr;
}

/*
 * EnterAt - Begin execution
 */
static OSErr M68K_EnterAt(CPUAddressSpace as, CPUAddr entry, CPUEnterFlags flags)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    UInt32 max_instructions = 100000;  /* Safety limit */

    if (!mas) {
        return paramErr;
    }

    serial_printf("[M68K] EnterAt: entry=0x%08X flags=0x%04X\n", entry, flags);

    /* Clear halted flag */
    mas->halted = false;

    /* Execute from entry point */
    M68K_Execute(mas, entry, max_instructions);

    if (mas->halted) {
        serial_printf("[M68K] Execution halted at PC=0x%08X\n", mas->regs.pc);
    } else {
        serial_printf("[M68K] Execution completed after %u instructions\n", max_instructions);
    }

    return noErr;
}

/*
 * Relocate - Apply relocations
 */
static OSErr M68K_Relocate(CPUAddressSpace as, CPUCodeHandle code,
                           const RelocTable* relocs, CPUAddr segBase,
                           CPUAddr jtBase, CPUAddr a5Base)
{
    M68KCodeHandle* mhandle = (M68KCodeHandle*)code;
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    UInt8* codeData;

    if (!mas || !mhandle || !relocs) {
        return paramErr;
    }

    codeData = (UInt8*)mhandle->hostMemory;

    /* Apply each relocation */
    for (UInt16 i = 0; i < relocs->count; i++) {
        const RelocEntry* reloc = &relocs->entries[i];
        UInt32 offset = reloc->atOffset;
        UInt32 value = 0;

        if (offset + 4 > mhandle->size) {
            return segmentRelocErr;
        }

        switch (reloc->kind) {
            case kRelocAbsSegBase:
                /* Patch absolute address with segment base */
                value = segBase + reloc->addend;
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                break;

            case kRelocA5Relative:
                /* Patch A5-relative offset */
                value = a5Base + reloc->addend;
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                break;

            case kRelocJTImport:
                /* Patch jump table import */
                value = jtBase + (reloc->jtIndex * 8); /* 8 bytes per JT entry */
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                break;

            case kRelocPCRel16:
            case kRelocPCRel32:
            case kRelocSegmentRef:
                /* TODO: Implement these relocation types */
                break;

            default:
                return segmentRelocErr;
        }
    }

    return noErr;
}

/*
 * AllocateMemory - Allocate memory in CPU address space
 */
static OSErr M68K_AllocateMemory(CPUAddressSpace as, Size size,
                                 CPUMapFlags flags, CPUAddr* outAddr)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;
    UInt32 addr;

    if (!mas || !outAddr) {
        return paramErr;
    }

    /* Find free space (simple bump allocator) */
    addr = 0x10000; /* Start at 64K */
    for (int i = 0; i < mas->numCodeSegs; i++) {
        UInt32 end = mas->codeSegBases[i] + mas->codeSegSizes[i];
        if (end > addr) {
            addr = end;
        }
    }

    /* Align to 16-byte boundary */
    addr = (addr + 15) & ~15;

    /* Check bounds */
    if (addr + size > mas->memorySize) {
        return memFullErr;
    }

    /* Zero memory */
    memset((UInt8*)mas->memory + addr, 0, size);

    *outAddr = addr;

    (void)flags; /* Unused for now */

    return noErr;
}

/*
 * WriteMemory - Write to CPU address space
 */
static OSErr M68K_WriteMemory(CPUAddressSpace as, CPUAddr addr,
                              const void* data, Size len)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas || !data || addr + len > mas->memorySize) {
        return paramErr;
    }

    memcpy((UInt8*)mas->memory + addr, data, len);
    return noErr;
}

/*
 * ReadMemory - Read from CPU address space
 */
static OSErr M68K_ReadMemory(CPUAddressSpace as, CPUAddr addr,
                             void* data, Size len)
{
    M68KAddressSpace* mas = (M68KAddressSpace*)as;

    if (!mas || !data || addr + len > mas->memorySize) {
        return paramErr;
    }

    memcpy(data, (UInt8*)mas->memory + addr, len);
    return noErr;
}

/*
 * Opcode Handler Declarations (from M68KOpcodes.c)
 */
extern void M68K_Op_MOVE(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_MOVEA(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_LEA(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_PEA(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_CLR(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_NOT(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_ADD(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_SUB(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_CMP(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_LINK(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_UNLK(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_JSR(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_JMP(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_BRA(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_BSR(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_Bcc(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_RTS(M68KAddressSpace* as, UInt16 opcode);
extern void M68K_Op_TRAP(M68KAddressSpace* as, UInt16 opcode);
extern UInt16 M68K_Fetch16(M68KAddressSpace* as);
extern void M68K_Fault(M68KAddressSpace* as, const char* reason);

/*
 * M68K_Step - Fetch and execute one instruction
 */
OSErr M68K_Step(M68KAddressSpace* as)
{
    UInt16 opcode;

    if (!as) {
        return paramErr;
    }

    if (as->halted) {
        return noErr;
    }

    /* Fetch opcode */
    opcode = M68K_Fetch16(as);

    /* Decode and dispatch */
    if ((opcode & 0xF000) == 0x0000) {
        /* 0xxx - Bit manipulation, MOVEP, immediate */
        if ((opcode & 0xFF00) == 0x4200) {
            M68K_Op_CLR(as, opcode);
        } else if ((opcode & 0xFF00) == 0x4600) {
            M68K_Op_NOT(as, opcode);
        } else if ((opcode & 0xC000) == 0x0000) {
            /* Could be MOVE with size bits 01/10/11 */
            M68K_Op_MOVE(as, opcode);
        } else {
            M68K_Fault(as, "Unimplemented 0xxx opcode");
        }
    } else if ((opcode & 0xC000) == 0x0000 || (opcode & 0xC000) == 0x4000 || (opcode & 0xC000) == 0x8000) {
        /* MOVE family - check for size bits in upper nibble */
        UInt8 size_bits = (opcode >> 12) & 3;
        if (size_bits == 1 || size_bits == 2 || size_bits == 3) {
            /* MOVE.B (01), MOVE.L (10), MOVE.W (11) */
            if ((opcode & 0x01C0) == 0x0040) {
                /* MOVEA - bit 6 set */
                M68K_Op_MOVEA(as, opcode);
            } else {
                M68K_Op_MOVE(as, opcode);
            }
        } else if ((opcode & 0xF1C0) == 0x41C0) {
            /* LEA */
            M68K_Op_LEA(as, opcode);
        } else if ((opcode & 0xFFC0) == 0x4840) {
            /* PEA */
            M68K_Op_PEA(as, opcode);
        } else if ((opcode & 0xFFC0) == 0x4E80) {
            /* JSR */
            M68K_Op_JSR(as, opcode);
        } else if ((opcode & 0xFFC0) == 0x4EC0) {
            /* JMP */
            M68K_Op_JMP(as, opcode);
        } else if ((opcode & 0xFFFF) == 0x4E75) {
            /* RTS */
            M68K_Op_RTS(as, opcode);
        } else if ((opcode & 0xFFF8) == 0x4E50) {
            /* LINK */
            M68K_Op_LINK(as, opcode);
        } else if ((opcode & 0xFFF8) == 0x4E58) {
            /* UNLK */
            M68K_Op_UNLK(as, opcode);
        } else if ((opcode & 0xFF00) == 0x4200) {
            /* CLR */
            M68K_Op_CLR(as, opcode);
        } else if ((opcode & 0xFF00) == 0x4600) {
            /* NOT */
            M68K_Op_NOT(as, opcode);
        } else {
            M68K_Fault(as, "Unimplemented 4xxx opcode");
        }
    } else if ((opcode & 0xF000) == 0x6000) {
        /* 6xxx - Branch instructions */
        if ((opcode & 0xFF00) == 0x6000) {
            M68K_Op_BRA(as, opcode);
        } else if ((opcode & 0xFF00) == 0x6100) {
            M68K_Op_BSR(as, opcode);
        } else {
            M68K_Op_Bcc(as, opcode);
        }
    } else if ((opcode & 0xF000) == 0x9000) {
        /* 9xxx - SUB */
        M68K_Op_SUB(as, opcode);
    } else if ((opcode & 0xF000) == 0xA000) {
        /* Axxx - A-line trap */
        M68K_Op_TRAP(as, opcode);
    } else if ((opcode & 0xF100) == 0xB000) {
        /* Bxxx - CMP */
        M68K_Op_CMP(as, opcode);
    } else if ((opcode & 0xF000) == 0xD000) {
        /* Dxxx - ADD */
        M68K_Op_ADD(as, opcode);
    } else {
        serial_printf("[M68K] ILLEGAL opcode 0x%04X at PC=0x%08X\n", opcode, as->regs.pc - 2);
        M68K_Fault(as, "Illegal opcode");
    }

    return noErr;
}

/*
 * M68K_Execute - Execute up to maxInstructions
 */
OSErr M68K_Execute(M68KAddressSpace* as, UInt32 startPC, UInt32 maxInstructions)
{
    UInt32 count = 0;

    if (!as) {
        return paramErr;
    }

    as->regs.pc = startPC;
    as->halted = false;

    while (count < maxInstructions && !as->halted) {
        M68K_Step(as);
        count++;
    }

    return noErr;
}
