/**
 * @file EventStructs.h
 * @brief Central location for Event Manager structure definitions
 *
 * This file defines all the shared structures used by the Event Manager
 * subsystem to avoid duplicate definitions.
 */

#ifndef EVENT_STRUCTS_H
#define EVENT_STRUCTS_H

#include "SystemTypes.h"

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* KeyMap type - 128 bits for all keys */
#ifndef KEYMAP_DEFINED
#define KEYMAP_DEFINED

#endif

/* Click information for double-click detection */
#ifndef CLICKINFO_DEFINED
#define CLICKINFO_DEFINED

#endif

/* Mouse tracking state */
#ifndef MOUSESTATE_DEFINED
#define MOUSESTATE_DEFINED

#endif

/* Keyboard state structure */
#ifndef KEYBOARDSTATE_DEFINED
#define KEYBOARDSTATE_DEFINED

#endif

/* Auto-repeat state */
#ifndef AUTOREPEATSTATE_DEFINED
#define AUTOREPEATSTATE_DEFINED

#endif

/* Dead key state for international input */
#ifndef DEADKEYSTATE_DEFINED
#define DEADKEYSTATE_DEFINED

#endif

/* Key translation state */
#ifndef KEYTRANSSTATE_DEFINED
#define KEYTRANSSTATE_DEFINED

#endif

#ifdef __cplusplus
}
#endif

#endif /* EVENT_STRUCTS_H */