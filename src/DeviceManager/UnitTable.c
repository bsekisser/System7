#include "MemoryMgr/MemoryManager.h"
#include "SuperCompat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * UnitTable.c
 * System 7.1 Device Manager - Unit Table Management Implementation
 *
 * Implements the unit table that maps driver reference numbers to
 * Device Control Entries (DCEs). This is the core data structure
 * for device driver management in System 7.1.
 *
 */

#include "CompatibilityFix.h"
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "DeviceManager/UnitTable.h"
#include "DeviceManager/DeviceTypes.h"
#include "MemoryMgr/memory_manager_types.h"


/* =============================================================================
 * Global Variables
 * ============================================================================= */

static UnitTable *g_unitTable = NULL;
static Boolean g_unitTableInitialized = false;

/* =============================================================================
 * Internal Function Declarations
 * ============================================================================= */

static SInt16 ExpandTable(SInt16 newSize);
static SInt16 RebuildHashTable(void);
static UnitTableEntryPtr FindEntryByRefNum(SInt16 refNum);
static UnitTableEntryPtr FindEntryByName(const UInt8 *driverName);
static UInt32 HashRefNum(SInt16 refNum);
static UInt32 HashDriverName(const UInt8 *driverName);
static SInt16 AllocateEntry(SInt16 preferredRefNum);
static void DeallocateEntry(UnitTableEntryPtr entry);
static Boolean ValidateEntry(UnitTableEntryPtr entry);
static void UpdateAccessTime(UnitTableEntryPtr entry);

/* =============================================================================
 * Unit Table Initialization and Shutdown
 * ============================================================================= */

SInt16 UnitTable_Initialize(SInt16 initialSize)
{
    if (g_unitTableInitialized) {
        return noErr; /* Already initialized */
    }

    if (initialSize <= 0 || initialSize > UNIT_TABLE_MAX_SIZE) {
        initialSize = UNIT_TABLE_INITIAL_SIZE;
    }

    /* Allocate unit table structure */
    g_unitTable = (UnitTable*)NewPtr(sizeof(UnitTable));
    if (g_unitTable == NULL) {
        return memFullErr;
    }

    memset(g_unitTable, 0, sizeof(UnitTable));

    /* Allocate entry array */
    g_unitTable->entries = (UnitTableEntryPtr*)NewPtr(sizeof(UnitTableEntryPtr) * initialSize);
    if (g_unitTable->entries == NULL) {
        DisposePtr((Ptr)g_unitTable);
        g_unitTable = NULL;
        return memFullErr;
    }

    memset(g_unitTable->entries, 0, sizeof(UnitTableEntryPtr) * initialSize);

    /* Initialize hash table */
    g_unitTable->hashSize = initialSize * 2; /* Hash table twice the size for better distribution */
    g_unitTable->hashTable = (UnitTableEntryPtr*)NewPtr(sizeof(UnitTableEntryPtr) * g_unitTable->hashSize);
    if (g_unitTable->hashTable == NULL) {
        DisposePtr((Ptr)g_unitTable->entries);
        DisposePtr((Ptr)g_unitTable);
        g_unitTable = NULL;
        return memFullErr;
    }

    memset(g_unitTable->hashTable, 0, sizeof(UnitTableEntryPtr) * g_unitTable->hashSize);

    /* Initialize table fields */
    g_unitTable->size = initialSize;
    g_unitTable->count = 0;
    g_unitTable->maxSize = UNIT_TABLE_MAX_SIZE;
    g_unitTable->nextFreeIndex = 0;
    g_unitTable->isLocked = false;
    g_unitTable->lockCount = 0;

    /* Reset statistics */
    g_unitTable->lookups = 0;
    g_unitTable->collisions = 0;
    g_unitTable->allocations = 0;
    g_unitTable->deallocations = 0;

    g_unitTableInitialized = true;
    return noErr;
}

void UnitTable_Shutdown(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return;
    }

    UnitTable_Lock();

    /* Deallocate all entries */
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        if (g_unitTable->entries[i] != NULL) {
            DeallocateEntry(g_unitTable->entries[i]);
            g_unitTable->entries[i] = NULL;
        }
    }

    /* Free table structures */
    if (g_unitTable->entries != NULL) {
        DisposePtr((Ptr)g_unitTable->entries);
        g_unitTable->entries = NULL;
    }

    if (g_unitTable->hashTable != NULL) {
        DisposePtr((Ptr)g_unitTable->hashTable);
        g_unitTable->hashTable = NULL;
    }

    DisposePtr((Ptr)g_unitTable);
    g_unitTable = NULL;

    g_unitTableInitialized = false;
}

UnitTablePtr UnitTable_GetInstance(void)
{
    return g_unitTable;
}

/* =============================================================================
 * Entry Allocation and Deallocation
 * ============================================================================= */

SInt16 UnitTable_AllocateEntry(SInt16 preferredRefNum)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    UnitTable_Lock();

    SInt16 refNum = AllocateEntry(preferredRefNum);

    UnitTable_Unlock();

    return refNum;
}

SInt16 UnitTable_DeallocateEntry(SInt16 refNum)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    if (!UnitTable_IsValidRefNum(refNum)) {
        return badUnitErr;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByRefNum(refNum);
    if (entry == NULL) {
        UnitTable_Unlock();
        return badUnitErr;
    }

    /* Remove from hash table */
    UInt32 hashIndex = HashRefNum(refNum) % g_unitTable->hashSize;
    UnitTableEntryPtr *hashChain = &g_unitTable->hashTable[hashIndex];

    while (*hashChain != NULL) {
        if (*hashChain == entry) {
            *hashChain = entry->next;
            break;
        }
        hashChain = &(*hashChain)->next;
    }

    /* Remove from main table */
    SInt16 tableIndex = RefNumToIndex(refNum);
    if (tableIndex >= 0 && tableIndex < g_unitTable->size) {
        g_unitTable->entries[tableIndex] = NULL;
    }

    DeallocateEntry(entry);
    g_unitTable->count--;
    g_unitTable->deallocations++;

    UnitTable_Unlock();

    return noErr;
}

static SInt16 AllocateEntry(SInt16 preferredRefNum)
{
    SInt16 refNum;

    if (preferredRefNum != 0 && UnitTable_IsValidRefNum(preferredRefNum)) {
        /* Check if preferred ref num is available */
        if (!UnitTable_IsRefNumInUse(preferredRefNum)) {
            refNum = preferredRefNum;
        } else {
            return unitEmptyErr; /* Already in use */
        }
    } else {
        /* Find next available reference number */
        refNum = UnitTable_GetNextAvailableRefNum();
        if (refNum < 0) {
            return refNum; /* Error code */
        }
    }

    /* Check if we need to expand the table */
    SInt16 tableIndex = RefNumToIndex(refNum);
    if (tableIndex >= g_unitTable->size) {
        SInt16 error = ExpandTable(tableIndex + UNIT_TABLE_GROWTH_INCREMENT);
        if (error != noErr) {
            return error;
        }
    }

    /* Allocate entry structure */
    UnitTableEntryPtr entry = (UnitTableEntryPtr)NewPtr(sizeof(UnitTableEntry));
    if (entry == NULL) {
        return memFullErr;
    }

    memset(entry, 0, sizeof(UnitTableEntry));

    /* Initialize entry */
    entry->refNum = refNum;
    entry->dceHandle = NULL;
    entry->flags = kUTEntryInUse;
    entry->lastAccess = 0; /* GetCurrentTicks() in real implementation */
    entry->openCount = 0;
    entry->next = NULL;

    /* Add to main table */
    g_unitTable->entries[tableIndex] = entry;

    /* Add to hash table */
    UInt32 hashIndex = HashRefNum(refNum) % g_unitTable->hashSize;
    entry->next = g_unitTable->hashTable[hashIndex];
    g_unitTable->hashTable[hashIndex] = entry;

    g_unitTable->count++;
    g_unitTable->allocations++;

    return refNum;
}

static void DeallocateEntry(UnitTableEntryPtr entry)
{
    if (entry != NULL) {
        /* Note: DCE handle should be disposed by caller */
        DisposePtr((Ptr)entry);
    }
}

/* =============================================================================
 * Entry Access Functions
 * ============================================================================= */

UnitTableEntryPtr UnitTable_GetEntry(SInt16 refNum)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return NULL;
    }

    if (!UnitTable_IsValidRefNum(refNum)) {
        return NULL;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByRefNum(refNum);
    if (entry != NULL) {
        UpdateAccessTime(entry);
        g_unitTable->lookups++;
    }

    UnitTable_Unlock();

    return entry;
}

SInt16 UnitTable_SetDCE(SInt16 refNum, DCEHandle dceHandle)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByRefNum(refNum);
    if (entry == NULL) {
        UnitTable_Unlock();
        return badUnitErr;
    }

    entry->dceHandle = dceHandle;
    UpdateAccessTime(entry);

    UnitTable_Unlock();

    return noErr;
}

DCEHandle UnitTable_GetDCE(SInt16 refNum)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return NULL;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByRefNum(refNum);
    DCEHandle dceHandle = NULL;

    if (entry != NULL) {
        dceHandle = entry->dceHandle;
        UpdateAccessTime(entry);
        g_unitTable->lookups++;
    }

    UnitTable_Unlock();

    return dceHandle;
}

/* =============================================================================
 * Search Functions
 * ============================================================================= */

SInt16 UnitTable_FindByName(const UInt8 *driverName)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || driverName == NULL) {
        return badUnitErr;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByName(driverName);
    SInt16 refNum = badUnitErr;

    if (entry != NULL) {
        refNum = entry->refNum;
        UpdateAccessTime(entry);
        g_unitTable->lookups++;
    }

    UnitTable_Unlock();

    return refNum;
}

SInt16 UnitTable_FindByDCE(DCEHandle dceHandle)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || dceHandle == NULL) {
        return badUnitErr;
    }

    UnitTable_Lock();

    SInt16 refNum = badUnitErr;

    /* Linear search through all entries */
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && entry->dceHandle == dceHandle) {
            refNum = entry->refNum;
            UpdateAccessTime(entry);
            g_unitTable->lookups++;
            break;
        }
    }

    UnitTable_Unlock();

    return refNum;
}

SInt16 UnitTable_Enumerate(Boolean (*callback)(SInt16 refNum, UnitTableEntryPtr entry, void *context),
                           void *context)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || callback == NULL) {
        return 0;
    }

    UnitTable_Lock();

    SInt16 count = 0;

    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && (entry->flags & kUTEntryInUse)) {
            Boolean shouldContinue = callback(entry->refNum, entry, context);
            count++;
            if (!shouldContinue) {
                break;
            }
        }
    }

    UnitTable_Unlock();

    return count;
}

SInt16 UnitTable_GetActiveRefNums(SInt16 *refNums, SInt16 maxCount)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || refNums == NULL || maxCount <= 0) {
        return 0;
    }

    UnitTable_Lock();

    SInt16 count = 0;

    for (SInt16 i = 0; i < g_unitTable->size && count < maxCount; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && (entry->flags & kUTEntryInUse)) {
            refNums[count++] = entry->refNum;
        }
    }

    UnitTable_Unlock();

    return count;
}

/* =============================================================================
 * Table Maintenance
 * ============================================================================= */

SInt16 UnitTable_Expand(SInt16 newSize)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    if (newSize <= g_unitTable->size || newSize > g_unitTable->maxSize) {
        return paramErr;
    }

    return ExpandTable(newSize);
}

static SInt16 ExpandTable(SInt16 newSize)
{
    /* Reallocate entry array */
    Size oldSize = g_unitTable->size * sizeof(UnitTableEntryPtr);
    UnitTableEntryPtr *newEntries = (UnitTableEntryPtr*)NewPtr(sizeof(UnitTableEntryPtr) * newSize);
    if (newEntries == NULL) {
        return memFullErr;
    }

    /* Copy old entries */
    if (g_unitTable->entries) {
        BlockMove(g_unitTable->entries, newEntries, oldSize);
        DisposePtr((Ptr)g_unitTable->entries);
    }

    /* Clear new entries */
    for (SInt16 i = g_unitTable->size; i < newSize; i++) {
        newEntries[i] = NULL;
    }

    g_unitTable->entries = newEntries;
    g_unitTable->size = newSize;

    /* Rebuild hash table if needed */
    if (g_unitTable->count > g_unitTable->hashSize / 2) {
        SInt16 error = RebuildHashTable();
        if (error != noErr) {
            return error;
        }
    }

    return noErr;
}

SInt16 UnitTable_Compact(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return 0;
    }

    UnitTable_Lock();

    SInt16 removed = 0;

    /* Remove unused entries */
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && !(entry->flags & kUTEntryInUse)) {
            g_unitTable->entries[i] = NULL;
            DeallocateEntry(entry);
            removed++;
        }
    }

    /* Rebuild hash table to remove dead references */
    RebuildHashTable();

    UnitTable_Unlock();

    return removed;
}

Boolean UnitTable_Validate(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return false;
    }

    UnitTable_Lock();

    Boolean isValid = true;

    /* Check table structure */
    if (g_unitTable->size <= 0 || g_unitTable->count < 0 ||
        g_unitTable->count > g_unitTable->size) {
        isValid = false;
    }

    /* Validate entries */
    if (isValid) {
        for (SInt16 i = 0; i < g_unitTable->size; i++) {
            UnitTableEntryPtr entry = g_unitTable->entries[i];
            if (entry != NULL && !ValidateEntry(entry)) {
                isValid = false;
                break;
            }
        }
    }

    UnitTable_Unlock();

    return isValid;
}

static SInt16 RebuildHashTable(void)
{
    /* Clear hash table */
    memset(g_unitTable->hashTable, 0, sizeof(UnitTableEntryPtr) * g_unitTable->hashSize);

    /* Rebuild hash chains */
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && (entry->flags & kUTEntryInUse)) {
            UInt32 hashIndex = HashRefNum(entry->refNum) % g_unitTable->hashSize;
            entry->next = g_unitTable->hashTable[hashIndex];
            g_unitTable->hashTable[hashIndex] = entry;
        }
    }

    return noErr;
}

SInt16 UnitTable_RebuildHash(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    UnitTable_Lock();
    SInt16 result = RebuildHashTable();
    UnitTable_Unlock();

    return result;
}

/* =============================================================================
 * Locking Functions
 * ============================================================================= */

void UnitTable_Lock(void)
{
    if (g_unitTable != NULL) {
        g_unitTable->isLocked = true;
        g_unitTable->lockCount++;
    }
}

void UnitTable_Unlock(void)
{
    if (g_unitTable != NULL && g_unitTable->lockCount > 0) {
        g_unitTable->lockCount--;
        if (g_unitTable->lockCount == 0) {
            g_unitTable->isLocked = false;
        }
    }
}

/* =============================================================================
 * Information and Statistics
 * ============================================================================= */

void UnitTable_GetStatistics(void *stats)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || stats == NULL) {
        return;
    }

    UnitTable_Lock();
    memcpy(stats, &g_unitTable->lookups, sizeof(UInt32) * 4); /* Copy statistics */
    UnitTable_Unlock();
}

void UnitTable_GetSizeInfo(SInt16 *size, SInt16 *count, SInt16 *maxSize)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        if (size) *size = 0;
        if (count) *count = 0;
        if (maxSize) *maxSize = 0;
        return;
    }

    UnitTable_Lock();

    if (size) *size = g_unitTable->size;
    if (count) *count = g_unitTable->count;
    if (maxSize) *maxSize = g_unitTable->maxSize;

    UnitTable_Unlock();
}

Boolean UnitTable_IsValidRefNum(SInt16 refNum)
{
    return (refNum >= MIN_DRIVER_REFNUM && refNum <= MAX_DRIVER_REFNUM);
}

Boolean UnitTable_IsRefNumInUse(SInt16 refNum)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return false;
    }

    UnitTable_Lock();

    UnitTableEntryPtr entry = FindEntryByRefNum(refNum);
    Boolean inUse = (entry != NULL && (entry->flags & kUTEntryInUse));

    UnitTable_Unlock();

    return inUse;
}

SInt16 UnitTable_GetNextAvailableRefNum(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return dsIOCoreErr;
    }

    UnitTable_Lock();

    /* Start from next free index and search for available refnum */
    for (SInt16 i = 1; i <= MAX_UNIT_TABLE_SIZE; i++) {
        SInt16 refNum = -i; /* Driver refnums are negative */
        if (!UnitTable_IsRefNumInUse(refNum)) {
            UnitTable_Unlock();
            return refNum;
        }
    }

    UnitTable_Unlock();

    return unitEmptyErr; /* No available reference numbers */
}

/* =============================================================================
 * Internal Helper Functions
 * ============================================================================= */

static UnitTableEntryPtr FindEntryByRefNum(SInt16 refNum)
{
    UInt32 hashIndex = HashRefNum(refNum) % g_unitTable->hashSize;
    UnitTableEntryPtr entry = g_unitTable->hashTable[hashIndex];

    while (entry != NULL) {
        if (entry->refNum == refNum) {
            return entry;
        }
        entry = entry->next;
        g_unitTable->collisions++;
    }

    return NULL;
}

static UnitTableEntryPtr FindEntryByName(const UInt8 *driverName)
{
    /* Linear search through all entries */
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL && entry->dceHandle != NULL) {
            DCEPtr dce = *(entry->dceHandle);
            if (dce != NULL && dce->dCtlDriver != NULL) {
                /* Compare driver names based on driver type */
                if (dce->dCtlFlags & FollowsNewRules_Mask) {
                    /* Modern driver - compare with interface name */
                    /* This would need implementation for modern drivers */
                } else {
                    /* Classic driver - compare Pascal strings */
                    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)dce->dCtlDriver;
                    if (drvrPtr != NULL) {
                        UInt8 nameLen = driverName[0];
                        if (nameLen == drvrPtr->drvrName[0] &&
                            memcmp(&driverName[1], &drvrPtr->drvrName[1], nameLen) == 0) {
                            return entry;
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

static UInt32 HashRefNum(SInt16 refNum)
{
    /* Simple hash function for reference numbers */
    return (UInt32)(refNum * 31 + 17);
}

static UInt32 HashDriverName(const UInt8 *driverName)
{
    if (driverName == NULL) {
        return 0;
    }

    UInt32 hash = 0;
    UInt8 nameLen = driverName[0];

    for (int i = 1; i <= nameLen; i++) {
        hash = hash * 31 + driverName[i];
    }

    return hash;
}

static Boolean ValidateEntry(UnitTableEntryPtr entry)
{
    if (entry == NULL) {
        return false;
    }

    /* Check basic fields */
    if (!UnitTable_IsValidRefNum(entry->refNum)) {
        return false;
    }

    if (!(entry->flags & kUTEntryInUse)) {
        return false;
    }

    return true;
}

static void UpdateAccessTime(UnitTableEntryPtr entry)
{
    if (entry != NULL) {
        entry->lastAccess = 0; /* GetCurrentTicks() in real implementation */
    }
}

/* =============================================================================
 * Reference Number Utilities
 * ============================================================================= */

SInt16 RefNumToIndex(SInt16 refNum)
{
    if (refNum < 0) {
        return (-refNum) - 1;
    }
    return -1; /* Invalid for positive numbers (file refnums) */
}

SInt16 IndexToRefNum(SInt16 index)
{
    if (index >= 0) {
        return -(index + 1);
    }
    return 0; /* Invalid */
}

Boolean IsDriverRefNum(SInt16 refNum)
{
    return UnitTable_IsValidRefNum(refNum);
}

Boolean IsFileRefNum(SInt16 refNum)
{
    return (refNum >= MIN_FILE_REFNUM && refNum <= MAX_FILE_REFNUM);
}

Boolean IsSystemDriverRefNum(SInt16 refNum)
{
    return (refNum >= SYSTEM_REFNUM_BASE && refNum <= MAX_DRIVER_REFNUM);
}

/* =============================================================================
 * Hash Utilities
 * ============================================================================= */

UInt32 ComputeRefNumHash(SInt16 refNum, SInt16 tableSize)
{
    return HashRefNum(refNum) % tableSize;
}

UInt32 ComputeNameHash(const UInt8 *driverName, SInt16 tableSize)
{
    return HashDriverName(driverName) % tableSize;
}

/* =============================================================================
 * Debug Functions
 * ============================================================================= */

void UnitTable_Dump(Boolean includeEmpty)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        printf("Unit table not initialized\n");
        return;
    }

    printf("Unit Table Dump:\n");
    printf("  Size: %d, Count: %d, Max: %d\n",
           g_unitTable->size, g_unitTable->count, g_unitTable->maxSize);
    printf("  Hash Size: %d\n", g_unitTable->hashSize);
    printf("  Statistics: Lookups=%u, Collisions=%u, Allocs=%u, Deallocs=%u\n",
           g_unitTable->lookups, g_unitTable->collisions,
           g_unitTable->allocations, g_unitTable->deallocations);

    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        UnitTableEntryPtr entry = g_unitTable->entries[i];
        if (entry != NULL || includeEmpty) {
            printf("  [%d]: ", i);
            if (entry != NULL) {
                printf("RefNum=%d, Flags=0x%X, DCE=%p\n",
                       entry->refNum, entry->flags, entry->dceHandle);
            } else {
                printf("(empty)\n");
            }
        }
    }
}

SInt16 UnitTable_VerifyConsistency(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL) {
        return 1;
    }

    SInt16 inconsistencies = 0;

    UnitTable_Lock();

    /* Check entry count */
    SInt16 actualCount = 0;
    for (SInt16 i = 0; i < g_unitTable->size; i++) {
        if (g_unitTable->entries[i] != NULL) {
            actualCount++;
        }
    }

    if (actualCount != g_unitTable->count) {
        inconsistencies++;
    }

    /* Check hash table consistency */
    for (SInt16 i = 0; i < g_unitTable->hashSize; i++) {
        UnitTableEntryPtr entry = g_unitTable->hashTable[i];
        while (entry != NULL) {
            /* Check if entry exists in main table */
            SInt16 tableIndex = RefNumToIndex(entry->refNum);
            if (tableIndex < 0 || tableIndex >= g_unitTable->size ||
                g_unitTable->entries[tableIndex] != entry) {
                inconsistencies++;
            }
            entry = entry->next;
        }
    }

    UnitTable_Unlock();

    return inconsistencies;
}

UInt32 UnitTable_GetLoadFactor(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || g_unitTable->hashSize == 0) {
        return 0;
    }

    return (g_unitTable->count * 100) / g_unitTable->hashSize;
}

UInt32 UnitTable_GetAvgChainLength(void)
{
    if (!g_unitTableInitialized || g_unitTable == NULL || g_unitTable->count == 0) {
        return 0;
    }

    UInt32 totalChainLength = 0;
    UInt32 usedSlots = 0;

    for (SInt16 i = 0; i < g_unitTable->hashSize; i++) {
        UnitTableEntryPtr entry = g_unitTable->hashTable[i];
        if (entry != NULL) {
            usedSlots++;
            UInt32 chainLength = 0;
            while (entry != NULL) {
                chainLength++;
                entry = entry->next;
            }
            totalChainLength += chainLength;
        }
    }

    return usedSlots > 0 ? totalChainLength / usedSlots : 0;
}
