/*
 * PackageManager.h - System 7.1 Package Manager Interface
 *
 * The Package Manager provides a unified interface for loading and
 * dispatching PACK resources. These packages contain code for various
 * system services like Standard File, List Manager, and others.
 *
 * Copyright (c) 2024 System7.1-Portable Project
 * MIT License
 */

#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "../DeskManager/Types.h"
#include "MemoryMgr/memory_manager_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Package IDs (PACK resources) */

/* Package Manager error codes */

/* Package flags */

/* Package selectors for common packages */

/* Standard File Package (PACK 3) selectors */

/* List Manager (PACK 0) selectors */

/* Package resource header structure */

/* Package dispatch table entry */

/* Package parameter block */

/* Package Manager Functions */

/* Initialization */
OSErr InitPacks(void);
OSErr InitAllPacks(void);
void ShutdownPacks(void);

/* Package loading and unloading */
OSErr LoadPackage(short packID);
OSErr UnloadPackage(short packID);
OSErr ReloadPackage(short packID);
Boolean IsPackageLoaded(short packID);

/* Package information */
Handle GetPackageHandle(short packID);
OSErr GetPackageInfo(short packID, PackageHeader* info);
OSErr GetPackageVersion(short packID, short* version);
short GetPackageRefCount(short packID);

/* Package execution */
OSErr CallPackage(short packID, short selector, void* params);
OSErr CallPackageWithResult(short packID, short selector,
                           void* params, void* result);

/* Package trap dispatchers (A9E7-A9F6) */
OSErr Pack0(short selector, void* params);  /* List Manager */
OSErr Pack1(short selector, void* params);  /* Reserved */
OSErr Pack2(short selector, void* params);  /* Disk Init */
OSErr Pack3(short selector, void* params);  /* Standard File */
OSErr Pack4(short selector, void* params);  /* SANE */
OSErr Pack5(short selector, void* params);  /* ELEMS */
OSErr Pack6(short selector, void* params);  /* Intl Utils */
OSErr Pack7(short selector, void* params);  /* Binary-Decimal */
OSErr Pack8(short selector, void* params);  /* Apple Events */
OSErr Pack9(short selector, void* params);  /* PPC Toolbox */
OSErr Pack10(short selector, void* params); /* Edition Manager */
OSErr Pack11(short selector, void* params); /* Color Picker */
OSErr Pack12(short selector, void* params); /* Sound Manager */
OSErr Pack13(short selector, void* params); /* Reserved */
OSErr Pack14(short selector, void* params); /* Help Manager */
OSErr Pack15(short selector, void* params); /* Picture Utils */

/* Internal package initialization */
OSErr InitPackage(short packID);
void ReleasePackage(short packID);

/* Package registration (for modern implementation) */
OSErr RegisterPackage(short packID, ProcPtr dispatchProc);
OSErr UnregisterPackage(short packID);

/* Utility functions */
void LockPackage(short packID);
void UnlockPackage(short packID);
OSErr PurgePackages(void);
long GetPackagesMemoryUsage(void);

#ifdef __cplusplus
}
#endif

#endif /* PACKAGEMANAGER_H */