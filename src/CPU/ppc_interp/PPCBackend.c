/*
 * PPCBackend.c - PowerPC Interpreter CPU Backend Implementation
 *
 * Implements ICPUBackend interface for PowerPC code execution via interpretation.
 * Runs on any host ISA (x86, ARM, Raspberry Pi, etc.) by interpreting PowerPC instructions.
 *
 * PLATFORM SUPPORT:
 * - x86 (IA32): Fully supported
 * - ARM (ARMv6, ARMv7, ARMv8): Fully supported, enables PowerPC compatibility on Raspberry Pi
 * - Other architectures: Should work with no source modifications due to explicit byte ordering
 *
 * CROSS-PLATFORM GUARANTEES:
 * - All PowerPC values are stored in big-endian format (PowerPC byte order)
 * - Memory operations use explicit byte reconstruction, never assume host endianness
 * - Page allocation is generic and works on all architectures
 * - No inline assembly or architecture-specific tricks
 */

#include "CPU/PPCInterp.h"
#include "CPU/PPCOpcodes.h"
#include "CPU/CPUBackend.h"
#include "CPU/LowMemGlobals.h"
#include "SegmentLoader/SegmentLoader.h"
#include "MemoryMgr/MemoryManager.h"
#include "System71StdLib.h"
#include <string.h>

/* Forward declarations of ICPUBackend methods */
static OSErr PPC_CreateAddressSpace(void* processHandle, CPUAddressSpace* out);
static OSErr PPC_DestroyAddressSpace(CPUAddressSpace as);
static OSErr PPC_MapExecutable(CPUAddressSpace as, const void* image, Size len,
                               CPUMapFlags flags, CPUCodeHandle* outHandle,
                               CPUAddr* outBase);
static OSErr PPC_UnmapExecutable(CPUAddressSpace as, CPUCodeHandle handle);
static OSErr PPC_SetRegisterA5(CPUAddressSpace as, CPUAddr a5);
static OSErr PPC_SetStacks(CPUAddressSpace as, CPUAddr usp, CPUAddr ssp);
static OSErr PPC_InstallTrap(CPUAddressSpace as, TrapNumber trapNum,
                             CPUTrapHandler handler, void* context);
static OSErr PPC_WriteJumpTableSlot(CPUAddressSpace as, CPUAddr slotAddr,
                                    CPUAddr target);
static OSErr PPC_MakeLazyJTStub(CPUAddressSpace as, CPUAddr slotAddr,
                                SInt16 segID, SInt16 entryIndex);
static OSErr PPC_EnterAt(CPUAddressSpace as, CPUAddr entry, CPUEnterFlags flags);
static OSErr PPC_Relocate(CPUAddressSpace as, CPUCodeHandle code,
                          const RelocTable* relocs, CPUAddr segBase,
                          CPUAddr jtBase, CPUAddr a5Base);
static OSErr PPC_AllocateMemory(CPUAddressSpace as, Size size,
                                CPUMapFlags flags, CPUAddr* outAddr);
static OSErr PPC_WriteMemory(CPUAddressSpace as, CPUAddr addr,
                             const void* data, Size len);
static OSErr PPC_ReadMemory(CPUAddressSpace as, CPUAddr addr,
                            void* data, Size len);

/*
 * Global PowerPC Backend Instance
 */
const ICPUBackend gPPCInterpreterBackend = {
    .CreateAddressSpace = PPC_CreateAddressSpace,
    .DestroyAddressSpace = PPC_DestroyAddressSpace,
    .MapExecutable = PPC_MapExecutable,
    .UnmapExecutable = PPC_UnmapExecutable,
    .SetRegisterA5 = PPC_SetRegisterA5,
    .SetStacks = PPC_SetStacks,
    .InstallTrap = PPC_InstallTrap,
    .WriteJumpTableSlot = PPC_WriteJumpTableSlot,
    .MakeLazyJTStub = PPC_MakeLazyJTStub,
    .EnterAt = PPC_EnterAt,
    .Relocate = PPC_Relocate,
    .AllocateMemory = PPC_AllocateMemory,
    .WriteMemory = PPC_WriteMemory,
    .ReadMemory = PPC_ReadMemory
};

/*
 * PowerPC Backend Initialization
 */
OSErr PPCBackend_Initialize(void)
{
    return CPUBackend_Register("ppc_interp", &gPPCInterpreterBackend);
}

/*
 * CreateAddressSpace - Allocate PowerPC address space
 */
static OSErr PPC_CreateAddressSpace(void* processHandle, CPUAddressSpace* out)
{
    PPCAddressSpace* as;

    (void)processHandle; /* Unused for now */

    serial_printf("[PPC] CreateAddressSpace: allocating PPCAddressSpace struct size=%u\n",
                  (unsigned)sizeof(PPCAddressSpace));
    as = (PPCAddressSpace*)NewPtr(sizeof(PPCAddressSpace));
    if (!as) {
        serial_printf("[PPC] FAIL: struct allocation memFullErr, MemError=%d\n", MemError());
        return memFullErr;
    }

    memset(as, 0, sizeof(PPCAddressSpace));
    as->baseAddr = 0;

    /* Initialize page table (all NULL = not allocated) */
    memset(as->pageTable, 0, sizeof(as->pageTable));

    serial_printf("[PPC] CreateAddressSpace: sparse 16MB virtual space ready\n");

    /* Initialize registers */
    memset(&as->regs, 0, sizeof(PPCRegs));
    as->regs.msr = 0x0000; /* User mode initially */

    *out = (CPUAddressSpace)as;
    return noErr;
}

/*
 * DestroyAddressSpace - Free address space
 */
static OSErr PPC_DestroyAddressSpace(CPUAddressSpace as)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;

    if (!pas) {
        return paramErr;
    }

    /* Free all allocated pages */
    for (int i = 0; i < PPC_NUM_PAGES; i++) {
        if (pas->pageTable[i]) {
            if (!MemoryManager_IsHeapPointer(pas->pageTable[i])) {
                DisposePtr((Ptr)pas->pageTable[i]);
            }
            pas->pageTable[i] = NULL;
        }
    }

    DisposePtr((Ptr)pas);
    return noErr;
}

/* Forward declaration */
void* PPC_GetPage(PPCAddressSpace* as, UInt32 addr, Boolean allocate);

/*
 * PPC_MemCopy - Copy data to paged memory (lazy page allocation)
 */
static OSErr PPC_MemCopy(PPCAddressSpace* as, UInt32 addr, const void* src, Size len)
{
    const UInt8* srcBytes = (const UInt8*)src;

    for (Size i = 0; i < len; i++) {
        void* page = PPC_GetPage(as, addr + i, true);
        if (!page) {
            return memFullErr;
        }
        UInt32 offset = (addr + i) & (PPC_PAGE_SIZE - 1);
        ((UInt8*)page)[offset] = srcBytes[i];
    }
    return noErr;
}

/*
 * PPC_GetPage - Get page for address, allocating if needed (lazy allocation)
 * Returns NULL if address out of range or allocation fails
 */
void* PPC_GetPage(PPCAddressSpace* as, UInt32 addr, Boolean allocate)
{
    UInt32 pageNum;
    void* page;

    /* Check address range */
    if (addr >= PPC_MAX_ADDR) {
        return NULL;
    }

    pageNum = addr >> PPC_PAGE_SHIFT;
    page = as->pageTable[pageNum];

    /* If page not allocated and allocation requested, allocate now */
    if (!page && allocate) {
        page = NewPtr(PPC_PAGE_SIZE);
        if (page) {
            memset(page, 0, PPC_PAGE_SIZE);
            as->pageTable[pageNum] = page;
            serial_printf("[PPC] Allocated page %u for addr 0x%08X\n", pageNum, addr);
        } else {
            serial_printf("[PPC] FAIL: page %u allocation failed, MemError=%d\n",
                         pageNum, MemError());
        }
    }

    return page;
}

/*
 * MapExecutable - Map code into address space
 */
static OSErr PPC_MapExecutable(CPUAddressSpace as, const void* image, Size len,
                               CPUMapFlags flags, CPUCodeHandle* outHandle,
                               CPUAddr* outBase)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    PPCCodeHandle* handle;
    UInt32 addr;

    if (!pas || !image || !outHandle || !outBase) {
        return paramErr;
    }

    /* Allocate code handle */
    handle = (PPCCodeHandle*)NewPtr(sizeof(PPCCodeHandle));
    if (!handle) {
        return memFullErr;
    }

    /* Find free address space (simple bump allocator) */
    addr = 0x1000; /* Start at 4K to avoid null pointers */
    for (int i = 0; i < pas->numCodeSegs; i++) {
        UInt32 end = pas->codeSegBases[i] + pas->codeSegSizes[i];
        if (end > addr) {
            addr = end;
        }
    }

    /* Align to 16-byte boundary */
    addr = (addr + 15) & ~15;

    /* Check bounds */
    if (addr + len > PPC_MAX_ADDR) {
        DisposePtr((Ptr)handle);
        return memFullErr;
    }

    /* Copy code into address space (allocates pages as needed) */
    if (PPC_MemCopy(pas, addr, image, len) != noErr) {
        DisposePtr((Ptr)handle);
        return memFullErr;
    }

    /* Track segment */
    if (pas->numCodeSegs < 256) {
        void* firstPage = PPC_GetPage(pas, addr, false);  /* Already allocated */
        pas->codeSegments[pas->numCodeSegs] = firstPage ? (UInt8*)firstPage + (addr & (PPC_PAGE_SIZE - 1)) : NULL;
        pas->codeSegBases[pas->numCodeSegs] = addr;
        pas->codeSegSizes[pas->numCodeSegs] = len;
        handle->segIndex = pas->numCodeSegs;
        pas->numCodeSegs++;
    } else {
        DisposePtr((Ptr)handle);
        return memFullErr;
    }

    handle->hostMemory = PPC_GetPage(pas, addr, false);
    if (handle->hostMemory) {
        handle->hostMemory = (UInt8*)handle->hostMemory + (addr & (PPC_PAGE_SIZE - 1));
    }
    handle->cpuAddr = addr;
    handle->size = len;

    *outHandle = (CPUCodeHandle)handle;
    *outBase = addr;

    (void)flags; /* Unused for now */

    return noErr;
}

/*
 * UnmapExecutable - Unmap code segment
 */
static OSErr PPC_UnmapExecutable(CPUAddressSpace as, CPUCodeHandle handle)
{
    PPCCodeHandle* phandle = (PPCCodeHandle*)handle;

    (void)as; /* Unused */

    if (!phandle) {
        return paramErr;
    }

    /* For now, just free the handle (memory stays allocated) */
    DisposePtr((Ptr)phandle);
    return noErr;
}

/*
 * SetRegisterA5 - Set r13 (SDA base, closest equivalent to 68K A5)
 * PowerPC uses r13 for Small Data Area, similar to 68K's A5 globals
 */
static OSErr PPC_SetRegisterA5(CPUAddressSpace as, CPUAddr a5)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;

    if (!pas) {
        return paramErr;
    }

    pas->regs.gpr[13] = a5; /* r13 = SDA base */
    return noErr;
}

/*
 * SetStacks - Configure stack pointer
 * PowerPC uses r1 as stack pointer
 */
static OSErr PPC_SetStacks(CPUAddressSpace as, CPUAddr usp, CPUAddr ssp)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;

    if (!pas) {
        return paramErr;
    }

    /* PowerPC user mode only has one stack, use usp */
    pas->regs.gpr[1] = usp; /* r1 = stack pointer */

    (void)ssp; /* PowerPC doesn't have separate supervisor stack in user mode */

    return noErr;
}

/*
 * InstallTrap - Install trap handler
 */
static OSErr PPC_InstallTrap(CPUAddressSpace as, TrapNumber trapNum,
                             CPUTrapHandler handler, void* context)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;

    trapNum &= 0x00FF;

    if (!pas) {
        return paramErr;
    }

    pas->trapHandlers[trapNum] = handler;
    pas->trapContexts[trapNum] = context;

    return noErr;
}

/*
 * WriteJumpTableSlot - Patch jump table entry
 */
static OSErr PPC_WriteJumpTableSlot(CPUAddressSpace as, CPUAddr slotAddr,
                                    CPUAddr target)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    extern void PPC_Write32(PPCAddressSpace* as, UInt32 addr, UInt32 value);

    if (!pas || slotAddr >= PPC_MAX_ADDR) {
        return paramErr;
    }

    /*
     * Write PowerPC jump sequence:
     *   +0: 0x3D600000 | (target >> 16)    ; lis r11, target@h
     *   +4: 0x616B0000 | (target & 0xFFFF) ; ori r11, r11, target@l
     *   +8: 0x7D6903A6                     ; mtctr r11
     *  +12: 0x4E800420                     ; bctr
     */
    UInt32 lis_insn = 0x3D600000 | ((target >> 16) & 0xFFFF);
    UInt32 ori_insn = 0x616B0000 | (target & 0xFFFF);
    UInt32 mtctr_insn = 0x7D6903A6;
    UInt32 bctr_insn = 0x4E800420;

    PPC_Write32(pas, slotAddr + 0, lis_insn);
    PPC_Write32(pas, slotAddr + 4, ori_insn);
    PPC_Write32(pas, slotAddr + 8, mtctr_insn);
    PPC_Write32(pas, slotAddr + 12, bctr_insn);

    return noErr;
}

/*
 * MakeLazyJTStub - Create lazy-loading stub
 */
static OSErr PPC_MakeLazyJTStub(CPUAddressSpace as, CPUAddr slotAddr,
                                SInt16 segID, SInt16 entryIndex)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    extern void PPC_Write32(PPCAddressSpace* as, UInt32 addr, UInt32 value);

    if (!pas || slotAddr >= PPC_MAX_ADDR) {
        return paramErr;
    }

    /*
     * Create lazy stub that triggers _LoadSeg via system call:
     *
     *   +0: 0x38600000 | segID    ; li r3, segID
     *   +4: 0x44000002             ; sc (system call)
     *   +8: 0x4E800020             ; blr (return)
     */
    UInt32 li_insn = 0x38600000 | (segID & 0xFFFF);
    UInt32 sc_insn = 0x44000002;
    UInt32 blr_insn = 0x4E800020;

    PPC_Write32(pas, slotAddr + 0, li_insn);
    PPC_Write32(pas, slotAddr + 4, sc_insn);
    PPC_Write32(pas, slotAddr + 8, blr_insn);

    (void)entryIndex; /* Stored in trap handler context */

    return noErr;
}

/*
 * EnterAt - Begin execution
 */
static OSErr PPC_EnterAt(CPUAddressSpace as, CPUAddr entry, CPUEnterFlags flags)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    UInt32 max_instructions = 100000;  /* Safety limit */

    if (!pas) {
        return paramErr;
    }

    serial_printf("[PPC] EnterAt: entry=0x%08X flags=0x%04X\n", entry, flags);

    /* Clear halted flag */
    pas->halted = false;

    /* Execute from entry point */
    PPC_Execute(pas, entry, max_instructions);

    if (pas->halted) {
        serial_printf("[PPC] Execution halted at PC=0x%08X\n", pas->regs.pc);
    } else {
        serial_printf("[PPC] Execution completed after %u instructions\n", max_instructions);
    }

    (void)flags; /* Unused for now */

    return noErr;
}

/*
 * Relocate - Apply relocations
 */
static OSErr PPC_Relocate(CPUAddressSpace as, CPUCodeHandle code,
                          const RelocTable* relocs, CPUAddr segBase,
                          CPUAddr jtBase, CPUAddr a5Base)
{
    PPCCodeHandle* phandle = (PPCCodeHandle*)code;
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    UInt8* codeData;
    const char* kindName;

    if (!pas || !phandle || !relocs) {
        return paramErr;
    }

    codeData = (UInt8*)phandle->hostMemory;

    serial_printf("[RELOC] Applying %d relocations to PowerPC segment at 0x%08X\n",
                  relocs->count, segBase);

    /* Apply each relocation */
    for (UInt16 i = 0; i < relocs->count; i++) {
        const RelocEntry* reloc = &relocs->entries[i];
        UInt32 offset = reloc->atOffset;
        UInt32 value = 0;
        SInt32 pcrel_offset;
        UInt32 patch_pc;

        if (offset + 4 > phandle->size) {
            serial_printf("[RELOC] ERROR: offset 0x%X exceeds segment size 0x%X\n",
                         offset, phandle->size);
            return segmentRelocErr;
        }

        switch (reloc->kind) {
            case kRelocAbsSegBase:
                /* Patch absolute address with segment base */
                kindName = "ABS_SEG_BASE";
                value = segBase + reloc->addend;
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> val=0x%08X (base=0x%08X addend=%d)\n",
                             kindName, offset, value, segBase, reloc->addend);
                break;

            case kRelocA5Relative:
                /* Patch A5-relative offset (r13 SDA base on PowerPC) */
                kindName = "A5_REL";
                value = a5Base + reloc->addend;
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> val=0x%08X (A5=0x%08X addend=%d)\n",
                             kindName, offset, value, a5Base, reloc->addend);
                break;

            case kRelocJTImport:
                /* Patch jump table import */
                kindName = "JT_IMPORT";
                value = jtBase + (reloc->jtIndex * 16); /* 16 bytes per PowerPC JT entry */
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> val=0x%08X (JT[%d])\n",
                             kindName, offset, value, reloc->jtIndex);
                break;

            case kRelocPCRel16:
                /* PC-relative 16-bit branch (PowerPC B instruction with LI field) */
                kindName = "PC_REL16";
                /* PC points to current instruction */
                patch_pc = segBase + offset;
                /* Calculate target address */
                value = segBase + reloc->addend;
                /* Calculate PC-relative offset */
                pcrel_offset = (SInt32)value - (SInt32)patch_pc;
                /* Check alignment (must be multiple of 4) */
                if (pcrel_offset & 3) {
                    serial_printf("[RELOC] ERROR: PC_REL16 not 4-byte aligned: offset=%d\n", pcrel_offset);
                    return segmentRelocErr;
                }
                /* Check 16-bit signed range (divided by 4 for instruction encoding) */
                if (pcrel_offset < -32768 || pcrel_offset > 32767) {
                    serial_printf("[RELOC] ERROR: PC_REL16 out of range: offset=%d\n", pcrel_offset);
                    return segmentRelocErr;
                }
                /* Patch as big-endian 16-bit in lower halfword of instruction */
                codeData[offset + 2] = (pcrel_offset >> 8) & 0xFF;
                codeData[offset + 3] = pcrel_offset & 0xFF;
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> disp=%+d (target=0x%08X PC=0x%08X)\n",
                             kindName, offset, pcrel_offset, value, patch_pc);
                break;

            case kRelocPCRel32:
                /* PC-relative 32-bit (PowerPC branch with 24-bit LI field) */
                kindName = "PC_REL32";
                patch_pc = segBase + offset;
                value = segBase + reloc->addend;
                pcrel_offset = (SInt32)value - (SInt32)patch_pc;
                /* Check alignment */
                if (pcrel_offset & 3) {
                    serial_printf("[RELOC] ERROR: PC_REL32 not 4-byte aligned: offset=%d\n", pcrel_offset);
                    return segmentRelocErr;
                }
                /* Patch 24-bit field in instruction (shift right 2 for encoding) */
                codeData[offset + 0] = (codeData[offset + 0] & 0xFC) | ((pcrel_offset >> 24) & 0x03);
                codeData[offset + 1] = (pcrel_offset >> 16) & 0xFF;
                codeData[offset + 2] = (pcrel_offset >> 8) & 0xFF;
                codeData[offset + 3] = pcrel_offset & 0xFC; /* Clear LK and AA bits */
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> disp=%+d (target=0x%08X PC=0x%08X)\n",
                             kindName, offset, pcrel_offset, value, patch_pc);
                break;

            case kRelocSegmentRef:
                /* Reference to another segment */
                kindName = "SEG_REF";
                value = segBase + reloc->addend;
                codeData[offset + 0] = (value >> 24) & 0xFF;
                codeData[offset + 1] = (value >> 16) & 0xFF;
                codeData[offset + 2] = (value >> 8) & 0xFF;
                codeData[offset + 3] = value & 0xFF;
                serial_printf("[RELOC] apply kind=%s at off=0x%X -> val=0x%08X (seg=%d addend=%d)\n",
                             kindName, offset, value, reloc->targetSegment, reloc->addend);
                break;

            default:
                serial_printf("[RELOC] ERROR: Unknown relocation kind %d\n", reloc->kind);
                return segmentRelocErr;
        }
    }

    serial_printf("[RELOC] Successfully applied all %d relocations\n", relocs->count);
    return noErr;
}

/*
 * AllocateMemory - Allocate memory in CPU address space
 */
static OSErr PPC_AllocateMemory(CPUAddressSpace as, Size size,
                                CPUMapFlags flags, CPUAddr* outAddr)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    UInt32 addr;
    extern void PPC_Write8(PPCAddressSpace* as, UInt32 addr, UInt8 value);

    if (!pas || !outAddr) {
        return paramErr;
    }

    /* Find free space (simple bump allocator) */
    addr = 0x10000; /* Start at 64K */
    for (int i = 0; i < pas->numCodeSegs; i++) {
        UInt32 end = pas->codeSegBases[i] + pas->codeSegSizes[i];
        if (end > addr) {
            addr = end;
        }
    }

    /* Align to 16-byte boundary */
    addr = (addr + 15) & ~15;

    /* Check bounds */
    if (addr + size > PPC_MAX_ADDR) {
        return memFullErr;
    }

    /* Zero memory */
    for (Size i = 0; i < size; i++) {
        PPC_Write8(pas, addr + i, 0);
    }

    *outAddr = addr;

    (void)flags; /* Unused for now */

    return noErr;
}

/*
 * WriteMemory - Write to CPU address space
 */
static OSErr PPC_WriteMemory(CPUAddressSpace as, CPUAddr addr,
                             const void* data, Size len)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;

    if (!pas || !data || addr + len > PPC_MAX_ADDR) {
        return paramErr;
    }

    return PPC_MemCopy(pas, addr, data, len);
}

/*
 * ReadMemory - Read from CPU address space
 */
static OSErr PPC_ReadMemory(CPUAddressSpace as, CPUAddr addr,
                            void* data, Size len)
{
    PPCAddressSpace* pas = (PPCAddressSpace*)as;
    extern UInt8 PPC_Read8(PPCAddressSpace* as, UInt32 addr);

    if (!pas || !data || addr + len > PPC_MAX_ADDR) {
        return paramErr;
    }

    for (Size i = 0; i < len; i++) {
        ((UInt8*)data)[i] = PPC_Read8(pas, addr + i);
    }

    return noErr;
}

/*
 * Opcode Handler Forward Declarations (from PPCOpcodes.c)
 */
extern UInt32 PPC_Fetch32(PPCAddressSpace* as);

/*
 * PPC_Step - Fetch and execute one instruction
 */
OSErr PPC_Step(PPCAddressSpace* as)
{
    UInt32 insn;
    UInt8 primary;
    UInt16 extended;

    if (!as) {
        return paramErr;
    }

    if (as->halted) {
        return noErr;
    }

    /* Fetch instruction (32-bit) */
    insn = PPC_Fetch32(as);

    /* Extract primary opcode (bits 0-5) */
    primary = PPC_PRIMARY_OPCODE(insn);

    /* Decode and dispatch */
    switch (primary) {
        case PPC_OP_MULLI:       /* 7 */
            PPC_Op_MULLI(as, insn);
            break;

        case PPC_OP_CMPLI:       /* 10 */
            PPC_Op_CMPLI(as, insn);
            break;

        case PPC_OP_CMPI:        /* 11 */
            PPC_Op_CMPI(as, insn);
            break;

        case PPC_OP_ADDI:        /* 14 */
            PPC_Op_ADDI(as, insn);
            break;

        case PPC_OP_ADDIS:       /* 15 */
            PPC_Op_ADDIS(as, insn);
            break;

        case PPC_OP_BC:          /* 16 */
            PPC_Op_BC(as, insn);
            break;

        case PPC_OP_SC:          /* 17 */
            PPC_Op_SC(as, insn);
            break;

        case PPC_OP_B:           /* 18 */
            PPC_Op_B(as, insn);
            break;

        case PPC_OP_EXT19:       /* 19 - Extended opcodes (branches to LR/CTR) */
            extended = PPC_EXTENDED_OPCODE(insn);
            switch (extended) {
                case PPC_XOP19_BCLR:
                    PPC_Op_BCLR(as, insn);
                    break;

                case PPC_XOP19_BCCTR:
                    PPC_Op_BCCTR(as, insn);
                    break;

                default:
                    PPC_Fault(as, "Unimplemented opcode 19 extended");
                    break;
            }
            break;

        case PPC_OP_ORI:         /* 24 */
            PPC_Op_ORI(as, insn);
            break;

        case PPC_OP_ORIS:        /* 25 */
            PPC_Op_ORIS(as, insn);
            break;

        case PPC_OP_XORI:        /* 26 */
            PPC_Op_XORI(as, insn);
            break;

        case PPC_OP_XORIS:       /* 27 */
            PPC_Op_XORIS(as, insn);
            break;

        case PPC_OP_ANDI_RC:     /* 28 */
            PPC_Op_ANDI_RC(as, insn);
            break;

        case PPC_OP_ANDIS_RC:    /* 29 */
            PPC_Op_ANDIS_RC(as, insn);
            break;

        case PPC_OP_EXT31:       /* 31 - Extended opcodes (arithmetic, logical, load/store) */
            extended = PPC_EXTENDED_OPCODE(insn);
            switch (extended) {
                case PPC_XOP_CMP:
                    PPC_Op_CMP(as, insn);
                    break;

                case PPC_XOP_CMPL:
                    PPC_Op_CMPL(as, insn);
                    break;

                case PPC_XOP_SUBF:
                    PPC_Op_SUBF(as, insn);
                    break;

                case PPC_XOP_AND:
                    PPC_Op_AND(as, insn);
                    break;

                case PPC_XOP_MULLW:
                    PPC_Op_MULLW(as, insn);
                    break;

                case PPC_XOP_ADD:
                    PPC_Op_ADD(as, insn);
                    break;

                case PPC_XOP_XOR:
                    PPC_Op_XOR(as, insn);
                    break;

                case PPC_XOP_OR:
                    PPC_Op_OR(as, insn);
                    break;

                case PPC_XOP_DIVW:
                    PPC_Op_DIVW(as, insn);
                    break;

                default:
                    PPC_Fault(as, "Unimplemented opcode 31 extended");
                    break;
            }
            break;

        case PPC_OP_LWZ:         /* 32 */
            PPC_Op_LWZ(as, insn);
            break;

        case PPC_OP_LBZ:         /* 34 */
            PPC_Op_LBZ(as, insn);
            break;

        case PPC_OP_STW:         /* 36 */
            PPC_Op_STW(as, insn);
            break;

        case PPC_OP_STB:         /* 38 */
            PPC_Op_STB(as, insn);
            break;

        case PPC_OP_LHZ:         /* 40 */
            PPC_Op_LHZ(as, insn);
            break;

        case PPC_OP_STH:         /* 44 */
            PPC_Op_STH(as, insn);
            break;

        default:
            serial_printf("[PPC] ILLEGAL opcode 0x%08X (primary=0x%02X) at PC=0x%08X\n",
                         insn, primary, as->regs.pc - 4);
            PPC_Fault(as, "Illegal opcode");
            break;
    }

    return noErr;
}

/*
 * PPC_Execute - Execute up to maxInstructions
 */
OSErr PPC_Execute(PPCAddressSpace* as, UInt32 startPC, UInt32 maxInstructions)
{
    UInt32 count = 0;

    if (!as) {
        return paramErr;
    }

    as->regs.pc = startPC;
    as->halted = false;

    while (count < maxInstructions && !as->halted) {
        PPC_Step(as);
        count++;
    }

    return noErr;
}
