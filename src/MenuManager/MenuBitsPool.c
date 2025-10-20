/**
 * @file MenuBitsPool.c
 * @brief Menu Bits Memory Pool Implementation
 *
 * Pool-based buffer management for menu background save/restore operations.
 * Eliminates heap fragmentation from repeated 50KB+ allocations by reusing
 * a fixed set of preallocated buffers.
 *
 * Copyright (c) 2025 System 7.1 Portable Project
 */

#include "SystemTypes.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include "MenuManager/MenuBitsPool.h"
#include <string.h>

/*---------------------------------------------------------------------------
 * Pool Structure
 *---------------------------------------------------------------------------*/

/* Saved bits record structure - matches what SaveBits uses */
typedef struct {
    Rect bounds;        /* Rectangle that was saved */
    SInt16 mode;        /* Save mode flags */
    void* bitsData;     /* Saved pixel data (preallocated from pool) */
    SInt32 dataSize;    /* Size of saved data */
    Boolean valid;      /* Handle is valid */
    Boolean fromPool;   /* This buffer came from the pool */
} SavedBitsRec, *SavedBitsPtr, **SavedBitsHandle;

/* Pool entry - tracks a preallocated buffer */
typedef struct {
    void* pixelBuffer;      /* Preallocated pixel data (fixed size) */
    SavedBitsRec record;    /* Metadata record */
    Boolean inUse;          /* Currently borrowed */
} PoolEntry;

/* Global pool state */
typedef struct {
    PoolEntry* entries;
    SInt16 numEntries;
    SInt32 bufferSize;      /* Size of each pixel buffer */
    Boolean initialized;
} MenuBitsPoolState;

static MenuBitsPoolState gMenuBitsPool = {0};

/* External framebuffer access */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

/*---------------------------------------------------------------------------
 * Pool Initialization and Shutdown
 *---------------------------------------------------------------------------*/

/**
 * Initialize the menu bits pool with preallocated buffers
 */
OSErr MenuBitsPool_Init(SInt16 numBuffers, SInt32 bufferSize) {
    extern void serial_printf(const char* fmt, ...);

    if (gMenuBitsPool.initialized) {
        serial_printf("[MBPOOL] Pool already initialized\n");
        return noErr;
    }

    if (numBuffers <= 0 || bufferSize <= 0) {
        serial_printf("[MBPOOL] Invalid pool parameters: %d buffers, %d bytes each\n",
                     numBuffers, bufferSize);
        return paramErr;
    }

    serial_printf("[MBPOOL] Initializing pool: %d buffers × %d bytes = %d KB total\n",
                 numBuffers, bufferSize, (numBuffers * bufferSize) / 1024);

    /* Allocate pool entry array */
    gMenuBitsPool.entries = (PoolEntry*)NewPtr(numBuffers * sizeof(PoolEntry));
    if (!gMenuBitsPool.entries) {
        serial_printf("[MBPOOL] Failed to allocate pool entries array\n");
        return memFullErr;
    }

    /* Initialize each pool entry */
    for (SInt16 i = 0; i < numBuffers; i++) {
        PoolEntry* entry = &gMenuBitsPool.entries[i];

        /* Allocate pixel buffer */
        entry->pixelBuffer = (void*)NewPtr(bufferSize);
        if (!entry->pixelBuffer) {
            serial_printf("[MBPOOL] Failed to allocate buffer %d\n", i);

            /* Free previously allocated buffers */
            for (SInt16 j = 0; j < i; j++) {
                DisposePtr((Ptr)gMenuBitsPool.entries[j].pixelBuffer);
            }
            DisposePtr((Ptr)gMenuBitsPool.entries);
            gMenuBitsPool.entries = NULL;

            return memFullErr;
        }

        /* Initialize record */
        entry->record.bitsData = entry->pixelBuffer;
        entry->record.dataSize = bufferSize;
        entry->record.valid = false;
        entry->record.fromPool = true;
        entry->inUse = false;

        serial_printf("[MBPOOL] Buffer %d allocated: %p\n", i, entry->pixelBuffer);
    }

    gMenuBitsPool.numEntries = numBuffers;
    gMenuBitsPool.bufferSize = bufferSize;
    gMenuBitsPool.initialized = true;

    serial_printf("[MBPOOL] Pool initialized successfully\n");
    return noErr;
}

/**
 * Shutdown the pool and free all resources
 */
OSErr MenuBitsPool_Shutdown(void) {
    extern void serial_printf(const char* fmt, ...);

    if (!gMenuBitsPool.initialized) {
        return noErr;
    }

    serial_printf("[MBPOOL] Shutting down pool\n");

    if (gMenuBitsPool.entries) {
        for (SInt16 i = 0; i < gMenuBitsPool.numEntries; i++) {
            PoolEntry* entry = &gMenuBitsPool.entries[i];
            if (entry->pixelBuffer) {
                DisposePtr((Ptr)entry->pixelBuffer);
            }
        }
        DisposePtr((Ptr)gMenuBitsPool.entries);
        gMenuBitsPool.entries = NULL;
    }

    gMenuBitsPool.initialized = false;
    serial_printf("[MBPOOL] Pool shutdown complete\n");

    return noErr;
}

/*---------------------------------------------------------------------------
 * Pool Allocation and Deallocation
 *---------------------------------------------------------------------------*/

/**
 * Allocate a buffer from the pool
 */
Handle MenuBitsPool_Allocate(const Rect* bounds) {
    extern void serial_printf(const char* fmt, ...);

    if (!gMenuBitsPool.initialized) {
        serial_printf("[MBPOOL] Pool not initialized\n");
        return NULL;
    }

    if (!bounds) {
        serial_printf("[MBPOOL] NULL bounds\n");
        return NULL;
    }

    /* Find first available buffer */
    for (SInt16 i = 0; i < gMenuBitsPool.numEntries; i++) {
        PoolEntry* entry = &gMenuBitsPool.entries[i];

        if (!entry->inUse) {
            /* Found available buffer - mark as in use */
            entry->inUse = true;
            entry->record.bounds = *bounds;
            entry->record.valid = false;  /* Will be set to true after data is copied */
            entry->record.mode = 0;

            serial_printf("[MBPOOL] Allocated buffer %d at %p\n", i, &entry->record);

            /* Return a handle to the record */
            /* Since this is a static buffer, we create a temporary handle */
            SavedBitsHandle handle = (SavedBitsHandle)NewHandle(sizeof(SavedBitsPtr));
            if (!handle) {
                entry->inUse = false;
                serial_printf("[MBPOOL] Failed to allocate handle for pool entry\n");
                return NULL;
            }

            HLock((Handle)handle);
            *handle = &entry->record;
            HUnlock((Handle)handle);

            return (Handle)handle;
        }
    }

    /* No available buffers */
    serial_printf("[MBPOOL] No available buffers (all %d in use)\n", gMenuBitsPool.numEntries);
    return NULL;
}

/**
 * Return a buffer to the pool
 */
OSErr MenuBitsPool_Free(Handle poolHandle) {
    extern void serial_printf(const char* fmt, ...);

    if (!gMenuBitsPool.initialized || !poolHandle) {
        return paramErr;
    }

    HLock(poolHandle);
    SavedBitsHandle handle = (SavedBitsHandle)poolHandle;
    SavedBitsPtr record = *handle;

    /* Find the pool entry that owns this record */
    Boolean found = false;
    for (SInt16 i = 0; i < gMenuBitsPool.numEntries; i++) {
        PoolEntry* entry = &gMenuBitsPool.entries[i];

        if (&entry->record == record && entry->inUse) {
            /* Found it - mark as available */
            entry->inUse = false;
            entry->record.valid = false;

            serial_printf("[MBPOOL] Freed buffer %d\n", i);
            found = true;
            break;
        }
    }

    HUnlock(poolHandle);
    DisposeHandle(poolHandle);

    if (!found) {
        serial_printf("[MBPOOL] Warning: Freed handle didn't belong to pool\n");
        return paramErr;
    }

    return noErr;
}

/*---------------------------------------------------------------------------
 * Pool Statistics
 *---------------------------------------------------------------------------*/

/**
 * Get pool statistics
 */
Boolean MenuBitsPool_GetStats(SInt16* outTotal, SInt16* outInUse) {
    if (!gMenuBitsPool.initialized) {
        return false;
    }

    if (outTotal) {
        *outTotal = gMenuBitsPool.numEntries;
    }

    if (outInUse) {
        SInt16 inUse = 0;
        for (SInt16 i = 0; i < gMenuBitsPool.numEntries; i++) {
            if (gMenuBitsPool.entries[i].inUse) {
                inUse++;
            }
        }
        *outInUse = inUse;
    }

    return true;
}
