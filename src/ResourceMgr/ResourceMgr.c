/*
 * ResourceMgr.c - Resource Manager implementation (read-only)
 * Based on Inside Macintosh: More Macintosh Toolbox
 * Clean-room implementation
 */

#include "SystemTypes.h"
#include "ResourceMgr/ResourceMgr.h"
#include "ResourceMgr/ResourceMgrPriv.h"
#include "System71StdLib.h"

/* External functions we need */
extern Handle NewHandle(UInt32 byteCount);
extern void DisposeHandle(Handle h);
extern void HLock(Handle h);
extern void HUnlock(Handle h);
extern void BlockMove(const void* srcPtr, void* destPtr, Size byteCount);
extern void serial_puts(const char* s);

/* Global state */
static ResourceMgrGlobals gResMgr = {
    .curResFile = -1,
    .resError = noErr,
    .resLoad = true,
    .nextUniqueID = 128
};

/* Byte-swapping utilities for big-endian resource data */
UInt16 read_be16(const UInt8* p) {
    return (p[0] << 8) | p[1];
}

UInt32 read_be32(const UInt8* p) {
    return ((UInt32)p[0] << 24) | ((UInt32)p[1] << 16) |
           ((UInt32)p[2] << 8) | p[3];
}

void write_be16(UInt8* p, UInt16 val) {
    p[0] = (val >> 8) & 0xFF;
    p[1] = val & 0xFF;
}

void write_be32(UInt8* p, UInt32 val) {
    p[0] = (val >> 24) & 0xFF;
    p[1] = (val >> 16) & 0xFF;
    p[2] = (val >> 8) & 0xFF;
    p[3] = val & 0xFF;
}

/* Initialize Resource Manager */
void InitResourceManager(void) {
    int i;

    serial_puts("[ResourceMgr] Initializing Resource Manager\n");

    /* Clear all resource file slots */
    for (i = 0; i < MAX_RES_FILES; i++) {
        gResMgr.resFiles[i].inUse = false;
        gResMgr.resFiles[i].refNum = -1;
    }

    /* Open system resource file (simplified - just use slot 0) */
    /* In a real implementation, this would open System.rsrc */
    gResMgr.resFiles[0].inUse = true;
    gResMgr.resFiles[0].refNum = 0;
    gResMgr.curResFile = 0;

    /* For now, we'll use embedded resource data */
    extern const unsigned char patterns_rsrc_data[];
    extern const unsigned int patterns_rsrc_size;

    gResMgr.resFiles[0].data = (UInt8*)patterns_rsrc_data;
    gResMgr.resFiles[0].dataSize = patterns_rsrc_size;

    /* Parse resource header and map */
    if (gResMgr.resFiles[0].dataSize >= sizeof(ResourceHeader)) {
        ResourceHeader* hdr = (ResourceHeader*)gResMgr.resFiles[0].data;
        UInt32 mapOffset = read_be32((UInt8*)&hdr->mapOffset);

        if (mapOffset + sizeof(ResMapHeader) <= gResMgr.resFiles[0].dataSize) {
            gResMgr.resFiles[0].map = (ResMapHeader*)(gResMgr.resFiles[0].data + mapOffset);
            serial_puts("[ResourceMgr] Resource map loaded successfully\n");
        } else {
            serial_puts("[ResourceMgr] Warning: Invalid resource map offset\n");
        }
    }

    serial_puts("[ResourceMgr] Resource Manager initialized\n");
    gResMgr.resError = noErr;
}

/* Shutdown Resource Manager */
void ShutdownResourceManager(void) {
    int i;

    for (i = 0; i < MAX_RES_FILES; i++) {
        if (gResMgr.resFiles[i].inUse && i != 0) {  /* Don't close system file */
            ResFile_Close(i);
        }
    }
}

/* Get last error */
OSErr ResError(void) {
    OSErr err = gResMgr.resError;
    gResMgr.resError = noErr;  /* Clear error after reading */
    return err;
}

/* Get current resource file */
SInt16 CurResFile(void) {
    return gResMgr.curResFile;
}

/* Set current resource file */
void UseResFile(SInt16 refNum) {
    if (refNum < 0 || refNum >= MAX_RES_FILES || !gResMgr.resFiles[refNum].inUse) {
        gResMgr.resError = badRefNum;
        return;
    }
    gResMgr.curResFile = refNum;
}

/* Find type in resource map */
TypeListEntry* ResMap_FindType(ResFile* file, ResType type) {
    ResMapHeader* map;
    UInt16 typeListOff;
    UInt16 numTypes;
    UInt8* typeList;
    int i;

    if (!file || !file->map) return NULL;

    map = file->map;
    typeListOff = read_be16((UInt8*)&map->typeListOffset);

    if (typeListOff == 0 || typeListOff >= gResMgr.resFiles[0].dataSize) {
        return NULL;
    }

    typeList = (UInt8*)map + typeListOff;
    numTypes = read_be16(typeList) + 1;  /* Count is stored as n-1 */
    typeList += 2;  /* Skip count */

    for (i = 0; i < numTypes; i++) {
        TypeListEntry* entry = (TypeListEntry*)(typeList + i * sizeof(TypeListEntry));
        ResType entryType = read_be32((UInt8*)&entry->resType);
        if (entryType == type) {
            return entry;
        }
    }

    return NULL;
}

/* Find resource by type and ID */
RefListEntry* ResMap_FindResource(ResFile* file, ResType type, ResID id) {
    TypeListEntry* typeEntry;
    UInt16 count;
    UInt16 refListOff;
    UInt8* refList;
    int i;

    typeEntry = ResMap_FindType(file, type);
    if (!typeEntry) return NULL;

    count = read_be16((UInt8*)&typeEntry->count) + 1;  /* Count is stored as n-1 */
    refListOff = read_be16((UInt8*)&typeEntry->refListOffset);

    /* Reference list offset is from start of type list */
    ResMapHeader* map = file->map;
    UInt16 typeListOff = read_be16((UInt8*)&map->typeListOffset);
    refList = (UInt8*)map + typeListOff + refListOff;

    for (i = 0; i < count; i++) {
        RefListEntry* ref = (RefListEntry*)(refList + i * sizeof(RefListEntry));
        ResID refID = (ResID)read_be16((UInt8*)&ref->resID);
        if (refID == id) {
            return ref;
        }
    }

    return NULL;
}

/* Load resource data from file */
Handle ResFile_LoadResource(ResFile* file, RefListEntry* ref) {
    UInt32 dataOffset;
    ResourceDataEntry* dataEntry;
    UInt32 dataLength;
    Handle h;
    UInt8* data;

    if (!file || !ref) return NULL;

    /* Reconstruct 24-bit offset */
    dataOffset = ((UInt32)ref->dataOffsetHi << 16) | read_be16((UInt8*)&ref->dataOffsetLo);

    /* Offset is from start of resource data section */
    ResourceHeader* hdr = (ResourceHeader*)file->data;
    UInt32 dataBase = read_be32((UInt8*)&hdr->dataOffset);
    UInt32 actualOffset = dataBase + dataOffset;

    if (actualOffset + sizeof(ResourceDataEntry) > file->dataSize) {
        gResMgr.resError = mapReadErr;
        return NULL;
    }

    dataEntry = (ResourceDataEntry*)(file->data + actualOffset);
    dataLength = read_be32((UInt8*)&dataEntry->length);

    if (actualOffset + sizeof(ResourceDataEntry) + dataLength > file->dataSize) {
        gResMgr.resError = mapReadErr;
        return NULL;
    }

    /* Allocate handle and copy data */
    h = NewHandle(dataLength);
    if (!h) {
        gResMgr.resError = noMemForRsrc;
        return NULL;
    }

    HLock(h);
    data = (UInt8*)*h;
    BlockMove((UInt8*)dataEntry + sizeof(ResourceDataEntry), data, dataLength);
    HUnlock(h);

    return h;
}

/* Get resource by type and ID */
Handle GetResource(ResType theType, ResID theID) {
    int i;
    RefListEntry* ref = NULL;
    ResFile* file = NULL;

    /* Search all open resource files, starting with current */
    for (i = gResMgr.curResFile; i >= 0; i--) {
        if (!gResMgr.resFiles[i].inUse) continue;

        ref = ResMap_FindResource(&gResMgr.resFiles[i], theType, theID);
        if (ref) {
            file = &gResMgr.resFiles[i];
            break;
        }
    }

    /* If not found in current chain, search other files */
    if (!ref) {
        for (i = 0; i < MAX_RES_FILES; i++) {
            if (!gResMgr.resFiles[i].inUse || i == gResMgr.curResFile) continue;

            ref = ResMap_FindResource(&gResMgr.resFiles[i], theType, theID);
            if (ref) {
                file = &gResMgr.resFiles[i];
                break;
            }
        }
    }

    if (!ref) {
        gResMgr.resError = resNotFound;
        return NULL;
    }

    /* Check if already loaded (stored in reserved field) */
    Handle h = (Handle)(uintptr_t)ref->reserved;
    if (h) {
        gResMgr.resError = noErr;
        return h;
    }

    /* Load the resource */
    if (gResMgr.resLoad) {
        h = ResFile_LoadResource(file, ref);
        if (h) {
            /* Cache the handle (cast through uintptr_t for safety) */
            ref->reserved = (UInt32)(uintptr_t)h;
        }
        return h;
    }

    gResMgr.resError = noErr;
    return NULL;  /* Resource exists but not loaded */
}

/* Get resource from current file only */
Handle Get1Resource(ResType theType, ResID theID) {
    RefListEntry* ref;
    ResFile* file;

    if (gResMgr.curResFile < 0) {
        gResMgr.resError = resFileNotOpen;
        return NULL;
    }

    file = &gResMgr.resFiles[gResMgr.curResFile];
    ref = ResMap_FindResource(file, theType, theID);

    if (!ref) {
        gResMgr.resError = resNotFound;
        return NULL;
    }

    /* Check if already loaded */
    Handle h = (Handle)(uintptr_t)ref->reserved;
    if (h) {
        gResMgr.resError = noErr;
        return h;
    }

    /* Load the resource */
    if (gResMgr.resLoad) {
        h = ResFile_LoadResource(file, ref);
        if (h) {
            ref->reserved = (UInt32)(uintptr_t)h;
        }
        return h;
    }

    gResMgr.resError = noErr;
    return NULL;
}

/* Get named resource (stub for now) */
Handle GetNamedResource(ResType theType, ConstStr255Param name) {
    (void)theType;
    (void)name;
    gResMgr.resError = resNotFound;
    return NULL;
}

/* Get named resource from current file (stub) */
Handle Get1NamedResource(ResType theType, ConstStr255Param name) {
    (void)theType;
    (void)name;
    gResMgr.resError = resNotFound;
    return NULL;
}

/* Release resource */
void ReleaseResource(Handle theResource) {
    /* In read-only mode, just dispose the handle */
    if (theResource) {
        DisposeHandle(theResource);
    }
}

/* Get resource info */
void GetResInfo(Handle theResource, ResID *theID, ResType *theType, char* name) {
    /* Stub - would need to track handle->resource mapping */
    (void)theResource;
    if (theID) *theID = 0;
    if (theType) *theType = 0;
    if (name) name[0] = 0;
    gResMgr.resError = resNotFound;
}

/* Get resource size on disk */
Size GetResourceSizeOnDisk(Handle theResource) {
    /* Stub - would need to track handle->resource mapping */
    (void)theResource;
    gResMgr.resError = resNotFound;
    return 0;
}

/* Get maximum resource size */
Size GetMaxResourceSize(Handle theResource) {
    return GetResourceSizeOnDisk(theResource);
}

/* Get home file of resource */
SInt16 HomeResFile(Handle theResource) {
    /* Stub - would need to track handle->resource mapping */
    (void)theResource;
    gResMgr.resError = resNotFound;
    return -1;
}

/* Detach resource from file */
void DetachResource(Handle theResource) {
    /* In read-only mode, this is a no-op */
    (void)theResource;
    gResMgr.resError = noErr;
}

/* Load resource data */
void LoadResource(Handle theResource) {
    /* In our implementation, resources are always loaded */
    (void)theResource;
    gResMgr.resError = noErr;
}

/* Set whether to auto-load resources */
void SetResLoad(Boolean load) {
    gResMgr.resLoad = load;
}

/* Open resource file (stub) */
SInt16 OpenResFile(const unsigned char* fileName) {
    (void)fileName;
    gResMgr.resError = resFNotFound;
    return -1;
}

/* Close resource file */
void CloseResFile(SInt16 refNum) {
    if (refNum < 0 || refNum >= MAX_RES_FILES || !gResMgr.resFiles[refNum].inUse) {
        gResMgr.resError = badRefNum;
        return;
    }

    if (refNum == 0) {
        /* Can't close system file */
        gResMgr.resError = badRefNum;
        return;
    }

    ResFile_Close(refNum);
}

/* Internal: Close resource file */
void ResFile_Close(SInt16 refNum) {
    ResFile* file = &gResMgr.resFiles[refNum];

    if (file->mapHandle) {
        DisposeHandle(file->mapHandle);
    }

    file->inUse = false;
    file->refNum = -1;
    file->data = NULL;
    file->map = NULL;

    /* If this was current file, switch to system */
    if (gResMgr.curResFile == refNum) {
        gResMgr.curResFile = 0;
    }
}

/* Write operation stubs (read-only implementation) */
void AddResource(Handle theData, ResType theType, ResID theID, ConstStr255Param name) {
    (void)theData; (void)theType; (void)theID; (void)name;
    gResMgr.resError = addResFailed;
}

void RemoveResource(Handle theResource) {
    (void)theResource;
    gResMgr.resError = rmvResFailed;
}

void WriteResource(Handle theResource) {
    (void)theResource;
    gResMgr.resError = mapReadErr;
}

void UpdateResFile(SInt16 refNum) {
    (void)refNum;
    gResMgr.resError = mapReadErr;
}

void ChangedResource(Handle theResource) {
    (void)theResource;
    gResMgr.resError = mapReadErr;
}

/* Get/set resource attributes */
SInt16 GetResAttrs(Handle theResource) {
    (void)theResource;
    return 0;
}

void SetResAttrs(Handle theResource, SInt16 attrs) {
    (void)theResource; (void)attrs;
    gResMgr.resError = resAttrErr;
}

/* Count resources */
SInt16 CountResources(ResType theType) {
    SInt16 count = 0;
    int i;

    for (i = 0; i < MAX_RES_FILES; i++) {
        if (!gResMgr.resFiles[i].inUse) continue;

        TypeListEntry* typeEntry = ResMap_FindType(&gResMgr.resFiles[i], theType);
        if (typeEntry) {
            count += read_be16((UInt8*)&typeEntry->count) + 1;
        }
    }

    return count;
}

SInt16 Count1Resources(ResType theType) {
    if (gResMgr.curResFile < 0) return 0;

    TypeListEntry* typeEntry = ResMap_FindType(&gResMgr.resFiles[gResMgr.curResFile], theType);
    if (typeEntry) {
        return read_be16((UInt8*)&typeEntry->count) + 1;
    }

    return 0;
}

/* Get indexed resource (stub) */
Handle GetIndResource(ResType theType, SInt16 index) {
    (void)theType; (void)index;
    gResMgr.resError = resNotFound;
    return NULL;
}

Handle Get1IndResource(ResType theType, SInt16 index) {
    (void)theType; (void)index;
    gResMgr.resError = resNotFound;
    return NULL;
}

/* Type enumeration (stub) */
void GetIndType(ResType *theType, SInt16 index) {
    (void)index;
    if (theType) *theType = 0;
    gResMgr.resError = resNotFound;
}

void Get1IndType(ResType *theType, SInt16 index) {
    (void)index;
    if (theType) *theType = 0;
    gResMgr.resError = resNotFound;
}

SInt16 CountTypes(void) {
    return 0;
}

SInt16 Count1Types(void) {
    return 0;
}

/* Unique ID generation */
SInt16 UniqueID(ResType theType) {
    (void)theType;
    return gResMgr.nextUniqueID++;
}

SInt16 Unique1ID(ResType theType) {
    return UniqueID(theType);
}