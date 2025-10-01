/*
 * ComponentResources.h
 *
 * Component Resources API - System 7.1 Portable Implementation
 * Handles component resource loading and management (thng resources)
 */

#ifndef COMPONENTRESOURCES_H
#define COMPONENTRESOURCES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Resource types */
#define kThingResourceType          'thng'  /* Component resource type */
#define kStringResourceType         'STR '  /* String resource type */
#define kStringListResourceType     'STR#'  /* String list resource type */
#define kIconResourceType           'ICON'  /* Icon resource type */
#define kIconFamilyResourceType     'icns'  /* Icon family resource type */
#define kVersionResourceType        'vers'  /* Version resource type */
#define kCodeResourceType           'CODE'  /* Code resource type */

/* Resource file management */

/* Resource initialization */
OSErr InitComponentResources(void);
void CleanupComponentResources(void);

/* Resource file operations */
SInt16 OpenComponentResourceFile(Component component);
OSErr CloseComponentResourceFile(SInt16 refNum);
OSErr RegisterComponentResourceFile(SInt16 resRefNum, SInt16 global);

/* Component resource loading */
OSErr LoadComponentResource(SInt16 resRefNum, OSType resType, SInt16 resID, Handle* resource);
OSErr LoadThingResource(SInt16 resRefNum, SInt16 resID, ComponentResource** resource);
OSErr LoadExtThingResource(SInt16 resRefNum, SInt16 resID, ExtComponentResource** resource);

/* Component resource parsing */
OSErr ParseComponentResource(Handle resourceHandle, ComponentResource** component);
OSErr ParseExtComponentResource(Handle resourceHandle, ExtComponentResource** component);
OSErr ExtractComponentDescriptionFromResource(ComponentResource* resource, ComponentDescription* description);

/* Component metadata extraction */
OSErr GetComponentName(Component component, Handle* nameHandle, char** nameString);
OSErr GetComponentInfoFromResource(Component component, Handle* infoHandle, char** infoString);
OSErr GetComponentIconFromResource(Component component, Handle* iconHandle);
OSErr GetComponentIconFamilyFromResource(Component component, Handle* iconFamilyHandle);
OSErr GetComponentVersionFromResource(Component component, SInt32* version);

/* Platform information handling */
OSErr GetComponentPlatformInfo(ExtComponentResource* resource, SInt16 platformType, ComponentPlatformInfo** info);
OSErr SelectBestPlatform(ExtComponentResource* resource, ComponentPlatformInfo** selectedInfo);
SInt16 GetCurrentPlatformType(void);

/* Resource creation and modification */
OSErr CreateComponentResource(ComponentDescription* description, ComponentResource** resource);
OSErr CreateExtComponentResource(ComponentDescription* description, ExtComponentResource** resource);
OSErr AddPlatformInfo(ExtComponentResource* resource, ComponentPlatformInfo* platformInfo);

/* Resource validation */
Boolean ValidateComponentResource(ComponentResource* resource);
Boolean ValidateExtComponentResource(ExtComponentResource* resource);
OSErr CheckResourceIntegrity(SInt16 resRefNum, OSType resType, SInt16 resID);

/* String resource utilities */
OSErr LoadStringResource(SInt16 resRefNum, SInt16 resID, char** string);
OSErr LoadStringListResource(SInt16 resRefNum, SInt16 resID, char*** strings, SInt32* count);
OSErr CreateStringResource(const char* string, Handle* resourceHandle);

/* Icon resource utilities */
OSErr LoadIconResource(SInt16 resRefNum, SInt16 resID, void** iconData, UInt32* iconSize);
OSErr CreateIconFromFile(const char* iconFilePath, Handle* iconHandle);
OSErr ConvertIconFormat(Handle srcIcon, UInt32 srcFormat, Handle* dstIcon, UInt32 dstFormat);

/* Version resource utilities */

OSErr LoadVersionResource(SInt16 resRefNum, SInt16 resID, ComponentVersionInfo* versionInfo);
OSErr CreateVersionResource(ComponentVersionInfo* versionInfo, Handle* resourceHandle);
SInt32 CompareVersions(ComponentVersionInfo* version1, ComponentVersionInfo* version2);

/* Cross-platform resource format support */

OSErr DetectResourceFormat(const char* filePath, ResourceFormat* format);
OSErr ConvertResourceFormat(const char* srcPath, ResourceFormat srcFormat,
                           const char* dstPath, ResourceFormat dstFormat);

/* Resource caching */

OSErr InitResourceCache(ResourceCache* cache, UInt32 maxCount);
OSErr CleanupResourceCache(ResourceCache* cache);
OSErr CacheResource(ResourceCache* cache, OSType resType, SInt16 resID, Handle resource);
Handle GetCachedResource(ResourceCache* cache, OSType resType, SInt16 resID);
OSErr InvalidateResourceCache(ResourceCache* cache);

/* Resource enumeration */

OSErr EnumerateResources(SInt16 resRefNum, OSType resType, ResourceEnumeratorFunc enumerator, void* userData);
OSErr EnumerateComponentResources(SInt16 resRefNum, ResourceEnumeratorFunc enumerator, void* userData);

/* Resource dependency tracking */

OSErr AddResourceDependency(Component component, ResourceDependency* dependency);
OSErr RemoveResourceDependency(Component component, ResourceDependency* dependency);
OSErr ResolveResourceDependencies(Component component);

/* Resource localization support */

OSErr LoadLocalizedResource(SInt16 resRefNum, OSType resType, SInt16 resID,
                           SInt16 languageCode, SInt16 regionCode, Handle* resource);
OSErr GetBestLocalizedResource(LocalizedResource* resources, SInt16 preferredLanguage,
                              SInt16 preferredRegion, Handle* bestResource);

/* Resource compression support */

OSErr CompressResource(Handle resource, ResourceCompressionType compression, Handle* compressedResource);
OSErr DecompressResource(Handle compressedResource, ResourceCompressionType compression, Handle* resource);
OSErr GetResourceCompressionInfo(Handle resource, ResourceCompressionType* compression, UInt32* originalSize);

/* Resource security and validation */
OSErr ValidateResourceSignature(SInt16 resRefNum, OSType resType, SInt16 resID);
OSErr SignResource(Handle resource, const char* privateKeyPath);
OSErr VerifyResourceSignature(Handle resource, const char* publicKeyPath);

/* Resource file format utilities */

OSErr ReadResourceFileHeader(SInt16 resRefNum, ResourceFileHeader* header);
OSErr WriteResourceFileHeader(SInt16 resRefNum, ResourceFileHeader* header);
OSErr ValidateResourceFile(SInt16 resRefNum);

/* Memory management for resources */
OSErr LoadResourceToMemory(SInt16 resRefNum, OSType resType, SInt16 resID, void** data, UInt32* size);
OSErr SaveResourceFromMemory(SInt16 resRefNum, OSType resType, SInt16 resID, void* data, UInt32 size);
OSErr EstimateResourceMemoryUsage(SInt16 resRefNum, UInt32* totalSize);

/* Resource debugging and introspection */

OSErr GetResourceDebugInfo(SInt16 resRefNum, OSType resType, SInt16 resID, ResourceDebugInfo* info);
OSErr DumpResourceInfo(SInt16 resRefNum, char** infoString);
OSErr ValidateAllResources(SInt16 resRefNum);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTRESOURCES_H */