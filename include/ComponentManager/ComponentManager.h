/*
 * ComponentManager.h
 *
 * Main Component Manager API - System 7.1 Portable Implementation
 * Provides dynamic component loading and management for multimedia applications
 *
 * This is a portable implementation of the Apple Macintosh Component Manager
 * API from System 7.1, enabling modular multimedia functionality.
 */

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "ComponentTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Basic types */
/* OSType is defined in MacTypes.h */
/* OSErr is defined in MacTypes.h *//* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Error codes */
/* noErr is defined in MacTypes.h */
#define badComponentInstance   -3000
#define badComponentSelector   -3001
#define componentNotFound      -3002
#define componentDllLoadErr    -3003
#define componentDllEntryErr   -3004
#define componentVersionErr    -3005
#define componentSecurityErr   -3006

/* Component Manager constants */
#define kAppleManufacturer          'appl'
#define kComponentResourceType      'thng'

#define kAnyComponentType           0
#define kAnyComponentSubType        0
#define kAnyComponentManufacturer   0
#define kAnyComponentFlagsMask      0

#define cmpWantsRegisterMessage     (1L<<31)

/* Standard component selectors */
#define kComponentOpenSelect        -1
#define kComponentCloseSelect       -2
#define kComponentCanDoSelect       -3
#define kComponentVersionSelect     -4
#define kComponentRegisterSelect    -5
#define kComponentTargetSelect      -6
#define kComponentUnregisterSelect  -7

/* Component resource extension flags */
#define componentDoAutoVersion              (1<<0)
#define componentWantsUnregister            (1<<1)
#define componentAutoVersionIncludeFlags    (1<<2)
#define componentHasMultiplePlatforms       (1<<3)

/* Set default component flags */
#define defaultComponentIdentical                       0
#define defaultComponentAnyFlags                        (1<<0)
#define defaultComponentAnyManufacturer                 (1<<1)
#define defaultComponentAnySubType                      (1<<2)
#define defaultComponentAnyFlagsAnyManufacturer         (defaultComponentAnyFlags+defaultComponentAnyManufacturer)
#define defaultComponentAnyFlagsAnyManufacturerAnySubType (defaultComponentAnyFlags+defaultComponentAnyManufacturer+defaultComponentAnySubType)

/* Platform types */
#define gestaltPowerPC              2
#define gestalt68k                  1

/* Component description structure - defined in ComponentTypes.h */

/* Resource specification */

/* Component resource structure - defined in ComponentTypes.h */

/* Platform information for multi-platform components - defined in ComponentTypes.h */

/* Component resource extension */

/* Extended component resource with multiple platforms */

/* Component call parameters - defined in ComponentTypes.h */

/* Opaque component and instance handles - defined in ComponentTypes.h */

/* Component routine function pointer */

/* Component Manager API Functions */

/* Component Database Add, Delete, and Query Routines */
Component RegisterComponent(ComponentDescription* cd, ComponentRoutine componentEntryPoint,
                          SInt16 global, Handle componentName, Handle componentInfo, Handle componentIcon);
Component RegisterComponentResource(Handle tr, SInt16 global);
OSErr UnregisterComponent(Component aComponent);

Component FindNextComponent(Component aComponent, ComponentDescription* looking);
SInt32 CountComponents(ComponentDescription* looking);

OSErr GetComponentInfo(Component aComponent, ComponentDescription* cd,
                      Handle componentName, Handle componentInfo, Handle componentIcon);
SInt32 GetComponentListModSeed(void);

/* Component Instance Allocation and Dispatch Routines */
ComponentInstance OpenComponent(Component aComponent);
OSErr CloseComponent(ComponentInstance aComponentInstance);

OSErr GetComponentInstanceError(ComponentInstance aComponentInstance);
void SetComponentInstanceError(ComponentInstance aComponentInstance, OSErr theError);

/* Direct calls to components */
SInt32 ComponentFunctionImplemented(ComponentInstance ci, SInt16 ftnNumber);
SInt32 GetComponentVersion(ComponentInstance ci);
SInt32 ComponentSetTarget(ComponentInstance ci, ComponentInstance target);

/* Component Management Routines */
SInt32 GetComponentRefcon(Component aComponent);
void SetComponentRefcon(Component aComponent, SInt32 theRefcon);

SInt16 OpenComponentResFile(Component aComponent);
OSErr CloseComponentResFile(SInt16 refnum);

/* Component Instance Management Routines */
Handle GetComponentInstanceStorage(ComponentInstance aComponentInstance);
void SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage);

SInt32 GetComponentInstanceA5(ComponentInstance aComponentInstance);
void SetComponentInstanceA5(ComponentInstance aComponentInstance, SInt32 theA5);

SInt32 CountComponentInstances(Component aComponent);

/* Utility functions for component dispatching */
SInt32 CallComponentFunction(ComponentParameters* params, ComponentFunction func);
SInt32 CallComponentFunctionWithStorage(Handle storage, ComponentParameters* params, ComponentFunction func);
SInt32 DelegateComponentCall(ComponentParameters* originalParams, ComponentInstance ci);

/* Default component management */
OSErr SetDefaultComponent(Component aComponent, SInt16 flags);
ComponentInstance OpenDefaultComponent(OSType componentType, OSType componentSubType);

/* Component capture/delegation */
Component CaptureComponent(Component capturedComponent, Component capturingComponent);
OSErr UncaptureComponent(Component aComponent);

/* Resource file management */
OSErr RegisterComponentResourceFile(SInt16 resRefNum, SInt16 global);
OSErr GetComponentIconSuite(Component aComponent, Handle* iconSuite);

/* Internal Component Manager initialization */
OSErr InitComponentManager(void);
void CleanupComponentManager(void);

/* Thread safety functions for Component Manager */
OSErr CreateComponentMutex(ComponentMutex** mutex);
void DestroyComponentMutex(ComponentMutex* mutex);
void LockComponentMutex(ComponentMutex* mutex);
void UnlockComponentMutex(ComponentMutex* mutex);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTMANAGER_H */