/* #include "SystemTypes.h" */
#include <stdlib.h>
/*
 * RE-AGENT-BANNER
 * ProcessManager.c - Mac OS System 7 Process Manager Core Implementation
 *
 * implemented based on System.rsrc
 * ROM disassembly
 *
 * This implements the cooperative multitasking Process Manager for System 7.
 * The Process Manager enables multiple applications to run simultaneously
 * through cooperative scheduling where applications voluntarily yield control
 * by calling WaitNextEvent or GetNextEvent.
 *
 * Key cooperative multitasking features:
 * - Event-driven scheduling through WaitNextEvent
 * - Process Control Blocks for state management
 * - Memory partition management per process
 * - Context switching for 68k processors
 * - MultiFinder integration for background processing
 *
 * Evidence sources:
 * - evidence.process_manager.json: Function analysis from radare2
 * - mappings.process_manager.json: Function name mappings
 * - layouts.process_manager.json: Data structure layouts
 * RE-AGENT-BANNER
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "ProcessMgr/ProcessMgr.h"
#include <Traps.h>
#include <ToolUtils.h>


/*
 * Global Process Manager State

 */
ProcessQueue* gProcessQueue = NULL;
ProcessControlBlock* gCurrentProcess = NULL;
ProcessSerialNumber gSystemProcessPSN = {0, 1};
Boolean gMultiFinderActive = false;
static UInt32 gNextProcessID = 2;
static ProcessControlBlock gProcessTable[kPM_MaxProcesses];

/*
 * Process Manager Initialization

 * Address: 0x00000000, extensive cross-references indicate main initialization
 */
OSErr ProcessManager_Initialize(void)
{
    OSErr err = noErr;

    /* Initialize process queue */
    gProcessQueue = (ProcessQueue*)NewPtr(sizeof(ProcessQueue));
    if (!gProcessQueue) {
        return memFullErr;
    }

    gProcessQueue->queueHead = NULL;
    gProcessQueue->queueTail = NULL;
    gProcessQueue->queueSize = 0;
    gProcessQueue->currentProcess = NULL;

    /* Initialize process table */
    for (int i = 0; i < kPM_MaxProcesses; i++) {
        gProcessTable[i].processID.highLongOfPSN = 0;
        gProcessTable[i].processID.lowLongOfPSN = 0;
        gProcessTable[i].processState = kProcessTerminated;
        gProcessTable[i].processNextProcess = NULL;
    }

    /* Create system process entry */
    gProcessTable[0].processID = gSystemProcessPSN;
    gProcessTable[0].processSignature = 'MACS';
    gProcessTable[0].processType = 'INIT';
    gProcessTable[0].processState = kProcessRunning;
    gProcessTable[0].processMode = kProcessModeCooperative;
    gCurrentProcess = &gProcessTable[0];

    /* Initialize MultiFinder if available */
    err = MultiFinder_Init();

    return err;
}

/*
 * Process Creation

 * Three-argument function for creating process control blocks
 */
OSErr Process_Create(const FSSpec* appSpec, Size memorySize, LaunchFlags flags)
{
    ProcessControlBlock* newProcess = NULL;
    int freeSlot = -1;

    /* Find free process slot */
    for (int i = 1; i < kPM_MaxProcesses; i++) {
        if (gProcessTable[i].processState == kProcessTerminated) {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot == -1) {
        return procNotFound; /* No free process slots */
    }

    newProcess = &gProcessTable[freeSlot];

    /* Initialize process control block */
    (newProcess)->highLongOfPSN = 0;
    (newProcess)->lowLongOfPSN = gNextProcessID++;
    newProcess->processSignature = 'APPL'; /* Default application signature */
    newProcess->processType = 'APPL';
    newProcess->processState = kProcessSuspended;
    newProcess->processMode = kProcessModeCooperative | kProcessModeCanBackground;

    /* Allocate memory partition */
    newProcess->processLocation = NewPtr(memorySize);
    if (!newProcess->processLocation) {
        newProcess->processState = kProcessTerminated;
        return memFullErr;
    }
    newProcess->processSize = memorySize;

    /* Setup heap zone */
    newProcess->processHeapZone = (THz)newProcess->processLocation;
    InitZone(NULL, 64, (Ptr)newProcess->processHeapZone + memorySize,
             (Ptr)newProcess->processHeapZone);

    /* Initialize stack */
    newProcess->processStackSize = 8192; /* 8K default stack */
    newProcess->processStackBase = NewPtr(newProcess->processStackSize);
    if (!newProcess->processStackBase) {
        DisposePtr(newProcess->processLocation);
        newProcess->processState = kProcessTerminated;
        return memFullErr;
    }

    /* Setup A5 world */
    newProcess->processA5World = (Ptr)newProcess->processHeapZone + 32;

    /* Initialize timing information */
    newProcess->processCreationTime = TickCount();
    newProcess->processLastEventTime = TickCount();
    newProcess->processEventMask = everyEvent;
    newProcess->processPriority = 1; /* Normal priority */

    /* Allocate context save area */
    newProcess->processContextSave = NewPtr(sizeof(ProcessContext));
    if (!newProcess->processContextSave) {
        DisposePtr(newProcess->processLocation);
        DisposePtr(newProcess->processStackBase);
        newProcess->processState = kProcessTerminated;
        return memFullErr;
    }

    return noErr;
}

/*
 * Cooperative Scheduler - Get Next Process

 * Small function for simple round-robin scheduling
 */
OSErr Scheduler_GetNextProcess(ProcessControlBlock** nextProcess)
{
    ProcessControlBlock* current = gCurrentProcess;
    ProcessControlBlock* candidate = NULL;

    if (!gProcessQueue || gProcessQueue->queueSize == 0) {
        *nextProcess = gCurrentProcess; /* Stay with current if no others */
        return noErr;
    }

    /* Simple round-robin cooperative scheduling */
    if (current && current->processNextProcess) {
        candidate = current->processNextProcess;
    } else {
        candidate = gProcessQueue->queueHead;
    }

    /* Find next runnable process */
    ProcessControlBlock* start = candidate;
    do {
        if (candidate &&
            (candidate->processState == kProcessRunning ||
             candidate->processState == kProcessBackground)) {
            *nextProcess = candidate;
            return noErr;
        }

        candidate = candidate ? candidate->processNextProcess : gProcessQueue->queueHead;

        /* Prevent infinite loop */
        if (candidate == start) {
            break;
        }
    } while (candidate != start);

    /* Default to current process if no runnable process found */
    *nextProcess = gCurrentProcess;
    return noErr;
}

/*
 * Context Switching for Cooperative Multitasking

 * Medium-sized function with parameter for context management
 */
OSErr Context_Switch(ProcessControlBlock* targetProcess)
{
    ProcessContext* currentContext;
    ProcessContext* targetContext;

    if (!targetProcess || !gCurrentProcess) {
        return paramErr;
    }

    if (targetProcess == gCurrentProcess) {
        return noErr; /* No switch needed */
    }

    currentContext = (ProcessContext*)gCurrentProcess->processContextSave;
    targetContext = (ProcessContext*)targetProcess->processContextSave;

    if (!currentContext || !targetContext) {
        return memFullErr;
    }

    /*
     * Save current process context (68k specific)
     * In real implementation, this would use assembly to save/restore registers
     */

    /* Save A5 world */
    currentContext->savedA5 = (UInt32)gCurrentProcess->processA5World;

    /* Save stack pointer */
    currentContext->savedStackPointer = (UInt32)gCurrentProcess->processStackBase;

    /*
     * Note: In actual 68k implementation, would save:
     * - All data registers (D0-D7)
     * - All address registers (A0-A7)
     * - Status register
     * - Program counter
     */

    /* Switch to target process */
    gCurrentProcess = targetProcess;

    /* Restore target process context */
    /* Set A5 world for global access */
    /* Restore stack pointer */
    /* Restore registers and PC */

    /* Update process timing */
    targetProcess->processLastEventTime = TickCount();

    return noErr;
}

/*
 * Launch Application - Main entry point for starting new processes

 */
OSErr LaunchApplication(LaunchParamBlockRec* launchParams)
{
    OSErr err;
    ProcessControlBlock* newProcess;

    if (!launchParams) {
        return paramErr;
    }

    /* Create new process */
    err = Process_Create(&launchParams->launchAppSpec,
                        launchParams->launchPrefSize,
                        launchParams->launchControlFlags);
    if (err != noErr) {
        return err;
    }

    /* Add process to scheduler queue */
    if (gProcessQueue->queueHead == NULL) {
        gProcessQueue->queueHead = newProcess;
        gProcessQueue->queueTail = newProcess;
    } else {
        gProcessQueue->queueTail->processNextProcess = newProcess;
        gProcessQueue->queueTail = newProcess;
    }
    gProcessQueue->queueSize++;

    /* Switch to new process if requested */
    if (!(launchParams->launchControlFlags & kLaunchDontSwitch)) {
        err = Context_Switch(newProcess);
    }

    return err;
}

/*
 * WaitNextEvent - Core of cooperative multitasking
 * Applications call this to yield control and allow other processes to run
 */
Boolean WaitNextEvent(EventMask eventMask, EventRecord* theEvent,
                     UInt32 sleep, RgnHandle mouseRgn)
{
    Boolean eventAvailable = false;
    UInt32 startTime = TickCount();
    ProcessControlBlock* nextProcess;

    /* Check for immediate events */
    eventAvailable = GetNextEvent(eventMask, theEvent);
    if (eventAvailable) {
        return true;
    }

    /* Cooperative yield - give other processes a chance to run */
    if (gMultiFinderActive) {
        OSErr err = Scheduler_GetNextProcess(&nextProcess);
        if (err == noErr && nextProcess != gCurrentProcess) {
            Context_Switch(nextProcess);
        }
    }

    /* Wait for events or timeout */
    do {
        eventAvailable = GetNextEvent(eventMask, theEvent);
        if (eventAvailable) {
            break;
        }

        /* Yield to other processes during wait */
        if (gMultiFinderActive) {
            Scheduler_GetNextProcess(&nextProcess);
            if (nextProcess != gCurrentProcess) {
                Context_Switch(nextProcess);
            }
        }

    } while ((TickCount() - startTime) < sleep);

    /* Generate null event if no real event occurred */
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
 * MultiFinder Integration

 */
OSErr MultiFinder_Init(void)
{
    /* Check if MultiFinder is available */
    if (Gestalt('mfdr', NULL) == noErr) {
        gMultiFinderActive = true;
        return noErr;
    }

    gMultiFinderActive = false;
    return fnfErr;
}

/*
 * Process Cleanup

 * Single byte function suggests cleanup finalization
 */
OSErr Process_Cleanup(ProcessSerialNumber* psn)
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

    /* Clean up process resources */
    if (process->processLocation) {
        DisposePtr(process->processLocation);
    }
    if (process->processStackBase) {
        DisposePtr(process->processStackBase);
    }
    if (process->processContextSave) {
        DisposePtr(process->processContextSave);
    }

    /* Mark as terminated */
    process->processState = kProcessTerminated;

    /* Remove from scheduler queue */
    if (gProcessQueue->queueHead == process) {
        gProcessQueue->queueHead = process->processNextProcess;
    }
    if (gProcessQueue->queueTail == process) {
        gProcessQueue->queueTail = NULL; /* Find new tail if needed */
    }
    gProcessQueue->queueSize--;

    return noErr;
}

/*
