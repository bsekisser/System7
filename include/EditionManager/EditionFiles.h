/*
 * EditionFiles.h
 *
 * Edition File Management API for Edition Manager
 * Handles file storage, format management, and metadata operations
 */

#ifndef __EDITION_FILES_H__
#define __EDITION_FILES_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "EditionManager/EditionManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * File Creation and Management
 */

/* Initialize an edition file with metadata */
OSErr InitializeEditionFile(const FSSpec* fileSpec, SectionHandle publisherH);

/* Verify edition file integrity */
OSErr VerifyEditionFile(const FSSpec* fileSpec, Boolean* isValid);

/* Repair corrupted edition file */
OSErr RepairEditionFile(const FSSpec* fileSpec);

/* Get edition file statistics */

OSErr GetEditionFileStats(const FSSpec* fileSpec, EditionFileStats* stats);

/*
 * Format Management
 */

/* Add a new format to an edition file */
OSErr AddFormatToEdition(EditionRefNum refNum, FormatType format);

/* Remove a format from an edition file */
OSErr RemoveFormatFromEdition(EditionRefNum refNum, FormatType format);

/* Get list of all formats in an edition */
OSErr GetEditionFormats(EditionRefNum refNum,
                       FormatType** formats,
                       UInt32* formatCount);

/* Check if edition supports a specific format */
OSErr EditionSupportsFormat(EditionRefNum refNum,
                           FormatType format,
                           Boolean* isSupported);

/* Get format data size */
OSErr GetFormatDataSize(EditionRefNum refNum,
                       FormatType format,
                       Size* dataSize);

/* Get format metadata */

OSErr GetFormatMetadata(EditionRefNum refNum,
                       FormatType format,
                       FormatMetadata* metadata);

/*
 * Data I/O Operations
 */

/* Read data from specific format at offset */
OSErr ReadFormatDataAtOffset(EditionRefNum refNum,
                            FormatType format,
                            UInt32 offset,
                            void* buffer,
                            Size* bufferSize);

/* Write data to specific format at offset */
OSErr WriteFormatDataAtOffset(EditionRefNum refNum,
                             FormatType format,
                             UInt32 offset,
                             const void* data,
                             Size dataSize);

/* Append data to a format */
OSErr AppendFormatData(EditionRefNum refNum,
                      FormatType format,
                      const void* data,
                      Size dataSize);

/* Truncate format data */
OSErr TruncateFormatData(EditionRefNum refNum,
                        FormatType format,
                        Size newSize);

/* Copy data between formats */
OSErr CopyFormatData(EditionRefNum sourceRef,
                    FormatType sourceFormat,
                    EditionRefNum destRef,
                    FormatType destFormat);

/*
 * File Position and Mark Management
 */

/* Set format mark for reading/writing */
OSErr SetFormatMarkInEditionFile(void* fileBlock, FormatType format, UInt32 mark);

/* Get current format mark */
OSErr GetFormatMarkFromEditionFile(void* fileBlock, FormatType format, UInt32* mark);

/* Seek to beginning of format data */
OSErr SeekToFormatStart(EditionRefNum refNum, FormatType format);

/* Seek to end of format data */
OSErr SeekToFormatEnd(EditionRefNum refNum, FormatType format);

/* Get current position in format data */
OSErr GetFormatPosition(EditionRefNum refNum,
                       FormatType format,
                       UInt32* position);

/*
 * File Synchronization and Integrity
 */

/* Force file sync to storage */
OSErr SyncEditionFile(void* fileBlock);

/* Update file metadata */
OSErr UpdateEditionFileMetadata(void* fileBlock);

/* Lock edition file for exclusive access */
OSErr LockEditionFile(EditionRefNum refNum, Boolean exclusive);

/* Unlock edition file */
OSErr UnlockEditionFile(EditionRefNum refNum);

/* Check if file is locked */
OSErr IsEditionFileLocked(EditionRefNum refNum, Boolean* isLocked);

/*
 * Backup and Recovery
 */

/* Create backup of edition file */
OSErr BackupEditionFile(const FSSpec* sourceFile, const FSSpec* backupFile);

/* Restore edition file from backup */
OSErr RestoreEditionFile(const FSSpec* backupFile, const FSSpec* targetFile);

/* Get backup file path */
OSErr GetBackupFilePath(const FSSpec* editionFile, FSSpec* backupFile);

/*
 * Metadata Management
 */

/* Set publisher information in edition metadata */
OSErr SetPublisherMetadata(EditionRefNum refNum,
                          SectionHandle publisherH,
                          const FSSpec* publisherDoc);

/* Get publisher information from edition metadata */
OSErr GetPublisherMetadata(EditionRefNum refNum,
                          SectionType* publisherType,
                          SInt32* publisherID,
                          OSType* publisherApp);

/* Update subscriber count */
OSErr UpdateSubscriberCount(EditionRefNum refNum, SInt32 delta);

/* Get subscriber count */
OSErr GetSubscriberCount(EditionRefNum refNum, UInt32* count);

/* Set custom metadata */
OSErr SetCustomMetadata(EditionRefNum refNum,
                       OSType metadataType,
                       const void* data,
                       Size dataSize);

/* Get custom metadata */
OSErr GetCustomMetadata(EditionRefNum refNum,
                       OSType metadataType,
                       void** data,
                       Size* dataSize);

/*
 * File Utilities
 */

/* Get edition file info */
OSErr GetEditionFileInfo(const FSSpec* fileSpec, EditionInfoRecord* info);

/* Check if file is an edition container */
OSErr IsEditionContainer(const FSSpec* fileSpec, Boolean* isEdition);

/* Get edition file type from content */
OSErr DetermineEditionFileType(const FSSpec* fileSpec, OSType* fileType);

/* Compact edition file (remove unused space) */
OSErr CompactEditionFile(const FSSpec* fileSpec);

/* Get file usage statistics */

OSErr GetFileUsageStats(EditionRefNum refNum, FileUsageStats* stats);

/*
 * Error Recovery and Diagnostics
 */

/* Validate file structure */
OSErr ValidateFileStructure(const FSSpec* fileSpec,
                           Boolean* isValid,
                           char* errorMessage,
                           Size messageSize);

/* Check for file corruption */
OSErr CheckFileCorruption(const FSSpec* fileSpec,
                         Boolean* isCorrupted,
                         UInt32* corruptionFlags);

/* Attempt to recover data from corrupted file */
OSErr RecoverDataFromCorruptedFile(const FSSpec* corruptedFile,
                                  const FSSpec* recoveredFile,
                                  FormatType* recoveredFormats,
                                  UInt32* formatCount);

/*
 * Performance and Optimization
 */

/* Set file caching options */
OSErr SetFileCaching(EditionRefNum refNum, Boolean enableCaching, Size cacheSize);

/* Prefetch format data */
OSErr PrefetchFormatData(EditionRefNum refNum, FormatType format);

/* Set read-ahead buffer size */
OSErr SetReadAheadSize(EditionRefNum refNum, Size bufferSize);

/* Get I/O performance statistics */

OSErr GetIOStats(EditionRefNum refNum, IOStats* stats);

/* Reset I/O statistics */
OSErr ResetIOStats(EditionRefNum refNum);

/*
 * Platform Abstraction
 */

/* Get platform-specific file handle */
OSErr GetPlatformFileHandle(EditionRefNum refNum, void** handle);

/* Set platform-specific file attributes */
OSErr SetPlatformFileAttributes(const FSSpec* fileSpec, UInt32 attributes);

/* Get platform-specific file attributes */
OSErr GetPlatformFileAttributes(const FSSpec* fileSpec, UInt32* attributes);

/*
 * Constants and Limits
 */

#define kMaxFormatsPerEdition 256
#define kMaxEditionFileSize (100 * 1024 * 1024)  /* 100MB */
#define kMinIOBufferSize 4096
#define kMaxIOBufferSize (1024 * 1024)           /* 1MB */
#define kDefaultReadAheadSize 32768               /* 32KB */

/* File flags */

/* Format flags */

/* Corruption flags */

/* Error codes specific to file operations */

#ifdef __cplusplus
}
#endif

#endif /* __EDITION_FILES_H__ */
