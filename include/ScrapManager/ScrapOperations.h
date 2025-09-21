/*
 * ScrapOperations.h - Scrap Operation APIs
 * System 7.1 Portable - Scrap Manager Component
 *
 * Defines high-level scrap operation functions for data transfer,
 * clipboard management, and inter-application communication.
 */

#ifndef SCRAP_OPERATIONS_H
#define SCRAP_OPERATIONS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "ScrapTypes.h"
#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * High-Level Scrap Operations
 */

/* Copy data to scrap with multiple formats */
OSErr CopyToScrap(const void *data, SInt32 size, ResType primaryType,
                 const ResType *additionalTypes, SInt16 typeCount);

/* Paste data from scrap with format preference */
OSErr PasteFromScrap(Handle *data, ResType *actualType,
                    const ResType *preferredTypes, SInt16 typeCount);

/* Copy text with automatic format detection */
OSErr CopyTextToScrap(const void *textData, SInt32 textSize,
                     Boolean includeFormatting);

/* Paste text with format conversion */
OSErr PasteTextFromScrap(Handle *textData, ResType preferredFormat);

/* Copy image with multiple format variants */
OSErr CopyImageToScrap(Handle imageData, ResType sourceFormat);

/* Paste image with format conversion */
OSErr PasteImageFromScrap(Handle *imageData, ResType preferredFormat,
                         SInt16 maxWidth, SInt16 maxHeight);

/* Copy file references to scrap */
OSErr CopyFilesToScrap(const FSSpec *files, SInt16 fileCount);

/* Paste file references from scrap */
OSErr PasteFilesFromScrap(FSSpec **files, SInt16 *fileCount);

/*
 * Batch Operations
 */

/* Copy multiple data items as a batch */
OSErr CopyBatchToScrap(const void **dataItems, const SInt32 *dataSizes,
                      const ResType *dataTypes, SInt16 itemCount);

/* Paste multiple data items as a batch */
OSErr PasteBatchFromScrap(void ***dataItems, SInt32 **dataSizes,
                         ResType **dataTypes, SInt16 *itemCount,
                         const ResType *requestedTypes, SInt16 requestCount);

/* Clear and replace entire scrap contents */
OSErr ReplaceScrapContents(const void **dataItems, const SInt32 *dataSizes,
                          const ResType *dataTypes, SInt16 itemCount);

/*
 * Streaming Operations
 */

/* Begin streaming data to scrap */
OSErr BeginScrapStream(ResType dataType, SInt32 estimatedSize,
                      SInt32 *streamID);

/* Write data to scrap stream */
OSErr WriteScrapStream(SInt32 streamID, const void *data, SInt32 size);

/* End streaming data to scrap */
OSErr EndScrapStream(SInt32 streamID);

/* Begin streaming data from scrap */
OSErr BeginScrapRead(ResType dataType, SInt32 *streamID, SInt32 *totalSize);

/* Read data from scrap stream */
OSErr ReadScrapStream(SInt32 streamID, void *buffer, SInt32 *bufferSize);

/* End streaming data from scrap */
OSErr EndScrapRead(SInt32 streamID);

/* Cancel streaming operation */
OSErr CancelScrapStream(SInt32 streamID);

/*
 * Asynchronous Operations
 */

/* Asynchronously copy data to scrap */
OSErr AsyncCopyToScrap(const void *data, SInt32 size, ResType dataType,
                      ScrapIOCallback completion, void *userData);

/* Asynchronously paste data from scrap */
OSErr AsyncPasteFromScrap(ResType dataType, ScrapIOCallback completion,
                         void *userData);

/* Check status of asynchronous operation */
OSErr CheckAsyncOperation(SInt32 operationID, Boolean *isComplete,
                         OSErr *result);

/* Cancel asynchronous operation */
OSErr CancelAsyncOperation(SInt32 operationID);

/*
 * Scrap Content Analysis
 */

/* Get summary of scrap contents */
OSErr GetScrapSummary(SInt16 *formatCount, SInt32 *totalSize,
                     ResType *primaryFormat);

/* Check if scrap contains specific content type */
Boolean ScrapContainsText(void);
Boolean ScrapContainsImages(void);
Boolean ScrapContainsFiles(void);
Boolean ScrapContainsSound(void);

/* Get best available format for purpose */
ResType GetBestTextFormat(void);
ResType GetBestImageFormat(void);
ResType GetBestSoundFormat(void);

/* Estimate data transfer time */
OSErr EstimateTransferTime(ResType dataType, SInt32 *milliseconds);

/*
 * Scrap History and Undo
 */

/* Save current scrap state for undo */
OSErr SaveScrapState(SInt32 *stateID);

/* Restore scrap from saved state */
OSErr RestoreScrapState(SInt32 stateID);

/* Clear saved scrap state */
OSErr ClearScrapState(SInt32 stateID);

/* Get scrap history count */
SInt16 GetScrapHistoryCount(void);

/* Clear all scrap history */
void ClearScrapHistory(void);

/*
 * Content Filtering and Validation
 */

/* Set content filter for incoming data */
OSErr SetScrapContentFilter(ResType dataType, Boolean (*filterFunc)(const void*, SInt32));

/* Remove content filter */
OSErr RemoveScrapContentFilter(ResType dataType);

/* Validate scrap content before paste */
OSErr ValidateScrapContent(ResType dataType, Boolean *isValid, Str255 reason);

/* Sanitize scrap content */
OSErr SanitizeScrapContent(ResType dataType, Handle *sanitizedData);

/*
 * Scrap Locking and Sharing
 */

/* Lock scrap for exclusive access */
OSErr LockScrap(SInt32 timeout, SInt32 *lockID);

/* Unlock scrap */
OSErr UnlockScrap(SInt32 lockID);

/* Check if scrap is locked */
Boolean IsScrapLocked(void);

/* Share scrap with specific applications */
OSErr ShareScrapWith(const ProcessSerialNumber *targetApps, SInt16 appCount);

/* Restrict scrap access */
OSErr RestrictScrapAccess(const ProcessSerialNumber *allowedApps, SInt16 appCount);

/* Clear access restrictions */
OSErr ClearScrapRestrictions(void);

/*
 * Performance and Monitoring
 */

/* Monitor scrap performance */
OSErr EnableScrapMonitoring(Boolean enable);

/* Get performance statistics */
OSErr GetScrapPerformanceStats(UInt32 *copyCount, UInt32 *pasteCount,
                              UInt32 *avgCopyTime, UInt32 *avgPasteTime);

/* Reset performance counters */
void ResetScrapPerformanceStats(void);

/* Set performance options */
OSErr SetScrapPerformanceOptions(Boolean enableCaching, Boolean enableCompression,
                                Boolean enablePrefetch);

/*
 * Error Handling and Recovery
 */

/* Set error handler for operations */
OSErr SetScrapErrorHandler(OSErr (*errorHandler)(OSErr, const char*, void*),
                          void *userData);

/* Get detailed error information */
OSErr GetScrapErrorDetails(OSErr errorCode, Str255 description,
                          Str255 suggestion);

/* Attempt to recover from error */
OSErr RecoverFromScrapError(OSErr errorCode);

/* Check scrap integrity */
OSErr CheckScrapIntegrity(Boolean *isValid, SInt32 *errorCount);

/* Repair scrap data if possible */
OSErr RepairScrapData(void);

/*
 * Debugging and Diagnostics
 */

/* Dump scrap contents to debug log */
void DumpScrapContents(void);

/* Trace scrap operations */
OSErr EnableScrapTracing(Boolean enable, const char *logFile);

/* Get memory usage details */
OSErr GetScrapMemoryDetails(SInt32 *heapUsed, SInt32 *tempUsed,
                           SInt32 *diskUsed, SInt32 *peakUsage);

/* Force garbage collection */
OSErr ForceScrapGC(void);

/*
 * Legacy Support Functions
 */

/* Get scrap in legacy format */
OSErr GetLegacyScrap(Handle *data, ResType dataType);

/* Put scrap in legacy format */
OSErr PutLegacyScrap(Handle data, ResType dataType);

/* Convert legacy scrap to modern format */
OSErr ConvertLegacyScrap(void);

/* Check if legacy format conversion is needed */
Boolean NeedsLegacyConversion(void);

/*
 * Operation Flags
 */
#define SCRAP_OP_SYNC           0x0001    /* Synchronous operation */
#define SCRAP_OP_ASYNC          0x0002    /* Asynchronous operation */
#define SCRAP_OP_NO_CONVERT     0x0004    /* Disable format conversion */
#define SCRAP_OP_FORCE_CONVERT  0x0008    /* Force format conversion */
#define SCRAP_OP_COMPRESS       0x0010    /* Compress data */
#define SCRAP_OP_ENCRYPT        0x0020    /* Encrypt sensitive data */
#define SCRAP_OP_VALIDATE       0x0040    /* Validate data integrity */
#define SCRAP_OP_NO_CACHE       0x0080    /* Disable caching */

/* Copy/Paste operation priorities */
#define SCRAP_PRIORITY_LOW      0
#define SCRAP_PRIORITY_NORMAL   1
#define SCRAP_PRIORITY_HIGH     2

#include "SystemTypes.h"
#define SCRAP_PRIORITY_CRITICAL 3

/* Stream buffer sizes */
#define SCRAP_STREAM_BUFFER_SMALL   1024
#define SCRAP_STREAM_BUFFER_MEDIUM  8192
#define SCRAP_STREAM_BUFFER_LARGE   32768

#ifdef __cplusplus
}
#endif

#endif /* SCRAP_OPERATIONS_H */
