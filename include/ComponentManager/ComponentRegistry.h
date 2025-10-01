/*
 * ComponentRegistry.h
 *
 * Component Registry API - System 7.1 Portable Implementation
 * Manages component registration, discovery, and database operations
 */

#ifndef COMPONENTREGISTRY_H
#define COMPONENTREGISTRY_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Registry entry structure */

/* Registry management functions */
OSErr InitComponentRegistry(void);
void CleanupComponentRegistry(void);

/* Component registration functions */
Component RegisterComponentInternal(ComponentDescription* cd, ComponentRoutine entryPoint,
                                   SInt16 global, Handle name, Handle info, Handle icon,
                                   Boolean isResourceBased, SInt16 resFileRefNum);

OSErr UnregisterComponentInternal(Component component);

/* Component search and enumeration */
Component FindFirstComponent(ComponentDescription* looking);
Component FindNextComponentInternal(Component previous, ComponentDescription* looking);
SInt32 CountComponentsInternal(ComponentDescription* looking);

/* Component information retrieval */
OSErr GetComponentInfoInternal(Component component, ComponentDescription* cd,
                              Handle componentName, Handle componentInfo, Handle componentIcon);

ComponentRegistryEntry* GetComponentEntry(Component component);
Boolean ValidateComponent(Component component);

/* Registry database management */
SInt32 GetRegistryModificationSeed(void);
void InvalidateRegistryCache(void);

/* Component loading and unloading */
OSErr LoadComponentModule(ComponentRegistryEntry* entry);
OSErr UnloadComponentModule(ComponentRegistryEntry* entry);

/* Component resource management */
OSErr LoadComponentResources(ComponentRegistryEntry* entry);
OSErr UnloadComponentResources(ComponentRegistryEntry* entry);

/* Component capability queries */
Boolean ComponentMatchesDescription(ComponentRegistryEntry* entry, ComponentDescription* looking);
Boolean ComponentSupportsSelector(ComponentRegistryEntry* entry, SInt16 selector);

/* Registry iteration */

void IterateRegistry(ComponentRegistryIteratorFunc iterator, void* userData);

/* Platform-specific module functions */
#ifdef PLATFORM_REMOVED_WIN32
#include <windows.h>
/* Handle is defined in MacTypes.h */
#define COMPONENT_MODULE_INVALID NULL
#elif defined(__APPLE__) || defined(__linux__)
/* Handle is defined in MacTypes.h */
#define COMPONENT_MODULE_INVALID NULL
#else
/* Handle is defined in MacTypes.h */
#define COMPONENT_MODULE_INVALID NULL
#endif

ComponentModuleHandle LoadComponentPlatformModule(const char* path);
void UnloadComponentPlatformModule(ComponentModuleHandle handle);
ComponentRoutine GetComponentPlatformEntryPoint(ComponentModuleHandle handle, const char* entryName);

/* Resource file registration */
OSErr RegisterComponentsFromResourceFile(SInt16 resFileRefNum, SInt16 global);
OSErr UnregisterComponentsFromResourceFile(SInt16 resFileRefNum);

/* Component validation and security */
Boolean ValidateComponentModule(const char* path);
Boolean CheckComponentSecurity(ComponentRegistryEntry* entry);

/* Registry persistence */
OSErr SaveRegistryToFile(const char* path);
OSErr LoadRegistryFromFile(const char* path);

/* Default component management */
OSErr SetDefaultComponentInternal(Component component, SInt16 flags);
Component GetDefaultComponent(OSType componentType, OSType componentSubType);

/* Component capture/delegation management */
OSErr CaptureComponentInternal(Component capturedComponent, Component capturingComponent);
OSErr UncaptureComponentInternal(Component component);
Component GetCapturedComponent(Component component);
Component GetCapturingComponent(Component component);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTREGISTRY_H */