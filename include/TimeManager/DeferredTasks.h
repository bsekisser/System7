/*
 * DeferredTasks.h
 *
 * Deferred Task Management for System 7.1 Time Manager
 *
 * This file provides deferred task queue management, allowing tasks to be
 * scheduled for execution at a later time without blocking the current
 * execution context.
 */

#ifndef DEFERREDTASKS_H
#define DEFERREDTASKS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "TimeManager/TimeManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Forward Declarations ===== */

/* Ptr is defined in MacTypes.h */

/* ===== Deferred Task Types ===== */

/**
 * Deferred Task Procedure
 *
 * Procedure called when a deferred task is executed.
 * The task is executed at a safe time when interrupts are enabled.
 *
 * @param taskPtr - Pointer to the DeferredTask structure
 * @param userData - User data associated with the task
 */

/* ===== Task Priority Levels ===== */

/* ===== Deferred Task Structure ===== */

/**
 * Deferred Task Record
 *
 * Represents a task that will be executed at a later time when it's
 * safe to do so (typically when interrupts are enabled and the system
 * is not in a critical section).
 */

/* ===== Deferred Task Queue Statistics ===== */

/* ===== Deferred Task Functions ===== */

/**
 * Initialize Deferred Task System
 *
 * Sets up the deferred task queue and execution mechanism.
 * Must be called during system initialization.
 *
 * @return OSErr - noErr on success, error code on failure
 */
OSErr InitDeferredTasks(void);

/**
 * Shutdown Deferred Task System
 *
 * Shuts down the deferred task system and executes or cancels
 * all remaining tasks.
 */
void ShutdownDeferredTasks(void);

/**
 * Queue Deferred Task
 *
 * Adds a task to the deferred execution queue. The task will be
 * executed when it's safe to do so, respecting priority ordering.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @param procedure - Procedure to execute
 * @param userData - User data to pass to procedure
 * @param priority - Execution priority
 * @param delay - Delay before execution (0 = immediate, microseconds)
 * @return OSErr - noErr on success, error code on failure
 */
OSErr QueueDeferredTask(DeferredTaskPtr taskPtr,
                       DeferredProcPtr procedure,
                       void *userData,
                       DeferredPriority priority,
                       UInt32 delay);

/**
 * Cancel Deferred Task
 *
 * Removes a task from the deferred execution queue.
 * If the task is currently executing, waits for completion.
 *
 * @param taskPtr - Pointer to DeferredTask structure to cancel
 * @return OSErr - noErr on success, error code on failure
 */
OSErr CancelDeferredTask(DeferredTaskPtr taskPtr);

/**
 * Execute Deferred Tasks
 *
 * Processes the deferred task queue and executes any tasks that are
 * ready to run. This should be called regularly by the system scheduler.
 *
 * @param maxTasks - Maximum number of tasks to execute (0 = no limit)
 * @param maxTime - Maximum time to spend executing tasks (microseconds, 0 = no limit)
 * @return UInt32 - Number of tasks executed
 */
UInt32 ExecuteDeferredTasks(UInt32 maxTasks, UInt32 maxTime);

/**
 * Flush Deferred Tasks
 *
 * Executes all pending deferred tasks immediately, regardless of
 * their scheduled execution time. Used during system shutdown.
 *
 * @param priority - Execute only tasks of this priority or higher
 * @return UInt32 - Number of tasks executed
 */
UInt32 FlushDeferredTasks(DeferredPriority priority);

/* ===== Task Management Functions ===== */

/**
 * Is Task Queued
 *
 * Checks if a deferred task is currently in the execution queue.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @return Boolean - true if task is queued, false otherwise
 */
Boolean IsDeferredTaskQueued(DeferredTaskPtr taskPtr);

/**
 * Is Task Executing
 *
 * Checks if a deferred task is currently being executed.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @return Boolean - true if task is executing, false otherwise
 */
Boolean IsDeferredTaskExecuting(DeferredTaskPtr taskPtr);

/**
 * Get Task Queue Position
 *
 * Returns the position of a task in the execution queue.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @return SInt32 - Queue position (0-based), -1 if not queued
 */
SInt32 GetDeferredTaskPosition(DeferredTaskPtr taskPtr);

/**
 * Set Task Priority
 *
 * Changes the priority of a queued task. If the task is not queued,
 * the priority is stored for the next time it's queued.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @param priority - New priority level
 * @return OSErr - noErr on success, error code on failure
 */
OSErr SetDeferredTaskPriority(DeferredTaskPtr taskPtr, DeferredPriority priority);

/* ===== Queue Status Functions ===== */

/**
 * Get Deferred Task Count
 *
 * Returns the number of tasks currently in the deferred task queue.
 *
 * @param priority - Count only tasks of this priority (-1 = all priorities)
 * @return UInt32 - Number of queued tasks
 */
UInt32 GetDeferredTaskCount(DeferredPriority priority);

/**
 * Get Deferred Task Statistics
 *
 * Returns detailed statistics about deferred task queue operation.
 *
 * @param stats - Pointer to DeferredTaskStats structure to fill
 * @return OSErr - noErr on success, error code on failure
 */
OSErr GetDeferredTaskStats(DeferredTaskStats *stats);

/**
 * Reset Deferred Task Statistics
 *
 * Resets all deferred task statistics counters to zero.
 */
void ResetDeferredTaskStats(void);

/* ===== Utility Functions ===== */

/**
 * Create Deferred Task
 *
 * Allocates and initializes a new DeferredTask structure.
 *
 * @param procedure - Procedure to execute
 * @param userData - User data to pass to procedure
 * @param priority - Initial priority level
 * @return DeferredTaskPtr - Pointer to new task, NULL on failure
 */
DeferredTaskPtr CreateDeferredTask(DeferredProcPtr procedure,
                                  void *userData,
                                  DeferredPriority priority);

/**
 * Destroy Deferred Task
 *
 * Cancels and deallocates a DeferredTask structure.
 *
 * @param taskPtr - Pointer to DeferredTask structure to destroy
 * @return OSErr - noErr on success, error code on failure
 */
OSErr DestroyDeferredTask(DeferredTaskPtr taskPtr);

/**
 * Clone Deferred Task
 *
 * Creates a copy of an existing DeferredTask structure.
 *
 * @param srcTask - Source task to clone
 * @param destTask - Pointer to destination task structure
 * @return OSErr - noErr on success, error code on failure
 */
OSErr CloneDeferredTask(DeferredTaskPtr srcTask, DeferredTaskPtr destTask);

/* ===== Advanced Features ===== */

/**
 * Set Task Retry Policy
 *
 * Configures automatic retry behavior for a deferred task.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @param maxRetries - Maximum number of retry attempts
 * @param retryDelay - Delay between retries (microseconds)
 * @return OSErr - noErr on success, error code on failure
 */
OSErr SetDeferredTaskRetryPolicy(DeferredTaskPtr taskPtr,
                                UInt32 maxRetries,
                                UInt32 retryDelay);

/**
 * Set Task Timeout
 *
 * Sets a maximum execution time for a deferred task.
 *
 * @param taskPtr - Pointer to DeferredTask structure
 * @param timeout - Maximum execution time (microseconds)
 * @return OSErr - noErr on success, error code on failure
 */
OSErr SetDeferredTaskTimeout(DeferredTaskPtr taskPtr, UInt32 timeout);

/* ===== Constants ===== */

/* Task limits */
#define MAX_DEFERRED_TASKS          1000        /* Maximum number of queued tasks */
#define MAX_TASK_EXECUTION_TIME     1000000     /* Maximum task execution time (1 second) */
#define DEFAULT_RETRY_DELAY         10000       /* Default retry delay (10ms) */
#define MAX_RETRIES                 3           /* Default maximum retries */

/* Special values */
#define DEFERRED_TASK_IMMEDIATE     0           /* Execute immediately */
#define DEFERRED_TASK_NO_LIMIT      0           /* No limit on execution time/count */
#define DEFERRED_TASK_ALL_PRIORITIES ((DeferredPriority)-1) /* All priority levels */

#ifdef __cplusplus
}
#endif

#endif /* DEFERREDTASKS_H */