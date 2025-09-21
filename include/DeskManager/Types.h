#ifndef DESKMANAGER_TYPES_H
#define DESKMANAGER_TYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/*
 * Types.h - Common Type Definitions for Desk Manager
 *
 * Provides common type definitions used throughout the Desk Manager
 * implementation, including Mac OS compatibility types and structures.
 */

/* Basic Mac OS Types */
/* Point type defined in MacTypes.h */

/* Rect type defined in MacTypes.h */

/* Forward declarations for Mac OS structures */

/* Event Record Structure */
/* EventRecord is in EventManager/EventTypes.h */

/* Event Types */
#define NULL_EVENT          0
#define MOUSE_DOWN          1
#define MOUSE_UP            2
#define KEY_DOWN            3
#define KEY_UP              4
#define AUTO_KEY            5
#define UPDATE_EVENT        6
#define DISK_EVENT          7
#define ACTIVATE_EVENT      8
#define OS_EVENT            15

/* Modifier Key Masks */
#define ACTIVE_FLAG         0x0001
#define BTN_STATE           0x0080
#define CMD_KEY             0x0100
#define SHIFT_KEY           0x0200
#define ALPHA_LOCK          0x0400
#define OPTION_KEY          0x0800
#define CONTROL_KEY         0x1000

/* String Types */
/* Str255 is defined in MacTypes.h *//* ConstStr255Param is defined in MacTypes.h */
/* Resource Types */
/* OSType is defined in MacTypes.h */

/* Handle is defined in MacTypes.h */

/* Error Codes */
/* OSErr is defined in MacTypes.h *//* noErr is defined in MacTypes.h */
#define memFullErr      -108
#define resNotFound     -192
#define paramErr        -50

/* Boolean type for compatibility */
/* Boolean is defined in MacTypes.h */#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* Utility Macros */
#define HiWord(x)       ((SInt16)(((SInt32)(x) >> 16) & 0xFFFF))
#define LoWord(x)       ((SInt16)((SInt32)(x) & 0xFFFF))
#define MAKE_LONG(hi, lo) (((SInt32)(hi) << 16) | ((SInt32)(lo) & 0xFFFF))

/* Rectangle Utilities */
#define RECT_WIDTH(r)   ((r)->right - (r)->left)
#define RECT_HEIGHT(r)  ((r)->bottom - (r)->top)

#include "SystemTypes.h"
#define POINT_IN_RECT(pt, r) \
    ((pt).h >= (r)->left && (pt).h < (r)->right && \
     (pt).v >= (r)->top && (pt).v < (r)->bottom)

