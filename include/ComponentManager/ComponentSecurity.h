/*
 * ComponentSecurity.h
 *
 * Component Security and Validation API - System 7.1 Portable Implementation
 * Handles component security, validation, and sandboxing
 */

#ifndef COMPONENTSECURITY_H
#define COMPONENTSECURITY_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Security levels */

/* Security permission flags */
#define kSecurityPermissionFileRead     (1<<0)   /* Read file system */
#define kSecurityPermissionFileWrite    (1<<1)   /* Write file system */
#define kSecurityPermissionNetwork      (1<<2)   /* Network access */
#define kSecurityPermissionRegistry     (1<<3)   /* Registry/preferences access */
#define kSecurityPermissionSystem       (1<<4)   /* System API access */
#define kSecurityPermissionMemory       (1<<5)   /* Memory allocation */
#define kSecurityPermissionThreads      (1<<6)   /* Thread creation */
#define kSecurityPermissionIPC          (1<<7)   /* Inter-process communication */
#define kSecurityPermissionHardware     (1<<8)   /* Hardware access */
#define kSecurityPermissionCrypto       (1<<9)   /* Cryptographic operations */

/* Security context */

/* Component signature information */

/* Signature types */
#define kSignatureTypeNone          0   /* No signature */
#define kSignatureTypeRSA           1   /* RSA signature */
#define kSignatureTypeDSA           2   /* DSA signature */
#define kSignatureTypeECDSA         3   /* ECDSA signature */
#define kSignatureTypeCustom        255 /* Custom signature scheme */

/* Hash types */
#define kHashTypeMD5                1   /* MD5 hash */
#define kHashTypeSHA1               2   /* SHA-1 hash */
#define kHashTypeSHA256             3   /* SHA-256 hash */
#define kHashTypeSHA512             4   /* SHA-512 hash */

/* Security initialization */
OSErr InitComponentSecurity(void);
void CleanupComponentSecurity(void);

/* Security context management */
OSErr CreateSecurityContext(ComponentSecurityLevel level, UInt32 permissions, ComponentSecurityContext** context);
OSErr DestroySecurityContext(ComponentSecurityContext* context);
OSErr CloneSecurityContext(ComponentSecurityContext* source, ComponentSecurityContext** destination);

/* Component validation */
OSErr ValidateComponent(Component component);
OSErr ValidateComponentFile(const char* filePath);
OSErr ValidateComponentSignature(Component component);
OSErr ValidateComponentPermissions(Component component, UInt32 requestedPermissions);

/* Component signing and verification */
OSErr SignComponent(const char* componentPath, const char* privateKeyPath, const char* certificatePath);
OSErr VerifyComponentSignature(const char* componentPath, const char* publicKeyPath);
OSErr ExtractComponentSignature(const char* componentPath, ComponentSignature** signature);
OSErr ValidateComponentCertificate(ComponentSignature* signature);

/* Security policy management */

OSErr SetSecurityPolicy(SecurityPolicy* policy);
OSErr GetSecurityPolicy(SecurityPolicy** policy);
OSErr LoadSecurityPolicyFromFile(const char* filePath);
OSErr SaveSecurityPolicyToFile(const char* filePath);

/* Component sandboxing */

OSErr CreateComponentSandbox(ComponentSecurityContext* securityContext, ComponentSandbox** sandbox);
OSErr DestroyComponentSandbox(ComponentSandbox* sandbox);
OSErr ApplySandboxToComponent(Component component, ComponentSandbox* sandbox);
OSErr ExecuteInSandbox(ComponentSandbox* sandbox, ComponentRoutine routine, ComponentParameters* params);

/* Security monitoring */

OSErr RegisterSecurityEventCallback(SecurityEventCallback callback, void* userData);
OSErr UnregisterSecurityEventCallback(SecurityEventCallback callback);
OSErr ReportSecurityEvent(Component component, ComponentSecurityEvent event, const char* details);

/* Trust management */

OSErr InitTrustDatabase(TrustDatabase** database);
OSErr CleanupTrustDatabase(TrustDatabase* database);
OSErr AddTrustedComponent(TrustDatabase* database, const char* componentIdentifier);
OSErr RemoveTrustedComponent(TrustDatabase* database, const char* componentIdentifier);
OSErr IsComponentTrusted(TrustDatabase* database, const char* componentIdentifier);
OSErr BlockComponent(TrustDatabase* database, const char* componentIdentifier);
OSErr UnblockComponent(TrustDatabase* database, const char* componentIdentifier);
OSErr IsComponentBlocked(TrustDatabase* database, const char* componentIdentifier);

/* Cryptographic operations */
OSErr ComputeComponentHash(const char* componentPath, UInt32 hashType, void** hash, UInt32* hashSize);
OSErr VerifyComponentIntegrity(const char* componentPath, void* expectedHash, UInt32 hashSize, UInt32 hashType);
OSErr EncryptComponentData(void* data, UInt32 dataSize, const char* keyPath, void** encryptedData, UInt32* encryptedSize);
OSErr DecryptComponentData(void* encryptedData, UInt32 encryptedSize, const char* keyPath, void** data, UInt32* dataSize);

/* Security auditing */

OSErr EnableSecurityAuditing(Boolean enable);
OSErr GetSecurityAuditLog(SecurityAuditLog** logs, SInt32* logCount);
OSErr ClearSecurityAuditLog(void);
OSErr SaveSecurityAuditLog(const char* filePath);

/* Quarantine management */

OSErr QuarantineComponent(Component component, const char* reason);
OSErr RestoreFromQuarantine(const char* componentPath);
OSErr GetQuarantineInfo(const char* componentPath, QuarantineInfo** info);
OSErr ListQuarantinedComponents(QuarantineInfo*** components, SInt32* count);

/* Platform-specific security implementations */
#ifdef PLATFORM_REMOVED_WIN32
#include <windows.h>
#include <wincrypt.h>

#elif defined(__APPLE__)
#include <Security/Security.h>

#elif defined(__linux__)

#endif

/* Security configuration */

OSErr LoadSecurityConfiguration(const char* configPath, SecurityConfiguration** config);
OSErr SaveSecurityConfiguration(const char* configPath, SecurityConfiguration* config);
OSErr ApplySecurityConfiguration(SecurityConfiguration* config);

/* Component privilege escalation prevention */
OSErr PreventPrivilegeEscalation(Component component);
OSErr CheckForPrivilegeEscalation(Component component, Boolean* detected);
OSErr MonitorComponentPrivileges(Component component, Boolean enable);

/* Secure component loading */
OSErr SecureLoadComponent(const char* componentPath, ComponentSecurityContext* securityContext, Component* component);
OSErr SecureUnloadComponent(Component component);
OSErr VerifyComponentBeforeExecution(Component component);

/* Security testing and fuzzing */
OSErr RunSecurityTests(Component component, Boolean* passed);
OSErr FuzzComponentAPI(Component component, UInt32 iterations);
OSErr ValidateComponentMemoryUsage(Component component);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTSECURITY_H */