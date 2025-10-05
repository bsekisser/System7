/*
 * SegmentLoader.c - Portable 68K Segment Loader Core
 *
 * ISA-agnostic segment loading logic for classic Mac OS applications.
 */

#include "SegmentLoader/SegmentLoader.h"
#include "SegmentLoader/CodeParser.h"
#include "SegmentLoader/SegmentLoaderLogging.h"
#include "ResourceMgr/resource_manager.h"
#include "MemoryMgr/MemoryManager.h"
#include "System71StdLib.h"
#include <string.h>

/* --- TEST HOOK: allow loader to fetch from the test store in SEGLOADER_TEST_BOOT --- */
#ifndef SL_GETRESOURCE
# ifdef SEGLOADER_TEST_BOOT
   /* Implemented in SegmentLoaderTest.c — returns Handle from the in-memory test store */
   extern Handle TestResource_Get(ResType theType, ResID theID);
#  define SL_GETRESOURCE(type, id) TestResource_Get((type),(id))
# else
#  define SL_GETRESOURCE(type, id) GetResource((type),(id))
# endif
#endif

/* Forward declarations */
static OSErr LoadCODE0AndSetupA5(SegmentLoaderContext* ctx);
static OSErr LoadCODE1(SegmentLoaderContext* ctx);

/*
 * SegmentLoader_Initialize - Initialize segment loader for process
 */
OSErr SegmentLoader_Initialize(ProcessControlBlock* pcb,
                               const char* cpuBackendName,
                               SegmentLoaderContext** outCtx)
{
    OSErr err;
    SegmentLoaderContext* ctx;
    const ICPUBackend* backend;

    if (!pcb || !outCtx) {
        return paramErr;
    }

    /* Get CPU backend */
    if (cpuBackendName) {
        backend = CPUBackend_Get(cpuBackendName);
    } else {
        backend = CPUBackend_GetDefault();
    }

    if (!backend) {
        return segmentLoaderErr;
    }

    /* Allocate context */
    ctx = (SegmentLoaderContext*)NewPtr(sizeof(SegmentLoaderContext));
    if (!ctx) {
        return memFullErr;
    }

    memset(ctx, 0, sizeof(SegmentLoaderContext));

    /* Initialize context */
    ctx->pcb = pcb;
    ctx->cpuBackend = backend;
    ctx->resFileRefNum = -1;

    /* Create CPU address space */
    err = backend->CreateAddressSpace(pcb, &ctx->cpuAS);
    if (err != noErr) {
        DisposePtr((Ptr)ctx);
        return err;
    }

    ctx->initialized = true;
    ctx->launchTime = TickCount();

    *outCtx = ctx;
    return noErr;
}

/*
 * SegmentLoader_Cleanup - Clean up segment loader
 */
OSErr SegmentLoader_Cleanup(SegmentLoaderContext* ctx)
{
    if (!ctx) {
        return paramErr;
    }

    /* Unload all segments */
    for (UInt16 i = 0; i < ctx->numSegments; i++) {
        if (ctx->segments[i].handle) {
            ctx->cpuBackend->UnmapExecutable(ctx->cpuAS,
                                            ctx->segments[i].handle);
        }
    }

    /* Destroy CPU address space */
    if (ctx->cpuAS) {
        ctx->cpuBackend->DestroyAddressSpace(ctx->cpuAS);
    }

    /* Close resource file */
    if (ctx->resFileRefNum >= 0) {
        CloseResFile(ctx->resFileRefNum);
    }

    DisposePtr((Ptr)ctx);
    return noErr;
}

/*
 * EnsureEntrySegmentsLoaded - Load CODE 0 and CODE 1
 */
OSErr EnsureEntrySegmentsLoaded(SegmentLoaderContext* ctx)
{
    OSErr err;

    if (!ctx || !ctx->initialized) {
        return paramErr;
    }

    /* Load CODE 0 and set up A5 world */
    err = LoadCODE0AndSetupA5(ctx);
    if (err != noErr) {
        return err;
    }

    /* Load CODE 1 (main entry segment) */
    err = LoadCODE1(ctx);
    if (err != noErr) {
        return err;
    }

    return noErr;
}

/*
 * LoadCODE0AndSetupA5 - Load CODE 0 and construct A5 world
 */
static OSErr LoadCODE0AndSetupA5(SegmentLoaderContext* ctx)
{
    Handle code0Handle;
    OSErr err;
    CODE0Info info;

    if (!ctx) {
        return paramErr;
    }

    /* Load CODE 0 resource */
    code0Handle = SL_GETRESOURCE('CODE', 0);
    if (!code0Handle) {
        SEG_LOG_ERROR("CODE 0 resource not found");
        return segmentNotFound;
    }
    SEG_LOG_INFO("CODE 0 resource loaded: handle=%p", code0Handle);

    /* Lock handle */
    HLock(code0Handle);

    /* Parse CODE 0 */
    err = ParseCODE0(*code0Handle, GetHandleSize(code0Handle), &info);
    if (err != noErr) {
        HUnlock(code0Handle);
        ReleaseResource(code0Handle);
        return err;
    }

    /* Defensive clamp: guard against bogus CODE 0 sizes */
    const UInt32 MAX_A5 = 1 * 1024 * 1024; /* 1MB guard */
    if (info.a5BelowSize > MAX_A5 || info.a5AboveSize > MAX_A5) {
        SEG_LOG_ERROR("CODE 0 sizes too large: below=%u above=%u",
                      info.a5BelowSize, info.a5AboveSize);
        HUnlock(code0Handle);
        ReleaseResource(code0Handle);
        return memFullErr;
    }

    /* Store CODE 0 info */
    ctx->code0Info = info;

    /* Set up A5 world */
    err = InstallA5World(ctx, &info);
    if (err != noErr) {
        HUnlock(code0Handle);
        ReleaseResource(code0Handle);
        return err;
    }

    /* Build jump table with lazy stubs */
    err = BuildJumpTable(ctx);
    if (err != noErr) {
        HUnlock(code0Handle);
        ReleaseResource(code0Handle);
        return err;
    }

    /* Clean up CODE 0 (not needed after A5 world is built) */
    HUnlock(code0Handle);
    ReleaseResource(code0Handle);

    return noErr;
}

/*
 * LoadCODE1 - Load CODE 1 (main entry segment)
 */
static OSErr LoadCODE1(SegmentLoaderContext* ctx)
{
    return LoadSegment(ctx, 1);
}

/*
 * LoadSegment - Load CODE segment on demand
 */
OSErr LoadSegment(SegmentLoaderContext* ctx, SInt16 segID)
{
    Handle codeHandle;
    OSErr err;
    CODEInfo info;
    CPUCodeHandle cpuHandle;
    CPUAddr baseAddr, entryAddr;
    const UInt8* codeData;
    Size codeSize;

    if (!ctx || !ctx->initialized) {
        return paramErr;
    }

    if (segID < 0 || segID >= kMaxSegments) {
        return paramErr;
    }

    /* Check if already loaded */
    if (segID < ctx->numSegments && ctx->segments[segID].state == kSegmentLoaded) {
        SEG_LOG_DEBUG("Segment %d already loaded, skipping", segID);
        return noErr; /* Already loaded */
    }

    SEG_LOG_INFO("Loading CODE %d...", segID);

    /* Load CODE resource */
    codeHandle = SL_GETRESOURCE('CODE', segID);
    if (!codeHandle) {
        SEG_LOG_ERROR("CODE %d resource not found", segID);
        return segmentNotFound;
    }
    SEG_LOG_INFO("CODE %d resource loaded: handle=%p size=%u", segID, codeHandle,
                 codeHandle ? GetHandleSize(codeHandle) : 0);

    HLock(codeHandle);
    codeData = (const UInt8*)*codeHandle;
    codeSize = GetHandleSize(codeHandle);

    /* Parse CODE N */
    err = ParseCODEN(codeData, codeSize, segID, &info);
    if (err != noErr) {
        HUnlock(codeHandle);
        ReleaseResource(codeHandle);
        return err;
    }

    /* Skip header and prologue */
    const UInt8* executableCode = codeData + CODEN_HEADER_SIZE + info.prologueSkip;
    Size executableSize = codeSize - CODEN_HEADER_SIZE - info.prologueSkip;

    /* Map into CPU address space */
    err = ctx->cpuBackend->MapExecutable(ctx->cpuAS, executableCode,
                                        executableSize,
                                        kCPUMapExecutable | kCPUMapLocked,
                                        &cpuHandle, &baseAddr);
    if (err != noErr) {
        HUnlock(codeHandle);
        ReleaseResource(codeHandle);
        return err;
    }

    /* Build relocation table */
    err = BuildRelocationTable(executableCode, executableSize, segID,
                               &info.relocTable);
    if (err != noErr) {
        ctx->cpuBackend->UnmapExecutable(ctx->cpuAS, cpuHandle);
        HUnlock(codeHandle);
        ReleaseResource(codeHandle);
        return err;
    }

    /* Apply relocations */
    err = ctx->cpuBackend->Relocate(ctx->cpuAS, cpuHandle, &info.relocTable,
                                   baseAddr, ctx->a5World.jtBase,
                                   ctx->a5World.a5Base);
    if (err != noErr) {
        FreeRelocationTable(&info.relocTable);
        ctx->cpuBackend->UnmapExecutable(ctx->cpuAS, cpuHandle);
        HUnlock(codeHandle);
        ReleaseResource(codeHandle);
        return err;
    }

    /* Calculate entry point */
    entryAddr = baseAddr + info.entryOffset;

    /* Store segment descriptor */
    if (segID >= ctx->numSegments) {
        ctx->numSegments = segID + 1;
    }

    ctx->segments[segID].handle = cpuHandle;
    ctx->segments[segID].baseAddr = baseAddr;
    ctx->segments[segID].entryAddr = entryAddr;
    ctx->segments[segID].size = executableSize;
    ctx->segments[segID].state = kSegmentLoaded;
    ctx->segments[segID].purgeable = false;
    ctx->segments[segID].segID = segID;
    ctx->segments[segID].refCount = 1;

    SEG_LOG_INFO("CODE %d loaded successfully:", segID);
    SEG_LOG_INFO("  baseAddr  = 0x%08X", baseAddr);
    SEG_LOG_INFO("  entryAddr = 0x%08X (base + 0x%X)", entryAddr, info.entryOffset);
    SEG_LOG_INFO("  size      = 0x%X bytes", executableSize);

    /* Clean up */
    FreeRelocationTable(&info.relocTable);
    HUnlock(codeHandle);
    ReleaseResource(codeHandle);

    return noErr;
}

/*
 * UnloadSegment - Unload segment (mark purgeable)
 */
OSErr UnloadSegment(SegmentLoaderContext* ctx, SInt16 segID)
{
    if (!ctx || segID < 0 || segID >= ctx->numSegments) {
        return paramErr;
    }

    CodeSegment* seg = &ctx->segments[segID];

    if (seg->state != kSegmentLoaded) {
        return noErr; /* Not loaded */
    }

    /* Decrement ref count */
    if (seg->refCount > 0) {
        seg->refCount--;
    }

    /* If ref count is zero, mark purgeable */
    if (seg->refCount == 0) {
        seg->state = kSegmentPurgeable;
        seg->purgeable = true;

        /* Could unmap here, but for MVP we keep it loaded */
    }

    return noErr;
}

/*
 * ResolveJumpIndex - Resolve jump table index to address
 */
OSErr ResolveJumpIndex(SegmentLoaderContext* ctx, SInt16 jtIndex,
                      CPUAddr* outAddr)
{
    CPUAddr slotAddr;
    UInt8 slotData[8];
    OSErr err;

    if (!ctx || !outAddr || !ctx->a5World.initialized) {
        return paramErr;
    }

    if (jtIndex < 0 || jtIndex >= ctx->a5World.jtCount) {
        return segmentJTErr;
    }

    /* Calculate slot address */
    slotAddr = ctx->a5World.jtBase + (jtIndex * ctx->a5World.jtEntrySize);

    /* Read slot */
    err = ctx->cpuBackend->ReadMemory(ctx->cpuAS, slotAddr, slotData, 8);
    if (err != noErr) {
        return err;
    }

    /* Parse jump instruction */
    UInt16 opcode = BE_Read16(slotData + 0);
    if (opcode == 0x4EF9) {
        /* JMP absolute.L - target is already resolved */
        *outAddr = BE_Read32(slotData + 2);
    } else if (opcode == 0x3F3C) {
        /* Lazy stub - need to load segment first */
        UInt16 segID = BE_Read16(slotData + 2);
        err = LoadSegment(ctx, segID);
        if (err != noErr) {
            return err;
        }

        /* Re-read slot after loading */
        err = ctx->cpuBackend->ReadMemory(ctx->cpuAS, slotAddr, slotData, 8);
        if (err != noErr) {
            return err;
        }

        *outAddr = BE_Read32(slotData + 2);
    } else {
        return segmentJTErr;
    }

    return noErr;
}

/*
 * InstallLoadSegTrap - Install _LoadSeg trap handler
 */
OSErr InstallLoadSegTrap(SegmentLoaderContext* ctx)
{
    /* _LoadSeg is trap 0xA9F0 */
    /* TODO: Implement trap handler */

    (void)ctx; /* Unused for now */

    return noErr;
}

/*
 * GetSegmentEntryPoint - Get entry point for segment
 */
OSErr GetSegmentEntryPoint(SegmentLoaderContext* ctx, SInt16 segID,
                          CPUAddr* outEntry)
{
    if (!ctx || !outEntry) {
        return paramErr;
    }

    if (segID < 0 || segID >= ctx->numSegments) {
        return paramErr;
    }

    if (ctx->segments[segID].state != kSegmentLoaded) {
        return segmentNotFound;
    }

    *outEntry = ctx->segments[segID].entryAddr;
    return noErr;
}
