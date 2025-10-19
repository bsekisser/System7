#include "MemoryMgr/MemoryManager.h"
/*
 * DAPreferences.c - Desk Accessory Preferences and Persistence
 *
 * Provides simple key-value storage for DA preferences. This implementation
 * stores preferences in memory. A production system would persist these to
 * a preferences file or system database.
 *
 * Derived from System 7.1 preferences model
 */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DeskManager/DeskManager.h"

/* Preference Entry Structure */
typedef struct PreferenceEntry {
    char key[64];
    char value[256];
    struct PreferenceEntry *next;
} PreferenceEntry;

/* DA Preferences Structure */
typedef struct DAPreferences {
    char daName[32];
    PreferenceEntry *entries;
    struct DAPreferences *next;
} DAPreferences;

/* Global preferences storage */
static DAPreferences *g_daPreferences = NULL;

/* Internal function prototypes */
static DAPreferences *DA_FindPreferences(const char *daName);
static DAPreferences *DA_CreatePreferences(const char *daName);
static PreferenceEntry *DA_FindPrefEntry(DAPreferences *prefs, const char *key);

/*
 * Set DA preference
 */
int DA_SetPreference(const char *daName, const char *key, const char *value)
{
    if (!daName || !key || !value) {
        return DESK_ERR_INVALID_PARAM;
    }

    /* Find or create DA preferences */
    DAPreferences *prefs = DA_FindPreferences(daName);
    if (!prefs) {
        prefs = DA_CreatePreferences(daName);
        if (!prefs) {
            return DESK_ERR_NO_MEMORY;
        }
        /* Add to list */
        prefs->next = g_daPreferences;
        g_daPreferences = prefs;
    }

    /* Find or create preference entry */
    PreferenceEntry *entry = DA_FindPrefEntry(prefs, key);
    if (!entry) {
        entry = NewPtr(sizeof(PreferenceEntry));
        if (!entry) {
            return DESK_ERR_NO_MEMORY;
        }
        strncpy(entry->key, key, sizeof(entry->key) - 1);
        entry->key[sizeof(entry->key) - 1] = '\0';
        entry->next = prefs->entries;
        prefs->entries = entry;
    }

    /* Set value */
    strncpy(entry->value, value, sizeof(entry->value) - 1);
    entry->value[sizeof(entry->value) - 1] = '\0';

    return DESK_ERR_NONE;
}

/*
 * Get DA preference
 */
int DA_GetPreference(const char *daName, const char *key, char *buffer, int bufferSize)
{
    if (!daName || !key || !buffer || bufferSize <= 0) {
        return DESK_ERR_INVALID_PARAM;
    }

    /* Find DA preferences */
    DAPreferences *prefs = DA_FindPreferences(daName);
    if (!prefs) {
        return DESK_ERR_NOT_FOUND;
    }

    /* Find preference entry */
    PreferenceEntry *entry = DA_FindPrefEntry(prefs, key);
    if (!entry) {
        return DESK_ERR_NOT_FOUND;
    }

    /* Copy value to buffer */
    strncpy(buffer, entry->value, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';

    return DESK_ERR_NONE;
}

/*
 * Save DA preferences to storage
 */
int DA_SavePreferences(const char *daName)
{
    /* TODO: Implement file-based persistence
     * For now, preferences are stored in memory only
     * A production system would write to System Folder:Preferences
     */
    if (daName == NULL) {
        /* Save all preferences */
        return DESK_ERR_NONE;
    }

    /* Save single DA preferences */
    return DESK_ERR_NONE;
}

/*
 * Load DA preferences from storage
 */
int DA_LoadPreferences(const char *daName)
{
    /* TODO: Implement file-based persistence
     * For now, preferences are loaded from memory only
     * A production system would read from System Folder:Preferences
     */
    if (daName == NULL) {
        /* Load all preferences */
        return DESK_ERR_NONE;
    }

    /* Load single DA preferences */
    return DESK_ERR_NONE;
}

/* Internal Functions */

/*
 * Find DA preferences by name
 */
static DAPreferences *DA_FindPreferences(const char *daName)
{
    if (!daName) {
        return NULL;
    }

    DAPreferences *prefs = g_daPreferences;
    while (prefs) {
        if (strcmp(prefs->daName, daName) == 0) {
            return prefs;
        }
        prefs = prefs->next;
    }

    return NULL;
}

/*
 * Create new DA preferences structure
 */
static DAPreferences *DA_CreatePreferences(const char *daName)
{
    if (!daName) {
        return NULL;
    }

    DAPreferences *prefs = NewPtr(sizeof(DAPreferences));
    if (!prefs) {
        return NULL;
    }

    strncpy(prefs->daName, daName, sizeof(prefs->daName) - 1);
    prefs->daName[sizeof(prefs->daName) - 1] = '\0';
    prefs->entries = NULL;
    prefs->next = NULL;

    return prefs;
}

/*
 * Find preference entry by key
 */
static PreferenceEntry *DA_FindPrefEntry(DAPreferences *prefs, const char *key)
{
    if (!prefs || !key) {
        return NULL;
    }

    PreferenceEntry *entry = prefs->entries;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}
