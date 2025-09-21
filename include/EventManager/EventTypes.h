/**
 * @file EventTypes.h
 * @brief Event type definitions and constants for System 7.1 Event Manager
 *
 * This file contains all the event type definitions, constants, and
 * data structures used by the Event Manager. It provides exact
 * compatibility with Mac OS System 7.1 event system.
 *
 * Copyright (c) 2024 System 7.1 Portable Project
 * All rights reserved.
 */

#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Basic geometric types */
/* Point type defined in MacTypes.h */

/* Rect type defined in MacTypes.h */

/* Event types - exact Mac OS System 7.1 values */

/* Event masks for filtering events */

/* Event message field bit masks */

/* OS event message codes (in high byte of message) */

/* Suspend/resume message flags (in low byte) */

/* Modifier key flags in event modifiers field */

/* Mouse button states */

/* Key codes for special keys */

/* EventRecord - the fundamental event structure */
/* EventRecord is in EventManager/EventTypes.h */

/* Event queue element structure */
typedef struct EvQEl {
    QElemPtr    qLink;
    SInt16      qType;
    EventRecord evtQWhat;
    void*       evtQMessage;
    SInt32      evtQWhen;
    Point       evtQWhere;
    SInt16      evtQModifiers;
} EvQEl;

/* KeyMap type */

/* Ptr is defined in MacTypes.h */

/* Queue header structure */
// 

/* Ptr is defined in MacTypes.h */

/* KeyMap type for keyboard state - 128 bits total */

/* Window pointer type */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* Region handle type */
/* Handle is defined in MacTypes.h */

/* Error codes specific to Event Manager */

/* Event queue constants */

/* Timing constants */

/* Mouse tracking constants */

/* Extended event information */

/* Click state tracking */

/* Keyboard auto-repeat state */

/* Platform-specific input state */

/* Event filter callback function type */

/* Modern input event types (for internal use) */

#ifdef __cplusplus
}
#endif

#endif /* EVENT_TYPES_H */
