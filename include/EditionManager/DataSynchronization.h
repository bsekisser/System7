/*
 * DataSynchronization.h
 *
 * Data Synchronization API for Edition Manager
 * Handles real-time data sharing, change detection, and automatic updates
 */

#ifndef __DATA_SYNCHRONIZATION_H__
#define __DATA_SYNCHRONIZATION_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Synchronization Modes
 */

/*
 * Synchronization Flags
 */

/*
 * Format-specific synchronization information
 */

/*
 * Synchronization statistics
 */

/*
 * Conflict resolution strategies
 */

/*
 * Core Synchronization Functions
 */

/* Set synchronization mode for a section */
OSErr SetSynchronizationMode(SectionHandle sectionH, SyncMode mode);

/* Get current synchronization mode */
OSErr GetSynchronizationMode(SectionHandle sectionH, SyncMode* mode);

/* Force immediate synchronization */
OSErr ForceSynchronization(SectionHandle sectionH);

/* Check if section needs synchronization */
OSErr NeedsSynchronization(SectionHandle sectionH, Boolean* needsSync);

/* Get time since last synchronization */
OSErr GetTimeSinceLastSync(SectionHandle sectionH, UInt32* seconds);

/*
 * Publisher Synchronization
 */

/* Synchronize publisher data to edition */
OSErr SyncPublisherToEdition(void* syncState);

/* Check if publisher data has changed */
OSErr CheckPublisherDataChanges(SectionHandle publisherH, Boolean* hasChanges);

/* Set publisher data change callback */

OSErr SetPublisherChangeCallback(SectionHandle publisherH,
                                PublisherChangeCallback callback,
                                void* userData);

/* Enable/disable automatic publishing */
OSErr SetAutoPublishing(SectionHandle publisherH, Boolean enable);

/* Get auto-publishing status */
OSErr GetAutoPublishing(SectionHandle publisherH, Boolean* isEnabled);

/*
 * Subscriber Synchronization
 */

/* Synchronize edition data to subscriber */
OSErr SyncEditionToSubscriber(void* syncState);

/* Check for edition updates */
OSErr CheckEditionUpdates(SectionHandle subscriberH, Boolean* hasUpdates);

/* Set subscriber update callback */

OSErr SetSubscriberUpdateCallback(SectionHandle subscriberH,
                                 SubscriberUpdateCallback callback,
                                 void* userData);

/* Enable/disable automatic subscription updates */
OSErr SetAutoSubscription(SectionHandle subscriberH, Boolean enable);

/* Get auto-subscription status */
OSErr GetAutoSubscription(SectionHandle subscriberH, Boolean* isEnabled);

/*
 * Change Detection
 */

/* Initialize change detection for a section */
OSErr InitializeChangeDetection(SectionHandle sectionH);

/* Register data for change monitoring */
OSErr RegisterDataForMonitoring(SectionHandle sectionH,
                               FormatType format,
                               const void* data,
                               Size dataSize);

/* Check for data changes */
OSErr DetectDataChanges(SectionHandle sectionH,
                       FormatType format,
                       Boolean* hasChanged);

/* Get change detection information */
OSErr GetChangeInfo(SectionHandle sectionH,
                   FormatType format,
                   FormatSyncInfo* syncInfo);

/* Reset change detection state */
OSErr ResetChangeDetection(SectionHandle sectionH, FormatType format);

/*
 * Conflict Resolution
 */

/* Set conflict resolution strategy */
OSErr SetConflictResolution(SectionHandle sectionH, ConflictResolution strategy);

/* Get conflict resolution strategy */
OSErr GetConflictResolution(SectionHandle sectionH, ConflictResolution* strategy);

/* Resolve synchronization conflict */
OSErr ResolveConflict(SectionHandle sectionH,
                     FormatType format,
                     const void* publisherData,
                     Size publisherSize,
                     const void* subscriberData,
                     Size subscriberSize,
                     void** resolvedData,
                     Size* resolvedSize);

/* Check for synchronization conflicts */
OSErr CheckForConflicts(SectionHandle sectionH, Boolean* hasConflicts);

/*
 * Performance and Optimization
 */

/* Set synchronization interval */
OSErr SetSyncInterval(UInt32 intervalMs);

/* Get synchronization interval */
OSErr GetSyncInterval(UInt32* intervalMs);

/* Enable/disable batched synchronization */
OSErr SetBatchedSync(Boolean enable);

/* Set sync batch size */
OSErr SetSyncBatchSize(UInt32 batchSize);

/* Enable/disable delta synchronization */
OSErr SetDeltaSync(SectionHandle sectionH, Boolean enable);

/* Get delta synchronization status */
OSErr GetDeltaSync(SectionHandle sectionH, Boolean* isEnabled);

/*
 * Statistics and Monitoring
 */

/* Get synchronization statistics */
OSErr GetSyncStatistics(SectionHandle sectionH, SyncStatistics* stats);

/* Reset synchronization statistics */
OSErr ResetSyncStatistics(SectionHandle sectionH);

/* Get global synchronization status */
OSErr GetGlobalSyncStatus(UInt32* activeSections,
                         UInt32* pendingSyncs,
                         UInt32* failedSyncs);

/* Enable/disable sync logging */
OSErr SetSyncLogging(Boolean enable, const char* logFilePath);

/*
 * Network and Remote Synchronization
 */

/* Enable network synchronization */
OSErr EnableNetworkSync(SectionHandle sectionH, const char* remoteAddress);

/* Disable network synchronization */
OSErr DisableNetworkSync(SectionHandle sectionH);

/* Set network sync credentials */
OSErr SetNetworkCredentials(const char* username, const char* password);

/* Get network sync status */
OSErr GetNetworkSyncStatus(SectionHandle sectionH,
                          Boolean* isConnected,
                          char* remoteAddress,
                          Size addressSize);

/*
 * Version Management
 */

/* Enable version tracking */
OSErr EnableVersionTracking(SectionHandle sectionH);

/* Get version history */
OSErr GetVersionHistory(SectionHandle sectionH,
                       FormatType format,
                       TimeStamp** versions,
                       UInt32* versionCount);

/* Revert to previous version */
OSErr RevertToVersion(SectionHandle sectionH,
                     FormatType format,
                     TimeStamp version);

/* Clean up old versions */
OSErr CleanupOldVersions(SectionHandle sectionH,
                        UInt32 keepCount);

/*
 * Advanced Synchronization Features
 */

/* Set custom synchronization filter */

OSErr SetSyncFilter(SectionHandle sectionH,
                   SyncFilterCallback filter,
                   void* userData);

/* Set data transformation callback */

OSErr SetDataTransform(SectionHandle sectionH,
                      FormatType format,
                      DataTransformCallback transform,
                      void* userData);

/* Enable/disable compression during sync */
OSErr SetSyncCompression(SectionHandle sectionH, Boolean enable);

/* Enable/disable encryption during sync */
OSErr SetSyncEncryption(SectionHandle sectionH,
                       Boolean enable,
                       const char* encryptionKey);

/*
 * Platform Integration Functions
 */

/* Associate with platform data sharing mechanism */
OSErr AssociateSectionWithPlatform(SectionHandle sectionH, const FSSpec* document);

/* Get edition modification time */
OSErr GetEditionModificationTime(const EditionContainerSpec* container,
                                TimeStamp* modTime);

/* Compare file specifications */
Boolean CompareFSSpec(const FSSpec* spec1, const FSSpec* spec2);

/* Initialize section formats for sync */
OSErr InitializeSectionFormats(void* syncState);

/*
 * Constants
 */

#define kDefaultSyncInterval 1000       /* 1 second */
#define kMinSyncInterval 100           /* 100ms minimum */
#define kMaxSyncInterval 60000         /* 60 seconds maximum */
#define kDefaultBatchSize 10           /* Default sync batch size */
#define kMaxVersionHistory 100         /* Maximum versions to keep */

/* Sync priority levels */

/* Error codes for synchronization */

#ifdef __cplusplus
}
#endif

#endif /* __DATA_SYNCHRONIZATION_H__ */
