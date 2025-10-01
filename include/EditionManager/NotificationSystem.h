/*
 * NotificationSystem.h
 *
 * Notification System API for Edition Manager
 * Handles update notifications, callbacks, and event distribution
 */

#ifndef __NOTIFICATION_SYSTEM_H__
#define __NOTIFICATION_SYSTEM_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Notification Event Types
 */

/*
 * Notification Priority Levels
 */

/*
 * Notification Event Structure
 */

/*
 * Notification Callback Type
 */

/*
 * Notification Statistics
 */

/*
 * Notification Filter
 */

/*
 * Core Notification Functions
 */

/* Initialize notification system */
OSErr InitializeNotificationSystem(void);

/* Clean up notification system */
void CleanupNotificationSystem(void);

/* Register for edition notifications */
OSErr RegisterForEditionNotifications(SectionHandle sectionH,
                                     const EditionContainerSpec* container);

/* Unregister from edition notifications */
OSErr UnregisterFromEditionNotifications(SectionHandle sectionH);

/* Set notification callback */
OSErr SetNotificationCallback(SectionHandle sectionH,
                             NotificationCallback callback,
                             void* userData,
                             UInt32 eventMask);

/* Remove notification callback */
OSErr RemoveNotificationCallback(SectionHandle sectionH);

/*
 * Event Posting and Processing
 */

/* Post notification event */
OSErr PostNotificationEvent(SectionHandle sectionH,
                           ResType eventClass,
                           ResType eventID,
                           const void* eventData,
                           Size dataSize);

/* Post high-priority notification */
OSErr PostPriorityNotification(SectionHandle sectionH,
                              ResType eventClass,
                              ResType eventID,
                              const void* eventData,
                              Size dataSize,
                              SInt32 priority);

/* Process notification queue */
OSErr ProcessNotificationQueue(void);

/* Flush all pending notifications */
OSErr FlushNotificationQueue(void);

/*
 * Event Filtering and Routing
 */

/* Set notification filter */
OSErr SetNotificationFilter(SectionHandle sectionH, const NotificationFilter* filter);

/* Get notification filter */
OSErr GetNotificationFilter(SectionHandle sectionH, NotificationFilter* filter);

/* Clear notification filter */
OSErr ClearNotificationFilter(SectionHandle sectionH);

/* Route notification to specific application */
OSErr RouteNotification(const NotificationEvent* event, AppRefNum targetApp);

/*
 * Notification Timing and Scheduling
 */

/* Set notification processing interval */
OSErr SetNotificationInterval(SInt32 intervalMs);

/* Get notification processing interval */
OSErr GetNotificationInterval(SInt32* intervalMs);

/* Schedule delayed notification */
OSErr ScheduleDelayedNotification(SectionHandle sectionH,
                                 ResType eventClass,
                                 ResType eventID,
                                 const void* eventData,
                                 Size dataSize,
                                 UInt32 delayMs);

/* Cancel scheduled notification */
OSErr CancelScheduledNotification(SectionHandle sectionH,
                                 ResType eventClass,
                                 ResType eventID);

/*
 * Batch Notifications
 */

/* Begin notification batch */
OSErr BeginNotificationBatch(void);

/* End notification batch and deliver */
OSErr EndNotificationBatch(void);

/* Cancel notification batch */
OSErr CancelNotificationBatch(void);

/* Set batch delivery mode */

OSErr SetBatchDeliveryMode(BatchDeliveryMode mode, UInt32 delayMs);

/*
 * Notification History and Logging
 */

/* Enable notification logging */
OSErr EnableNotificationLogging(const char* logFilePath, Boolean appendMode);

/* Disable notification logging */
OSErr DisableNotificationLogging(void);

/* Get notification history */
OSErr GetNotificationHistory(SectionHandle sectionH,
                            NotificationEvent** events,
                            UInt32* eventCount,
                            UInt32 maxEvents);

/* Clear notification history */
OSErr ClearNotificationHistory(SectionHandle sectionH);

/*
 * Statistics and Monitoring
 */

/* Get notification statistics */
OSErr GetNotificationStatistics(NotificationStatistics* stats);

/* Reset notification statistics */
OSErr ResetNotificationStatistics(void);

/* Get notification performance metrics */

OSErr GetNotificationPerformance(NotificationPerformanceMetrics* metrics);

/*
 * Advanced Notification Features
 */

/* Set notification coalescing */
OSErr SetNotificationCoalescing(Boolean enable, UInt32 windowMs);

/* Set notification throttling */
OSErr SetNotificationThrottling(UInt32 maxNotificationsPerSecond);

/* Set notification compression */
OSErr SetNotificationCompression(Boolean enable);

/* Set duplicate notification filtering */
OSErr SetDuplicateFiltering(Boolean enable, UInt32 windowMs);

/*
 * Error Handling and Recovery
 */

/* Set notification error handler */

OSErr SetNotificationErrorHandler(NotificationErrorHandler handler, void* userData);

/* Retry failed notifications */
OSErr RetryFailedNotifications(void);

/* Get failed notification count */
OSErr GetFailedNotificationCount(UInt32* count);

/*
 * Platform Integration
 */

/* Post section event to platform */
OSErr PostSectionEventToPlatform(SectionHandle sectionH, AppRefNum toApp, ResType classID);

/* Create AppleEvent-style notification */
OSErr CreateSectionAppleEvent(const void* notification);

/* Integrate with platform notification system */
OSErr IntegrateWithPlatformNotifications(Boolean enable);

/* Set platform notification options */

OSErr SetPlatformNotificationOptions(const PlatformNotificationOptions* options);

/*
 * Network Notifications
 */

/* Enable network notifications */
OSErr EnableNetworkNotifications(const char* serverAddress, UInt16 port);

/* Disable network notifications */
OSErr DisableNetworkNotifications(void);

/* Send network notification */
OSErr SendNetworkNotification(const char* targetAddress,
                             const NotificationEvent* event);

/* Register network notification handler */

OSErr RegisterNetworkNotificationHandler(NetworkNotificationHandler handler,
                                        void* userData);

/*
 * Security and Access Control
 */

/* Set notification security policy */

OSErr SetNotificationSecurity(NotificationSecurityLevel level);

/* Set notification access permissions */
OSErr SetNotificationPermissions(SectionHandle sectionH,
                                AppRefNum allowedApp,
                                UInt32 permissions);

/* Validate notification sender */
OSErr ValidateNotificationSender(const NotificationEvent* event,
                                AppRefNum senderApp,
                                Boolean* isValid);

/*
 * Debugging and Diagnostics
 */

/* Enable notification debugging */
OSErr EnableNotificationDebugging(Boolean enable);

/* Dump notification queue state */
OSErr DumpNotificationQueue(const char* outputPath);

/* Validate notification system integrity */
OSErr ValidateNotificationSystem(Boolean* isValid, char* errorMessage, Size messageSize);

/* Get notification system memory usage */
OSErr GetNotificationMemoryUsage(Size* totalMemory, Size* activeMemory);

/*
 * Constants and Limits
 */

#define kMaxNotificationQueueSize 1000      /* Maximum queued notifications */
#define kMaxNotificationDataSize 8192       /* Maximum notification data size */
#define kDefaultNotificationInterval 100    /* Default processing interval (ms) */
#define kMaxNotificationHistory 500         /* Maximum history entries */
#define kMinNotificationInterval 10         /* Minimum processing interval (ms) */
#define kMaxNotificationInterval 10000      /* Maximum processing interval (ms) */

/* Notification delivery modes */

/* Error codes specific to notifications */

#ifdef __cplusplus
}
#endif

#endif /* __NOTIFICATION_SYSTEM_H__ */
