/*
 * ComponentInstances.h
 *
 * Component Instance Management API - System 7.1 Portable Implementation
 * Manages component instance lifecycle, storage, and properties
 */

#ifndef COMPONENTINSTANCES_H
#define COMPONENTINSTANCES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"
#include "ComponentRegistry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Component instance structure */

/* Instance flags */
#define kComponentInstanceFlagValid         (1<<0)  /* Instance is valid */
#define kComponentInstanceFlagClosing       (1<<1)  /* Instance is being closed */
#define kComponentInstanceFlagThreadSafe    (1<<2)  /* Instance is thread-safe */
#define kComponentInstanceFlagExclusive     (1<<3)  /* Exclusive access required */
#define kComponentInstanceFlagDelegate      (1<<4)  /* Instance delegates calls */
#define kComponentInstanceFlagSecure        (1<<5)  /* Secure instance */

/* Instance management initialization */
OSErr InitComponentInstances(void);
void CleanupComponentInstances(void);

/* Component instance creation and destruction */
ComponentInstance CreateComponentInstance(ComponentRegistryEntry* component);
OSErr DestroyComponentInstance(ComponentInstance instance);
OSErr ValidateComponentInstance(ComponentInstance instance);

/* Component instance operations */
ComponentInstance OpenComponentInstance(Component component);
OSErr CloseComponentInstance(ComponentInstance instance);
OSErr ReferenceComponentInstance(ComponentInstance instance);
OSErr DereferenceComponentInstance(ComponentInstance instance);

/* Component instance data access */
ComponentInstanceData* GetComponentInstanceData(ComponentInstance instance);
ComponentRegistryEntry* GetInstanceComponent(ComponentInstance instance);
Handle GetInstanceStorage(ComponentInstance instance);
OSErr SetInstanceStorage(ComponentInstance instance, Handle storage);

/* Component instance properties */
OSErr GetInstanceError(ComponentInstance instance);
void SetInstanceError(ComponentInstance instance, OSErr error);

SInt32 GetInstanceA5(ComponentInstance instance);
void SetInstanceA5(ComponentInstance instance, SInt32 a5);

SInt32 GetInstanceRefcon(ComponentInstance instance);
void SetInstanceRefcon(ComponentInstance instance, SInt32 refcon);

ComponentInstance GetInstanceTarget(ComponentInstance instance);
OSErr SetInstanceTarget(ComponentInstance instance, ComponentInstance target);

/* Component instance enumeration */
SInt32 CountInstances(Component component);
ComponentInstance GetFirstInstance(Component component);
ComponentInstance GetNextInstance(ComponentInstance instance);

/* Instance iteration */

void IterateInstances(Component component, InstanceIteratorFunc iterator, void* userData);
void IterateAllInstances(InstanceIteratorFunc iterator, void* userData);

/* Component instance storage management */
OSErr AllocateInstanceStorage(ComponentInstance instance, UInt32 size);
OSErr ReallocateInstanceStorage(ComponentInstance instance, UInt32 newSize);
OSErr FreeInstanceStorage(ComponentInstance instance);
UInt32 GetInstanceStorageSize(ComponentInstance instance);

/* Instance storage utilities */
OSErr CopyInstanceStorage(ComponentInstance source, ComponentInstance destination);
OSErr SaveInstanceStorage(ComponentInstance instance, void** data, UInt32* size);
OSErr RestoreInstanceStorage(ComponentInstance instance, void* data, UInt32 size);

/* Component instance thread safety */
OSErr LockInstance(ComponentInstance instance);
OSErr UnlockInstance(ComponentInstance instance);
OSErr TryLockInstance(ComponentInstance instance);
Boolean IsInstanceLocked(ComponentInstance instance);

/* Component instance delegation */
OSErr DelegateInstance(ComponentInstance instance, ComponentInstance target);
OSErr UndelegateInstance(ComponentInstance instance);
ComponentInstance GetDelegationTarget(ComponentInstance instance);
ComponentInstance GetDelegationSource(ComponentInstance instance);

/* Component instance lifecycle callbacks */

OSErr RegisterInstanceEventCallback(InstanceEventCallback callback, void* userData);
OSErr UnregisterInstanceEventCallback(InstanceEventCallback callback);

/* Component instance persistence */

OSErr SaveInstanceToFile(ComponentInstance instance, const char* filePath);
OSErr LoadInstanceFromFile(const char* filePath, ComponentInstance* instance);
OSErr SaveInstanceToStream(ComponentInstance instance, void* stream);
OSErr LoadInstanceFromStream(void* stream, ComponentInstance* instance);

/* Component instance debugging */

OSErr GetInstanceDebugInfo(ComponentInstance instance, InstanceDebugInfo* info);
OSErr DumpInstanceInfo(ComponentInstance instance, char** infoString);
OSErr ValidateAllInstances(void);

/* Component instance performance monitoring */

OSErr GetInstancePerformanceStats(ComponentInstance instance, InstancePerformanceStats* stats);
OSErr ResetInstancePerformanceStats(ComponentInstance instance);
OSErr EnableInstanceProfiling(ComponentInstance instance, Boolean enable);

/* Component instance resource management */

OSErr SetInstanceResourceQuota(ComponentInstance instance, InstanceResourceQuota* quota);
OSErr GetInstanceResourceQuota(ComponentInstance instance, InstanceResourceQuota* quota);
OSErr GetInstanceResourceUsage(ComponentInstance instance, InstanceResourceQuota* usage);

/* Component instance security */

OSErr CreateInstanceSecurityContext(ComponentInstance instance, InstanceSecurityContext* context);
OSErr DestroyInstanceSecurityContext(InstanceSecurityContext* context);
OSErr ValidateInstanceSecurity(ComponentInstance instance);

/* Component instance cleanup and leak detection */
OSErr CleanupOrphanedInstances(void);
OSErr DetectInstanceLeaks(void);
OSErr ForceCloseAllInstances(Component component);
OSErr GetInstanceLeakReport(char** report);

/* Component instance caching */

OSErr InitInstanceCache(InstanceCache* cache, UInt32 size, UInt32 maxAge);
OSErr CleanupInstanceCache(InstanceCache* cache);
OSErr CacheInstance(InstanceCache* cache, ComponentInstance instance);
ComponentInstance GetCachedInstance(InstanceCache* cache, Component component);
OSErr InvalidateInstanceCache(InstanceCache* cache);

/* Component instance migration (for component updates) */
OSErr MigrateInstance(ComponentInstance oldInstance, Component newComponent, ComponentInstance* newInstance);
OSErr BeginInstanceMigration(ComponentInstance instance);
OSErr CompleteInstanceMigration(ComponentInstance oldInstance, ComponentInstance newInstance);
OSErr AbortInstanceMigration(ComponentInstance instance);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTINSTANCES_H */