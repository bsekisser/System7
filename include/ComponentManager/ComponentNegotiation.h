/*
 * ComponentNegotiation.h
 *
 * Component Capability Negotiation API - System 7.1 Portable Implementation
 * Handles capability negotiation and versioning between components
 */

#ifndef COMPONENTNEGOTIATION_H
#define COMPONENTNEGOTIATION_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Capability descriptor */

/* Capability types */
#define kCapabilityTypeCodec        'cdec'  /* Codec capability */
#define kCapabilityTypeEffect       'efct'  /* Effect capability */
#define kCapabilityTypeImporter     'impt'  /* Importer capability */
#define kCapabilityTypeExporter     'expt'  /* Exporter capability */
#define kCapabilityTypeRenderer     'rndr'  /* Renderer capability */
#define kCapabilityTypeCompressor   'comp'  /* Compressor capability */
#define kCapabilityTypeDecompressor 'dcmp'  /* Decompressor capability */
#define kCapabilityTypeGeneric      'genc'  /* Generic capability */

/* Capability flags */
#define kCapabilityFlagRequired     (1<<0)  /* Required capability */
#define kCapabilityFlagOptional     (1<<1)  /* Optional capability */
#define kCapabilityFlagPreferred    (1<<2)  /* Preferred capability */
#define kCapabilityFlagDeprecated   (1<<3)  /* Deprecated capability */
#define kCapabilityFlagExperimental (1<<4)  /* Experimental capability */
#define kCapabilityFlagPlatformSpecific (1<<5) /* Platform-specific capability */

/* Negotiation context */

/* Negotiation flags */
#define kNegotiationFlagStrict      (1<<0)  /* Strict negotiation - all required capabilities must match */
#define kNegotiationFlagBestEffort  (1<<1)  /* Best effort - use best available capabilities */
#define kNegotiationFlagFallback    (1<<2)  /* Allow fallback to lower versions */
#define kNegotiationFlagCache       (1<<3)  /* Cache negotiation results */

/* Version compatibility */

/* Compatibility flags */
#define kVersionCompatibilityBackward   (1<<0)  /* Backward compatible */
#define kVersionCompatibilityForward    (1<<1)  /* Forward compatible */
#define kVersionCompatibilityExact      (1<<2)  /* Exact version match required */
#define kVersionCompatibilityRange      (1<<3)  /* Version range supported */

/* Negotiation initialization */
OSErr InitComponentNegotiation(void);
void CleanupComponentNegotiation(void);

/* Capability management */
OSErr RegisterComponentCapability(Component component, ComponentCapability* capability);
OSErr UnregisterComponentCapability(Component component, OSType capabilityType, OSType capabilitySubType);
OSErr GetComponentCapabilities(Component component, ComponentCapability** capabilities, SInt32* count);

/* Capability queries */
Boolean ComponentHasCapability(Component component, OSType capabilityType, OSType capabilitySubType);
OSErr GetComponentCapability(Component component, OSType capabilityType, OSType capabilitySubType, ComponentCapability** capability);
SInt32 GetCapabilityVersion(Component component, OSType capabilityType, OSType capabilitySubType);

/* Capability negotiation */
OSErr NegotiateCapabilities(CapabilityNegotiationContext* context);
OSErr RequestCapability(Component requestingComponent, Component providingComponent,
                       OSType capabilityType, OSType capabilitySubType, SInt32 minVersion,
                       ComponentCapability** negotiatedCapability);

/* Version compatibility checking */
OSErr CheckVersionCompatibility(SInt32 requestedVersion, SInt32 availableVersion,
                               VersionCompatibility* compatibility, Boolean* isCompatible);
OSErr GetVersionCompatibilityInfo(Component component, VersionCompatibility* compatibility);
Boolean IsVersionCompatible(SInt32 version1, SInt32 version2, UInt32 compatibilityFlags);

/* Capability matching */

OSErr FindMatchingCapabilities(ComponentCapability* availableCapabilities,
                              CapabilityMatchCriteria* criteria,
                              ComponentCapability*** matches, SInt32* matchCount);
SInt32 RankCapabilityMatch(ComponentCapability* capability, CapabilityMatchCriteria* criteria);

/* Capability enumeration */

OSErr EnumerateComponentCapabilities(Component component, CapabilityEnumeratorFunc enumerator, void* userData);
OSErr EnumerateAllCapabilities(OSType capabilityType, CapabilityEnumeratorFunc enumerator, void* userData);

/* Capability dependencies */

OSErr AddCapabilityDependency(Component component, CapabilityDependency* dependency);
OSErr RemoveCapabilityDependency(Component component, OSType dependentType, OSType dependentSubType);
OSErr ResolveCapabilityDependencies(Component component);
OSErr CheckCapabilityDependencies(Component component, Boolean* allSatisfied);

/* Codec-specific negotiation */

OSErr NegotiateCodecCapability(Component codec, CodecCapability* requested, CodecCapability** negotiated);
Boolean IsCodecCompatible(CodecCapability* capability1, CodecCapability* capability2);

/* Effect-specific negotiation */

OSErr NegotiateEffectCapability(Component effect, EffectCapability* requested, EffectCapability** negotiated);

/* Capability caching */

OSErr InitCapabilityCache(UInt32 maxEntries, UInt32 timeoutSeconds);
OSErr CleanupCapabilityCache(void);
OSErr CacheComponentCapabilities(Component component, ComponentCapability* capabilities, SInt32 count);
OSErr GetCachedCapabilities(Component component, ComponentCapability** capabilities, SInt32* count);
OSErr InvalidateCapabilityCache(Component component);

/* Dynamic capability negotiation */

OSErr RegisterNegotiationCallback(OSType capabilityType, CapabilityNegotiationCallback callback, void* userData);
OSErr UnregisterNegotiationCallback(OSType capabilityType, CapabilityNegotiationCallback callback);

/* Capability profiles */

OSErr CreateCapabilityProfile(const char* profileName, CapabilityProfile** profile);
OSErr DestroyCapabilityProfile(CapabilityProfile* profile);
OSErr AddCapabilityToProfile(CapabilityProfile* profile, ComponentCapability* capability);
OSErr NegotiateWithProfile(Component component, CapabilityProfile* profile, ComponentCapability** result);

/* Capability serialization */
OSErr SerializeCapabilities(ComponentCapability* capabilities, SInt32 count, void** data, UInt32* size);
OSErr DeserializeCapabilities(void* data, UInt32 size, ComponentCapability** capabilities, SInt32* count);
OSErr SaveCapabilitiesToFile(ComponentCapability* capabilities, SInt32 count, const char* filePath);
OSErr LoadCapabilitiesFromFile(const char* filePath, ComponentCapability** capabilities, SInt32* count);

/* Capability debugging and introspection */
OSErr DumpComponentCapabilities(Component component, char** capabilitiesString);
OSErr ValidateCapabilityConsistency(Component component);
OSErr GetCapabilityNegotiationStats(Component component, UInt32* successCount, UInt32* failureCount);

/* Platform-specific capability handling */
OSErr GetPlatformSpecificCapabilities(Component component, SInt16 platformType,
                                     ComponentCapability** capabilities, SInt32* count);
OSErr FilterCapabilitiesByPlatform(ComponentCapability* capabilities, SInt32 count,
                                  SInt16 platformType, ComponentCapability** filtered, SInt32* filteredCount);

/* Capability upgrade and migration */
OSErr UpgradeCapability(ComponentCapability* oldCapability, SInt32 newVersion, ComponentCapability** newCapability);
OSErr MigrateCapabilities(Component oldComponent, Component newComponent);
OSErr GetCapabilityUpgradePath(ComponentCapability* capability, SInt32 targetVersion,
                              ComponentCapability*** upgradePath, SInt32* pathLength);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTNEGOTIATION_H */