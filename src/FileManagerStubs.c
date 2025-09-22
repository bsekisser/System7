/*
 * FileManagerStubs.c - Stub implementations for File Manager dependencies
 *
 * This file provides minimal stub implementations for File Manager
 * functions and types that are not yet fully implemented.
 */

#include "SystemTypes.h"
#include "FileManager.h"
#include "FileManager_Internal.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Global variables
 * ============================================================================ */

/* Global state and hooks are now defined in FileManager.c */
extern FSGlobals g_FSGlobals;
extern PlatformHooks g_PlatformHooks;

/* ============================================================================
 * Stub Implementations
 * ============================================================================ */

/* Volume Management */
VCB* VCB_Alloc(void) {
    serial_printf("VCB_Alloc stub\n");
    return (VCB*)calloc(1, sizeof(VCB));
}

void VCB_Free(VCB* vcb) {
    serial_printf("VCB_Free stub\n");
    if (vcb) free(vcb);
}

VCB* VCB_Find(VolumeRefNum vRefNum) {
    serial_printf("VCB_Find stub: vRefNum=%d\n", vRefNum);
    VCB* vcb = g_FSGlobals.vcbQueue;
    while (vcb) {
        if (vcb->base.vcbVRefNum == vRefNum) {
            return vcb;
        }
        vcb = vcb->vcbNext;
    }
    return NULL;
}

VCB* VCB_FindByName(const UInt8* name) {
    serial_printf("VCB_FindByName stub\n");
    return NULL;
}

OSErr VCB_Mount(UInt16 drvNum, VCB** newVCB) {
    serial_printf("VCB_Mount stub: drvNum=%d\n", drvNum);
    return nsvErr;
}

OSErr VCB_Unmount(VCB* vcb) {
    serial_printf("VCB_Unmount stub\n");
    return noErr;
}

OSErr VCB_Flush(VCB* vcb) {
    serial_printf("VCB_Flush stub\n");
    return noErr;
}

OSErr VCB_Update(VCB* vcb) {
    serial_printf("VCB_Update stub\n");
    return noErr;
}

/* File Control Block Management */
FCB* FCB_Alloc(void) {
    serial_printf("FCB_Alloc stub\n");
    if (g_FSGlobals.fcbFree < g_FSGlobals.fcbCount) {
        FCB* fcb = &g_FSGlobals.fcbArray[g_FSGlobals.fcbFree];
        g_FSGlobals.fcbFree = fcb->fcbRefNum;
        fcb->fcbRefNum = g_FSGlobals.fcbFree + 1;
        return fcb;
    }
    return NULL;
}

void FCB_Free(FCB* fcb) {
    serial_printf("FCB_Free stub\n");
    if (fcb) {
        memset(fcb, 0, sizeof(FCB));
    }
}

FCB* FCB_Find(FileRefNum refNum) {
    serial_printf("FCB_Find stub: refNum=%d\n", refNum);
    if (refNum > 0 && refNum <= g_FSGlobals.fcbCount) {
        FCB* fcb = &g_FSGlobals.fcbArray[refNum - 1];
        if (fcb->base.fcbFlNm != 0) {
            return fcb;
        }
    }
    return NULL;
}

FCB* FCB_FindByID(VCB* vcb, UInt32 fileID) {
    serial_printf("FCB_FindByID stub: fileID=%d\n", fileID);
    return NULL;
}

OSErr FCB_Open(VCB* vcb, UInt32 dirID, const UInt8* name, UInt8 permission, FCB** newFCB) {
    serial_printf("FCB_Open stub\n");
    return fnfErr;
}

OSErr FCB_Close(FCB* fcb) {
    serial_printf("FCB_Close stub\n");
    return noErr;
}

OSErr FCB_Flush(FCB* fcb) {
    serial_printf("FCB_Flush stub\n");
    return noErr;
}

/* Working Directory Management */
WDCB* WDCB_Alloc(void) {
    serial_printf("WDCB_Alloc stub\n");
    return (WDCB*)calloc(1, sizeof(WDCB));
}

void WDCB_Free(WDCB* wdcb) {
    serial_printf("WDCB_Free stub\n");
    if (wdcb) free(wdcb);
}

WDCB* WDCB_Find(WDRefNum wdRefNum) {
    serial_printf("WDCB_Find stub: wdRefNum=%d\n", wdRefNum);
    return NULL;
}

OSErr WDCB_Create(VCB* vcb, UInt32 dirID, UInt32 procID, WDCB** newWDCB) {
    serial_printf("WDCB_Create stub\n");
    return tmwdoErr;
}

/* B-tree Operations */
OSErr BTree_Open(VCB* vcb, UInt32 fileID, BTCB** btcb) {
    serial_printf("BTree_Open stub: fileID=%d\n", fileID);
    return btNoErr;
}

OSErr BTree_Close(BTCB* btcb) {
    serial_printf("BTree_Close stub\n");
    return btNoErr;
}

OSErr BTree_Search(BTCB* btcb, const void* key, void* record, UInt16* recordSize, UInt32* hint) {
    serial_printf("BTree_Search stub\n");
    return btRecNotFnd;
}

OSErr BTree_Insert(BTCB* btcb, const void* key, const void* record, UInt16 recordSize) {
    serial_printf("BTree_Insert stub\n");
    return btNoErr;
}

OSErr BTree_Delete(BTCB* btcb, const void* key) {
    serial_printf("BTree_Delete stub\n");
    return btNoErr;
}

OSErr BTree_GetNode(BTCB* btcb, UInt32 nodeNum, void** nodePtr) {
    serial_printf("BTree_GetNode stub: nodeNum=%d\n", nodeNum);
    return btNoErr;
}

OSErr BTree_ReleaseNode(BTCB* btcb, UInt32 nodeNum) {
    serial_printf("BTree_ReleaseNode stub: nodeNum=%d\n", nodeNum);
    return btNoErr;
}

OSErr BTree_FlushNode(BTCB* btcb, UInt32 nodeNum) {
    serial_printf("BTree_FlushNode stub: nodeNum=%d\n", nodeNum);
    return btNoErr;
}

/* Catalog Operations */
OSErr Cat_Open(VCB* vcb) {
    serial_printf("Cat_Open stub\n");
    return noErr;
}

OSErr Cat_Close(VCB* vcb) {
    serial_printf("Cat_Close stub\n");
    return noErr;
}

OSErr Cat_Lookup(VCB* vcb, UInt32 dirID, const UInt8* name, void* catData, UInt32* hint) {
    serial_printf("Cat_Lookup stub: dirID=%d\n", dirID);
    return fnfErr;
}

OSErr Cat_Create(VCB* vcb, UInt32 dirID, const UInt8* name, UInt8 type, void* catData) {
    serial_printf("Cat_Create stub\n");
    return noErr;
}

OSErr Cat_Delete(VCB* vcb, UInt32 dirID, const UInt8* name) {
    serial_printf("Cat_Delete stub\n");
    return noErr;
}

OSErr Cat_Rename(VCB* vcb, UInt32 dirID, const UInt8* oldName, const UInt8* newName) {
    serial_printf("Cat_Rename stub\n");
    return noErr;
}

OSErr Cat_Move(VCB* vcb, UInt32 srcDirID, const UInt8* name, UInt32 dstDirID) {
    serial_printf("Cat_Move stub\n");
    return noErr;
}

OSErr Cat_GetInfo(VCB* vcb, UInt32 dirID, const UInt8* name, CInfoPBRec* pb) {
    serial_printf("Cat_GetInfo stub\n");
    return fnfErr;
}

OSErr Cat_SetInfo(VCB* vcb, UInt32 dirID, const UInt8* name, const CInfoPBRec* pb) {
    serial_printf("Cat_SetInfo stub\n");
    return noErr;
}

CNodeID Cat_GetNextID(VCB* vcb) {
    serial_printf("Cat_GetNextID stub\n");
    static CNodeID nextID = 100;
    return nextID++;
}

/* Extent Management */
OSErr Ext_Open(VCB* vcb) {
    serial_printf("Ext_Open stub\n");
    return noErr;
}

OSErr Ext_Close(VCB* vcb) {
    serial_printf("Ext_Close stub\n");
    return noErr;
}

OSErr Ext_Allocate(VCB* vcb, UInt32 fileID, UInt8 forkType, UInt32 blocks, ExtDataRec extents) {
    serial_printf("Ext_Allocate stub: fileID=%d, blocks=%d\n", fileID, blocks);
    return noErr;
}

OSErr Ext_Deallocate(VCB* vcb, UInt32 fileID, UInt8 forkType, UInt32 startBlock) {
    serial_printf("Ext_Deallocate stub\n");
    return noErr;
}

OSErr Ext_Map(VCB* vcb, FCB* fcb, UInt32 fileBlock, UInt32* physBlock, UInt32* contiguous) {
    serial_printf("Ext_Map stub: fileBlock=%d\n", fileBlock);
    return noErr;
}

OSErr Ext_Extend(VCB* vcb, FCB* fcb, UInt32 newSize) {
    serial_printf("Ext_Extend stub: newSize=%d\n", newSize);
    return noErr;
}

OSErr Ext_Truncate(VCB* vcb, FCB* fcb, UInt32 newSize) {
    serial_printf("Ext_Truncate stub: newSize=%d\n", newSize);
    return noErr;
}

/* Allocation Bitmap Management */
OSErr Alloc_Init(VCB* vcb) {
    serial_printf("Alloc_Init stub\n");
    return noErr;
}

OSErr Alloc_Close(VCB* vcb) {
    serial_printf("Alloc_Close stub\n");
    return noErr;
}

OSErr Alloc_Blocks(VCB* vcb, UInt32 startHint, UInt32 minBlocks, UInt32 maxBlocks,
                   UInt32* actualStart, UInt32* actualCount) {
    serial_printf("Alloc_Blocks stub: minBlocks=%d, maxBlocks=%d\n", minBlocks, maxBlocks);
    return dskFulErr;
}

OSErr Alloc_Free(VCB* vcb, UInt32 startBlock, UInt32 blockCount) {
    serial_printf("Alloc_Free stub: startBlock=%d, count=%d\n", startBlock, blockCount);
    return noErr;
}

UInt32 Alloc_CountFree(VCB* vcb) {
    serial_printf("Alloc_CountFree stub\n");
    return 0;
}

Boolean Alloc_Check(VCB* vcb, UInt32 startBlock, UInt32 blockCount) {
    serial_printf("Alloc_Check stub\n");
    return false;
}

/* Cache Management */
OSErr Cache_Init(UInt32 cacheSize) {
    serial_printf("Cache_Init stub: size=%d\n", cacheSize);
    return noErr;
}

void Cache_Shutdown(void) {
    serial_printf("Cache_Shutdown stub\n");
}

OSErr Cache_GetBlock(VCB* vcb, UInt32 blockNum, CacheBuffer** buffer) {
    serial_printf("Cache_GetBlock stub: blockNum=%d\n", blockNum);
    return ioErr;
}

OSErr Cache_ReleaseBlock(CacheBuffer* buffer, Boolean dirty) {
    serial_printf("Cache_ReleaseBlock stub\n");
    return noErr;
}

OSErr Cache_FlushVolume(VCB* vcb) {
    serial_printf("Cache_FlushVolume stub\n");
    return noErr;
}

OSErr Cache_FlushAll(void) {
    serial_printf("Cache_FlushAll stub\n");
    return noErr;
}

void Cache_Invalidate(VCB* vcb) {
    serial_printf("Cache_Invalidate stub\n");
}

/* I/O Operations */
OSErr IO_ReadBlocks(VCB* vcb, UInt32 startBlock, UInt32 blockCount, void* buffer) {
    serial_printf("IO_ReadBlocks stub: start=%d, count=%d\n", startBlock, blockCount);
    return ioErr;
}

OSErr IO_WriteBlocks(VCB* vcb, UInt32 startBlock, UInt32 blockCount, const void* buffer) {
    serial_printf("IO_WriteBlocks stub: start=%d, count=%d\n", startBlock, blockCount);
    return ioErr;
}

OSErr IO_Format(UInt16 drvNum, const UInt8* volName, UInt32 volSize) {
    serial_printf("IO_Format stub: drvNum=%d\n", drvNum);
    return noErr;
}

OSErr IO_ReadFork(FCB* fcb, UInt32 offset, UInt32 count, void* buffer, UInt32* actual) {
    serial_printf("IO_ReadFork stub: offset=%d, count=%d\n", offset, count);
    if (actual) *actual = 0;
    return ioErr;
}

OSErr IO_WriteFork(FCB* fcb, UInt32 offset, UInt32 count, const void* buffer, UInt32* actual) {
    serial_printf("IO_WriteFork stub: offset=%d, count=%d\n", offset, count);
    if (actual) *actual = 0;
    return ioErr;
}

/* All locking functions are now defined in FileManager.c */

/* POSIX stubs for bare metal kernel */
int sched_yield(void) {
    /* In a bare metal kernel, just return success */
    return 0;
}

time_t time(time_t* t) {
    /* Return a fake timestamp */
    time_t fake_time = 0x60000000;  /* Some arbitrary value */
    if (t) *t = fake_time;
    return fake_time;
}

/* Utility Functions */
OSErr FS_CompareNames(const UInt8* name1, const UInt8* name2, Boolean* equal) {
    serial_printf("FS_CompareNames stub\n");
    *equal = false;
    return noErr;
}

OSErr FS_CopyName(const UInt8* src, UInt8* dst, UInt8 maxLen) {
    serial_printf("FS_CopyName stub\n");
    if (src && dst) {
        UInt8 len = src[0];
        if (len > maxLen) len = maxLen;
        memcpy(dst, src, len + 1);
    }
    return noErr;
}

UInt32 FS_GetTime(void) {
    serial_printf("FS_GetTime stub\n");
    return 0;
}

OSErr FS_ValidateName(const UInt8* name) {
    serial_printf("FS_ValidateName stub\n");
    if (!name || name[0] == 0 || name[0] > 31) {
        return bdNamErr;
    }
    return noErr;
}