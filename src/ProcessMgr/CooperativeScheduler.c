/* #include "SystemTypes.h" */
/*
 * RE-AGENT-BANNER
 * CooperativeScheduler.c - System 7 Cooperative Multitasking Scheduler
 *
 * implemented based on System.rsrc
 *
 * This implements the revolutionary cooperative multitasking scheduler introduced
 * in Mac OS System 7. Unlike preemptive multitasking, cooperative multitasking
 * relies on applications voluntarily yielding control through WaitNextEvent calls.
 *
 * Key cooperative scheduling features:
 * - Event-driven yield points via WaitNextEvent
 * - Round-robin scheduling among cooperative processes
 * - Background processing support
 * - Process priority management
 * - Integration with Event Manager for seamless multitasking
 *
 * Evidence sources:
 * - mappings.process_manager.json: Scheduler function mappings
 * - layouts.process_manager.json: Process queue and scheduling structures
 * - evidence.process_manager.json: System 7 multitasking evidence
 * RE-AGENT-BANNER
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"

#include "ProcessMgr/ProcessMgr.h"
#include "EventManager/EventTypes.h"
/* #include <OSUtils.h>
 - utilities in MacTypes.h */

/*
 * Scheduler State Variables

 */
static UInt32 gSchedulerTicks = 0;
static UInt32 gLastYieldTime = 0;
static UInt16 gSchedulerQuantum = 6; /* 1/10 second at 60Hz */
static Boolean gSchedulerActive = false;

/*
 * Forward Declarations
 */
static OSErr Scheduler_AddToQueue(ProcessControlBlock* process);
static OSErr Scheduler_RemoveFromQueue(ProcessControlBlock* process);
static Boolean Scheduler_ShouldYield(void);
static ProcessControlBlock* Scheduler_FindNextRunnable(ProcessControlBlock* current);

/*
 * Initialize Cooperative Scheduler

 */
OSErr Scheduler_Initialize(void)
{
    gSchedulerTicks = 0;
    gLastYieldTime = TickCount();
    gSchedulerActive = true;

    /* Install scheduler as part of vertical blanking interrupt */
    /* In real implementation, this would hook into VBL task queue */

    return noErr;
}

/*
 * Process Yielding - Core of Cooperative Multitasking

 */
OSErr Process_Yield(void)
{
    ProcessControlBlock* nextProcess;
    OSErr err;

    if (!gSchedulerActive || !gCurrentProcess) {
        return noErr;
    }

    /* Update yield timing */
    gLastYieldTime = TickCount();

    /* Find next process to run */
    err = Scheduler_GetNextProcess(&nextProcess);
    if (err != noErr) {
        return err;
    }

    /* Switch to next process if different */
    if (nextProcess != gCurrentProcess) {
        /* Update current process state for cooperative yield */
        if (gCurrentProcess->processState == kProcessRunning) {
            gCurrentProcess->processState = kProcessBackground;
        }

        /* Switch context */
        err = Context_Switch(nextProcess);
        if (err == noErr) {
            nextProcess->processState = kProcessRunning;
        }
    }

    return err;
}

/*
 * Enhanced Process Scheduler with Priority Support

 */
OSErr Scheduler_GetNextProcess(ProcessControlBlock** nextProcess)
{
    ProcessControlBlock* candidate;
    ProcessControlBlock* highestPriority = NULL;
    UInt16 maxPriority = 0;

    if (!nextProcess) {
        return paramErr;
    }

    *nextProcess = gCurrentProcess; /* Default to current */

    if (!gProcessQueue || gProcessQueue->queueSize == 0) {
        return noErr;
    }

    /*
     * Cooperative Round-Robin with Priority Consideration
    
     */

    /* First pass: Look for higher priority processes */
    candidate = gProcessQueue->queueHead;
    while (candidate) {
        if ((candidate->processState == kProcessRunning ||
             candidate->processState == kProcessBackground) &&
            candidate->processPriority > maxPriority) {
            highestPriority = candidate;
            maxPriority = candidate->processPriority;
        }
        candidate = candidate->processNextProcess;
    }

    /* If we found a higher priority process, use it */
    if (highestPriority &&
        highestPriority->processPriority > gCurrentProcess->processPriority) {
        *nextProcess = highestPriority;
        return noErr;
    }

    /* Otherwise, use round-robin among equal priority processes */
    candidate = Scheduler_FindNextRunnable(gCurrentProcess);
    if (candidate) {
        *nextProcess = candidate;
    }

    return noErr;
}

/*
 * Find Next Runnable Process in Round-Robin Fashion

 */
static ProcessControlBlock* Scheduler_FindNextRunnable(ProcessControlBlock* current)
{
    ProcessControlBlock* candidate;
    ProcessControlBlock* start;

    if (!current || !gProcessQueue) {
        return NULL;
    }

    /* Start search from next process in queue */
    candidate = current->processNextProcess;
    if (!candidate) {
        candidate = gProcessQueue->queueHead;
    }

    start = candidate;

    /* Find next runnable process */
    do {
        if (candidate &&
            (candidate->processState == kProcessRunning ||
             candidate->processState == kProcessBackground) &&
            candidate != current) {
            return candidate;
        }

        candidate = candidate->processNextProcess;
        if (!candidate) {
            candidate = gProcessQueue->queueHead;
        }

    } while (candidate && candidate != start);

    return current; /* Return current if no other runnable process */
}

/*
 * Process Suspension for Background Operation

 */
OSErr Process_Suspend(ProcessSerialNumber* psn)
{
    ProcessControlBlock* process = NULL;

    /* Find process by PSN */
    for (int i = 0; i < kPM_MaxProcesses; i++) {
        if (gProcessTable[i].processID.lowLongOfPSN == psn->lowLongOfPSN &&
            gProcessTable[i].processID.highLongOfPSN == psn->highLongOfPSN) {
            process = &gProcessTable[i];
            break;
        }
    }

    if (!process) {
        return procNotFound;
    }

    /* Only suspend if process accepts suspension */
    if (!(process->processMode & kProcessModeAcceptSuspend)) {
        return paramErr;
    }

    /* Update process state */
    ProcessState oldState = process->processState;
    process->processState = kProcessSuspended;

    /* If suspending current process, yield to next */
    if (process == gCurrentProcess) {
        Process_Yield();
    }

    return noErr;
}

/*
 * Process Resume from Suspension

 */
OSErr Process_Resume(ProcessSerialNumber* psn)
{
    ProcessControlBlock* process = NULL;

    /* Find process by PSN */
    for (int i = 0; i < kPM_MaxProcesses; i++) {
        if (gProcessTable[i].processID.lowLongOfPSN == psn->lowLongOfPSN &&
            gProcessTable[i].processID.highLongOfPSN == psn->highLongOfPSN) {
            process = &gProcessTable[i];
            break;
        }
    }

    if (!process) {
        return procNotFound;
    }

    if (process->processState != kProcessSuspended) {
        return paramErr; /* Not suspended */
    }

    /* Resume process */
    if (process->processMode & kProcessModeCanBackground) {
        process->processState = kProcessBackground;
    } else {
        process->processState = kProcessRunning;
    }

    return noErr;
}

/*
 * Set Process as Frontmost (Active)

 */
OSErr SetFrontProcess(ProcessSerialNumber* psn)
{
    ProcessControlBlock* process = NULL;
    ProcessControlBlock* oldFront = gCurrentProcess;

    /* Find target process */
    for (int i = 0; i < kPM_MaxProcesses; i++) {
        if (gProcessTable[i].processID.lowLongOfPSN == psn->lowLongOfPSN &&
            gProcessTable[i].processID.highLongOfPSN == psn->highLongOfPSN) {
            process = &gProcessTable[i];
            break;
        }
    }

    if (!process) {
        return procNotFound;
    }

    if (process->processState == kProcessTerminated) {
        return paramErr;
    }

    /* Move current process to background if it can background */
    if (oldFront && (oldFront->processMode & kProcessModeCanBackground)) {
        oldFront->processState = kProcessBackground;
    }

    /* Bring target process to front */
    process->processState = kProcessRunning;
    process->processPriority = 2; /* Boost priority for front process */

    /* Switch to new front process */
    return Context_Switch(process);
}

/*
 * Background Process Management

 */
OSErr Process_AllowBackgroundProcessing(ProcessSerialNumber* psn, Boolean allow)
{
    ProcessControlBlock* process = NULL;

    /* Find process by PSN */
    for (int i = 0; i < kPM_MaxProcesses; i++) {
        if (gProcessTable[i].processID.lowLongOfPSN == psn->lowLongOfPSN &&
            gProcessTable[i].processID.highLongOfPSN == psn->highLongOfPSN) {
            process = &gProcessTable[i];
            break;
        }
    }

    if (!process) {
        return procNotFound;
    }

    /* Update background processing mode */
    if (allow) {
        process->processMode |= kProcessModeCanBackground;
    } else {
        process->processMode &= ~kProcessModeCanBackground;
        /* If currently background, bring to front */
        if (process->processState == kProcessBackground) {
            process->processState = kProcessRunning;
        }
    }

    return noErr;
}

/*
 * Check if Scheduler Should Yield (Time Slice Expired)

 */
static Boolean Scheduler_ShouldYield(void)
{
    UInt32 currentTime = TickCount();

    /* Yield if time quantum expired */
    if ((currentTime - gLastYieldTime) >= gSchedulerQuantum) {
        return true;
    }

    /* Yield if higher priority process is waiting */
    ProcessControlBlock* candidate = gProcessQueue->queueHead;
    while (candidate) {
        if (candidate != gCurrentProcess &&
            (candidate->processState == kProcessRunning ||
             candidate->processState == kProcessBackground) &&
            candidate->processPriority > gCurrentProcess->processPriority) {
            return true;
        }
        candidate = candidate->processNextProcess;
    }

    return false;
}

/*
 * Enhanced WaitNextEvent with Better Cooperative Scheduling

 */
Boolean Enhanced_WaitNextEvent(EventMask eventMask, EventRecord* theEvent,
                              UInt32 sleep, RgnHandle mouseRgn)
{
    Boolean eventAvailable = false;
    UInt32 startTime = TickCount();
    UInt32 yieldInterval = 3; /* Yield every 3 ticks (1/20 second) */
    UInt32 lastYield = startTime;

    /* Immediate event check */
    eventAvailable = GetNextEvent(eventMask, theEvent);
    if (eventAvailable) {
        return true;
    }

    /* Cooperative multitasking loop */
    do {
        /* Check for events */
        eventAvailable = GetNextEvent(eventMask, theEvent);
        if (eventAvailable) {
            break;
        }

        /* Yield to other processes periodically */
        UInt32 currentTime = TickCount();
        if (gMultiFinderActive && (currentTime - lastYield) >= yieldInterval) {
            Process_Yield();
            lastYield = currentTime;
        }

        /* Small delay to prevent busy waiting */
        /* In real implementation, would use more sophisticated timing */

    } while ((TickCount() - startTime) < sleep);

    /* Generate null event if timeout */
    if (!eventAvailable) {
        theEvent->what = nullEvent;
        theEvent->message = 0;
        theEvent->when = TickCount();
        theEvent->modifiers = 0;
        GetMouse(&theEvent->where);
        eventAvailable = true;
    }

    return eventAvailable;
}

/*
 * Add Process to Scheduler Queue

 */
static OSErr Scheduler_AddToQueue(ProcessControlBlock* process)
{
    if (!process || !gProcessQueue) {
        return paramErr;
    }

    /* Add to end of queue */
    if (gProcessQueue->queueTail) {
        gProcessQueue->queueTail->processNextProcess = process;
        gProcessQueue->queueTail = process;
    } else {
        gProcessQueue->queueHead = process;
        gProcessQueue->queueTail = process;
    }

    process->processNextProcess = NULL;
    gProcessQueue->queueSize++;

    return noErr;
}

/*
 * Remove Process from Scheduler Queue

 */
static OSErr Scheduler_RemoveFromQueue(ProcessControlBlock* process)
{
    ProcessControlBlock* current;
    ProcessControlBlock* previous = NULL;

    if (!process || !gProcessQueue) {
        return paramErr;
    }

    /* Find process in queue */
    current = gProcessQueue->queueHead;
    while (current) {
        if (current == process) {
            /* Remove from queue */
            if (previous) {
                previous->processNextProcess = current->processNextProcess;
            } else {
                gProcessQueue->queueHead = current->processNextProcess;
            }

            if (current == gProcessQueue->queueTail) {
                gProcessQueue->queueTail = previous;
            }

            gProcessQueue->queueSize--;
            current->processNextProcess = NULL;
            return noErr;
        }
        previous = current;
        current = current->processNextProcess;
    }

    return procNotFound;
}

/*
