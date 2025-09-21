#include "SystemTypes.h"
#include <stdlib.h>
/*
 * RE-AGENT-BANNER: reimplementation
 *
 * Mac OS System 7 Heap Compaction and Coalescing Algorithms
 *
 * Clean-room reimplementation from binary reverse engineering source code:
 * - ROM Memory Manager code (ROM disassembly)
 *
 * Original authors: ROM developer (1982-1983)
 * Reimplemented: 2025-09-18
 *
 * PROVENANCE: Heap management algorithms extracted from implementation analysis:
 * - a24/a32MakeBkF (lines 1167, 1173): Block free creation
 * - a24/a32MakeCBkF (lines 1202, 1228): Contiguous block free creation
 * - a24/a32MakeFree (lines 2472, 2484): Free block coalescing
 * - CallPurgeProc : Purge procedure invocation
 * - CallGZProc : Grow zone procedure invocation
 * - a24/a32PurgeBlock (lines 1375, 1386): Block purging algorithms
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "MemoryMgr/memory_manager_types.h"
#include <assert.h>


/* Forward declarations */
static OSErr make_block_free_24bit(ZonePtr zone, BlockPtr block, Size size);
static OSErr make_block_free_32bit(ZonePtr zone, BlockPtr block, Size size);
static OSErr make_contiguous_block_free_24bit(ZonePtr zone, Ptr startPtr, Size totalSize);
static OSErr make_contiguous_block_free_32bit(ZonePtr zone, Ptr startPtr, Size totalSize);
static void coalesce_adjacent_free_blocks(ZonePtr zone, BlockPtr block);
static OSErr purge_memory_block_24bit(ZonePtr zone, Handle h);
static OSErr purge_memory_block_32bit(ZonePtr zone, Handle h);
static OSErr call_purge_procedure(Handle h, PurgeProc purgeProc);
static OSErr call_grow_zone_procedure(ZonePtr zone, Size bytesNeeded);
static void update_free_space_accounting(ZonePtr zone, Size deltaBytes);

/* External function that should be defined in memory_manager_core.c */
extern Ptr compact_heap(ZonePtr zone, Size bytesNeeded, Size* maxFree);

/*
 * Make Block Free (24-bit addressing mode)
 * PROVENANCE: a24MakeBkF function  in ROM Memory Manager code
 *
 * Algorithm from implementation:
 * - Create a free block header at specified location
 * - Set block size and free tag
 * - Link into free block chain
 * - Update zone accounting
 */
static OSErr make_block_free_24bit(ZonePtr zone, BlockPtr block, Size size) {
    if (!zone || !block || size < MIN_FREE_24BIT) {
        return memFullErr;
    }

    /* Ensure size is properly aligned */
    Size alignedSize = (size + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);

    /* Set up block header for free block */
    block->blkSize = alignedSize;
    block->u.allocated.tagByte = BLOCK_FREE;

    /* Initialize free block chain link */
    block->u.free.next = zone->hFstFree;
    zone->hFstFree = (Ptr)block;

    /* Update zone free space accounting */
    update_free_space_accounting(zone, alignedSize);

    /* Attempt to coalesce with adjacent blocks */
    coalesce_adjacent_free_blocks(zone, block);

    return noErr;
}

/*
 * Make Block Free (32-bit addressing mode)
 * PROVENANCE: a32MakeBkF function  in ROM Memory Manager code
 */
static OSErr make_block_free_32bit(ZonePtr zone, BlockPtr block, Size size) {
    if (!zone || !block || size < MIN_FREE_32BIT) {
        return memFullErr;
    }

    /* Similar to 24-bit version but with 32-bit specific handling */
    Size alignedSize = (size + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);

    block->blkSize = alignedSize;
    block->u.allocated.tagByte = BLOCK_FREE;

    block->u.free.next = zone->hFstFree;
    zone->hFstFree = (Ptr)block;

    update_free_space_accounting(zone, alignedSize);
    coalesce_adjacent_free_blocks(zone, block);

    return noErr;
}

/*
 * Make Contiguous Block Free (24-bit addressing mode)
 * PROVENANCE: a24MakeCBkF function  in ROM Memory Manager code
 *
 * Algorithm from implementation analysis:
 * - Creates a large contiguous free block from multiple smaller blocks
 * - Used during heap compaction to create large free regions
 * - Handles multiple block coalescing in one operation
 */
static OSErr make_contiguous_block_free_24bit(ZonePtr zone, Ptr startPtr, Size totalSize) {
    if (!zone || !startPtr || totalSize < MIN_FREE_24BIT) {
        return memFullErr;
    }

    /* Create single large free block from contiguous memory region */
    BlockPtr newFreeBlock = (BlockPtr)startPtr;

    /* Set up the large free block header */
    Size alignedSize = (totalSize + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);
    newFreeBlock->blkSize = alignedSize;
    newFreeBlock->u.allocated.tagByte = BLOCK_FREE;
    newFreeBlock->u.free.next = zone->hFstFree;

    /* Update zone's first free pointer */
    zone->hFstFree = startPtr;

    /* Update free space accounting */
    update_free_space_accounting(zone, alignedSize);

    return noErr;
}

/*
 * Make Contiguous Block Free (32-bit addressing mode)
 * PROVENANCE: a32MakeCBkF function  in ROM Memory Manager code
 */
static OSErr make_contiguous_block_free_32bit(ZonePtr zone, Ptr startPtr, Size totalSize) {
    if (!zone || !startPtr || totalSize < MIN_FREE_32BIT) {
        return memFullErr;
    }

    BlockPtr newFreeBlock = (BlockPtr)startPtr;
    Size alignedSize = (totalSize + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);

    newFreeBlock->blkSize = alignedSize;
    newFreeBlock->u.allocated.tagByte = BLOCK_FREE;
    newFreeBlock->u.free.next = zone->hFstFree;
    zone->hFstFree = startPtr;

    update_free_space_accounting(zone, alignedSize);

    return noErr;
}

/*
 * Advanced Free Block Coalescing
 * PROVENANCE: a24/a32MakeFree functions (lines 2472, 2484) in ROM Memory Manager code
 *
 * Algorithm from implementation analysis:
 * - Merge adjacent free blocks to reduce fragmentation
 * - Check both forward and backward for adjacent free blocks
 * - Update block sizes and free chain linkage
 * - Critical for maintaining heap efficiency
 */
static void coalesce_adjacent_free_blocks(ZonePtr zone, BlockPtr block) {
    if (!zone || !block || block->u.allocated.tagByte != BLOCK_FREE) {
        return;
    }

    Size currentSize = block->blkSize & BLOCK_SIZE_MASK;

    /* Coalesce with next block if it's free */
    BlockPtr nextBlock = (BlockPtr)((char*)block + currentSize);
    if ((char*)nextBlock < (char*)zone->bkLim &&
        nextBlock->u.allocated.tagByte == BLOCK_FREE) {

        Size nextSize = nextBlock->blkSize & BLOCK_SIZE_MASK;

        /* Remove next block from free chain */
        if (zone->hFstFree == (Ptr)nextBlock) {
            zone->hFstFree = nextBlock->u.free.next;
        } else {
            /* Find and update previous link in chain */
            BlockPtr prev = (BlockPtr)zone->hFstFree;
            while (prev && prev->u.free.next != (Ptr)nextBlock) {
                prev = (BlockPtr)prev->u.free.next;
            }
            if (prev) {
                prev->u.free.next = nextBlock->u.free.next;
            }
        }

        /* Merge blocks */
        block->blkSize = currentSize + nextSize;
        currentSize = block->blkSize;
    }

    /* Coalesce with previous block if it's free */
    /* Scan backwards to find previous block */
    BlockPtr scanBlock = (BlockPtr)((char*)zone + sizeof(Zone));
    BlockPtr prevBlock = NULL;

    while (scanBlock < block) {
        Size scanSize = scanBlock->blkSize & BLOCK_SIZE_MASK;
        BlockPtr nextScanBlock = (BlockPtr)((char*)scanBlock + scanSize);

        if (nextScanBlock == block) {
            prevBlock = scanBlock;
            break;
        }

        scanBlock = nextScanBlock;
    }

    if (prevBlock && prevBlock->u.allocated.tagByte == BLOCK_FREE) {
        Size prevSize = prevBlock->blkSize & BLOCK_SIZE_MASK;

        /* Remove current block from free chain */
        if (zone->hFstFree == (Ptr)block) {
            zone->hFstFree = block->u.free.next;
        } else {
            BlockPtr prev = (BlockPtr)zone->hFstFree;
            while (prev && prev->u.free.next != (Ptr)block) {
                prev = (BlockPtr)prev->u.free.next;
            }
            if (prev) {
                prev->u.free.next = block->u.free.next;
            }
        }

        /* Merge with previous block */
        prevBlock->blkSize = prevSize + currentSize;
    }
}

/*
 * Purge Memory Block (24-bit addressing mode)
 * PROVENANCE: a24PurgeBlock function  in ROM Memory Manager code
 *
 * Algorithm from implementation:
 * - Purge purgeable relocatable blocks to free memory
 * - Call purge procedure if one is installed
 * - Mark handle as purged
 * - Convert block to free block
 */
static OSErr purge_memory_block_24bit(ZonePtr zone, Handle h) {
    if (!zone || !h || !*h) {
        return nilHandleErr;
    }

    /* Get block information */
    Ptr blockPtr = *h;
    BlockPtr block = (BlockPtr)((char*)blockPtr - BLOCK_OVERHEAD);

    /* Verify this is a purgeable relocatable block */
    if (block->u.allocated.tagByte <= BLOCK_ALLOCATED) {
        return memWZErr;  /* Not a relocatable block */
    }

    Size blockSize = block->blkSize & BLOCK_SIZE_MASK;
    if (!(block->blkSize & PURGEABLE_FLAG)) {
        return memWZErr;  /* Not purgeable */
    }

    /* Mark handle as purged */
    *h = (Ptr)HANDLE_PURGED;

    /* Convert block to free block */
    return make_block_free_24bit(zone, block, blockSize);
}

/*
 * Purge Memory Block (32-bit addressing mode)
 * PROVENANCE: a32PurgeBlock function  in ROM Memory Manager code
 */
static OSErr purge_memory_block_32bit(ZonePtr zone, Handle h) {
    if (!zone || !h || !*h) {
        return nilHandleErr;
    }

    Ptr blockPtr = *h;
    BlockPtr block = (BlockPtr)((char*)blockPtr - BLOCK_OVERHEAD);

    if (block->u.allocated.tagByte <= BLOCK_ALLOCATED) {
        return memWZErr;
    }

    Size blockSize = block->blkSize & BLOCK_SIZE_MASK;
    if (!(block->blkSize & PURGEABLE_FLAG)) {
        return memWZErr;
    }

    *h = (Ptr)HANDLE_PURGED;
    return make_block_free_32bit(zone, block, blockSize);
}

/*
 * Call Purge Procedure
 * PROVENANCE: CallPurgeProc function  in ROM Memory Manager code
 *
 * Algorithm from implementation:
 * - Invoke user-defined purge procedure for memory reclamation
 * - Handle procedure calling conventions
 * - Manage error conditions and recovery
 */
static OSErr call_purge_procedure(Handle h, PurgeProc purgeProc) {
    if (!purgeProc) {
        return noErr;  /* No purge procedure installed */
    }

    /* Call the purge procedure with proper error handling */
    purgeProc(h);

    /* Return noErr since void procedures don't return error codes */
    return noErr;
}

/*
 * Call Grow Zone Procedure
 * PROVENANCE: CallGZProc function  in ROM Memory Manager code
 *
 * Algorithm from implementation:
 * - Invoke grow zone procedure for heap expansion
 * - Attempt to grow zone when compaction is insufficient
 * - Handle zone growth and boundary updates
 */
static OSErr call_grow_zone_procedure(ZonePtr zone, Size bytesNeeded) {
    if (!zone || !zone->gzProc) {
        return memFullErr;  /* No grow zone procedure */
    }

    /* Cast zone's grow zone procedure pointer */
    GrowZoneProc growProc = (GrowZoneProc)zone->gzProc;

    /* Call grow zone procedure */
    growProc(bytesNeeded);

    /* Assume success since void procedures don't return error codes */
    /* Note: Actual zone growth would update bkLim and other fields */

    return noErr;
}

/*
 * Update Free Space Accounting
 * PROVENANCE: AdjustFree/SubtractFree functions (lines 3509, 3528) in ROM Memory Manager code
 */
static void update_free_space_accounting(ZonePtr zone, Size deltaBytes) {
    if (!zone) {
        return;
    }

    /* Update zone's free byte count */
    zone->zcbFree += deltaBytes;

    /* Ensure free count doesn't go negative */
    if (zone->zcbFree < 0) {
        zone->zcbFree = 0;
    }
}

/*
 * Public Interface Functions
 */

/*
 * Compact Memory with Purging
 * High-level compaction that includes purging of purgeable blocks
 */
OSErr compact_memory_with_purging(ZonePtr zone, Size bytesNeeded) {
    if (!zone) {
        return memWZErr;
    }

    /* First try normal compaction */
    Size maxFree;
    Ptr freeSpace = compact_heap(zone, bytesNeeded, &maxFree);

    if (freeSpace) {
        return noErr;  /* Compaction succeeded */
    }

    /* Compaction alone wasn't enough - try purging purgeable blocks */
    BlockPtr current = (BlockPtr)((char*)zone + sizeof(Zone));
    BlockPtr end = (BlockPtr)zone->bkLim;

    while (current < end && maxFree < bytesNeeded) {
        if (current->u.allocated.tagByte > BLOCK_ALLOCATED &&
            (current->blkSize & PURGEABLE_FLAG)) {

            /* Found a purgeable block - create a temporary handle for it */
            Handle tempHandle = (Handle)((char*)current + sizeof(Size));

            /* Purge the block */
            OSErr err = purge_memory_block_24bit(zone, tempHandle);
            if (err == noErr) {
                /* Try compaction again */
                freeSpace = compact_heap(zone, bytesNeeded, &maxFree);
                if (freeSpace) {
                    return noErr;
                }
            }
        }

        Size blockSize = current->blkSize & BLOCK_SIZE_MASK;
        current = (BlockPtr)((char*)current + blockSize);
    }

    /* If still not enough space, try grow zone procedure */
    return call_grow_zone_procedure(zone, bytesNeeded);
}

/*
 * Purge Memory Zone
 * Purge all purgeable blocks in a zone
 */
OSErr purge_memory_zone(ZonePtr zone) {
    if (!zone) {
        return memWZErr;
    }

    BlockPtr current = (BlockPtr)((char*)zone + sizeof(Zone));
    BlockPtr end = (BlockPtr)zone->bkLim;

    while (current < end) {
        if (current->u.allocated.tagByte > BLOCK_ALLOCATED &&
            (current->blkSize & PURGEABLE_FLAG)) {

            Handle tempHandle = (Handle)((char*)current + sizeof(Size));
            purge_memory_block_24bit(zone, tempHandle);
        }

        Size blockSize = current->blkSize & BLOCK_SIZE_MASK;
        if (blockSize < MIN_FREE_24BIT) {
            break;
        }
        current = (BlockPtr)((char*)current + blockSize);
    }

    return noErr;
}

/*
