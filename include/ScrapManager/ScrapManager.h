/*
 * ScrapManager.h - Main Scrap Manager API
 * System 7.1 Portable - Scrap Manager Component
 *
 * Main header file for the Mac OS Scrap Manager, providing clipboard functionality
 * for inter-application data exchange with modern platform integration.
 */

#ifndef SCRAP_MANAGER_H
#define SCRAP_MANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "ScrapTypes.h"
#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Core Scrap Manager Functions
 * The classic Mac OS Scrap Manager API functions are declared in ScrapTypes.h
 * with their authentic System 7 signatures
 */

/*
 * Extended Scrap Manager Functions
 * These provide additional functionality for modern systems
 */

/* Initialize the Scrap Manager */
OSErr InitScrapManager(void);

/* Cleanup and shutdown the Scrap Manager */
void CleanupScrapManager(void);

/* Get available scrap formats */
OSErr GetScrapFormats(ResType *types, SInt16 *count, SInt16 maxTypes);

/* Check if a specific format is available */
/* Note: ScrapTypes.h declares ScrapHasFlavor for this purpose */

/* Get the size of data for a specific format */
/* Note: ScrapTypes.h declares ScrapGetFlavorSize for this purpose */

/* Put data with format conversion */
OSErr PutScrapWithConversion(SInt32 length, ResType theType,
                            const void *source, Boolean allowConversion);

/* Get data with format conversion */
OSErr GetScrapWithConversion(Handle destHandle, ResType theType,
                            SInt32 *offset, Boolean allowConversion);

/* Register a format converter */
OSErr RegisterScrapConverter(ResType sourceType, ResType destType,
                            ScrapConverterProc converter, void *refCon);

/* Unregister a format converter */
OSErr UnregisterScrapConverter(ResType sourceType, ResType destType);

/*
 * Scrap File Management Functions
 */

/* Set the scrap file location */
OSErr SetScrapFile(ConstStr255Param fileName, SInt16 vRefNum, SInt32 dirID);

/* Get the current scrap file location */
OSErr GetScrapFile(Str255 fileName, SInt16 *vRefNum, SInt32 *dirID);

/* Force save scrap to file */
OSErr SaveScrapToFile(void);

/* Load scrap from a specific file */
OSErr LoadScrapFromFile(ConstStr255Param fileName, SInt16 vRefNum, SInt32 dirID);

/*
 * Memory Management Functions
 */

/* Set memory allocation preferences */
OSErr SetScrapMemoryPrefs(SInt32 memoryThreshold, SInt32 diskThreshold);

/* Get current memory usage */
OSErr GetScrapMemoryInfo(SInt32 *memoryUsed, SInt32 *diskUsed,
                        SInt32 *totalSize);

/* Compact scrap memory */
OSErr CompactScrapMemory(void);

/* Purge least recently used scrap data */
OSErr PurgeScrapData(SInt32 bytesToPurge);

/*
 * Inter-Application Functions
 */

/* Register for scrap change notifications */
OSErr RegisterScrapChangeCallback(ScrapChangeCallback callback, void *userData);

/* Unregister scrap change notifications */
OSErr UnregisterScrapChangeCallback(ScrapChangeCallback callback);

/* Get scrap ownership information */
OSErr GetScrapOwner(ProcessSerialNumber *psn, Str255 processName);

/* Set scrap ownership */
OSErr SetScrapOwner(const ProcessSerialNumber *psn);

/* Send scrap change notification */
void NotifyScrapChange(void);

/*
 * Modern Clipboard Integration Functions
 */

/* Initialize modern clipboard integration */
OSErr InitModernClipboard(void);

/* Shutdown modern clipboard integration */
void CleanupModernClipboard(void);

/* Sync with native platform clipboard */
OSErr SyncWithNativeClipboard(Boolean toNative);

/* Register platform format mapping */
OSErr RegisterPlatformFormat(ResType macType, UInt32 platformFormat,
                            const char *formatName);

/* Map Mac format to platform format */
UInt32 MacToPlatformFormat(ResType macType);

/* Map platform format to Mac format */
ResType PlatformToMacFormat(UInt32 platformFormat);

/* Check if native clipboard has changed */
Boolean HasNativeClipboardChanged(void);

/* Get data from native clipboard */
OSErr GetNativeClipboardData(UInt32 platformFormat, Handle *data);

/* Put data to native clipboard */
OSErr PutNativeClipboardData(UInt32 platformFormat, Handle data);

/*
 * Utility Functions
 */

/* Get scrap data type name */
void GetScrapTypeName(ResType theType, Str255 typeName);

/* Validate scrap data integrity */
OSErr ValidateScrapData(void);

/* Get scrap statistics */
OSErr GetScrapStats(UInt32 *putCount, UInt32 *getCount,
                   UInt32 *conversionCount, UInt32 *errorCount);

/* Reset scrap statistics */
void ResetScrapStats(void);

/* Enable/disable debug logging */
void SetScrapDebugMode(Boolean enable);

/* Get last error information */
OSErr GetLastScrapError(Str255 errorString);

/*
 * Legacy Compatibility Functions
 * These maintain compatibility with older System versions
 */

/* Get TextEdit scrap length (legacy) */
SInt16 TEGetScrapLength(void);

/* Copy from scrap to TextEdit (legacy) */
OSErr TEFromScrap(void);

/* Copy from TextEdit to scrap (legacy) */
OSErr TEToScrap(void);

/* Get scrap handle directly (legacy) */
Handle GetScrapHandle(void);

/* Set scrap handle directly (legacy) */
OSErr SetScrapHandle(Handle scrapHandle);

/*
 * Constants for backward compatibility
 */
#define InfoScrapTrap    0xA9F9
#define UnloadScrapTrap  0xA9FA
#define LoadScrapTrap    0xA9FB
#define ZeroScrapTrap    0xA9FC
#define GetScrapTrap     0xA9FD
#define PutScrapTrap     0xA9FE

/* Legacy error codes */
#define noScrapErr       scrapNoScrap
#define noTypeErr        scrapNoTypeError

#ifdef __cplusplus
}
#endif

#endif /* SCRAP_MANAGER_H */
