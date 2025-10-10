# Segregated Freelist Design Proposal

## Current Architecture

**Single circular doubly-linked list** sorted by address:
- All free blocks in one list regardless of size
- First-fit allocation algorithm
- O(n) search time where n = number of free blocks
- Address-sorted for efficient coalescing

## Problems with Current Design

1. **Poor cache locality** - List traversal jumps across memory
2. **O(n) allocation time** - Must search entire list to find fit
3. **Fragmentation** - Small and large blocks intermixed
4. **Contention** - All allocations compete for same list (if multi-threaded later)

## Proposed: Size-Class Segregated Lists

### Architecture

Multiple freelists organized by size class:

```
Size Classes (powers of 2):
- 0-64 bytes    → list[0]
- 65-128 bytes  → list[1]
- 129-256 bytes → list[2]
- 257-512 bytes → list[3]
- 513-1KB       → list[4]
- 1KB-2KB       → list[5]
- 2KB-4KB       → list[6]
- 4KB+          → list[7] (large blocks)
```

### Data Structures

```c
#define NUM_SIZE_CLASSES 8

typedef struct ZoneInfo {
    u8*         base;
    u8*         limit;

    /* Segregated freelists by size class */
    FreeNode*   freelists[NUM_SIZE_CLASSES];

    u32         bytesUsed;
    u32         bytesFree;

    /* Master pointer table */
    void**      mpBase;
    u32         mpCount;

    /* M68K mapping */
    UInt32      m68kBase;
    UInt32      m68kLimit;

    char        name[32];
} ZoneInfo;

/* Get size class index for a block size */
static inline u32 get_size_class(u32 size) {
    if (size <= 64) return 0;
    if (size <= 128) return 1;
    if (size <= 256) return 2;
    if (size <= 512) return 3;
    if (size <= 1024) return 4;
    if (size <= 2048) return 5;
    if (size <= 4096) return 6;
    return 7;  /* 4KB+ */
}
```

### Allocation Algorithm

```c
static BlockHeader* find_fit_segregated(ZoneInfo* z, u32 need) {
    u32 class = get_size_class(need);

    /* Try exact size class first */
    if (z->freelists[class]) {
        FreeNode* it = z->freelists[class];
        FreeNode* start = it;
        do {
            BlockHeader* b = freenode_to_block(it);
            if (b->size >= need) return b;
            it = it->next;
        } while (it != start);
    }

    /* Try larger size classes */
    for (u32 c = class + 1; c < NUM_SIZE_CLASSES; c++) {
        if (z->freelists[c]) {
            /* Return first block from larger class */
            return freenode_to_block(z->freelists[c]);
        }
    }

    return NULL;  /* No fit found */
}
```

### Insertion Algorithm

```c
static void freelist_insert_segregated(ZoneInfo* z, BlockHeader* b) {
    /* Validate block size */
    if (b->size < MIN_BLOCK_SIZE) return;

    u32 class = get_size_class(b->size);
    FreeNode* n = block_to_freenode(b);
    FreeNode** head = &z->freelists[class];

    b->flags = BF_FREE;

    if (!*head) {
        /* First block in this size class */
        *head = n;
        n->next = n->prev = n;
        return;
    }

    /* Insert at head (LIFO for better cache locality) */
    n->next = *head;
    n->prev = (*head)->prev;
    (*head)->prev->next = n;
    (*head)->prev = n;
    *head = n;  /* New head */
}
```

### Removal Algorithm

```c
static void freelist_remove_segregated(ZoneInfo* z, BlockHeader* b) {
    if (!b) return;

    u32 class = get_size_class(b->size);
    FreeNode* n = block_to_freenode(b);
    FreeNode** head = &z->freelists[class];

    if (!n || !n->next || !n->prev) {
        *head = NULL;
        return;
    }

    if (n->next == n) {
        /* Last node in class */
        *head = NULL;
        return;
    }

    n->prev->next = n->next;
    n->next->prev = n->prev;

    if (*head == n) {
        *head = n->next;
    }

    n->next = n->prev = NULL;
}
```

## Benefits

1. **O(1) best case allocation** - Direct hit to correct size class
2. **Better cache locality** - Similar-sized blocks clustered together
3. **Reduced fragmentation** - Size classes naturally group allocations
4. **Faster validation** - Can validate each list independently
5. **Scalability** - Can add more size classes as needed
6. **Stats per class** - Can track allocation patterns

## Tradeoffs

1. **More complex** - 8 lists instead of 1
2. **Slight memory overhead** - 8 head pointers (64 bytes on 64-bit)
3. **Coalescing complexity** - May need to move blocks between classes
4. **External fragmentation** - Small block in large class wastes potential

## Migration Strategy

### Phase 1: Dual Implementation
- Keep existing single-list code
- Add segregated list code alongside
- Compile-time flag to switch between them
- Test both in parallel

### Phase 2: Validation
- Extensive testing with segregated lists
- Compare performance metrics
- Stress test with window create/destroy cycles

### Phase 3: Migration
- Switch default to segregated lists
- Remove old single-list code after burn-in period

## Performance Expectations

### Current (Single List)
- Allocation: O(n) where n = number of free blocks
- Worst case: ~1000 iterations for fragmented heap
- Best case: O(1) if first block fits

### Segregated Lists
- Allocation: O(m) where m = blocks in size class
- Typical m << n (10-100x smaller)
- Best case: O(1) for exact size match
- Expected: 10-100x faster allocation

## Implementation Complexity

**Estimated effort:** 4-6 hours
- 2 hours: Core segregated list implementation
- 1 hour: Update coalescing logic
- 1 hour: Migration of allocation/deallocation
- 1-2 hours: Testing and validation

## Recommendation

**Implement segregated freelists** for better performance and maintainability.

The current bug fixes (minimum block size) have solved the immediate corruption issue, but segregated lists would provide:
- Better long-term performance
- Easier debugging (can inspect each size class)
- More predictable behavior
- Foundation for future optimizations

## Alternative: Hybrid Approach

Keep segregated lists for small blocks (0-4KB) and a single list for large blocks (4KB+):
- Most allocations are small (windows, controls, text)
- Large blocks are rare and benefit less from segregation
- Reduces complexity while keeping most benefits

```c
#define NUM_SMALL_CLASSES 7    /* 64B to 4KB */
FreeNode* smallLists[NUM_SMALL_CLASSES];
FreeNode* largeList;           /* 4KB+ */
```

This hybrid approach balances complexity vs. performance.
