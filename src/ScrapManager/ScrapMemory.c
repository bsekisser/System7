/* #include "SystemTypes.h" */
#include <stdlib.h>
#include <string.h>
/*
 * ScrapMemory.c - Scrap Memory Management and Reference Counting
 * System 7.1 Portable - Scrap Manager Component
 *
 * Implements memory management for scrap data including reference counting,
 * memory optimization, garbage collection, and intelligent caching.
 */

// #include "CompatibilityFix.h" // Removed

#include "ScrapManager/ScrapManager.h"
#include "ScrapManager/ScrapTypes.h"
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "MemoryMgr/memory_manager_types.h"
/* #include "ErrorCodes.h"
 - error codes in MacTypes.h */

/* Memory block tracking */
typedef struct MemoryBlock {
    void            *ptr;               /* Pointer to memory block */
    SInt32         size;               /* Size of block */
    SInt32         refCount;           /* Reference count */
    time_t          lastAccess;         /* Last access time */
    Boolean         isLocked;           /* Block is locked */
    Boolean         isPurgeable;        /* Block can be purged */
    ResType         dataType;           /* Type of data in block */
    UInt32        checksum;           /* Data integrity checksum */
    struct MemoryBlock *next;           /* Next block in list */
} MemoryBlock;

/* Memory cache entry */
typedef struct CacheEntry {
    ResType         type;               /* Data type */
    Handle          dataHandle;         /* Cached data */
    SInt32         size;               /* Data size */
    time_t          createTime;         /* When cached */
    time_t          lastAccess;         /* Last access */
    SInt32         accessCount;        /* Number of accesses */
    Boolean         isDirty;            /* Needs to be saved */
} CacheEntry;

/* Memory management state */
typedef struct {
    MemoryBlock     *blockList;         /* List of allocated blocks */
    CacheEntry      cache[32];          /* Format cache */
    SInt16         cacheCount;         /* Number of cached items */
    SInt32         totalAllocated;     /* Total memory allocated */
    SInt32         maxMemoryUsage;     /* Maximum memory allowed */
    SInt32         memoryThreshold;    /* Threshold for disk storage */
    SInt32         cacheHits;          /* Cache hit counter */
    SInt32         cacheMisses;        /* Cache miss counter */
    Boolean         autoCompact;        /* Auto-compact memory */
    Boolean         enableCache;        /* Enable format caching */
} MemoryState;

static MemoryState gMemoryState = {0};
static Boolean gMemoryInitialized = false;

/* Internal function prototypes */
static OSErr InitializeMemoryManager(void);
static MemoryBlock *AllocateMemoryBlock(SInt32 size, ResType dataType);
static void FreeMemoryBlock(MemoryBlock *block);
static MemoryBlock *FindMemoryBlock(void *ptr);
static OSErr CompactMemoryBlocks(void);
static OSErr PurgeOldestBlocks(SInt32 bytesToFree);
static UInt32 CalculateChecksum(const void *data, SInt32 size);
static OSErr ValidateMemoryBlock(MemoryBlock *block);
static CacheEntry *FindCacheEntry(ResType type);
static OSErr AddToCacheEntry(ResType type, Handle data);
static OSErr RemoveFromCache(ResType type);
static OSErr PurgeCache(SInt32 maxAge);

/*
 * Memory Management Functions
 */

OSErr CompactScrapMemory(void)
{
    OSErr err = noErr;
    SInt32 beforeSize, afterSize;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    beforeSize = gMemoryState.totalAllocated;

    /* Compact memory blocks */
    err = CompactMemoryBlocks();
    if (err != noErr) {
        return err;
    }

    /* Purge old cache entries */
    PurgeCache(300); /* Remove entries older than 5 minutes */

    afterSize = gMemoryState.totalAllocated;

    /* Update statistics */
    if (beforeSize > afterSize) {
        /* Successfully freed memory */
    }

    return noErr;
}

OSErr PurgeScrapData(SInt32 bytesToPurge)
{
    OSErr err = noErr;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (bytesToPurge <= 0) {
        return paramErr;
    }

    /* First try to purge cache */
    SInt32 cacheSize = 0;
    SInt16 i;

    for (i = 0; i < gMemoryState.cacheCount; i++) {
        if (gMemoryState.cache[i].dataHandle) {
            cacheSize += GetHandleSize(gMemoryState.cache[i].dataHandle);
        }
    }

    if (cacheSize >= bytesToPurge) {
        /* Purge oldest cache entries first */
        time_t cutoffTime = time(NULL) - 60; /* Entries older than 1 minute */

        for (i = 0; i < gMemoryState.cacheCount && bytesToPurge > 0; i++) {
            if (gMemoryState.cache[i].lastAccess < cutoffTime) {
                SInt32 entrySize = gMemoryState.cache[i].size;
                RemoveFromCache(gMemoryState.cache[i].type);
                bytesToPurge -= entrySize;
            }
        }
    }

    /* If still need to purge more, purge oldest memory blocks */
    if (bytesToPurge > 0) {
        err = PurgeOldestBlocks(bytesToPurge);
    }

    return err;
}

OSErr SetScrapMemoryPrefs(SInt32 memoryThreshold, SInt32 diskThreshold)
{
    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (memoryThreshold > 0) {
        gMemoryState.memoryThreshold = memoryThreshold;
    }

    if (diskThreshold > 0) {
        /* This would be used by ScrapFile.c */
    }

    return noErr;
}

OSErr GetScrapMemoryInfo(SInt32 *memoryUsed, SInt32 *diskUsed, SInt32 *totalSize)
{
    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (memoryUsed) {
        *memoryUsed = gMemoryState.totalAllocated;
    }

    if (diskUsed) {
        /* This would be provided by ScrapFile.c */
        *diskUsed = 0;
    }

    if (totalSize) {
        *totalSize = gMemoryState.totalAllocated;
    }

    return noErr;
}

/*
 * Handle Management Functions
 */

Handle NewScrapHandle(SInt32 size, ResType dataType)
{
    Handle h;
    MemoryBlock *block;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (size < 0) {
        return NULL;
    }

    /* Check memory limits */
    if (gMemoryState.totalAllocated + size > gMemoryState.maxMemoryUsage) {
        /* Try to free some memory */
        if (PurgeScrapData(size) != noErr) {
            return NULL;
        }
    }

    h = NewHandle(size);
    if (h == NULL) {
        return NULL;
    }

    /* Track the handle */
    block = AllocateMemoryBlock(size, dataType);
    if (block) {
        block->ptr = (void *)h;
        gMemoryState.totalAllocated += size;
    }

    return h;
}

void DisposeScrapHandle(Handle h)
{
    MemoryBlock *block;

    if (!h) {
        return;
    }

    /* Find and remove tracking block */
    block = FindMemoryBlock((void *)h);
    if (block) {
        gMemoryState.totalAllocated -= block->size;
        FreeMemoryBlock(block);
    }

    DisposeHandle(h);
}

OSErr SetScrapHandleSize(Handle h, SInt32 newSize)
{
    MemoryBlock *block;
    SInt32 oldSize;

    if (!h) {
        return paramErr;
    }

    oldSize = GetHandleSize(h);
    SetHandleSize(h, newSize);

    if (MemError() != noErr) {
        return MemError();
    }

    /* Update tracking */
    block = FindMemoryBlock((void *)h);
    if (block) {
        gMemoryState.totalAllocated += (newSize - oldSize);
        block->size = newSize;
        block->lastAccess = time(NULL);
    }

    return noErr;
}

/*
 * Reference Counting Functions
 */

OSErr RetainScrapData(Handle h)
{
    MemoryBlock *block;

    if (!h) {
        return paramErr;
    }

    block = FindMemoryBlock((void *)h);
    if (block) {
        block->refCount++;
        block->lastAccess = time(NULL);
        return noErr;
    }

    return paramErr;
}

OSErr ReleaseScrapData(Handle h)
{
    MemoryBlock *block;

    if (!h) {
        return paramErr;
    }

    block = FindMemoryBlock((void *)h);
    if (block) {
        if (block->refCount > 0) {
            block->refCount--;
        }

        /* If reference count reaches zero and purgeable, purge it */
        if (block->refCount == 0 && block->isPurgeable) {
            /* Mark for purging but don't immediately free */
            block->lastAccess = 0;
        }

        return noErr;
    }

    return paramErr;
}

SInt32 GetScrapDataRefCount(Handle h)
{
    MemoryBlock *block;

    if (!h) {
        return 0;
    }

    block = FindMemoryBlock((void *)h);
    if (block) {
        return block->refCount;
    }

    return 0;
}

/*
 * Caching Functions
 */

OSErr CacheScrapData(ResType type, Handle data)
{
    CacheEntry *entry;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (!gMemoryState.enableCache || !data) {
        return paramErr;
    }

    /* Check if already cached */
    entry = FindCacheEntry(type);
    if (entry) {
        /* Update existing cache entry */
        if (entry->dataHandle && entry->dataHandle != data) {
            DisposeHandle(entry->dataHandle);
        }
        entry->dataHandle = data;
        entry->size = GetHandleSize(data);
        entry->lastAccess = time(NULL);
        entry->accessCount++;
        entry->isDirty = false;
        return noErr;
    }

    /* Add new cache entry */
    return AddToCacheEntry(type, data);
}

Handle GetCachedScrapData(ResType type)
{
    CacheEntry *entry;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    if (!gMemoryState.enableCache) {
        return NULL;
    }

    entry = FindCacheEntry(type);
    if (entry && entry->dataHandle) {
        entry->lastAccess = time(NULL);
        entry->accessCount++;
        gMemoryState.cacheHits++;
        return entry->dataHandle;
    }

    gMemoryState.cacheMisses++;
    return NULL;
}

OSErr InvalidateCachedData(ResType type)
{
    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    return RemoveFromCache(type);
}

/*
 * Memory Statistics Functions
 */

OSErr GetMemoryStatistics(SInt32 *blocksAllocated, SInt32 *totalMemory,
                         SInt32 *cacheHits, SInt32 *cacheMisses)
{
    MemoryBlock *block;
    SInt32 blockCount = 0;

    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    /* Count blocks */
    block = gMemoryState.blockList;
    while (block) {
        blockCount++;
        block = block->next;
    }

    if (blocksAllocated) *blocksAllocated = blockCount;
    if (totalMemory) *totalMemory = gMemoryState.totalAllocated;
    if (cacheHits) *cacheHits = gMemoryState.cacheHits;
    if (cacheMisses) *cacheMisses = gMemoryState.cacheMisses;

    return noErr;
}

void ResetMemoryStatistics(void)
{
    if (!gMemoryInitialized) {
        InitializeMemoryManager();
    }

    gMemoryState.cacheHits = 0;
    gMemoryState.cacheMisses = 0;
}

/*
 * Internal Helper Functions
 */

static OSErr InitializeMemoryManager(void)
{
    if (gMemoryInitialized) {
        return noErr;
    }

    memset(&gMemoryState, 0, sizeof(gMemoryState));

    gMemoryState.maxMemoryUsage = 2 * 1024 * 1024; /* 2MB default */
    gMemoryState.memoryThreshold = 32 * 1024;      /* 32KB threshold */
    gMemoryState.autoCompact = true;
    gMemoryState.enableCache = true;

    gMemoryInitialized = true;
    return noErr;
}

static MemoryBlock *AllocateMemoryBlock(SInt32 size, ResType dataType)
{
    MemoryBlock *block = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    if (!block) {
        return NULL;
    }

    memset(block, 0, sizeof(MemoryBlock));
    block->size = size;
    block->refCount = 1;
    block->lastAccess = time(NULL);
    block->dataType = dataType;
    block->isPurgeable = true;

    /* Add to list */
    block->next = gMemoryState.blockList;
    gMemoryState.blockList = block;

    return block;
}

static void FreeMemoryBlock(MemoryBlock *block)
{
    MemoryBlock *current, *prev = NULL;

    if (!block) {
        return;
    }

    /* Remove from list */
    current = gMemoryState.blockList;
    while (current) {
        if (current == block) {
            if (prev) {
                prev->next = current->next;
            } else {
                gMemoryState.blockList = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }

    free(block);
}

static MemoryBlock *FindMemoryBlock(void *ptr)
{
    MemoryBlock *block = gMemoryState.blockList;

    while (block) {
        if (block->ptr == ptr) {
            return block;
        }
        block = block->next;
    }

    return NULL;
}

static OSErr CompactMemoryBlocks(void)
{
    MemoryBlock *block = gMemoryState.blockList;
    MemoryBlock *next;

    /* Remove unused blocks */
    while (block) {
        next = block->next;

        if (block->refCount == 0 && block->isPurgeable &&
            (time(NULL) - block->lastAccess) > 60) { /* 1 minute old */

            if (block->ptr) {
                DisposeHandle((Handle)block->ptr);
                gMemoryState.totalAllocated -= block->size;
            }

            FreeMemoryBlock(block);
        }

        block = next;
    }

    return noErr;
}

static OSErr PurgeOldestBlocks(SInt32 bytesToFree)
{
    MemoryBlock *oldest = NULL;
    MemoryBlock *block;
    SInt32 freedBytes = 0;

    while (freedBytes < bytesToFree) {
        oldest = NULL;

        /* Find oldest purgeable block */
        block = gMemoryState.blockList;
        while (block) {
            if (block->refCount == 0 && block->isPurgeable) {
                if (!oldest || block->lastAccess < oldest->lastAccess) {
                    oldest = block;
                }
            }
            block = block->next;
        }

        if (!oldest) {
            break; /* No more purgeable blocks */
        }

        /* Purge the oldest block */
        if (oldest->ptr) {
            DisposeHandle((Handle)oldest->ptr);
            freedBytes += oldest->size;
            gMemoryState.totalAllocated -= oldest->size;
        }

        FreeMemoryBlock(oldest);
    }

    return (freedBytes >= bytesToFree) ? noErr : memFullErr;
}

static UInt32 CalculateChecksum(const void *data, SInt32 size)
{
    const UInt8 *bytes = (const UInt8 *)data;
    UInt32 checksum = 0;
    SInt32 i;

    for (i = 0; i < size; i++) {
        checksum = (checksum << 1) ^ bytes[i];
    }

    return checksum;
}

static OSErr ValidateMemoryBlock(MemoryBlock *block)
{
    if (!block || !block->ptr) {
        return paramErr;
    }

    /* Validate handle */
    Handle h = (Handle)block->ptr;
    if (GetHandleSize(h) != block->size) {
        return scrapCorruptError;
    }

    return noErr;
}

static CacheEntry *FindCacheEntry(ResType type)
{
    SInt16 i;

    for (i = 0; i < gMemoryState.cacheCount; i++) {
        if (gMemoryState.cache[i].type == type) {
            return &gMemoryState.cache[i];
        }
    }

    return NULL;
}

static OSErr AddToCacheEntry(ResType type, Handle data)
{
    CacheEntry *entry;

    if (gMemoryState.cacheCount >= 32) {
        /* Cache is full - remove oldest entry */
        time_t oldestTime = time(NULL);
        SInt16 oldestIndex = 0;
        SInt16 i;

        for (i = 0; i < gMemoryState.cacheCount; i++) {
            if (gMemoryState.cache[i].lastAccess < oldestTime) {
                oldestTime = gMemoryState.cache[i].lastAccess;
                oldestIndex = i;
            }
        }

        RemoveFromCache(gMemoryState.cache[oldestIndex].type);
    }

    entry = &gMemoryState.cache[gMemoryState.cacheCount];
    entry->type = type;
    entry->dataHandle = data;
    entry->size = GetHandleSize(data);
    entry->createTime = time(NULL);
    entry->lastAccess = entry->createTime;
    entry->accessCount = 1;
    entry->isDirty = false;

    gMemoryState.cacheCount++;
    return noErr;
}

static OSErr RemoveFromCache(ResType type)
{
    SInt16 i, j;

    for (i = 0; i < gMemoryState.cacheCount; i++) {
        if (gMemoryState.cache[i].type == type) {
            /* Dispose handle if we own it */
            if (gMemoryState.cache[i].dataHandle) {
                /* Don't dispose - caller owns the handle */
            }

            /* Shift remaining entries down */
            for (j = i; j < gMemoryState.cacheCount - 1; j++) {
                gMemoryState.cache[j] = gMemoryState.cache[j + 1];
            }

            gMemoryState.cacheCount--;
            return noErr;
        }
    }

    return scrapNoTypeError;
}

static OSErr PurgeCache(SInt32 maxAge)
{
    time_t cutoffTime = time(NULL) - maxAge;
    SInt16 i;

    for (i = gMemoryState.cacheCount - 1; i >= 0; i--) {
        if (gMemoryState.cache[i].lastAccess < cutoffTime) {
            RemoveFromCache(gMemoryState.cache[i].type);
        }
    }

    return noErr;
}
