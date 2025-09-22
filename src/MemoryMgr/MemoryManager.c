/* Classic Mac Memory Manager Implementation */
#include "../../include/MemoryMgr/MemoryManager.h"
#include <string.h>

/* Serial debug output */
extern void serial_printf(const char* fmt, ...);

/* Alignment */
#define ALIGN     8u
#define BLKHDR_SZ ((u32)sizeof(BlockHeader))

static inline u32 align_up(u32 n) {
    return (n + (ALIGN-1)) & ~(ALIGN-1);
}

/* Global zones */
static ZoneInfo gSystemZone;       /* System heap */
static ZoneInfo gAppZone;          /* Application heap */
static ZoneInfo* gCurrentZone = NULL;

/* Static memory for zones - 8MB total */
static u8 gSystemHeap[2 * 1024 * 1024];    /* 2MB system heap */
static u8 gAppHeap[6 * 1024 * 1024];       /* 6MB app heap */

/* Master pointer tables */
static void* gSystemMasters[1024];         /* 1024 system handles */
static void* gAppMasters[4096];            /* 4096 app handles */

/* ======================== Free List Management ======================== */

static FreeNode* block_to_freenode(BlockHeader* b) {
    return (FreeNode*)((u8*)b + BLKHDR_SZ);
}

static BlockHeader* freenode_to_block(FreeNode* n) {
    return (BlockHeader*)((u8*)n - BLKHDR_SZ);
}

static void freelist_insert(ZoneInfo* z, BlockHeader* b) {
    b->flags = BF_FREE;
    FreeNode* n = block_to_freenode(b);

    if (!z->freeHead) {
        z->freeHead = n;
        n->next = n->prev = n;
        return;
    }

    /* Insert sorted by address for easier coalescing */
    FreeNode* it = z->freeHead;
    while (it->next != z->freeHead && it->next < n) {
        it = it->next;
    }

    n->next = it->next;
    n->prev = it;
    it->next->prev = n;
    it->next = n;

    /* Update head if we're smallest */
    if (n < z->freeHead) {
        z->freeHead = n;
    }
}

static void freelist_remove(ZoneInfo* z, BlockHeader* b) {
    FreeNode* n = block_to_freenode(b);

    if (n->next == n) {
        z->freeHead = NULL;
        return;
    }

    n->prev->next = n->next;
    n->next->prev = n->prev;

    if (z->freeHead == n) {
        z->freeHead = n->next;
    }
}

/* ======================== Coalescing ======================== */

static BlockHeader* coalesce_forward(ZoneInfo* z, BlockHeader* b) {
    u8* end = (u8*)b + b->size;
    if (end >= z->limit) return b;

    BlockHeader* next = (BlockHeader*)end;
    if (next->flags & BF_FREE) {
        freelist_remove(z, next);
        b->size += next->size;

        /* Update next block's prevSize */
        u8* after = (u8*)b + b->size;
        if (after < z->limit) {
            BlockHeader* nxt = (BlockHeader*)after;
            nxt->prevSize = b->size;
        }
    }
    return b;
}

static BlockHeader* coalesce_backward(ZoneInfo* z, BlockHeader* b) {
    if (b->prevSize) {
        BlockHeader* prev = (BlockHeader*)((u8*)b - b->prevSize);
        if (prev->flags & BF_FREE) {
            freelist_remove(z, prev);
            prev->size += b->size;

            /* Update next block's prevSize */
            u8* after = (u8*)prev + prev->size;
            if (after < z->limit) {
                BlockHeader* nxt = (BlockHeader*)after;
                nxt->prevSize = prev->size;
            }
            b = prev;
        }
    }
    return b;
}

/* ======================== Zone Management ======================== */

void InitZone(ZoneInfo* zone, void* memory, u32 size, void** masterTable, u32 masterCount) {
    memset(zone, 0, sizeof(*zone));
    zone->base = (u8*)memory;
    zone->limit = zone->base + size;

    /* Create one big free block spanning the zone */
    BlockHeader* b = (BlockHeader*)zone->base;
    b->size = align_up(size);
    b->flags = BF_FREE;
    b->prevSize = 0;
    b->masterPtr = NULL;

    /* Free list points to this block */
    zone->freeHead = (FreeNode*)((u8*)b + BLKHDR_SZ);
    zone->freeHead->next = zone->freeHead->prev = zone->freeHead;
    zone->bytesFree = size - BLKHDR_SZ;
    zone->bytesUsed = BLKHDR_SZ;

    /* Master pointer table */
    zone->mpBase = masterTable;
    zone->mpCount = masterCount;
    zone->mpNextFree = 0;
    for (u32 i = 0; i < masterCount; i++) {
        zone->mpBase[i] = NULL;
    }
}

ZoneInfo* GetZone(void) {
    return gCurrentZone;
}

void SetZone(ZoneInfo* zone) {
    gCurrentZone = zone;
}

u32 FreeMem(void) {
    return gCurrentZone ? gCurrentZone->bytesFree : 0;
}

u32 MaxMem(void) {
    if (!gCurrentZone || !gCurrentZone->freeHead) return 0;

    u32 maxBlock = 0;
    FreeNode* it = gCurrentZone->freeHead;
    do {
        BlockHeader* b = freenode_to_block(it);
        if (b->size > maxBlock) {
            maxBlock = b->size;
        }
        it = it->next;
    } while (it != gCurrentZone->freeHead);

    return maxBlock > BLKHDR_SZ ? maxBlock - BLKHDR_SZ : 0;
}

/* ======================== Block Allocation ======================== */

static BlockHeader* find_fit(ZoneInfo* z, u32 need) {
    if (!z->freeHead) return NULL;

    FreeNode* it = z->freeHead;
    do {
        BlockHeader* b = freenode_to_block(it);
        if (b->size >= need) return b;
        it = it->next;
    } while (it != z->freeHead);

    return NULL;
}

static void split_block(ZoneInfo* z, BlockHeader* b, u32 need) {
    u32 remain = b->size - need;
    freelist_remove(z, b);

    if (remain >= BLKHDR_SZ + ALIGN) {
        /* Create a tail free block */
        BlockHeader* tail = (BlockHeader*)((u8*)b + need);
        tail->size = remain;
        tail->flags = BF_FREE;
        tail->prevSize = need;
        tail->masterPtr = NULL;

        /* Fix next block's prevSize */
        u8* after = (u8*)tail + tail->size;
        if (after < z->limit) {
            BlockHeader* nxt = (BlockHeader*)after;
            nxt->prevSize = tail->size;
        }

        /* Shrink b */
        b->size = need;
        b->flags = 0;

        freelist_insert(z, tail);
    } else {
        b->flags = 0;
    }
}

/* ======================== Ptr Operations ======================== */

void* NewPtr(u32 byteCount) {
    ZoneInfo* z = gCurrentZone;
    if (!z) return NULL;

    u32 need = align_up(byteCount + BLKHDR_SZ);
    BlockHeader* b = find_fit(z, need);

    if (!b) {
        /* Try compaction */
        if (CompactMem(need) < need) return NULL;
        b = find_fit(z, need);
        if (!b) return NULL;
    }

    split_block(z, b, need);
    b->flags |= BF_PTR;
    b->masterPtr = NULL;
    z->bytesUsed += b->size;
    z->bytesFree -= b->size;

    return (u8*)b + BLKHDR_SZ;
}

void* NewPtrClear(u32 byteCount) {
    void* p = NewPtr(byteCount);
    if (p) memset(p, 0, byteCount);
    return p;
}

void DisposePtr(void* p) {
    if (!p) return;

    ZoneInfo* z = gCurrentZone;
    if (!z) return;

    BlockHeader* b = (BlockHeader*)((u8*)p - BLKHDR_SZ);
    b->flags &= ~(BF_PTR);
    z->bytesUsed -= b->size;
    z->bytesFree += b->size;

    /* Coalesce and insert */
    b = coalesce_forward(z, b);
    b = coalesce_backward(z, b);
    freelist_insert(z, b);
}

u32 GetPtrSize(void* p) {
    if (!p) return 0;
    BlockHeader* b = (BlockHeader*)((u8*)p - BLKHDR_SZ);
    return b->size - BLKHDR_SZ;
}

/* ======================== Handle Operations ======================== */

static void** MP_Alloc(ZoneInfo* z) {
    /* Simple linear search for free master pointer */
    for (u32 i = 0; i < z->mpCount; i++) {
        if (z->mpBase[i] == NULL) {
            /* Reserve the slot */
            z->mpBase[i] = (void*)1;  /* Temporary marker */
            return &z->mpBase[i];
        }
    }
    return NULL;
}

static void MP_Free(ZoneInfo* z, void** mp) {
    if (mp >= z->mpBase && mp < z->mpBase + z->mpCount) {
        *mp = NULL;
    }
}

Handle NewHandle(u32 byteCount) {
    ZoneInfo* z = gCurrentZone;
    if (!z) return NULL;

    void** mp = MP_Alloc(z);
    if (!mp) return NULL;

    u32 need = align_up(byteCount + BLKHDR_SZ);
    BlockHeader* b = find_fit(z, need);

    if (!b) {
        /* Try compaction */
        if (CompactMem(need) < need) {
            MP_Free(z, mp);
            return NULL;
        }
        b = find_fit(z, need);
        if (!b) {
            MP_Free(z, mp);
            return NULL;
        }
    }

    split_block(z, b, need);
    b->flags |= BF_HANDLE;
    b->masterPtr = (Handle)mp;  /* Store backpointer */
    *mp = (u8*)b + BLKHDR_SZ;    /* Master pointer points to data */
    z->bytesUsed += b->size;
    z->bytesFree -= b->size;

    return (Handle)mp;
}

Handle NewHandleClear(u32 byteCount) {
    Handle h = NewHandle(byteCount);
    if (h && *h) memset(*h, 0, byteCount);
    return h;
}

void DisposeHandle(Handle h) {
    if (!h || !*h) {
        if (h) *h = NULL;
        return;
    }

    ZoneInfo* z = gCurrentZone;
    if (!z) return;

    u8* p = (u8*)*h;
    BlockHeader* b = (BlockHeader*)(p - BLKHDR_SZ);

    /* Clear master pointer */
    *h = NULL;

    b->flags &= ~(BF_HANDLE | BF_LOCKED | BF_PURGEABLE);
    b->masterPtr = NULL;
    z->bytesUsed -= b->size;
    z->bytesFree += b->size;

    b = coalesce_forward(z, b);
    b = coalesce_backward(z, b);
    freelist_insert(z, b);
}

void HLock(Handle h) {
    if (h && *h) {
        BlockHeader* b = (BlockHeader*)((u8*)*h - BLKHDR_SZ);
        b->flags |= BF_LOCKED;
    }
}

void HUnlock(Handle h) {
    if (h && *h) {
        BlockHeader* b = (BlockHeader*)((u8*)*h - BLKHDR_SZ);
        b->flags &= ~BF_LOCKED;
    }
}

void HPurge(Handle h) {
    if (!h || !*h) return;
    BlockHeader* b = (BlockHeader*)((u8*)*h - BLKHDR_SZ);
    b->flags |= BF_PURGEABLE;
}

void HNoPurge(Handle h) {
    if (!h || !*h) return;
    BlockHeader* b = (BlockHeader*)((u8*)*h - BLKHDR_SZ);
    b->flags &= ~BF_PURGEABLE;
}

u32 GetHandleSize(Handle h) {
    if (!h || !*h) return 0;
    BlockHeader* b = (BlockHeader*)((u8*)*h - BLKHDR_SZ);
    return b->size - BLKHDR_SZ;
}

/* ======================== Compaction ======================== */

u32 CompactMem(u32 cbNeeded) {
    ZoneInfo* z = gCurrentZone;
    if (!z) return 0;

    /* First, try purging */
    PurgeMem(cbNeeded);

    /* Then compact: move unlocked handles together */
    u8* scan = z->base;
    u8* dest = z->base;

    while (scan < z->limit) {
        BlockHeader* b = (BlockHeader*)scan;

        if (b->flags & BF_FREE) {
            scan += b->size;
            continue;
        }

        /* Can we move this block? */
        if ((b->flags & BF_HANDLE) && !(b->flags & BF_LOCKED)) {
            if (scan != dest) {
                /* Move the block */
                BlockHeader* d = (BlockHeader*)dest;
                u32 dataSize = b->size - BLKHDR_SZ;

                /* Copy block header and data */
                memcpy(d, b, b->size);

                /* Update master pointer */
                if (d->masterPtr && *(d->masterPtr)) {
                    *(d->masterPtr) = (u8*)d + BLKHDR_SZ;
                }

                /* Update prevSize of following block */
                u8* after = dest + d->size;
                if (after < z->limit) {
                    BlockHeader* nxt = (BlockHeader*)after;
                    nxt->prevSize = d->size;
                }

                dest += d->size;
                scan += b->size;
                continue;
            }
        }

        /* Non-movable or locked - skip any gap and continue */
        if (scan != dest && (scan - dest) >= BLKHDR_SZ + ALIGN) {
            /* Create free block in the gap */
            BlockHeader* fb = (BlockHeader*)dest;
            fb->size = scan - dest;
            fb->flags = BF_FREE;
            fb->prevSize = (dest > z->base) ?
                ((BlockHeader*)(dest - ((BlockHeader*)(scan))->prevSize))->size : 0;
            fb->masterPtr = NULL;
            freelist_insert(z, fb);
        }

        dest = scan + b->size;
        scan = dest;
    }

    /* Create trailing free block if needed */
    if (dest < z->limit) {
        BlockHeader* tail = (BlockHeader*)dest;
        tail->flags = BF_FREE;
        tail->size = z->limit - dest;
        tail->prevSize = dest > z->base ?
            ((BlockHeader*)(dest - BLKHDR_SZ))->size : 0;
        tail->masterPtr = NULL;
        freelist_insert(z, tail);
    }

    return MaxMem();
}

void PurgeMem(u32 cbNeeded) {
    ZoneInfo* z = gCurrentZone;
    if (!z) return;

    /* Walk heap looking for purgeable handles */
    u8* scan = z->base;
    while (scan < z->limit) {
        BlockHeader* b = (BlockHeader*)scan;

        if ((b->flags & BF_HANDLE) &&
            (b->flags & BF_PURGEABLE) &&
            !(b->flags & BF_LOCKED)) {

            /* Purge this handle */
            Handle h = b->masterPtr;
            if (h) {
                *h = NULL;  /* Clear master pointer */
            }

            /* Free the block */
            b->flags = BF_FREE;
            z->bytesUsed -= b->size;
            z->bytesFree += b->size;

            /* Coalesce */
            b = coalesce_forward(z, b);
            b = coalesce_backward(z, b);
            freelist_insert(z, b);

            /* Check if we have enough */
            if (MaxMem() >= cbNeeded) return;
        }

        scan += b->size;
    }
}

/* ======================== C Library Interface ======================== */

void* malloc(size_t size) {
    serial_printf("malloc: requested size %u\n", (u32)size);
    void* result = NewPtr((u32)size);
    serial_printf("malloc: returning %p\n", result);
    return result;
}

void free(void* ptr) {
    DisposePtr(ptr);
}

void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    /* Get old size */
    BlockHeader* b = (BlockHeader*)((u8*)ptr - BLKHDR_SZ);
    u32 oldSize = b->size - BLKHDR_SZ;

    /* Allocate new block */
    void* newPtr = malloc(size);
    if (!newPtr) return NULL;

    /* Copy data */
    memcpy(newPtr, ptr, (size < oldSize) ? size : oldSize);

    /* Free old block */
    free(ptr);

    return newPtr;
}

/* ======================== Initialization ======================== */

void InitMemoryManager(void) {
    serial_printf("MM: InitMemoryManager started\n");

    /* Initialize System Zone */
    InitZone(&gSystemZone, gSystemHeap, sizeof(gSystemHeap),
             gSystemMasters, sizeof(gSystemMasters)/sizeof(void*));
    /* strcpy not available in kernel */
    gSystemZone.name[0] = 'S'; gSystemZone.name[1] = 0;
    serial_printf("MM: System Zone initialized (%u KB)\n", sizeof(gSystemHeap) / 1024);

    /* Initialize Application Zone */
    InitZone(&gAppZone, gAppHeap, sizeof(gAppHeap),
             gAppMasters, sizeof(gAppMasters)/sizeof(void*));
    /* strcpy not available in kernel */
    gAppZone.name[0] = 'A'; gAppZone.name[1] = 0;
    serial_printf("MM: App Zone initialized (%u KB)\n", sizeof(gAppHeap) / 1024);

    /* Set current zone to app zone */
    gCurrentZone = &gAppZone;

    serial_printf("MM: Current zone set to App Zone\n");
    serial_printf("MM: Total memory: %u MB\n",
                  (sizeof(gSystemHeap) + sizeof(gAppHeap)) / (1024*1024));
    serial_printf("MM: Free memory: %u KB\n", FreeMem() / 1024);
    serial_printf("MM: InitMemoryManager complete\n");
}

/* ======================== Debug Support ======================== */

void CheckHeap(ZoneInfo* zone) {
    if (!zone) zone = gCurrentZone;
    if (!zone) return;

    u32 totalSize = 0;
    u32 freeSize = 0;
    u32 usedSize = 0;
    u32 blockCount = 0;

    u8* scan = zone->base;
    while (scan < zone->limit) {
        BlockHeader* b = (BlockHeader*)scan;
        blockCount++;

        if (b->flags & BF_FREE) {
            freeSize += b->size;
        } else {
            usedSize += b->size;
        }
        totalSize += b->size;

        scan += b->size;
    }

    serial_printf("Heap check: %u blocks, %u used, %u free, %u total\n",
                  blockCount, usedSize, freeSize, totalSize);
}

void DumpHeap(ZoneInfo* zone) {
    if (!zone) zone = gCurrentZone;
    if (!zone) return;

    serial_printf("=== Heap Dump: %s ===\n", zone->name);

    u8* scan = zone->base;
    while (scan < zone->limit) {
        BlockHeader* b = (BlockHeader*)scan;

        char* type = "????";
        if (b->flags & BF_FREE) type = "FREE";
        else if (b->flags & BF_PTR) type = "PTR ";
        else if (b->flags & BF_HANDLE) {
            if (b->flags & BF_LOCKED) {
                type = (b->flags & BF_PURGEABLE) ? "HLKP" : "HLOK";
            } else {
                type = (b->flags & BF_PURGEABLE) ? "HNDP" : "HNDL";
            }
        }

        serial_printf("  %08x: %s size=%5u prev=%5u",
                      (u32)scan, type, b->size, b->prevSize);

        if (b->flags & BF_HANDLE && b->masterPtr) {
            serial_printf(" mp=%08x", (u32)b->masterPtr);
            if (*b->masterPtr) {
                serial_printf(" *mp=%08x", (u32)*b->masterPtr);
            }
        }
        serial_printf("\n");

        scan += b->size;
        if (scan > zone->limit) {
            serial_printf("  ERROR: Block extends past zone limit!\n");
            break;
        }
    }

    serial_printf("=== End Heap Dump ===\n");
}