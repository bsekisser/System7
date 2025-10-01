/*
 * File: VoiceManager.h
 *
 * Contains: Voice selection and management for Speech Manager
 *
 * Written by: Claude Code (Portable Implementation)
 *
 *
 * Description: This header provides voice management functionality
 *              including voice enumeration, selection, and properties.
 */

#ifndef _VOICEMANAGER_H_
#define _VOICEMANAGER_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SpeechManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Voice Management Constants ===== */

/* Voice search locations */

/* Voice flags */

/* Voice quality levels */

/* ===== Extended Voice Structures ===== */

/* Extended voice information */

/* Voice enumeration callback */

/* ===== Voice Management Functions ===== */

/* Initialization and cleanup */
OSErr InitializeVoiceManager(void);
void CleanupVoiceManager(void);

/* Voice enumeration and search */
OSErr RefreshVoiceList(VoiceSearchFlags searchFlags);
OSErr EnumerateVoices(VoiceEnumerationProc callback, void *userData);
OSErr FindVoiceByName(const char *voiceName, VoiceSpec *voice);
OSErr FindVoiceByLanguage(short language, short region, VoiceSpec *voice);
OSErr FindVoiceByGender(VoiceGender gender, VoiceSpec *voice);
OSErr FindBestVoice(const VoiceDescription *criteria, VoiceSpec *voice);

/* Voice information */
OSErr GetVoiceInfoExtended(VoiceSpec *voice, VoiceInfoExtended *info);
OSErr GetVoiceCapabilities(VoiceSpec *voice, long *capabilities);
OSErr GetVoiceLanguages(VoiceSpec *voice, short *languages, short *count);

/* Voice validation */
OSErr ValidateVoice(VoiceSpec *voice);
Boolean IsVoiceAvailable(VoiceSpec *voice);
OSErr GetVoiceAvailability(VoiceSpec *voice, Boolean *available, char **reason);

/* Default voice management */
OSErr SetDefaultVoice(VoiceSpec *voice);
OSErr GetDefaultVoice(VoiceSpec *voice);
OSErr ResetToSystemDefaultVoice(void);

/* Voice installation and removal */
OSErr InstallVoice(const char *voicePath, VoiceSpec *installedVoice);
OSErr RemoveVoice(VoiceSpec *voice);
OSErr GetVoiceInstallPath(VoiceSpec *voice, char *path, long pathSize);

/* Voice preferences */
OSErr SaveVoicePreferences(VoiceSpec *voice, const char *prefName);
OSErr LoadVoicePreferences(const char *prefName, VoiceSpec *voice);
OSErr DeleteVoicePreferences(const char *prefName);

/* Voice comparison and sorting */
int CompareVoices(const VoiceSpec *voice1, const VoiceSpec *voice2);
OSErr SortVoiceList(VoiceSpec *voices, short count, int (*compareFunc)(const VoiceSpec *, const VoiceSpec *));

/* Voice resource management */
OSErr LoadVoiceResources(VoiceSpec *voice);
OSErr UnloadVoiceResources(VoiceSpec *voice);
OSErr GetVoiceResourceInfo(VoiceSpec *voice, OSType resourceType, void **resourceData, long *resourceSize);

/* Voice testing */
OSErr TestVoice(VoiceSpec *voice, const char *testText);
OSErr GetVoiceTestResults(VoiceSpec *voice, Boolean *success, char **errorMessage);

/* Voice filtering */

OSErr FilterVoices(VoiceFilterProc filter, void *filterData, VoiceSpec **filteredVoices, short *count);

/* Voice groups and categories */
OSErr GetVoiceCategories(char ***categories, short *categoryCount);
OSErr GetVoicesInCategory(const char *category, VoiceSpec **voices, short *count);
OSErr SetVoiceCategory(VoiceSpec *voice, const char *category);

/* Voice compatibility */
OSErr CheckVoiceCompatibility(VoiceSpec *voice, long systemVersion, Boolean *compatible);
OSErr GetMinimumSystemVersion(VoiceSpec *voice, long *minVersion);

/* Voice licensing */
OSErr GetVoiceLicense(VoiceSpec *voice, char **licenseText);
OSErr AcceptVoiceLicense(VoiceSpec *voice, Boolean accept);
Boolean IsVoiceLicenseAccepted(VoiceSpec *voice);

/* ===== Voice Manager Notifications ===== */

/* Voice change notification types */

/* Voice notification callback */

/* Notification registration */
OSErr RegisterVoiceNotification(VoiceNotificationProc callback, void *userData);
OSErr UnregisterVoiceNotification(VoiceNotificationProc callback);

/* ===== Utility Functions ===== */

/* Voice spec utilities */
Boolean VoiceSpecsEqual(const VoiceSpec *voice1, const VoiceSpec *voice2);
OSErr CopyVoiceSpec(const VoiceSpec *source, VoiceSpec *dest);
OSErr VoiceSpecToString(const VoiceSpec *voice, char *string, long stringSize);
OSErr StringToVoiceSpec(const char *string, VoiceSpec *voice);

/* Voice description utilities */
OSErr CopyVoiceDescription(const VoiceDescription *source, VoiceDescription *dest);
void FreeVoiceDescription(VoiceDescription *desc);

/* Voice list utilities */
OSErr AllocateVoiceList(short count, VoiceSpec **voices);
void FreeVoiceList(VoiceSpec *voices);
OSErr DuplicateVoiceList(const VoiceSpec *source, short count, VoiceSpec **dest);

#ifdef __cplusplus
}
#endif

#endif /* _VOICEMANAGER_H_ */