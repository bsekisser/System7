/*
 * TimeManagerCore.c - Core scheduler with side-table
 * Based on Inside Macintosh: Operating System Utilities (Time Manager)
 */

#include "SystemTypes.h"
#include "TimeManager/TimeManager.h"
#include "TimeManager/TimeManagerPriv.h"
#include "TimeManager/TimeBase.h"

#define TM_MAX_TASKS 512
#define TM_FLAG_PERIODIC 0x0001

/* Private task entry with scheduling metadata */
typedef struct {
    TMTask   *task;
    UInt64    absDeadlineUS;
    UInt32    periodUS;
    UInt32    gen;
    UInt16    inHeap;
    UInt16    heapIndex;
} TMEntry;

/* Static allocation */
static TMEntry gEntries[TM_MAX_TASKS];
static TMEntry* gHeap[TM_MAX_TASKS];
static UInt32 gHeapSize = 0;
static UInt32 gGenCounter = 1;
static Boolean gInitialized = false;

/* Deferred queue interface */
extern void EnqueueDeferred(TMTask *task, UInt32 gen);

/* Interrupt masking stubs */
static inline UInt32 DisableInterrupts(void) { return 0; }
static inline void RestoreInterrupts(UInt32 state) { (void)state; }

/* Find entry by task pointer */
static TMEntry* FindEntry(TMTask *task) {
    for (UInt32 i = 0; i < TM_MAX_TASKS; i++) {
        if (gEntries[i].task == task) {
            return &gEntries[i];
        }
    }
    return 0;
}

/* Min-heap operations */
static void HeapSiftUp(UInt32 index) {
    while (index > 0) {
        UInt32 parent = (index - 1) / 2;
        if ((int64_t)(gHeap[parent]->absDeadlineUS - gHeap[index]->absDeadlineUS) <= 0) {
            break;
        }
        TMEntry* temp = gHeap[parent];
        gHeap[parent] = gHeap[index];
        gHeap[index] = temp;
        gHeap[parent]->heapIndex = parent;
        gHeap[index]->heapIndex = index;
        index = parent;
    }
}

static void HeapSiftDown(UInt32 index) {
    while (2 * index + 1 < gHeapSize) {
        UInt32 left = 2 * index + 1;
        UInt32 right = 2 * index + 2;
        UInt32 smallest = index;
        
        if ((int64_t)(gHeap[smallest]->absDeadlineUS - gHeap[left]->absDeadlineUS) > 0) {
            smallest = left;
        }
        if (right < gHeapSize && 
            (int64_t)(gHeap[smallest]->absDeadlineUS - gHeap[right]->absDeadlineUS) > 0) {
            smallest = right;
        }
        
        if (smallest == index) break;
        
        TMEntry* temp = gHeap[index];
        gHeap[index] = gHeap[smallest];
        gHeap[smallest] = temp;
        gHeap[index]->heapIndex = index;
        gHeap[smallest]->heapIndex = smallest;
        index = smallest;
    }
}

static void HeapPush(TMEntry* entry) {
    if (gHeapSize >= TM_MAX_TASKS) return;
    
    gHeap[gHeapSize] = entry;
    entry->heapIndex = gHeapSize;
    entry->inHeap = 1;
    gHeapSize++;
    HeapSiftUp(gHeapSize - 1);
}

static TMEntry* HeapPop(void) {
    if (gHeapSize == 0) return 0;
    
    TMEntry* result = gHeap[0];
    result->inHeap = 0;
    
    gHeapSize--;
    if (gHeapSize > 0) {
        gHeap[0] = gHeap[gHeapSize];
        gHeap[0]->heapIndex = 0;
        HeapSiftDown(0);
    }
    
    return result;
}

static void HeapRemove(TMEntry* entry) {
    if (!entry->inHeap) return;
    
    UInt32 index = entry->heapIndex;
    entry->inHeap = 0;
    
    gHeapSize--;
    if (gHeapSize > 0 && index < gHeapSize) {
        gHeap[index] = gHeap[gHeapSize];
        gHeap[index]->heapIndex = index;
        
        /* Fix heap property */
        if (index > 0 && (int64_t)(gHeap[(index-1)/2]->absDeadlineUS - 
                                    gHeap[index]->absDeadlineUS) > 0) {
            HeapSiftUp(index);
        } else {
            HeapSiftDown(index);
        }
    }
}

/* Rearm hardware timer for next deadline */
static void RearmNextInterrupt(void) {
    if (gHeapSize > 0) {
        ProgramNextTimerInterrupt(gHeap[0]->absDeadlineUS);
    } else {
        ProgramNextTimerInterrupt(0);
    }
}

/* Core API implementation */
OSErr Core_Initialize(void) {
    for (UInt32 i = 0; i < TM_MAX_TASKS; i++) {
        gEntries[i].task = 0;
        gEntries[i].inHeap = 0;
    }
    gHeapSize = 0;
    gGenCounter = 1;
    gInitialized = true;
    return noErr;
}

void Core_Shutdown(void) {
    gInitialized = false;
    ProgramNextTimerInterrupt(0);
}

OSErr Core_InsertTask(TMTask *task) {
    if (!gInitialized) return tmNotActive;
    if (!task) return tmParamErr;
    
    UInt32 irq = DisableInterrupts();
    
    /* Find free entry */
    TMEntry* entry = 0;
    for (UInt32 i = 0; i < TM_MAX_TASKS; i++) {
        if (gEntries[i].task == 0) {
            entry = &gEntries[i];
            break;
        }
    }
    
    if (!entry) {
        RestoreInterrupts(irq);
        return tmQueueFull;
    }
    
    entry->task = task;
    entry->absDeadlineUS = 0;
    entry->periodUS = 0;
    entry->gen = gGenCounter++;
    entry->inHeap = 0;
    
    RestoreInterrupts(irq);
    return noErr;
}

OSErr Core_RemoveTask(TMTask *task) {
    if (!gInitialized) return tmNotActive;
    if (!task) return tmParamErr;
    
    UInt32 irq = DisableInterrupts();
    
    TMEntry* entry = FindEntry(task);
    if (!entry) {
        RestoreInterrupts(irq);
        return tmNotActive;
    }
    
    if (entry->inHeap) {
        HeapRemove(entry);
        RearmNextInterrupt();
    }
    
    entry->task = 0;
    entry->gen++;
    
    RestoreInterrupts(irq);
    return noErr;
}

OSErr Core_PrimeTask(TMTask *task, UInt32 delayUS) {
    if (!gInitialized) return tmNotActive;
    if (!task) return tmParamErr;
    
    UInt32 irq = DisableInterrupts();
    
    TMEntry* entry = FindEntry(task);
    if (!entry) {
        RestoreInterrupts(irq);
        return tmNotActive;
    }
    
    /* Remove from heap if already scheduled */
    if (entry->inHeap) {
        HeapRemove(entry);
    }
    
    /* Compute deadline */
    UnsignedWide now;
    Microseconds(&now);
    UInt64 nowUS = ((UInt64)now.hi << 32) | now.lo;
    
    entry->absDeadlineUS = nowUS + delayUS;
    entry->periodUS = (task->qType & TM_FLAG_PERIODIC) ? delayUS : 0;
    entry->gen++;
    
    /* Insert into heap */
    HeapPush(entry);
    RearmNextInterrupt();
    
    RestoreInterrupts(irq);
    return noErr;
}

OSErr Core_CancelTask(TMTask *task) {
    if (!gInitialized) return tmNotActive;
    if (!task) return tmParamErr;
    
    UInt32 irq = DisableInterrupts();
    
    TMEntry* entry = FindEntry(task);
    if (!entry || !entry->inHeap) {
        RestoreInterrupts(irq);
        return tmNotActive;
    }
    
    HeapRemove(entry);
    entry->gen++;
    RearmNextInterrupt();
    
    RestoreInterrupts(irq);
    return noErr;
}

UInt32 Core_GetActiveCount(void) {
    return gHeapSize;
}

/* ISR callback - expire all due tasks */
void Core_ExpireDue(UInt64 nowUS) {
    UInt32 irq = DisableInterrupts();
    
    while (gHeapSize > 0) {
        TMEntry* entry = gHeap[0];
        
        /* Check if expired */
        if ((int64_t)(entry->absDeadlineUS - nowUS) > 0) {
            break;
        }
        
        /* Remove from heap */
        HeapPop();
        
        /* Enqueue callback */
        EnqueueDeferred(entry->task, entry->gen);
        
        /* Reschedule if periodic */
        if (entry->periodUS > 0) {
            entry->absDeadlineUS += entry->periodUS;
            entry->gen++;
            HeapPush(entry);
        }
    }
    
    RearmNextInterrupt();
    RestoreInterrupts(irq);
}
