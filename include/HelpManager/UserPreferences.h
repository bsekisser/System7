/*
 * UserPreferences.h - Help Manager User Preferences and Settings
 *
 * This file defines structures and functions for managing user preferences
 * related to help balloons, timing, appearance, and behavior.
 */

#ifndef USERPREFERENCES_H
#define USERPREFERENCES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "HelpManager.h"
#include "HelpBalloons.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Preference categories */

/* Preference data types */

/* Preference storage locations */

/* General help preferences */

/* Appearance preferences */

/* Timing preferences */

/* Behavior preferences */

/* Accessibility preferences */

/* Advanced preferences */

/* Complete preference set */

/* Preference change notification */

/* Preference change callback */

/* Preference validation callback */

/* Preference initialization */
OSErr HMPrefInit(void);
void HMPrefShutdown(void);

/* Preference loading and saving */
OSErr HMPrefLoad(HMPrefStorage storage);
OSErr HMPrefSave(HMPrefStorage storage);
OSErr HMPrefResetToDefaults(void);
OSErr HMPrefResetCategory(HMPrefCategory category);

/* General preference accessors */
OSErr HMPrefGet(const char *prefName, HMPrefType *prefType, void *value, short maxSize);
OSErr HMPrefSet(const char *prefName, HMPrefType prefType, const void *value);
Boolean HMPrefExists(const char *prefName);
OSErr HMPrefRemove(const char *prefName);

/* Typed preference accessors */
Boolean HMPrefGetBoolean(const char *prefName, Boolean defaultValue);
OSErr HMPrefSetBoolean(const char *prefName, Boolean value);

short HMPrefGetInteger(const char *prefName, short defaultValue);
OSErr HMPrefSetInteger(const char *prefName, short value);

float HMPrefGetFloat(const char *prefName, float defaultValue);
OSErr HMPrefSetFloat(const char *prefName, float value);

OSErr HMPrefGetString(const char *prefName, char *value, short maxLength,
                     const char *defaultValue);
OSErr HMPrefSetString(const char *prefName, const char *value);

Point HMPrefGetPoint(const char *prefName, Point defaultValue);
OSErr HMPrefSetPoint(const char *prefName, Point value);

Rect HMPrefGetRect(const char *prefName, Rect defaultValue);
OSErr HMPrefSetRect(const char *prefName, Rect value);

RGBColor HMPrefGetColor(const char *prefName, RGBColor defaultValue);
OSErr HMPrefSetColor(const char *prefName, RGBColor value);

/* Category-specific accessors */
OSErr HMPrefGetGeneral(HMGeneralPrefs *prefs);
OSErr HMPrefSetGeneral(const HMGeneralPrefs *prefs);

OSErr HMPrefGetAppearance(HMAppearancePrefs *prefs);
OSErr HMPrefSetAppearance(const HMAppearancePrefs *prefs);

OSErr HMPrefGetTiming(HMTimingPrefs *prefs);
OSErr HMPrefSetTiming(const HMTimingPrefs *prefs);

OSErr HMPrefGetBehavior(HMBehaviorPrefs *prefs);
OSErr HMPrefSetBehavior(const HMBehaviorPrefs *prefs);

OSErr HMPrefGetAccessibility(HMAccessibilityPrefs *prefs);
OSErr HMPrefSetAccessibility(const HMAccessibilityPrefs *prefs);

OSErr HMPrefGetAdvanced(HMAdvancedPrefs *prefs);
OSErr HMPrefSetAdvanced(const HMAdvancedPrefs *prefs);

/* Preference notification */
OSErr HMPrefRegisterChangeCallback(HMPrefCategory category, HMPrefChangeCallback callback,
                                  void *userData);
OSErr HMPrefUnregisterChangeCallback(HMPrefChangeCallback callback);

OSErr HMPrefRegisterValidateCallback(const char *prefName, HMPrefValidateCallback callback,
                                   void *userData);
OSErr HMPrefUnregisterValidateCallback(const char *prefName);

/* Preference enumeration */
OSErr HMPrefGetCategoryNames(HMPrefCategory category, char ***prefNames, short *count);
OSErr HMPrefGetAllNames(char ***prefNames, short *count);

/* Preference import/export */
OSErr HMPrefExport(const char *filePath, HMPrefCategory category);
OSErr HMPrefImport(const char *filePath, Boolean overwriteExisting);

OSErr HMPrefExportToText(const char *filePath, HMPrefCategory category);
OSErr HMPrefImportFromText(const char *filePath);

/* Preference synchronization */
OSErr HMPrefSyncWithSystem(void);
OSErr HMPrefSyncBetweenUsers(void);
OSErr HMPrefBackup(const char *backupPath);
OSErr HMPrefRestore(const char *backupPath);

/* Preference validation */
Boolean HMPrefValidate(void);
Boolean HMPrefValidateCategory(HMPrefCategory category);
OSErr HMPrefGetValidationErrors(char ***errors, short *errorCount);

/* Default preference management */
OSErr HMPrefGetDefault(const char *prefName, HMPrefType *prefType, void *value, short maxSize);
OSErr HMPrefSetDefault(const char *prefName, HMPrefType prefType, const void *value);
OSErr HMPrefResetToDefault(const char *prefName);

/* Preference migration */
OSErr HMPrefMigrateFromVersion(short fromVersion, short toVersion);
OSErr HMPrefGetVersion(short *version);
OSErr HMPrefSetVersion(short version);

/* System integration */
OSErr HMPrefSyncWithAccessibilitySettings(void);
OSErr HMPrefSyncWithAppearanceSettings(void);
OSErr HMPrefSyncWithLanguageSettings(void);

/* Preference security */
OSErr HMPrefLock(const char *prefName);
OSErr HMPrefUnlock(const char *prefName);
Boolean HMPrefIsLocked(const char *prefName);

/* Debugging and diagnostics */
OSErr HMPrefDump(void);
OSErr HMPrefGetDebugInfo(char *debugInfo, long maxLength);
OSErr HMPrefValidateIntegrity(void);

/* Memory management */
void HMPrefDisposeStringArray(char **strings, short count);

/* Platform-specific preferences */
#ifdef PLATFORM_REMOVED_APPLE
OSErr HMPrefSyncWithNSUserDefaults(void);
#endif

#ifdef PLATFORM_REMOVED_WIN32
OSErr HMPrefSyncWithRegistry(void);
#endif

#ifdef PLATFORM_REMOVED_LINUX
OSErr HMPrefSyncWithGSettings(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* USERPREFERENCES_H */