#include "SuperCompat.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
/*
 * TimeManagerCore.c
 *
 * Core Time Manager Implementation for System 7.1 - Portable C Version
 *
 * This file implements the core Time Manager functionality, converted from the
 *
 * The Time Manager provides high-precision timing services, task scheduling,
 * and time-based operations essential for System 7.1 operation.
 */

#include "CompatibilityFix.h"
#include <time.h>
#include "SystemTypes.h"
#include <time.h>
#include "System71StdLib.h"
#include <time.h>

#include "TimeManager/TimeManager.h"
#include <time.h>
#include "TimeManager/TimeManager.h"
#include <time.h>
#include "MicrosecondTimer.h"
#include <time.h>
#include "TimeBase.h"
#include <time.h>
#include "DeferredTasks.h"
#include <time.h>


/* ===== Global Time Manager State ===== */
static TimeMgrPrivate *gTimeMgrPrivate = NULL;
static pthread_mutex_t gTimeMgrMutex = PTHREAD_MUTEX_INITIALIZER;
static Boolean gTimeMgrInitialized = false;
static UInt32 gNextTaskID = 1;

/* ===== Private Function Prototypes ===== */
static OSErr AllocateTimeMgrPrivate(void);
static void DeallocateTimeMgrPrivate(void);
static OSErr SetupTimerThread(void);
static void ShutdownTimerThread(void);
static void* TimerThreadProc(void* param);
static uint64_t GetSystemTimeNS(void);
static UInt32 GenerateTaskID(void);

/* ===== Time Manager Initialization ===== */

/**
 * Initialize the Time Manager
 *
 * Sets up the Time Manager's global data structures, timer interrupt handling,
 * and runtime calibration for processor-speed independence.
 */
OSErr InitTimeMgr(void)
{
    OSErr err;

    // Check if already initialized
    if (gTimeMgrInitialized) {
        return NO_ERR;
    }

    // Lock for thread safety
    if (pthread_mutex_lock(&gTimeMgrMutex) != 0) {
        return -1; // Mutex error
    }

    // Double-check initialization under lock
    if (gTimeMgrInitialized) {
        pthread_mutex_unlock(&gTimeMgrMutex);
        return NO_ERR;
    }

    // Allocate private data structure
    err = AllocateTimeMgrPrivate();
    if (err != NO_ERR) {
        pthread_mutex_unlock(&gTimeMgrMutex);
        return err;
    }

    // Initialize time base
    err = InitTimeBase();
    if (err != NO_ERR) {
        DeallocateTimeMgrPrivate();
        pthread_mutex_unlock(&gTimeMgrMutex);
        return err;
    }

    // Initialize platform timer
    err = InitPlatformTimer();
    if (err != NO_ERR) {
        ShutdownTimeBase();
        DeallocateTimeMgrPrivate();
        pthread_mutex_unlock(&gTimeMgrMutex);
        return err;
    }

    // Initialize deferred task system
    err = InitDeferredTasks();
    if (err != NO_ERR) {
        ShutdownPlatformTimer();
        ShutdownTimeBase();
        DeallocateTimeMgrPrivate();
        pthread_mutex_unlock(&gTimeMgrMutex);
        return err;
    }

    // Setup timer thread for interrupt simulation
    err = SetupTimerThread();
    if (err != NO_ERR) {
        ShutdownDeferredTasks();
        ShutdownPlatformTimer();
        ShutdownTimeBase();
        DeallocateTimeMgrPrivate();
        pthread_mutex_unlock(&gTimeMgrMutex);
        return err;
    }

    // Calibrate timer
    err = CalibrateTimer();
    if (err != NO_ERR) {
        // Non-fatal error - continue with default calibration
    }

    // Store system start time
    gTimeMgrPrivate->startTime = GetSystemTimeNS();

    // Mark as initialized
    gTimeMgrInitialized = true;

    pthread_mutex_unlock(&gTimeMgrMutex);
    return NO_ERR;
}

/**
 * Shutdown the Time Manager
 *
 * Cleans up Time Manager resources and stops all active timers.
 */
void ShutdownTimeMgr(void)
{
    if (!gTimeMgrInitialized) {
        return;
    }

    // Lock for thread safety
    if (pthread_mutex_lock(&gTimeMgrMutex) != 0) {
        return; // Mutex error
    }

    if (!gTimeMgrInitialized) {
        pthread_mutex_unlock(&gTimeMgrMutex);
        return;
    }

    // Shutdown in reverse order of initialization
    ShutdownTimerThread();
    ShutdownDeferredTasks();
    ShutdownPlatformTimer();
    ShutdownTimeBase();
    DeallocateTimeMgrPrivate();

    gTimeMgrInitialized = false;

    pthread_mutex_unlock(&gTimeMgrMutex);
}

/* ===== Task Management Functions ===== */

/**
 * Install Time Manager Task
 *
 * Initializes a Time Manager Task structure for use.
 */
OSErr InsTime(TMTaskPtr tmTaskPtr)
{
    if (!tmTaskPtr) {
        return -1; // Invalid parameter
    }

    if (!gTimeMgrInitialized) {
        return -2; // Time Manager not initialized
    }

    // Initialize the task structure
    memset(tmTaskPtr, 0, sizeof(TMTask));

    // Clear flags - task is initially inactive
    tmTaskPtr->qType = QTASK_TIMER_TYPE;
    tmTaskPtr->isActive = false;
    tmTaskPtr->isExtended = false;

    return NO_ERR;
}

/**
 * Install Extended Time Manager Task
 *
 * Like InsTime, but for extended TMTask structures that support drift-free
 * fixed-frequency timing.
 */
OSErr InsXTime(TMTaskPtr tmTaskPtr)
{
    OSErr err = InsTime(tmTaskPtr);
    if (err != NO_ERR) {
        return err;
    }

    // Mark as extended task
    tmTaskPtr->qType |= QTASK_EXTENDED_FLAG;
    tmTaskPtr->isExtended = true;

    return NO_ERR;
}

/* ===== Private Implementation Functions ===== */

/**
 * Allocate Time Manager Private Data
 */
static OSErr AllocateTimeMgrPrivate(void)
{
    gTimeMgrPrivate = (TimeMgrPrivate*)calloc(1, sizeof(TimeMgrPrivate));
    if (!gTimeMgrPrivate) {
        return -1; // Memory allocation failed
    }

    // Initialize private data
    gTimeMgrPrivate->activePtr = NULL;
    gTimeMgrPrivate->timerAdjust = 1; // Initial adjustment value
    gTimeMgrPrivate->retryAdjust = (1 << (TICK_SCALE - 1)) + (1 << TICK_SCALE); // From original
    gTimeMgrPrivate->currentTime = 0;
    gTimeMgrPrivate->backLog = 0;
    gTimeMgrPrivate->highUSecs = 0;
    gTimeMgrPrivate->lowUSecs = 0;
    gTimeMgrPrivate->fractUSecs = 0;
    gTimeMgrPrivate->curTimeThresh = 0;
    gTimeMgrPrivate->initialized = true;

    return NO_ERR;
}

/**
 * Deallocate Time Manager Private Data
 */
static void DeallocateTimeMgrPrivate(void)
{
    if (gTimeMgrPrivate) {
        free(gTimeMgrPrivate);
        gTimeMgrPrivate = NULL;
    }
}

/**
 * Setup Timer Thread
 *
 * Creates a high-priority thread to simulate timer interrupts.
 */
static OSErr SetupTimerThread(void)
{
    pthread_t *timerThread;
    pthread_attr_t attr;
    struct sched_param param;
    int result;

    // Allocate thread handle storage
    timerThread = (pthread_t*)malloc(sizeof(pthread_t));
    if (!timerThread) {
        return -1; // Memory allocation failed
    }

    // Initialize thread attributes
    result = pthread_attr_init(&attr);
    if (result != 0) {
        free(timerThread);
        return -2; // Thread attribute initialization failed
    }

    // Set thread to be detached
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Try to set high priority for timer accuracy
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) == 0) {
        param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;
        pthread_attr_setschedparam(&attr, &param);
    }

    // Create the timer thread
    result = pthread_create(timerThread, &attr, TimerThreadProc, NULL);
    pthread_attr_destroy(&attr);

    if (result != 0) {
        free(timerThread);
        return -3; // Thread creation failed
    }

    gTimeMgrPrivate->timerThread = timerThread;
    return NO_ERR;
}

/**
 * Shutdown Timer Thread
 */
static void ShutdownTimerThread(void)
{
    if (gTimeMgrPrivate && gTimeMgrPrivate->timerThread) {
        pthread_t *timerThread = (pthread_t*)gTimeMgrPrivate->timerThread;

        // Signal thread to exit (implementation would set a flag)
        // For now, we'll cancel the thread
        pthread_cancel(*timerThread);
        pthread_join(*timerThread, NULL);

        free(timerThread);
        gTimeMgrPrivate->timerThread = NULL;
    }
}

/**
 * Timer Thread Procedure
 *
 * High-priority thread that simulates the VIA timer interrupts.
 * This provides the basic timing heartbeat for the Time Manager.
 */
static void* TimerThreadProc(void* param)
{
    struct timespec sleepTime;
    uint64_t lastTime, currentTime;
    UInt32 elapsed;

    (void)param; // Unused parameter

    // Set thread cancellation type
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Get initial time
    lastTime = GetSystemTimeNS();

    // Timer loop - approximate the original VIA timer behavior
    while (true) {
        // Sleep for approximately 20 microseconds (original resolution)
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 20000; // 20 microseconds
        nanosleep(&sleepTime, NULL);

        // Check for thread cancellation
        pthread_testcancel();

        // Update time and check for expired timers
        currentTime = GetSystemTimeNS();
        elapsed = (UInt32)((currentTime - lastTime) / 1000); // Convert to microseconds

        if (elapsed > 0) {
            // Update the time base
            UpdateTimeBase(elapsed);

            // Process any expired timers (would call Timer2Int equivalent)
            // This is handled by TimerInterrupts.c in the full implementation
        }

        lastTime = currentTime;
    }

    return NULL;
}

/**
 * Get System Time in Nanoseconds
 */
static uint64_t GetSystemTimeNS(void)
{
    struct timespec ts;

    // Use monotonic clock for consistent timing
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }

    // Fallback to realtime clock
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }

    return 0; // Error case
}

/**
 * Generate Unique Task ID
 */
static UInt32 GenerateTaskID(void)
{
    return __sync_fetch_and_add(&gNextTaskID, 1);
}

/* ===== Utility Functions ===== */

/**
 * Multiply and Merge utility
 *
 * Performs 32-bit by 32-bit multiplication with 64-bit intermediate result,
 * then merges selected bits to produce final 32-bit result.
 */
UInt32 MultAndMerge(UInt32 multiplicand, UInt32 multiplier, UInt32 mergeMask)
{
    uint64_t product = (uint64_t)multiplicand * (uint64_t)multiplier;
    UInt32 high = (UInt32)(product >> 32);
    UInt32 low = (UInt32)product;

    if (mergeMask == 0) {
        return high;
    }

    // Check for overflow in high word where merge mask has bits set
    UInt32 maskBits = low & mergeMask;
    if ((high + (maskBits >> 31)) & (~mergeMask >> 31)) {
        return 0xFFFFFFFF; // Overflow
    }

    return high | maskBits;
}

/* ===== Time Conversion Functions ===== */

/**
 * Convert milliseconds to internal time format
 */
UInt32 MillisecondsToInternal(UInt32 milliseconds)
{
    return MultAndMerge(milliseconds, MS_TO_INTERNAL,
                       (UInt32)(-1) << MS_TO_INT_FRACT_BITS);
}

/**
 * Convert microseconds to internal time format
 */
UInt32 MicrosecondsToInternal(UInt32 microseconds)
{
    return MultAndMerge(microseconds, US_TO_INTERNAL, 0);
}

/**
 * Convert internal time to milliseconds
 */
UInt32 InternalToMilliseconds(UInt32 internal)
{
    return MultAndMerge(internal, INTERNAL_TO_MS, 0);
}

/**
 * Convert internal time to microseconds
 */
UInt32 InternalToMicroseconds(UInt32 internal)
{
    UInt32 result = MultAndMerge(internal, INTERNAL_TO_US,
                                  (UInt32)(-1) << INT_TO_US_FRACT_BITS);
    // Rotate result to align properly (from original implementation)
    return (result << (32 - INT_TO_US_FRACT_BITS)) |
           (result >> INT_TO_US_FRACT_BITS);
}

/* ===== Status Functions ===== */

/**
 * Get current Time Manager status
 */
Boolean IsTimeMgrActive(void)
{
    return gTimeMgrInitialized && (gTimeMgrPrivate != NULL);
}

/**
 * Get number of active timer tasks
 */
SInt32 GetActiveTaskCount(void)
{
    SInt32 count = 0;
    TMTaskPtr current;

    if (!gTimeMgrInitialized || !gTimeMgrPrivate) {
        return 0;
    }

    // Lock and count active tasks
    if (pthread_mutex_lock(&gTimeMgrMutex) == 0) {
        current = gTimeMgrPrivate->activePtr;
        while (current) {
            count++;
            current = current->qLink;
        }
        pthread_mutex_unlock(&gTimeMgrMutex);
    }

    return count;
}

/**
 * Get current backlog time
 */
UInt32 GetBacklogTime(void)
{
    if (!gTimeMgrInitialized || !gTimeMgrPrivate) {
        return 0;
    }

    return gTimeMgrPrivate->backLog;
}
