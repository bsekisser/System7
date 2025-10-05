/*
 * M68KBackend.c - 68K Interpreter CPU Backend Implementation
 *
 * Implements ICPUBackend interface for 68K code execution via interpretation.
 * Runs on any host ISA (x86, ARM, etc.) by interpreting 68K instructions.
 */

#include "CPU/M68KInterp.h"
#include "CPU/CPUBackend.h"
#include "SegmentLoader/SegmentLoader.h"
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

    as = (M68KAddressSpace*)NewPtr(sizeof(M68KAddressSpace));
    if (!as) {
        return memFullErr;
    }

    memset(as, 0, sizeof(M68KAddressSpace));

    /* Allocate 16MB address space (classic Mac limit) */
    as->memorySize = 16 * 1024 * 1024;
    as->memory = NewPtr(as->memorySize);
    if (!as->memory) {
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

    if (!mas) {
        return paramErr;
    }

    /* Set PC to entry point */
    mas->regs.pc = entry;

    /*
     * TODO: Start interpreter loop
     * For now, this is a stub that returns immediately
     */

    (void)flags; /* Unused */

    /* In real implementation, would:
     * - Save host CPU context
     * - Enter interpreter loop (M68K_Execute)
     * - Run until exit trap or error
     */

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
 * M68K Interpreter Core (Minimal Implementation)
 *
 * This is a simplified interpreter for demonstration purposes.
 * A full implementation would decode and execute all 68K instructions.
 */
OSErr M68K_Execute(M68KAddressSpace* as, UInt32 startPC, UInt32 maxInstructions)
{
    if (!as) {
        return paramErr;
    }

    as->regs.pc = startPC;

    /* TODO: Implement instruction fetch/decode/execute loop */

    (void)maxInstructions; /* Unused */

    return noErr;
}

OSErr M68K_Step(M68KAddressSpace* as)
{
    if (!as) {
        return paramErr;
    }

    /* TODO: Fetch and execute one instruction */

    return noErr;
}
