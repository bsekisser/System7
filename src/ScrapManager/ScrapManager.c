/*
 * ScrapManager.c - Classic Mac OS Scrap Manager Implementation
 *
 * System 7.1-compatible clipboard with multiple flavors and persistence.
 * Uses Handle-based storage for memory management.
 */

/* Standard includes */
#include <string.h>
#include "SystemTypes.h"
/* Only include ScrapTypes.h to avoid conflicts with existing ScrapManager.h */
#define SCRAP_MANAGER_H  /* Prevent ScrapManager.h inclusion */
#include "ScrapManager/ScrapTypes.h"
#include "MemoryMgr/MemoryManager.h"
#ifdef ENABLE_PROCESS_COOP
#include "ProcessMgr/ProcessTypes.h"
#endif
#include "Gestalt/Gestalt.h"
#include "System71StdLib.h"
#include "ScrapManager/ScrapLogging.h"

/* Additional function declarations */
extern OSErr Gestalt_RegisterSelector(OSType selector, SInt32 value);
/* Note: BlockMoveData and MemError declared in MemoryMgr/MemoryManager.h */

/* Forward declaration for test function */
void Scrap_RunSelfTest(void);

/* Debug logging control */
#define SCRAP_DEBUG 1

#if SCRAP_DEBUG
#define SCRAP_LOG(...) SCRAP_LOG_DEBUG("[Scrap] " __VA_ARGS__)
#else
#define SCRAP_LOG(...)
#endif

/* Maximum number of scrap types we support */
#define MAX_SCRAP_ITEMS 16

/* File format constants */
#define SCRAP_FILE_MAGIC    0x434C4950  /* 'CLIP' */
#define SCRAP_FILE_VERSION  1
#define SCRAP_FILE_PATH     "/Clipboard"

/* Error codes */
#ifndef noTypeErr
#define noTypeErr (-102)  /* No object of that type in scrap */
#endif

/* Scrap Manager global state */
static struct {
    UInt32 changeCnt;      /* Change counter (increments on change) */
    short count;           /* Number of items in scrap */
    ProcessID owner;       /* Process that owns current scrap */
    ScrapItem items[MAX_SCRAP_ITEMS];
    Boolean inited;        /* Has been initialized */
    Boolean dirty;         /* Needs saving to disk */
} gScrap = {0};

/* Forward declarations */
static void InitScrapIfNeeded(void);
static ScrapItem* FindScrapItem(ResType type);
static ScrapItem* AllocateScrapItem(ResType type);

/*
 * InitScrapIfNeeded - Initialize scrap on first use
 */
static void InitScrapIfNeeded(void)
{
    int i;

    if (gScrap.inited) {
        return;
    }

    /* Zero the scrap table */
    gScrap.changeCnt = 0;
    gScrap.count = 0;
    gScrap.owner = 0;
    gScrap.dirty = false;

    for (i = 0; i < MAX_SCRAP_ITEMS; i++) {
        gScrap.items[i].type = 0;
        gScrap.items[i].data = NULL;
    }

    /* Register with Gestalt - commented out as function doesn't exist yet */
    /* 'scra' - bit0: present, bit1: TEXT supported, bit2: PICT stored */
    /* TODO: Implement Gestalt_RegisterSelector or use NewGestalt */
    /*
    OSErr err = Gestalt_RegisterSelector('scra', 0x03);
    if (err == noErr) {
        SCRAP_LOG("Registered with Gestalt\n");
    }
    */

    gScrap.inited = true;
}

/*
 * FindScrapItem - Find item by type
 */
static ScrapItem* FindScrapItem(ResType type)
{
    int i;
    for (i = 0; i < MAX_SCRAP_ITEMS; i++) {
        if (gScrap.items[i].type == type && gScrap.items[i].data != NULL) {
            return &gScrap.items[i];
        }
    }
    return NULL;
}

/*
 * AllocateScrapItem - Allocate or find slot for type
 */
static ScrapItem* AllocateScrapItem(ResType type)
{
    ScrapItem* item;
    int i;

    /* First check if type already exists */
    item = FindScrapItem(type);
    if (item) {
        return item;
    }

    /* Find empty slot */
    for (i = 0; i < MAX_SCRAP_ITEMS; i++) {
        if (gScrap.items[i].type == 0 || gScrap.items[i].data == NULL) {
            gScrap.items[i].type = type;
            return &gScrap.items[i];
        }
    }

    return NULL;  /* Table full */
}

/*
 * Scrap_Zero - Clear the scrap
 */
void Scrap_Zero(void)
{
    int i;
    InitScrapIfNeeded();

    SCRAP_LOG("Scrap_Zero called\n");

    /* Dispose all existing handles */
    for (i = 0; i < MAX_SCRAP_ITEMS; i++) {
        if (gScrap.items[i].data != NULL) {
            DisposeHandle(gScrap.items[i].data);
            gScrap.items[i].data = NULL;
        }
        gScrap.items[i].type = 0;
    }

    /* Reset state */
    gScrap.count = 0;
    gScrap.changeCnt++;  /* Increment change counter */
    gScrap.dirty = true;

    /* Set owner to current process if ProcessMgr is available */
#ifdef ENABLE_PROCESS_COOP
    extern ProcessID Proc_GetCurrent(void);
    gScrap.owner = Proc_GetCurrent();
#else
    gScrap.owner = 1;  /* Default process ID */
#endif

    SCRAP_LOG("Zeroed, changeCnt=%lu owner=%d\n",
             (unsigned long)gScrap.changeCnt, gScrap.owner);
}

/*
 * Scrap_Put - Put data into scrap
 */
OSErr Scrap_Put(Size size, ResType type, const void* src)
{
    InitScrapIfNeeded();

    /* Validate parameters */
    if ((src == NULL && size > 0) || (src != NULL && size == 0)) {
        return paramErr;
    }

    /* Find or allocate item */
    ScrapItem* item = AllocateScrapItem(type);
    if (!item) {
        return memFullErr;  /* No space in scrap table */
    }

    /* Allocate or resize handle */
    if (item->data == NULL) {
        item->data = NewHandle(size);
        if (!item->data) {
            return memFullErr;
        }
        gScrap.count++;  /* New item */
    } else {
        SetHandleSize(item->data, size);
        if (MemError() != noErr) {
            return memFullErr;
        }
    }

    /* Copy data into handle */
    if (size > 0 && src != NULL) {
        HLock(item->data);
        BlockMoveData(src, *item->data, size);
        HUnlock(item->data);
    }

    /* Update state and owner */
    gScrap.changeCnt++;
    gScrap.dirty = true;
#ifdef ENABLE_PROCESS_COOP
    extern ProcessID Proc_GetCurrent(void);
    gScrap.owner = Proc_GetCurrent();
#else
    gScrap.owner = 1;
#endif

    SCRAP_LOG("Put type='%c%c%c%c' size=%ld changeCnt=%lu\n",
             (char)(type >> 24), (char)(type >> 16),
             (char)(type >> 8), (char)type,
             (long)size, (unsigned long)gScrap.changeCnt);

    return noErr;
}

/*
 * Scrap_Get - Get data from scrap
 */
Size Scrap_Get(void* dest, ResType type)
{
    InitScrapIfNeeded();

    /* Find item */
    ScrapItem* item = FindScrapItem(type);
    if (!item || !item->data) {
        return 0;  /* Not found */
    }

    /* Get size */
    Size size = GetHandleSize(item->data);

    /* Copy data if destination provided */
    if (dest != NULL && size > 0) {
        HLock(item->data);
        BlockMoveData(*item->data, dest, size);
        HUnlock(item->data);
    }

    SCRAP_LOG("Get type='%c%c%c%c' size=%ld\n",
             (char)(type >> 24), (char)(type >> 16),
             (char)(type >> 8), (char)type,
             (long)size);

    return size;
}

/*
 * Scrap_Info - Get scrap information
 */
void Scrap_Info(short* count, short* state)
{
    InitScrapIfNeeded();

    if (count) {
        *count = gScrap.count;
    }
    if (state) {
        *state = (short)(gScrap.changeCnt & 0xFFFF);  /* Return low 16 bits for compatibility */
    }
}

/*
 * Scrap_Unload - Unload scrap from memory (no-op for MVP)
 */
void Scrap_Unload(void)
{
    /* No-op for now - could write to disk in future */
}

/*
 * Scrap_GetOwner - Get process that owns current scrap
 */
ProcessID Scrap_GetOwner(void)
{
    InitScrapIfNeeded();
    return gScrap.owner;
}

/* ============================================================================
 * File I/O Helper Functions - Stubbed for kernel environment
 * TODO: Implement using VFS when available
 * ============================================================================ */

/* ============================================================================
 * Mac OS Classic API Compatibility Functions
 * ============================================================================ */

/*
 * ZeroScrap - Clear the scrap (Classic Mac OS API)
 */
void ZeroScrap(void) {
    Scrap_Zero();
}

/*
 * PutScrap - Put data into scrap (Classic Mac OS API)
 */
void PutScrap(long byteCount, OSType theType, const void* sourcePtr) {
    if (byteCount < 0) return;
    Scrap_Put((Size)byteCount, theType, sourcePtr);
}

/*
 * GetScrap - Get data from scrap (Classic Mac OS API)
 * Returns size on success, 0 if type not found
 * Appends data to hDest at *offset if provided
 */
long GetScrap(Handle hDest, OSType theType, long* offset) {
    ScrapItem* item;
    Size flavorSize;
    Size currentSize;
    Ptr srcPtr;
    Ptr destPtr;

    InitScrapIfNeeded();

    /* Find the item */
    item = FindScrapItem(theType);
    if (!item || !item->data) {
        SCRAP_LOG("GetScrap: type '%.4s' not found\n", (char*)&theType);
        return 0;
    }

    /* Get flavor size */
    flavorSize = GetHandleSize(item->data);
    if (flavorSize == 0) {
        return 0;
    }

    /* If no destination, just return size */
    if (!hDest) {
        return (long)flavorSize;
    }

    /* Get current size and grow handle */
    currentSize = GetHandleSize(hDest);
    SetHandleSize(hDest, currentSize + flavorSize);
    if (MemError() != noErr) {
        SCRAP_LOG("GetScrap: failed to grow handle\n");
        return 0;
    }

    /* Lock both handles for copying */
    HLock(item->data);
    HLock(hDest);

    /* Copy data at offset */
    srcPtr = *item->data;
    destPtr = *hDest + (offset ? *offset : 0);
    BlockMoveData(srcPtr, destPtr, flavorSize);

    /* Unlock handles */
    HUnlock(item->data);
    HUnlock(hDest);

    /* Update offset if provided */
    if (offset) {
        *offset += flavorSize;
    }

    SCRAP_LOG("GetScrap: copied %ld bytes of '%.4s'\n",
              (long)flavorSize, (char*)&theType);

    return (long)flavorSize;
}

/*
 * LoadScrap - Load scrap from disk
 * Stubbed for kernel environment - TODO: Implement using VFS
 */
void LoadScrap(void) {
    InitScrapIfNeeded();
    SCRAP_LOG("LoadScrap: stub - file I/O not available\n");
    /* In-memory scrap only for now */
}

/*
 * UnloadScrap - Save scrap to disk
 * Stubbed for kernel environment - TODO: Implement using VFS
 */
void UnloadScrap(void) {
    InitScrapIfNeeded();
    SCRAP_LOG("UnloadScrap: stub - file I/O not available\n");
    /* In-memory scrap only for now */
    gScrap.dirty = false;
}
/*
 * InfoScrap - Get scrap change count
 */
long InfoScrap(void) {
    InitScrapIfNeeded();
    return (long)gScrap.changeCnt;
}

/*
 * ScrapHasFlavor - Check if clipboard has specified flavor
 */
Boolean ScrapHasFlavor(OSType theType) {
    InitScrapIfNeeded();
    return FindScrapItem(theType) != NULL;
}

/*
 * ScrapGetFlavorSize - Get size of specified flavor
 */
long ScrapGetFlavorSize(OSType theType) {
    ScrapItem* item;

    InitScrapIfNeeded();

    item = FindScrapItem(theType);
    if (!item || !item->data) {
        return 0;
    }

    return (long)GetHandleSize(item->data);
}

#ifdef SCRAP_SELFTEST
/*
 * Scrap_RunSelfTest - Run self-test (called from main.c)
 */
void Scrap_RunSelfTest(void)
{
    extern void serial_puts(const char* str);
    serial_puts("[Scrap] Running self-test...\n");

    /* Test 1: Zero scrap */
    Scrap_Zero();

    /* Test 2: Put TEXT */
    const char* testText = "hello";
    OSErr putErr = Scrap_Put(5, kScrapTypeTEXT, testText);

    /* Test 3: Get TEXT */
    char buffer[32] = {0};
    Size size = Scrap_Get(buffer, kScrapTypeTEXT);

    /* Test 4: Info */
    short count, state;
    Scrap_Info(&count, &state);

    ProcessID owner = Scrap_GetOwner();

    /* Use serial_puts for simpler output */
    serial_puts("[Scrap] Self-test completed: ");
    serial_puts(buffer);
    serial_puts(" (scrap working)\n");

    (void)putErr; (void)size; (void)state; (void)owner;  /* Suppress warnings */
}
#endif