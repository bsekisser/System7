#include "SuperCompat.h"
#include <string.h>
/*
 * DriverLoader.c
 * System 7.1 Device Manager - Driver Loading and Resource Management
 *
 * Implements driver resource loading, validation, and installation
 * from both classic Mac OS 'DRVR' resources and modern driver formats.
 *
 */

#include "CompatibilityFix.h"
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DriverInterface.h"
#include "DeviceManager/UnitTable.h"
#include "ResourceManager.h"
#include "MemoryMgr/memory_manager_types.h"


/* =============================================================================
 * Constants
 * ============================================================================= */

#define DRVR_RESOURCE_TYPE      'DRVR'
#define DRIVER_SIGNATURE        0x4D41  /* 'MA' - Macintosh signature */
#define MAX_DRIVER_NAME_LENGTH  255
#define MAX_SEARCH_ATTEMPTS     16

/* =============================================================================
 * Global Variables
 * ============================================================================= */

static struct {
    UInt32 driversLoaded;
    UInt32 loadAttempts;
    UInt32 loadFailures;
    UInt32 validationFailures;
    UInt32 resourceNotFound;
    UInt32 memoryErrors;
} g_loaderStats = {0};

/* =============================================================================
 * Internal Function Declarations
 * ============================================================================= */

static SInt16 LoadDriverFromResource(const UInt8 *driverName, SInt16 resID, Handle *driverHandle);
static SInt16 LoadDriverFromSystemFile(const UInt8 *driverName, Handle *driverHandle);
static SInt16 LoadDriverFromSlotROM(const UInt8 *driverName, Handle *driverHandle);
static SInt16 ValidateDriverResource(Handle driverHandle);
static SInt16 ParseDriverName(const UInt8 *name, char *parsedName, SInt16 maxLen);
static Boolean CompareDriverNames(const UInt8 *name1, const UInt8 *name2);
static SInt16 GetDriverResourceID(const UInt8 *driverName);
static Handle SearchDriverResources(const UInt8 *driverName);
static SInt16 CreateDriverFromTemplate(const char *templateName, Handle *driverHandle);

/* =============================================================================
 * Driver Loading Functions
 * ============================================================================= */

Handle LoadDriverResource(const UInt8 *driverName, SInt16 resID)
{
    Handle driverHandle = NULL;
    SInt16 error;

    if (driverName == NULL) {
        g_loaderStats.loadFailures++;
        return NULL;
    }

    g_loaderStats.loadAttempts++;

    /* First try to load by resource ID if specified */
    if (resID != 0) {
        error = LoadDriverFromResource(driverName, resID, &driverHandle);
        if (error == noErr && driverHandle != NULL) {
            g_loaderStats.driversLoaded++;
            return driverHandle;
        }
    }

    /* Search by name in system file */
    error = LoadDriverFromSystemFile(driverName, &driverHandle);
    if (error == noErr && driverHandle != NULL) {
        g_loaderStats.driversLoaded++;
        return driverHandle;
    }

    /* Search in slot ROM */
    error = LoadDriverFromSlotROM(driverName, &driverHandle);
    if (error == noErr && driverHandle != NULL) {
        g_loaderStats.driversLoaded++;
        return driverHandle;
    }

    /* Try resource search */
    driverHandle = SearchDriverResources(driverName);
    if (driverHandle != NULL) {
        g_loaderStats.driversLoaded++;
        return driverHandle;
    }

    /* Try creating from template */
    char parsedName[256];
    if (ParseDriverName(driverName, parsedName, sizeof(parsedName)) == noErr) {
        error = CreateDriverFromTemplate(parsedName, &driverHandle);
        if (error == noErr && driverHandle != NULL) {
            g_loaderStats.driversLoaded++;
            return driverHandle;
        }
    }

    g_loaderStats.resourceNotFound++;
    g_loaderStats.loadFailures++;
    return NULL;
}

SInt16 FindDriverByName(const UInt8 *driverName)
{
    if (driverName == NULL) {
        return paramErr;
    }

    return UnitTable_FindByName(driverName);
}

SInt16 AllocateUnitTableEntry(void)
{
    return UnitTable_GetNextAvailableRefNum();
}

SInt16 DeallocateUnitTableEntry(SInt16 refNum)
{
    return UnitTable_DeallocateEntry(refNum);
}

/* =============================================================================
 * Resource Loading Implementation
 * ============================================================================= */

static SInt16 LoadDriverFromResource(const UInt8 *driverName, SInt16 resID, Handle *driverHandle)
{
    if (driverHandle == NULL) {
        return paramErr;
    }

    *driverHandle = NULL;

    /* Try to get resource by ID */
    Handle resHandle = GetResource(DRVR_RESOURCE_TYPE, resID);
    if (resHandle == NULL) {
        return resNotFound;
    }

    /* Validate the resource */
    SInt16 error = ValidateDriverResource(resHandle);
    if (error != noErr) {
        g_loaderStats.validationFailures++;
        ReleaseResource(resHandle);
        return error;
    }

    /* Detach resource so it won't be purged */
    DetachResource(resHandle);

    *driverHandle = resHandle;
    return noErr;
}

static SInt16 LoadDriverFromSystemFile(const UInt8 *driverName, Handle *driverHandle)
{
    if (driverName == NULL || driverHandle == NULL) {
        return paramErr;
    }

    *driverHandle = NULL;

    /* Get resource ID for driver name */
    SInt16 resID = GetDriverResourceID(driverName);
    if (resID <= 0) {
        return resNotFound;
    }

    /* Try to load from system file */
    Handle resHandle = GetResource(DRVR_RESOURCE_TYPE, resID);
    if (resHandle == NULL) {
        return resNotFound;
    }

    /* Validate driver */
    SInt16 error = ValidateDriverResource(resHandle);
    if (error != noErr) {
        g_loaderStats.validationFailures++;
        ReleaseResource(resHandle);
        return error;
    }

    /* Check name matches */
    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*resHandle;
    if (!CompareDriverNames(driverName, drvrPtr->drvrName)) {
        ReleaseResource(resHandle);
        return resNotFound;
    }

    DetachResource(resHandle);
    *driverHandle = resHandle;
    return noErr;
}

static SInt16 LoadDriverFromSlotROM(const UInt8 *driverName, Handle *driverHandle)
{
    if (driverName == NULL || driverHandle == NULL) {
        return paramErr;
    }

    *driverHandle = NULL;

    /* Placeholder for slot ROM driver loading */
    /* In the original Mac OS, this would search slot ROM for drivers */
    /* For our portable implementation, we'll simulate this */

    return resNotFound;
}

static Handle SearchDriverResources(const UInt8 *driverName)
{
    if (driverName == NULL) {
        return NULL;
    }

    /* Search through all DRVR resources */
    SInt16 resCount = CountResources(DRVR_RESOURCE_TYPE);
    for (SInt16 i = 1; i <= resCount; i++) {
        Handle resHandle = GetIndResource(DRVR_RESOURCE_TYPE, i);
        if (resHandle == NULL) {
            continue;
        }

        /* Validate and check name */
        if (ValidateDriverResource(resHandle) == noErr) {
            DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*resHandle;
            if (CompareDriverNames(driverName, drvrPtr->drvrName)) {
                DetachResource(resHandle);
                return resHandle;
            }
        }

        ReleaseResource(resHandle);
    }

    return NULL;
}

static SInt16 CreateDriverFromTemplate(const char *templateName, Handle *driverHandle)
{
    if (templateName == NULL || driverHandle == NULL) {
        return paramErr;
    }

    *driverHandle = NULL;

    /* Create a minimal driver template */
    /* This is used when no actual driver resource is found */

    size_t templateSize = sizeof(DriverHeader) + strlen(templateName) + 1;
    Handle h = NewHandle(templateSize);
    if (h == NULL) {
        g_loaderStats.memoryErrors++;
        return memFullErr;
    }

    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*h;
    memset(drvrPtr, 0, sizeof(DriverHeader));

    /* Set up minimal driver header */
    drvrPtr->drvrFlags = Read_Enable_Mask | Write_Enable_Mask |
                        Control_Enable_Mask | Status_Enable_Mask;
    drvrPtr->drvrDelay = 0;
    drvrPtr->drvrEMask = 0;
    drvrPtr->drvrMenu = 0;
    drvrPtr->drvrOpen = sizeof(DriverHeader);
    drvrPtr->drvrPrime = sizeof(DriverHeader) + 4;
    drvrPtr->drvrCtl = sizeof(DriverHeader) + 8;
    drvrPtr->drvrStatus = sizeof(DriverHeader) + 12;
    drvrPtr->drvrClose = sizeof(DriverHeader) + 16;

    /* Set driver name */
    UInt8 nameLen = strlen(templateName);
    if (nameLen > 255) nameLen = 255;
    drvrPtr->drvrName[0] = nameLen;
    memcpy(&drvrPtr->drvrName[1], templateName, nameLen);

    *driverHandle = h;
    return noErr;
}

/* =============================================================================
 * Driver Validation
 * ============================================================================= */

static SInt16 ValidateDriverResource(Handle driverHandle)
{
    if (driverHandle == NULL) {
        return paramErr;
    }

    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*driverHandle;
    if (drvrPtr == NULL) {
        return memFullErr;
    }

    size_t handleSize = GetHandleSize(driverHandle);
    if (handleSize < sizeof(DriverHeader)) {
        return dInstErr;
    }

    /* Validate driver header */
    return ValidateDriver(drvrPtr, handleSize) ? noErr : dInstErr;
}

/* =============================================================================
 * Name Parsing and Comparison
 * ============================================================================= */

static SInt16 ParseDriverName(const UInt8 *name, char *parsedName, SInt16 maxLen)
{
    if (name == NULL || parsedName == NULL || maxLen <= 0) {
        return paramErr;
    }

    /* Handle Pascal string */
    UInt8 nameLen = name[0];
    if (nameLen == 0 || nameLen >= maxLen) {
        return paramErr;
    }

    memcpy(parsedName, &name[1], nameLen);
    parsedName[nameLen] = '\0';

    return noErr;
}

static Boolean CompareDriverNames(const UInt8 *name1, const UInt8 *name2)
{
    if (name1 == NULL || name2 == NULL) {
        return false;
    }

    /* Both are Pascal strings */
    UInt8 len1 = name1[0];
    UInt8 len2 = name2[0];

    if (len1 != len2) {
        return false;
    }

    return memcmp(&name1[1], &name2[1], len1) == 0;
}

static SInt16 GetDriverResourceID(const UInt8 *driverName)
{
    if (driverName == NULL) {
        return 0;
    }

    /* Simple hash-based resource ID generation */
    /* In real Mac OS, this would be a more sophisticated lookup */
    UInt32 hash = 0;
    UInt8 nameLen = driverName[0];

    for (int i = 1; i <= nameLen; i++) {
        hash = hash * 31 + driverName[i];
    }

    /* Map to a reasonable resource ID range */
    return (SInt16)((hash % 1000) + 128);
}

/* =============================================================================
 * Driver Installation Helpers
 * ============================================================================= */

SInt16 InstallDriverResource(Handle driverResource, const UInt8 *driverName)
{
    if (driverResource == NULL) {
        return paramErr;
    }

    /* Validate the driver */
    SInt16 error = ValidateDriverResource(driverResource);
    if (error != noErr) {
        return error;
    }

    /* Get the driver header */
    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*driverResource;

    /* Allocate reference number */
    SInt16 refNum = AllocateUnitTableEntry();
    if (refNum < 0) {
        return refNum;
    }

    /* Install the driver */
    error = DrvrInstall(drvrPtr, refNum);
    if (error != noErr) {
        DeallocateUnitTableEntry(refNum);
        return error;
    }

    return refNum;
}

SInt16 RemoveDriverResource(SInt16 refNum)
{
    return DrvrRemove(refNum);
}

/* =============================================================================
 * Modern Driver Support
 * ============================================================================= */

SInt16 LoadModernDriver(const char *driverPath, ModernDriverInterfacePtr *driverInterface)
{
    if (driverPath == NULL || driverInterface == NULL) {
        return paramErr;
    }

    *driverInterface = NULL;

    /* Placeholder for loading modern drivers from shared libraries */
    /* In a full implementation, this would use dlopen/LoadLibrary */

    return fnfErr; /* Not implemented */
}

SInt16 UnloadModernDriver(ModernDriverInterfacePtr driverInterface)
{
    if (driverInterface == NULL) {
        return paramErr;
    }

    /* Cleanup modern driver */
    if (driverInterface->cleanup != NULL) {
        driverInterface->cleanup(driverInterface->driverContext);
    }

    return noErr;
}

/* =============================================================================
 * Driver Statistics and Information
 * ============================================================================= */

void GetDriverLoaderStats(void *stats)
{
    if (stats != NULL) {
        memcpy(stats, &g_loaderStats, sizeof(g_loaderStats));
    }
}

SInt16 GetLoadedDriverCount(void)
{
    return g_loaderStats.driversLoaded;
}

SInt16 EnumerateLoadedDrivers(void (*callback)(SInt16 refNum, const UInt8 *name, void *context), void *context)
{
    if (callback == NULL) {
        return paramErr;
    }

    /* Get all active reference numbers */
    SInt16 refNums[MAX_UNIT_TABLE_SIZE];
    SInt16 count = UnitTable_GetActiveRefNums(refNums, MAX_UNIT_TABLE_SIZE);

    for (SInt16 i = 0; i < count; i++) {
        DCEHandle dceHandle = UnitTable_GetDCE(refNums[i]);
        if (dceHandle != NULL) {
            DCEPtr dce = *dceHandle;
            if (dce != NULL && dce->dCtlDriver != NULL) {
                UInt8 driverName[256] = {0};

                /* Get driver name */
                if (dce->dCtlFlags & FollowsNewRules_Mask) {
                    /* Modern driver */
                    ModernDriverInterfacePtr modernIf = (ModernDriverInterfacePtr)dce->dCtlDriver;
                    if (modernIf->driverName != NULL) {
                        size_t nameLen = strlen(modernIf->driverName);
                        if (nameLen > 255) nameLen = 255;
                        driverName[0] = (UInt8)nameLen;
                        memcpy(&driverName[1], modernIf->driverName, nameLen);
                    }
                } else {
                    /* Classic driver */
                    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)dce->dCtlDriver;
                    memcpy(driverName, drvrPtr->drvrName, 256);
                }

                callback(refNums[i], driverName, context);
            }
        }
    }

    return count;
}

/* =============================================================================
 * Resource Management Utilities
 * ============================================================================= */

Boolean IsDriverResourceValid(Handle driverResource)
{
    return ValidateDriverResource(driverResource) == noErr;
}

SInt16 GetDriverResourceInfo(Handle driverResource, UInt8 *name, SInt16 *version, UInt32 *flags)
{
    if (driverResource == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateDriverResource(driverResource);
    if (error != noErr) {
        return error;
    }

    DriverHeaderPtr drvrPtr = (DriverHeaderPtr)*driverResource;

    if (name != NULL) {
        memcpy(name, drvrPtr->drvrName, drvrPtr->drvrName[0] + 1);
    }

    if (version != NULL) {
        *version = 1; /* Default version */
    }

    if (flags != NULL) {
        *flags = drvrPtr->drvrFlags;
    }

    return noErr;
}

SInt16 CloneDriverResource(Handle sourceDriver, Handle *clonedDriver)
{
    if (sourceDriver == NULL || clonedDriver == NULL) {
        return paramErr;
    }

    *clonedDriver = NULL;

    size_t size = GetHandleSize(sourceDriver);
    Handle clone = NewHandle(size);
    if (clone == NULL) {
        return memFullErr;
    }

    /* Copy driver data */
    memcpy(*clone, *sourceDriver, size);

    *clonedDriver = clone;
    return noErr;
}
