/*
 * ScrapManager.c - Minimal Scrap Manager (Clipboard) Implementation
 *
 * Provides inter-process clipboard functionality for System 7.1-style OS.
 * Uses Handle-based storage for memory management.
 */

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

/* Additional function declarations */
extern OSErr Gestalt_RegisterSelector(OSType selector, SInt32 value);
extern void BlockMoveData(const void* src, void* dest, Size size);
extern OSErr MemError(void);
extern void serial_printf(const char* fmt, ...);

/* Maximum number of scrap types we support */
#define MAX_SCRAP_ITEMS 8

/* Error codes */
#ifndef noTypeErr
#define noTypeErr (-102)  /* No object of that type in scrap */
#endif

/* Scrap Manager global state */
static struct {
    short state;           /* Scrap state counter (increments on change) */
    short count;           /* Number of items in scrap */
    ProcessID owner;       /* Process that owns current scrap */
    ScrapItem items[MAX_SCRAP_ITEMS];
    Boolean inited;        /* Has been initialized */
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
    if (gScrap.inited) {
        return;
    }

    /* Zero the scrap table */
    gScrap.state = 0;
    gScrap.count = 0;
    gScrap.owner = 0;

    for (int i = 0; i < MAX_SCRAP_ITEMS; i++) {
        gScrap.items[i].type = 0;
        gScrap.items[i].data = NULL;
    }

    /* Register with Gestalt - commented out as function doesn't exist yet */
    /* 'scra' - bit0: present, bit1: TEXT supported, bit2: PICT stored */
    /* TODO: Implement Gestalt_RegisterSelector or use NewGestalt */
    /*
    OSErr err = Gestalt_RegisterSelector('scra', 0x03);
    if (err == noErr) {
        serial_printf("[Scrap] Registered with Gestalt\n");
    }
    */

    gScrap.inited = true;
}

/*
 * FindScrapItem - Find item by type
 */
static ScrapItem* FindScrapItem(ResType type)
{
    for (int i = 0; i < MAX_SCRAP_ITEMS; i++) {
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
    /* First check if type already exists */
    ScrapItem* item = FindScrapItem(type);
    if (item) {
        return item;
    }

    /* Find empty slot */
    for (int i = 0; i < MAX_SCRAP_ITEMS; i++) {
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
    InitScrapIfNeeded();

    serial_printf("[Scrap] Scrap_Zero called\n");

    /* Dispose all existing handles */
    for (int i = 0; i < MAX_SCRAP_ITEMS; i++) {
        if (gScrap.items[i].data != NULL) {
            DisposeHandle(gScrap.items[i].data);
            gScrap.items[i].data = NULL;
        }
        gScrap.items[i].type = 0;
    }

    /* Reset state */
    gScrap.count = 0;
    gScrap.state++;  /* Increment state to indicate change */

    /* Set owner to current process if ProcessMgr is available */
#ifdef ENABLE_PROCESS_COOP
    extern ProcessID Proc_GetCurrent(void);
    gScrap.owner = Proc_GetCurrent();
#else
    gScrap.owner = 1;  /* Default process ID */
#endif

    serial_printf("[Scrap] Zeroed, state=%d owner=%d\n",
                 gScrap.state, gScrap.owner);
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
    gScrap.state++;
#ifdef ENABLE_PROCESS_COOP
    extern ProcessID Proc_GetCurrent(void);
    gScrap.owner = Proc_GetCurrent();
#else
    gScrap.owner = 1;
#endif

    serial_printf("[Scrap] Put type='%c%c%c%c' size=%ld state=%d\n",
                 (char)(type >> 24), (char)(type >> 16),
                 (char)(type >> 8), (char)type,
                 (long)size, gScrap.state);

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

    serial_printf("[Scrap] Get type='%c%c%c%c' size=%ld\n",
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
        *state = gScrap.state;
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
 * Mac OS Classic API Compatibility Functions
 * ============================================================================ */

/*
 * ZeroScrap - Clear the scrap (Classic Mac OS API)
 */
OSErr ZeroScrap(void) {
    Scrap_Zero();
    return noErr;
}

/*
 * PutScrap - Put data into scrap (Classic Mac OS API)
 */
OSErr PutScrap(SInt32 length, ResType theType, const void *source) {
    return Scrap_Put((Size)length, theType, source);
}

/*
 * GetScrap - Get data from scrap (Classic Mac OS API)
 * Returns size on success, negative error code on failure
 */
SInt32 GetScrap(Handle hDest, ResType theType, SInt32 *offset) {
    Size size;

    /* First get the size */
    size = Scrap_Get(NULL, theType);
    if (size == 0) {
        return noTypeErr;  /* Type not found */
    }

    /* If no destination handle, just return size */
    if (!hDest) {
        if (offset) *offset = 0;
        return (SInt32)size;
    }

    /* Resize destination handle */
    SetHandleSize(hDest, size);
    if (MemError() != noErr) {
        return MemError();
    }

    /* Get the data */
    HLock(hDest);
    size = Scrap_Get(*hDest, theType);
    HUnlock(hDest);

    if (offset) *offset = 0;

    return (SInt32)size;
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