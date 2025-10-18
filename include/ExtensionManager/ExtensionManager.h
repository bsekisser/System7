/*
 * ExtensionManager.h - System 7.1 Portable Extension Manager API
 *
 * Main API for managing system extensions (INITs), device drivers (DRVRs),
 * and control definitions (CDEFs). Handles discovery, loading, initialization,
 * and lifecycle management of system extensions.
 */

#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include "SystemTypes.h"
#include "ExtensionManager/ExtensionTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * EXTENSION MANAGER LIFECYCLE
 * ======================================================================== */

/**
 * Initialize the Extension Manager
 * Called during system startup after basic subsystems are ready
 *
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_Initialize(void);

/**
 * Shutdown Extension Manager and unload all extensions
 * Called during system shutdown
 */
void ExtensionManager_Shutdown(void);

/**
 * Check if Extension Manager is initialized
 * @return true if initialized, false otherwise
 */
Boolean ExtensionManager_IsInitialized(void);

/* ========================================================================
 * EXTENSION DISCOVERY AND SCANNING
 * ======================================================================== */

/**
 * Scan System Folder for extensions
 * Discovers all INIT resources in the System Folder
 *
 * @param rescan If true, ignore cache and re-scan filesystem
 * @return Number of extensions found
 */
SInt16 ExtensionManager_ScanForExtensions(Boolean rescan);

/**
 * Get count of discovered extensions
 * @return Number of extensions found but not yet loaded
 */
SInt16 ExtensionManager_GetDiscoveredCount(void);

/**
 * Get count of active (loaded and initialized) extensions
 * @return Number of active extensions
 */
SInt16 ExtensionManager_GetActiveCount(void);

/* ========================================================================
 * EXTENSION LOADING AND INITIALIZATION
 * ======================================================================== */

/**
 * Load and initialize all discovered INIT extensions in priority order
 * Called during system startup
 *
 * @return extNoErr if all loaded successfully, error code if any fail
 */
OSErr ExtensionManager_LoadAllExtensions(void);

/**
 * Load a specific extension by name
 * @param name Extension name to load
 * @param outRefNum If non-NULL, filled with extension reference number
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_LoadByName(const char *name, SInt16 *outRefNum);

/**
 * Load extension by resource ID
 * @param resourceType Resource type ('INIT', 'CDEF', etc.)
 * @param resourceID Resource ID in System file
 * @param outRefNum If non-NULL, filled with extension reference number
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_LoadByID(OSType resourceType, SInt16 resourceID,
                                 SInt16 *outRefNum);

/* ========================================================================
 * EXTENSION QUERIES AND ITERATION
 * ======================================================================== */

/**
 * Get extension by reference number
 * @param refNum Extension reference number
 * @return Pointer to extension record or NULL if not found
 */
Extension* ExtensionManager_GetByRefNum(SInt16 refNum);

/**
 * Get extension by name
 * @param name Extension name to find
 * @return Pointer to extension record or NULL if not found
 */
Extension* ExtensionManager_GetByName(const char *name);

/**
 * Get first extension in registry (for iteration)
 * @return Pointer to first extension or NULL if no extensions
 */
Extension* ExtensionManager_GetFirstExtension(void);

/**
 * Get next extension in registry (for iteration)
 * @param current Current extension pointer
 * @return Pointer to next extension or NULL if at end
 */
Extension* ExtensionManager_GetNextExtension(Extension *current);

/* ========================================================================
 * EXTENSION CONTROL
 * ======================================================================== */

/**
 * Enable or disable an extension
 * Takes effect on next system restart
 *
 * @param refNum Extension reference number
 * @param enable If true, enable; if false, disable
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_SetEnabled(SInt16 refNum, Boolean enable);

/**
 * Check if extension is enabled
 * @param refNum Extension reference number
 * @return true if enabled, false if disabled
 */
Boolean ExtensionManager_IsEnabled(SInt16 refNum);

/**
 * Unload an extension from memory
 * @param refNum Extension reference number
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_Unload(SInt16 refNum);

/**
 * Reload an extension
 * Unloads and re-initializes the extension
 *
 * @param refNum Extension reference number
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_Reload(SInt16 refNum);

/* ========================================================================
 * EXTENSION INFORMATION
 * ======================================================================== */

/**
 * Get extension name
 * @param refNum Extension reference number
 * @param outName Buffer to store name (MAX_EXTENSION_NAME bytes)
 * @param outNameLen If non-NULL, filled with name length (not including null)
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_GetName(SInt16 refNum, char *outName,
                                UInt16 *outNameLen);

/**
 * Get extension type
 * @param refNum Extension reference number
 * @return Extension type or EXTTYPE_UNKNOWN if not found
 */
ExtensionType ExtensionManager_GetType(SInt16 refNum);

/**
 * Get extension state
 * @param refNum Extension reference number
 * @return Current extension state
 */
ExtensionState ExtensionManager_GetState(SInt16 refNum);

/**
 * Get extension code size
 * @param refNum Extension reference number
 * @return Size in bytes or 0 if not found
 */
SInt32 ExtensionManager_GetCodeSize(SInt16 refNum);

/**
 * Get extension version
 * @param refNum Extension reference number
 * @param outMajor If non-NULL, filled with major version
 * @param outMinor If non-NULL, filled with minor version
 * @return extNoErr on success, error code on failure
 */
OSErr ExtensionManager_GetVersion(SInt16 refNum, SInt16 *outMajor,
                                   SInt16 *outMinor);

/* ========================================================================
 * EXTENSION STATISTICS AND DEBUGGING
 * ======================================================================== */

/**
 * Get total memory used by extensions
 * @return Total bytes allocated for extension code
 */
SInt32 ExtensionManager_GetTotalMemoryUsed(void);

/**
 * Get extension load statistics
 * @param outLoadTime If non-NULL, filled with total load time (ticks)
 * @param outInitTime If non-NULL, filled with total init time (ticks)
 * @return extNoErr on success
 */
OSErr ExtensionManager_GetLoadStatistics(SInt32 *outLoadTime,
                                         SInt32 *outInitTime);

/**
 * Dump extension registry to console (debug only)
 * Prints information about all loaded extensions
 */
void ExtensionManager_DumpRegistry(void);

/**
 * Enable/disable debug logging
 * @param enable If true, enable debug logging
 */
void ExtensionManager_SetDebugMode(Boolean enable);

/* ========================================================================
 * INTERNAL FUNCTIONS (Not for direct use)
 * ======================================================================== */

/**
 * Get the extension registry (internal use)
 * @return Pointer to extension registry
 */
ExtensionRegistry* ExtensionManager_GetRegistry(void);

/**
 * Register a discovered extension in the registry (internal)
 * @param extension Extension record to register
 * @return Reference number assigned or extMaxExtensions on error
 */
SInt16 ExtensionManager_RegisterExtension(Extension *extension);

#ifdef __cplusplus
}
#endif

#endif /* EXTENSIONMANAGER_H */
