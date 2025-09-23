#include "SuperCompat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * DeviceInterrupts.c
 * System 7.1 Device Manager - Device Interrupt Handling
 *
 * Implements device interrupt handling and simulation for the Device Manager.
 * This module provides interrupt-driven I/O capabilities and manages
 * completion routines for asynchronous operations.
 *
 * Based on the original System 7.1 DeviceMgr.a assembly source.
 */

#include "CompatibilityFix.h"
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceTypes.h"
#include "DeviceManager/DriverInterface.h"
#include "DeviceManager/UnitTable.h"
#include <signal.h>
#include <time.h>


/* =============================================================================
 * Constants and Types
 * ============================================================================= */

#define MAX_INTERRUPT_HANDLERS      64
#define INTERRUPT_STACK_SIZE        8192
#define MAX_COMPLETION_QUEUE        256
#define INTERRUPT_PRIORITY_LEVELS   8

typedef enum {
    kInterruptTypeDisk      = 1,
    kInterruptTypeNetwork   = 2,
    kInterruptTypeSerial    = 3,
    kInterruptTypeTimer     = 4,
    kInterruptTypeVIA       = 5,
    kInterruptTypeSCC       = 6,
    kInterruptTypeSCSI      = 7,
    kInterruptTypeCustom    = 8
} InterruptType;

typedef struct InterruptHandler {
    SInt16             refNum;          /* Driver reference number */
    InterruptType       type;            /* Interrupt type */
    UInt32            priority;        /* Interrupt priority */
    Boolean                isEnabled;       /* Handler enabled flag */
    UInt32            interruptCount;  /* Number of interrupts handled */
    void               *context;         /* Handler context */
    struct InterruptHandler *next;       /* Next handler in chain */
} InterruptHandler, *InterruptHandlerPtr;

typedef struct CompletionQueueEntry {
    IOParamPtr          pb;              /* Parameter block */
    SInt16             result;          /* Operation result */
    UInt32            timestamp;       /* Completion timestamp */
    Boolean                isValid;         /* Entry is valid */
} CompletionQueueEntry;

/* =============================================================================
 * Global Variables
 * ============================================================================= */

static InterruptHandlerPtr g_interruptHandlers[INTERRUPT_PRIORITY_LEVELS] = {NULL};
static CompletionQueueEntry g_completionQueue[MAX_COMPLETION_QUEUE];
static UInt32 g_completionQueueHead = 0;
static UInt32 g_completionQueueTail = 0;
static UInt32 g_completionQueueCount = 0;

static Boolean g_interruptsEnabled = false;
static Boolean g_interruptSystemInitialized = false;
static UInt32 g_interruptNestingLevel = 0;
static UInt32 g_totalInterrupts = 0;

/* Statistics */
static struct {
    UInt32 totalInterrupts;
    UInt32 handledInterrupts;
    UInt32 spuriousInterrupts;
    UInt32 completionRoutinesCalled;
    UInt32 queueOverflows;
    UInt32 maxNestingLevel;
} g_interruptStats = {0};

/* =============================================================================
 * Internal Function Declarations
 * ============================================================================= */

static SInt16 RegisterInterruptHandler(SInt16 refNum, InterruptType type, UInt32 priority);
static SInt16 UnregisterInterruptHandler(SInt16 refNum);
static void ProcessInterrupt(InterruptType type, UInt32 data);
static void CallInterruptHandler(InterruptHandlerPtr handler, UInt32 data);
static SInt16 QueueCompletion(IOParamPtr pb, SInt16 result);
static void ProcessCompletionQueue(void);
static void ExecuteCompletionRoutine(IOParamPtr pb);
static InterruptHandlerPtr FindInterruptHandler(SInt16 refNum);
static UInt32 GetCurrentTimestamp(void);
static void SignalHandler(int signum);

/* =============================================================================
 * Interrupt System Initialization
 * ============================================================================= */

SInt16 DeviceInterrupts_Initialize(void)
{
    if (g_interruptSystemInitialized) {
        return noErr;
    }

    /* Initialize interrupt handler chains */
    for (int i = 0; i < INTERRUPT_PRIORITY_LEVELS; i++) {
        g_interruptHandlers[i] = NULL;
    }

    /* Initialize completion queue */
    memset(g_completionQueue, 0, sizeof(g_completionQueue));
    g_completionQueueHead = 0;
    g_completionQueueTail = 0;
    g_completionQueueCount = 0;

    /* Reset statistics */
    memset(&g_interruptStats, 0, sizeof(g_interruptStats));

    /* Reset global state */
    g_interruptsEnabled = false;
    g_interruptNestingLevel = 0;
    g_totalInterrupts = 0;

    /* Install signal handlers for interrupt simulation */
#ifdef SIGALRM
    signal(SIGALRM, SignalHandler);
#endif
#ifdef SIGUSR1
    signal(SIGUSR1, SignalHandler);
#endif
#ifdef SIGUSR2
    signal(SIGUSR2, SignalHandler);
#endif

    g_interruptSystemInitialized = true;
    return noErr;
}

void DeviceInterrupts_Shutdown(void)
{
    if (!g_interruptSystemInitialized) {
        return;
    }

    /* Disable interrupts */
    g_interruptsEnabled = false;

    /* Unregister all interrupt handlers */
    for (int i = 0; i < INTERRUPT_PRIORITY_LEVELS; i++) {
        InterruptHandlerPtr handler = g_interruptHandlers[i];
        while (handler != NULL) {
            InterruptHandlerPtr next = handler->next;
            free(handler);
            handler = next;
        }
        g_interruptHandlers[i] = NULL;
    }

    /* Process any remaining completions */
    ProcessCompletionQueue();

    /* Restore signal handlers */
#ifdef SIGALRM
    signal(SIGALRM, SIG_DFL);
#endif
#ifdef SIGUSR1
    signal(SIGUSR1, SIG_DFL);
#endif
#ifdef SIGUSR2
    signal(SIGUSR2, SIG_DFL);
#endif

    g_interruptSystemInitialized = false;
}

void DeviceInterrupts_Enable(void)
{
    if (g_interruptSystemInitialized) {
        g_interruptsEnabled = true;
    }
}

void DeviceInterrupts_Disable(void)
{
    g_interruptsEnabled = false;
}

Boolean DeviceInterrupts_AreEnabled(void)
{
    return g_interruptsEnabled;
}

/* =============================================================================
 * Interrupt Handler Registration
 * ============================================================================= */

static SInt16 RegisterInterruptHandler(SInt16 refNum, InterruptType type, UInt32 priority)
{
    if (!g_interruptSystemInitialized) {
        return dsIOCoreErr;
    }

    if (!UnitTable_IsValidRefNum(refNum)) {
        return badUnitErr;
    }

    if (priority >= INTERRUPT_PRIORITY_LEVELS) {
        priority = INTERRUPT_PRIORITY_LEVELS - 1;
    }

    /* Check if handler already exists */
    InterruptHandlerPtr existing = FindInterruptHandler(refNum);
    if (existing != NULL) {
        return dupFNErr; /* Already registered */
    }

    /* Allocate new handler */
    InterruptHandlerPtr handler = (InterruptHandlerPtr)malloc(sizeof(InterruptHandler));
    if (handler == NULL) {
        return memFullErr;
    }

    /* Initialize handler */
    handler->refNum = refNum;
    handler->type = type;
    handler->priority = priority;
    handler->isEnabled = true;
    handler->interruptCount = 0;
    handler->context = NULL;

    /* Add to priority chain */
    handler->next = g_interruptHandlers[priority];
    g_interruptHandlers[priority] = handler;

    return noErr;
}

static SInt16 UnregisterInterruptHandler(SInt16 refNum)
{
    if (!g_interruptSystemInitialized) {
        return dsIOCoreErr;
    }

    /* Search all priority levels */
    for (int priority = 0; priority < INTERRUPT_PRIORITY_LEVELS; priority++) {
        InterruptHandlerPtr *current = &g_interruptHandlers[priority];

        while (*current != NULL) {
            if ((*current)->refNum == refNum) {
                InterruptHandlerPtr handler = *current;
                *current = handler->next;
                free(handler);
                return noErr;
            }
            current = &(*current)->next;
        }
    }

    return fnfErr; /* Handler not found */
}

static InterruptHandlerPtr FindInterruptHandler(SInt16 refNum)
{
    for (int priority = 0; priority < INTERRUPT_PRIORITY_LEVELS; priority++) {
        InterruptHandlerPtr handler = g_interruptHandlers[priority];
        while (handler != NULL) {
            if (handler->refNum == refNum) {
                return handler;
            }
            handler = handler->next;
        }
    }
    return NULL;
}

/* =============================================================================
 * Interrupt Processing
 * ============================================================================= */

SInt16 SimulateDeviceInterrupt(SInt16 refNum, UInt32 interruptType)
{
    if (!g_interruptSystemInitialized || !g_interruptsEnabled) {
        return noErr; /* Interrupts disabled */
    }

    /* Find the interrupt handler */
    InterruptHandlerPtr handler = FindInterruptHandler(refNum);
    if (handler == NULL) {
        g_interruptStats.spuriousInterrupts++;
        return fnfErr;
    }

    /* Process the interrupt */
    ProcessInterrupt((InterruptType)interruptType, 0);

    return noErr;
}

static void ProcessInterrupt(InterruptType type, UInt32 data)
{
    if (!g_interruptsEnabled) {
        return;
    }

    g_totalInterrupts++;
    g_interruptStats.totalInterrupts++;
    g_interruptNestingLevel++;

    if (g_interruptNestingLevel > g_interruptStats.maxNestingLevel) {
        g_interruptStats.maxNestingLevel = g_interruptNestingLevel;
    }

    /* Process handlers in priority order (highest first) */
    for (int priority = INTERRUPT_PRIORITY_LEVELS - 1; priority >= 0; priority--) {
        InterruptHandlerPtr handler = g_interruptHandlers[priority];

        while (handler != NULL) {
            if (handler->isEnabled && handler->type == type) {
                CallInterruptHandler(handler, data);
                g_interruptStats.handledInterrupts++;
            }
            handler = handler->next;
        }
    }

    g_interruptNestingLevel--;

    /* Process completion queue if we're at the top level */
    if (g_interruptNestingLevel == 0) {
        ProcessCompletionQueue();
    }
}

static void CallInterruptHandler(InterruptHandlerPtr handler, UInt32 data)
{
    if (handler == NULL) {
        return;
    }

    handler->interruptCount++;

    /* Get the DCE for this driver */
    DCEHandle dceHandle = UnitTable_GetDCE(handler->refNum);
    if (dceHandle == NULL) {
        return;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL || !(dce->dCtlFlags & Is_Active_Mask)) {
        return;
    }

    /* Check if there are pending I/O operations */
    IOParamPtr pb = DequeueIORequest(dce);
    if (pb != NULL) {
        /* Complete the I/O operation */
        SInt16 result = noErr; /* Simulate successful completion */

        /* Update actual count for read/write operations */
        if (pb->ioBuffer != NULL && pb->ioReqCount > 0) {
            pb->ioActCount = pb->ioReqCount; /* Simulate full transfer */
        }

        /* Queue completion */
        QueueCompletion(pb, result);
    }
}

/* =============================================================================
 * Completion Queue Management
 * ============================================================================= */

static SInt16 QueueCompletion(IOParamPtr pb, SInt16 result)
{
    if (pb == NULL) {
        return paramErr;
    }

    /* Check for queue overflow */
    if (g_completionQueueCount >= MAX_COMPLETION_QUEUE) {
        g_interruptStats.queueOverflows++;
        return queueOverflow;
    }

    /* Add to completion queue */
    CompletionQueueEntry *entry = &g_completionQueue[g_completionQueueTail];
    entry->pb = pb;
    entry->result = result;
    entry->timestamp = GetCurrentTimestamp();
    entry->isValid = true;

    g_completionQueueTail = (g_completionQueueTail + 1) % MAX_COMPLETION_QUEUE;
    g_completionQueueCount++;

    return noErr;
}

static void ProcessCompletionQueue(void)
{
    while (g_completionQueueCount > 0) {
        CompletionQueueEntry *entry = &g_completionQueue[g_completionQueueHead];

        if (entry->isValid && entry->pb != NULL) {
            /* Set result in parameter block */
            entry->pb->ioResult = entry->result;

            /* Execute completion routine */
            ExecuteCompletionRoutine(entry->pb);

            g_interruptStats.completionRoutinesCalled++;
        }

        /* Mark entry as processed */
        entry->isValid = false;
        entry->pb = NULL;

        g_completionQueueHead = (g_completionQueueHead + 1) % MAX_COMPLETION_QUEUE;
        g_completionQueueCount--;
    }
}

static void ExecuteCompletionRoutine(IOParamPtr pb)
{
    if (pb == NULL) {
        return;
    }

    /* Call completion routine if provided */
    if (pb->ioCompletion != NULL) {
        IOCompletionProc completion = (IOCompletionProc)pb->ioCompletion;
        completion(pb);
    }
}

/* =============================================================================
 * Device-Specific Interrupt Functions
 * ============================================================================= */

SInt16 RegisterDriverInterrupt(SInt16 refNum, UInt32 interruptType)
{
    if (!UnitTable_IsValidRefNum(refNum)) {
        return badUnitErr;
    }

    /* Default priority based on interrupt type */
    UInt32 priority = 4; /* Medium priority */

    switch (interruptType) {
        case kInterruptTypeTimer:
            priority = 7; /* High priority */
            break;
        case kInterruptTypeSCSI:
        case kInterruptTypeDisk:
            priority = 6; /* High priority */
            break;
        case kInterruptTypeNetwork:
        case kInterruptTypeSerial:
            priority = 5; /* Medium-high priority */
            break;
        case kInterruptTypeVIA:
        case kInterruptTypeSCC:
            priority = 4; /* Medium priority */
            break;
        default:
            priority = 3; /* Low-medium priority */
            break;
    }

    return RegisterInterruptHandler(refNum, (InterruptType)interruptType, priority);
}

SInt16 UnregisterDriverInterrupt(SInt16 refNum)
{
    return UnregisterInterruptHandler(refNum);
}

SInt16 EnableDriverInterrupt(SInt16 refNum, Boolean enable)
{
    InterruptHandlerPtr handler = FindInterruptHandler(refNum);
    if (handler == NULL) {
        return fnfErr;
    }

    handler->isEnabled = enable;
    return noErr;
}

/* =============================================================================
 * Timer-Based Interrupt Simulation
 * ============================================================================= */

SInt16 StartPeriodicInterrupt(SInt16 refNum, UInt32 intervalTicks)
{
    /* Placeholder for timer-based periodic interrupts */
    /* In a real implementation, this would set up a timer */

    return RegisterDriverInterrupt(refNum, kInterruptTypeTimer);
}

SInt16 StopPeriodicInterrupt(SInt16 refNum)
{
    return UnregisterDriverInterrupt(refNum);
}

void TriggerPeriodicInterrupts(void)
{
    /* Simulate periodic timer interrupts */
    ProcessInterrupt(kInterruptTypeTimer, GetCurrentTimestamp());
}

/* =============================================================================
 * I/O Completion Interface
 * ============================================================================= */

void CompleteAsyncIO(IOParamPtr pb, SInt16 result)
{
    if (pb == NULL) {
        return;
    }

    /* Queue the completion */
    QueueCompletion(pb, result);

    /* Process completions if not in interrupt context */
    if (g_interruptNestingLevel == 0) {
        ProcessCompletionQueue();
    }
}

Boolean IsIOCompletionPending(IOParamPtr pb)
{
    if (pb == NULL) {
        return false;
    }

    /* Check if PB is in the completion queue */
    for (UInt32 i = 0; i < g_completionQueueCount; i++) {
        UInt32 index = (g_completionQueueHead + i) % MAX_COMPLETION_QUEUE;
        if (g_completionQueue[index].isValid && g_completionQueue[index].pb == pb) {
            return true;
        }
    }

    return false;
}

void ProcessPendingCompletions(void)
{
    if (!g_interruptSystemInitialized) {
        return;
    }

    /* Process completions if not in interrupt context */
    if (g_interruptNestingLevel == 0) {
        ProcessCompletionQueue();
    }
}

/* =============================================================================
 * Signal Handler for Interrupt Simulation
 * ============================================================================= */

static void SignalHandler(int signum)
{
    if (!g_interruptsEnabled) {
        return;
    }

    InterruptType type = kInterruptTypeCustom;

    switch (signum) {
#ifdef SIGALRM
        case SIGALRM:
            type = kInterruptTypeTimer;
            break;
#endif
#ifdef SIGUSR1
        case SIGUSR1:
            type = kInterruptTypeDisk;
            break;
#endif
#ifdef SIGUSR2
        case SIGUSR2:
            type = kInterruptTypeNetwork;
            break;
#endif
        default:
            type = kInterruptTypeCustom;
            break;
    }

    ProcessInterrupt(type, 0);
}

/* =============================================================================
 * Statistics and Information
 * ============================================================================= */

void GetInterruptStatistics(void *stats)
{
    if (stats != NULL) {
        memcpy(stats, &g_interruptStats, sizeof(g_interruptStats));
    }
}

UInt32 GetInterruptCount(SInt16 refNum)
{
    InterruptHandlerPtr handler = FindInterruptHandler(refNum);
    return handler ? handler->interruptCount : 0;
}

UInt32 GetCompletionQueueDepth(void)
{
    return g_completionQueueCount;
}

UInt32 GetInterruptNestingLevel(void)
{
    return g_interruptNestingLevel;
}

/* =============================================================================
 * Utility Functions
 * ============================================================================= */

static UInt32 GetCurrentTimestamp(void)
{
    /* Simple timestamp based on system time */
    return (UInt32)time(NULL);
}

Boolean IsInInterruptContext(void)
{
    return g_interruptNestingLevel > 0;
}

void YieldToInterrupts(void)
{
    /* Allow interrupt processing */
    if (g_interruptNestingLevel == 0) {
        ProcessCompletionQueue();
    }
}

/* =============================================================================
 * Debug and Testing Functions
 * ============================================================================= */

void DumpInterruptHandlers(void)
{
    printf("Interrupt Handlers:\n");
    for (int priority = 0; priority < INTERRUPT_PRIORITY_LEVELS; priority++) {
        InterruptHandlerPtr handler = g_interruptHandlers[priority];
        if (handler != NULL) {
            printf("  Priority %d:\n", priority);
            while (handler != NULL) {
                printf("    RefNum=%d, Type=%d, Enabled=%s, Count=%u\n",
                       handler->refNum, handler->type,
                       handler->isEnabled ? "Yes" : "No",
                       handler->interruptCount);
                handler = handler->next;
            }
        }
    }

    printf("Completion Queue: %u/%u entries\n",
           g_completionQueueCount, MAX_COMPLETION_QUEUE);
    printf("Interrupt Statistics:\n");
    printf("  Total: %u, Handled: %u, Spurious: %u\n",
           g_interruptStats.totalInterrupts,
           g_interruptStats.handledInterrupts,
           g_interruptStats.spuriousInterrupts);
    printf("  Completions: %u, Overflows: %u, Max Nesting: %u\n",
           g_interruptStats.completionRoutinesCalled,
           g_interruptStats.queueOverflows,
           g_interruptStats.maxNestingLevel);
}

SInt16 InjectTestInterrupt(SInt16 refNum, UInt32 type, UInt32 data)
{
    /* Testing function to inject interrupts */
    if (!g_interruptSystemInitialized) {
        return dsIOCoreErr;
    }

    ProcessInterrupt((InterruptType)type, data);
    return noErr;
}
