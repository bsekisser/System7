/*
 * DRVRLoader.h - DRVR (Device Driver) Resource Loader
 *
 * System 7.1 Portable Device Driver loader
 * Loads and registers device driver resources with the Device Manager
 */

#ifndef DRVR_LOADER_H
#define DRVR_LOADER_H

#include "SystemTypes.h"
#include "ExtensionTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * DEVICE DRIVER CONSTANTS
 * ======================================================================== */

#define MAX_DRIVERS                 32      /* Maximum device drivers */
#define MAX_DRIVER_NAME             64      /* Max driver name length */

/* Driver unit number range */
#define DRIVER_UNIT_MIN             0
#define DRIVER_UNIT_MAX             31

/* Driver open/close control codes */
#define DRIVER_CSOPEN               0       /* Open driver */
#define DRIVER_CSCLOSE              1       /* Close driver */
#define DRIVER_CSREAD               2       /* Read from driver */
#define DRIVER_CSWRITE              3       /* Write to driver */
#define DRIVER_CSCONTROL            4       /* Control operations */
#define DRIVER_CSSTATUS             5       /* Get driver status */

/* ========================================================================
 * DEVICE DRIVER ENTRY
 * ======================================================================== */

typedef struct DRVREntry {
    SInt16          resourceID;             /* Resource ID in System file */
    Handle          resourceHandle;         /* Loaded resource handle */
    SInt32          resourceSize;           /* Size in bytes */

    /* Entry point */
    DriverEntryProc driverProc;             /* Loaded entry point */

    /* Unit assignment */
    SInt16          unitNumber;             /* Device unit number */
    Boolean         registered;             /* Registered with Device Manager */

    /* Metadata */
    char            name[MAX_DRIVER_NAME]; /* Driver name */
    SInt16          version;                /* Driver version */
    SInt32          flags;                  /* Driver flags */

    /* Statistics */
    SInt32          openCount;              /* Number of times opened */

    /* Linked list for multiple drivers */
    struct DRVREntry *next;
    struct DRVREntry *prev;
} DRVREntry;

/* ========================================================================
 * DEVICE DRIVER LOADER API
 * ======================================================================== */

/**
 * Initialize DRVR Loader subsystem
 * @return extNoErr on success, error code on failure
 */
OSErr DRVRLoader_Initialize(void);

/**
 * Shutdown DRVR Loader (unload all registered drivers)
 */
void DRVRLoader_Shutdown(void);

/**
 * Load a DRVR resource and register with Device Manager
 * @param resourceID Resource ID of DRVR to load
 * @param outUnitNumber If non-NULL, filled with assigned unit number
 * @return extNoErr on success, error code on failure
 */
OSErr DRVRLoader_LoadDriver(SInt16 resourceID, SInt16 *outUnitNumber);

/**
 * Load all DRVR resources from System file
 * @return Number of drivers successfully loaded
 */
SInt16 DRVRLoader_LoadAllDrivers(void);

/**
 * Get DRVR entry by unit number
 * @param unitNumber Device unit number
 * @return Pointer to DRVR entry or NULL if not found
 */
DRVREntry* DRVRLoader_GetByUnitNumber(SInt16 unitNumber);

/**
 * Get DRVR entry by resource ID
 * @param resourceID Resource ID in System file
 * @return Pointer to DRVR entry or NULL if not found
 */
DRVREntry* DRVRLoader_GetByResourceID(SInt16 resourceID);

/**
 * Get DRVR entry by name
 * @param name Driver name to find
 * @return Pointer to DRVR entry or NULL if not found
 */
DRVREntry* DRVRLoader_GetByName(const char *name);

/**
 * Unload a DRVR and unregister from Device Manager
 * @param unitNumber Unit number to unload
 * @return extNoErr on success, error code on failure
 */
OSErr DRVRLoader_UnloadDriver(SInt16 unitNumber);

/**
 * Get count of loaded drivers
 * @return Number of loaded device drivers
 */
SInt16 DRVRLoader_GetLoadedCount(void);

/**
 * Call driver control code (for driver communication)
 * @param unitNumber Device unit number
 * @param csCode Control/status code
 * @param pb Parameter block (driver-specific)
 * @return Driver-specific error code
 */
OSErr DRVRLoader_CallDriver(SInt16 unitNumber, SInt16 csCode, void *pb);

/**
 * Dump DRVR registry (debug)
 */
void DRVRLoader_DumpRegistry(void);

/**
 * Check if DRVR Loader is initialized
 * @return true if initialized, false otherwise
 */
Boolean DRVRLoader_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* DRVR_LOADER_H */
