/*
 * CDEFLoader.h - CDEF (Control Definition) Resource Loader
 *
 * System 7.1 Portable CDEF resource loader and management
 * Loads and registers control definition resources with the ControlManager
 */

#ifndef CDEF_LOADER_H
#define CDEF_LOADER_H

#include "SystemTypes.h"
#include "ExtensionTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * CDEF RESOURCE STRUCTURE
 * ======================================================================== */

/* CDEF Resource Entry - metadata for a control definition */
typedef struct CDEFEntry {
    SInt16          resourceID;             /* Resource ID in System file */
    Handle          resourceHandle;         /* Loaded resource handle */
    SInt32          resourceSize;           /* Size in bytes */

    /* Entry point */
    ControlDefProcPtr defProc;              /* Loaded entry point */

    /* Registration with ControlManager */
    SInt16          procID;                 /* Control procedure ID */
    Boolean         registered;             /* Registered with ControlManager */

    /* Metadata */
    char            name[64];               /* CDEF name */
    SInt16          version;                /* CDEF version */

    /* Linked list for multiple CDEFs */
    struct CDEFEntry *next;
    struct CDEFEntry *prev;
} CDEFEntry;

/* ========================================================================
 * CDEF LOADER API
 * ======================================================================== */

/**
 * Initialize CDEF Loader subsystem
 * @return extNoErr on success, error code on failure
 */
OSErr CDEFLoader_Initialize(void);

/**
 * Shutdown CDEF Loader (unload all registered CDEFs)
 */
void CDEFLoader_Shutdown(void);

/**
 * Load a CDEF resource and register with ControlManager
 * @param resourceID Resource ID of CDEF to load
 * @param outProcID If non-NULL, filled with assigned proc ID
 * @return extNoErr on success, error code on failure
 */
OSErr CDEFLoader_LoadCDEF(SInt16 resourceID, SInt16 *outProcID);

/**
 * Load all CDEF resources from System file
 * @return Number of CDEFs successfully loaded
 */
SInt16 CDEFLoader_LoadAllCDEFs(void);

/**
 * Get CDEF entry by proc ID
 * @param procID Control procedure ID
 * @return Pointer to CDEF entry or NULL if not found
 */
CDEFEntry* CDEFLoader_GetByProcID(SInt16 procID);

/**
 * Get CDEF entry by resource ID
 * @param resourceID Resource ID in System file
 * @return Pointer to CDEF entry or NULL if not found
 */
CDEFEntry* CDEFLoader_GetByResourceID(SInt16 resourceID);

/**
 * Unload a CDEF and unregister from ControlManager
 * @param procID Procedure ID to unload
 * @return extNoErr on success, error code on failure
 */
OSErr CDEFLoader_UnloadCDEF(SInt16 procID);

/**
 * Get count of loaded CDEFs
 * @return Number of loaded control definitions
 */
SInt16 CDEFLoader_GetLoadedCount(void);

/**
 * Dump CDEF registry (debug)
 */
void CDEFLoader_DumpRegistry(void);

/**
 * Check if CDEF Loader is initialized
 * @return true if initialized, false otherwise
 */
Boolean CDEFLoader_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* CDEF_LOADER_H */
