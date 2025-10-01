/*
 * ComponentLoader.h
 *
 * Component Loader API - System 7.1 Portable Implementation
 * Handles dynamic loading and unloading of component modules
 */

#ifndef COMPONENTLOADER_H
#define COMPONENTLOADER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"
#include "ComponentRegistry.h"
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Component module types */

/* Component loader context */

/* Component loading flags */
#define kComponentLoadFlagLazy          (1<<0)  /* Lazy loading - load when first accessed */
#define kComponentLoadFlagGlobal        (1<<1)  /* Global loading - visible to all processes */
#define kComponentLoadFlagSecure        (1<<2)  /* Secure loading - verify signature */
#define kComponentLoadFlagSandbox       (1<<3)  /* Sandbox the component */
#define kComponentLoadFlagNetwork       (1<<4)  /* Allow network access */
#define kComponentLoadFlagDeep          (1<<5)  /* Deep binding - resolve all symbols */

/* Component loader initialization */
OSErr InitComponentLoader(void);
void CleanupComponentLoader(void);

/* Component module loading */
OSErr LoadComponent(ComponentRegistryEntry* entry, UInt32 loadFlags);
OSErr UnloadComponent(ComponentRegistryEntry* entry);
OSErr ReloadComponent(ComponentRegistryEntry* entry);

/* Component module discovery */
OSErr ScanForComponents(const char* searchPath, Boolean recursive);
OSErr RegisterComponentsInDirectory(const char* directoryPath, Boolean recursive);

/* Platform-specific loading functions */
OSErr LoadNativeComponent(ComponentLoaderContext* context);
OSErr LoadResourceComponent(ComponentLoaderContext* context);
OSErr LoadScriptComponent(ComponentLoaderContext* context);
OSErr LoadNetworkComponent(ComponentLoaderContext* context);

/* Component entry point resolution */
ComponentRoutine ResolveComponentEntryPoint(ComponentLoaderContext* context);
void* ResolveComponentSymbol(ComponentLoaderContext* context, const char* symbolName);

/* Component validation */
Boolean ValidateComponentModule(const char* modulePath);
Boolean VerifyComponentSignature(const char* modulePath);
OSErr CheckComponentCompatibility(ComponentLoaderContext* context);

/* Component metadata extraction */
OSErr ExtractComponentDescription(const char* modulePath, ComponentDescription* description);
OSErr ExtractComponentResources(const char* modulePath, Handle* name, Handle* info, Handle* icon);
SInt32 ExtractComponentVersion(const char* modulePath);

/* Resource-based component support */
OSErr LoadComponentFromResource(SInt16 resFileRefNum, SInt16 resID, ComponentRegistryEntry* entry);
OSErr LoadThingResource(SInt16 resFileRefNum, SInt16 resID, ComponentResource** resource);
OSErr LoadExtThingResource(SInt16 resFileRefNum, SInt16 resID, ExtComponentResource** resource);

/* Component hot-plugging support */

OSErr RegisterHotPlugCallback(ComponentHotPlugCallback callback, void* userData);
OSErr UnregisterHotPlugCallback(ComponentHotPlugCallback callback);
OSErr EnableComponentHotPlug(const char* watchPath);
OSErr DisableComponentHotPlug(void);

/* Component dependency management */

OSErr AddComponentDependency(ComponentRegistryEntry* entry, ComponentDependency* dependency);
OSErr RemoveComponentDependency(ComponentRegistryEntry* entry, ComponentDependency* dependency);
OSErr ResolveDependencies(ComponentRegistryEntry* entry);
OSErr CheckDependencies(ComponentRegistryEntry* entry);

/* Component sandboxing (platform-specific) */
#ifdef PLATFORM_REMOVED_WIN32
#include <windows.h>

#elif defined(__APPLE__)

#elif defined(__linux__)

#else

#endif

OSErr CreateComponentSandbox(ComponentLoaderContext* context, ComponentSandbox* sandbox);
OSErr DestroyComponentSandbox(ComponentSandbox* sandbox);
OSErr ExecuteInSandbox(ComponentSandbox* sandbox, ComponentRoutine routine, ComponentParameters* params);

/* Component search paths */
OSErr AddComponentSearchPath(const char* path);
OSErr RemoveComponentSearchPath(const char* path);
OSErr GetComponentSearchPaths(char*** paths, SInt32* count);
OSErr SetComponentSearchPaths(char** paths, SInt32 count);

/* Component caching */

OSErr InitComponentCache(ComponentCache* cache, const char* cachePath);
OSErr CleanupComponentCache(ComponentCache* cache);
OSErr CacheComponent(ComponentCache* cache, const char* modulePath);
OSErr GetCachedComponent(ComponentCache* cache, const char* modulePath, ComponentLoaderContext* context);
OSErr InvalidateComponentCache(ComponentCache* cache);

/* Error handling and debugging */

const char* GetComponentLoadErrorString(ComponentLoadError error);
OSErr GetLastComponentLoadError(ComponentLoadError* error, char** errorMessage);

/* Component performance monitoring */

OSErr GetComponentLoadStats(ComponentLoadStats* stats);
OSErr ResetComponentLoadStats(void);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTLOADER_H */