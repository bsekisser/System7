/*
 * EditionManagerPrivate.h
 *
 * Private Edition Manager structures and internal API
 * Contains implementation details not exposed to applications
 */

#ifndef __EDITION_MANAGER_PRIVATE_H__
#define __EDITION_MANAGER_PRIVATE_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"
#include <pthread.h>

/* Internal data structures */

/* Global Edition Manager state */

/* Edition file control block */

/* Format converter structure */

/* Publisher control block (based on original Mac OS structure) */

/* Internal function prototypes */

/* Platform abstraction layer */
OSErr InitDataSharingPlatform(void);
void CleanupDataSharingPlatform(void);

OSErr RegisterSectionWithPlatform(SectionHandle sectionH, const FSSpec* document);
void UnregisterSectionFromPlatform(SectionHandle sectionH);
OSErr AssociateSectionWithPlatform(SectionHandle sectionH, const FSSpec* document);

OSErr PostSectionEventToPlatform(SectionHandle sectionH, AppRefNum toApp, ResType classID);
OSErr ProcessDataSharingBackground(void);

/* File system operations */
OSErr CreateAliasFromFSSpec(const FSSpec* fileSpec, Handle* alias);
OSErr UpdateAlias(const FSSpec* fileSpec, Handle alias, Boolean* wasUpdated);
OSErr GetEditionFileInfo(const FSSpec* fileSpec, EditionInfoRecord* info);

/* Edition file management */
OSErr CreateEditionFileInternal(const FSSpec* fileSpec, OSType creator);
OSErr OpenEditionFileInternal(const FSSpec* fileSpec, Boolean forWriting, EditionFileBlock** fileBlock);
OSErr CloseEditionFileInternal(EditionFileBlock* fileBlock);

/* Format conversion */
OSErr RegisterFormatConverter(FormatType fromFormat, FormatType toFormat,
                             OSErr (*convertProc)(const void*, Size, void**, Size*));
OSErr ConvertFormat(FormatType fromFormat, FormatType toFormat,
                   const void* inputData, Size inputSize,
                   void** outputData, Size* outputSize);

/* Data synchronization */
OSErr SynchronizePublisherData(SectionHandle publisherH);
OSErr SynchronizeSubscriberData(SectionHandle subscriberH);
OSErr NotifySubscribers(SectionHandle publisherH);

/* Thread safety */
void LockSectionList(void);
void UnlockSectionList(void);

/* Memory management */
Handle NewHandleFromData(const void* data, Size size);
OSErr ResizeHandle(Handle handle, Size newSize);
void DisposeHandleData(Handle handle);

/* Utility functions */
TimeStamp GetCurrentTimeStamp(void);
Boolean CompareFSSpec(const FSSpec* spec1, const FSSpec* spec2);
OSErr CopyFSSpec(const FSSpec* source, FSSpec* dest);

/* Error handling */
void LogEditionError(const char* function, OSErr error, const char* message);

/* Constants for internal use */
#define kMaxSupportedFormats 32
#define kDefaultIOBufferSize 8192
#define kEditionMagicNumber 0x45444954  /* 'EDIT' */
#define kClosedFile -1

/* Platform-specific constants */
#ifdef PLATFORM_REMOVED_WIN32
    #define kPlatformPathSeparator '\\'
    #define kPlatformMaxPath 260
#else
    #define kPlatformPathSeparator '/'
    #define kPlatformMaxPath 1024
#endif

/* Debug macros */
#ifdef DEBUG_EDITION_MANAGER
    #define EDITION_DEBUG(fmt, ...) printf("[EditionMgr] " fmt "\n", ##__VA_ARGS__)
    #define EDITION_ERROR(fmt, ...) fprintf(stderr, "[EditionMgr ERROR] " fmt "\n", ##__VA_ARGS__)
#else
    #define EDITION_DEBUG(fmt, ...)
    #define EDITION_ERROR(fmt, ...)
#endif

/* Global variables (defined in EditionManagerCore.c) */
extern EditionManagerGlobals* gEditionGlobals;
extern Boolean gEditionPackInitialized;

#endif /* __EDITION_MANAGER_PRIVATE_H__ */
