#ifndef BACKGROUND_NOTIFICATIONS_H
#define BACKGROUND_NOTIFICATIONS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "NotificationManager/NotificationManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Background Notification Types */

/* Background Task States */

/* Background Notification Request */
           /* Type of notification */
    OSType                      appSignature;   /* Application signature */
    StringPtr                   appName;        /* Application name */
    BackgroundTaskState         taskState;      /* Current task state */
    UInt32                      taskID;         /* Task identifier */
    Handle                      taskData;       /* Task-specific data */
    StringPtr                   message;        /* Notification message */
    Handle                      icon;           /* Task icon */
    Handle                      sound;          /* Notification sound */
    UInt32                      timestamp;      /* When notification occurred */
    long                        refCon;         /* Application reference */
    NMProcPtr                   callback;       /* Response callback */
    Boolean                     persistent;     /* Keep until acknowledged */
    Boolean                     urgent;         /* High priority notification */
} BackgroundNotificationRequest, *BackgroundNotificationPtr;

/* Background Task Registration */
       /* Application signature */
    StringPtr       appName;            /* Application name */
    UInt32          taskID;             /* Unique task ID */
    BackgroundTaskState state;          /* Current state */
    UInt32          registrationTime;   /* When registered */
    UInt32          lastActivity;       /* Last activity time */
    Boolean         notifyOnStateChange;/* Notify on state changes */
    Boolean         notifyOnError;      /* Notify on errors */
    Boolean         notifyOnCompletion; /* Notify on completion */
    NMProcPtr       statusCallback;     /* Status change callback */
    long            callbackRefCon;     /* Callback reference */
    void           *taskContext;        /* Task-specific context */
} BackgroundTaskRegistration, *BackgroundTaskPtr;

/* System Resource Monitoring */

/* Background Notification Callbacks */

/* Background Notification Management */
OSErr BGNotifyInit(void);
void BGNotifyCleanup(void);

/* Task Registration */
OSErr BGRegisterTask(BackgroundTaskPtr taskPtr);
OSErr BGUnregisterTask(UInt32 taskID);
OSErr BGUpdateTaskState(UInt32 taskID, BackgroundTaskState newState);
OSErr BGSetTaskCallback(UInt32 taskID, NMProcPtr callback, long refCon);

/* Background Notifications */
OSErr BGPostNotification(BackgroundNotificationPtr bgNotifyPtr);
OSErr BGPostSystemNotification(BackgroundNotificationType type,
                              StringPtr message,
                              Handle icon);
OSErr BGPostTaskNotification(UInt32 taskID,
                           BackgroundNotificationType type,
                           StringPtr message);

/* System Resource Monitoring */
OSErr BGStartResourceMonitoring(ResourceMonitorProc monitorProc, void *context);
OSErr BGStopResourceMonitoring(void);
OSErr BGGetSystemStatus(SystemResourceStatus *status);
OSErr BGSetResourceThresholds(UInt32 minMemory, UInt32 minDiskSpace, short minBattery);

/* Background Event Handling */
OSErr BGRegisterEventHandler(BackgroundNotificationType type,
                           BackgroundEventProc eventProc,
                           void *context);
OSErr BGUnregisterEventHandler(BackgroundNotificationType type);
OSErr BGTriggerEvent(BackgroundNotificationType type, void *eventData);

/* Task Management */
OSErr BGGetTaskList(BackgroundTaskPtr *taskList, short *count);
OSErr BGGetTaskInfo(UInt32 taskID, BackgroundTaskPtr taskPtr);
OSErr BGFindTaskBySignature(OSType appSignature, UInt32 *taskID);
OSErr BGFindTaskByName(StringPtr appName, UInt32 *taskID);

/* Task State Management */
OSErr BGSuspendTask(UInt32 taskID);
OSErr BGResumeTask(UInt32 taskID);
OSErr BGTerminateTask(UInt32 taskID);
OSErr BGSetTaskPriority(UInt32 taskID, short priority);

/* Background Processing */
void BGProcessBackgroundTasks(void);
void BGCheckResourceStatus(void);
void BGHandlePendingEvents(void);
OSErr BGScheduleCallback(NMProcPtr callback, long refCon, UInt32 delay);

/* Application Integration */
OSErr BGNotifyAppStateChange(OSType appSignature, Boolean isBackground);
OSErr BGNotifyAppTermination(OSType appSignature);
OSErr BGNotifyAppLaunch(OSType appSignature, StringPtr appName);

/* Error Handling */
OSErr BGReportTaskError(UInt32 taskID, OSErr errorCode, StringPtr errorMessage);
OSErr BGGetLastTaskError(UInt32 taskID, OSErr *errorCode, StringPtr errorMessage);
OSErr BGClearTaskError(UInt32 taskID);

/* Configuration */
OSErr BGSetNotificationEnabled(BackgroundNotificationType type, Boolean enabled);
Boolean BGIsNotificationEnabled(BackgroundNotificationType type);
OSErr BGSetGlobalEnabled(Boolean enabled);
Boolean BGIsGlobalEnabled(void);

/* Utility Functions */
UInt32 BGGenerateTaskID(void);
OSErr BGValidateTaskPtr(BackgroundTaskPtr taskPtr);
OSErr BGValidateNotificationPtr(BackgroundNotificationPtr bgNotifyPtr);
StringPtr BGGetNotificationTypeName(BackgroundNotificationType type);

/* Internal Functions */
void BGUpdateTaskActivity(UInt32 taskID);
void BGCheckTaskTimeouts(void);
void BGCleanupCompletedTasks(void);
OSErr BGAddTaskToRegistry(BackgroundTaskPtr taskPtr);
OSErr BGRemoveTaskFromRegistry(UInt32 taskID);

/* Platform Integration */
OSErr BGPlatformInit(void);
void BGPlatformCleanup(void);
OSErr BGPlatformRegisterTask(BackgroundTaskPtr taskPtr);
OSErr BGPlatformUnregisterTask(UInt32 taskID);
OSErr BGPlatformUpdateTaskState(UInt32 taskID, BackgroundTaskState state);

/* Constants */
#define BG_MAX_TASKS                100     /* Maximum background tasks */
#define BG_TASK_TIMEOUT             3600    /* Task timeout in ticks */
#define BG_RESOURCE_CHECK_INTERVAL  60      /* Resource check interval */
#define BG_DEFAULT_MIN_MEMORY       1048576 /* Default minimum memory (1MB) */
#define BG_DEFAULT_MIN_DISK         10485760 /* Default minimum disk (10MB) */
#define BG_DEFAULT_MIN_BATTERY      10      /* Default minimum battery (10%) */

/* Error Codes */
#define bgErrNotInitialized     -41000      /* Background notifications not initialized */
#define bgErrTaskNotFound       -41001      /* Task not found */
#define bgErrTaskExists         -41002      /* Task already exists */
#define bgErrInvalidTaskID      -41003      /* Invalid task ID */
#define bgErrTooManyTasks       -41004      /* Too many tasks registered */
#define bgErrInvalidState       -41005      /* Invalid task state */
#define bgErrResourceFailure    -41006      /* System resource failure */
#define bgErrNotSupported       -41007      /* Operation not supported */

#ifdef __cplusplus
}
#endif

#endif /* BACKGROUND_NOTIFICATIONS_H */
