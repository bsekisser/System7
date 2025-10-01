/*
 * Date & Time Control Panel Header
 *
 * RE-AGENT-BANNER
 * SOURCE: Date & Time control panel (ROM-based) (ROM-based)

 * Mappings: 
 * Layouts: 
 * Architecture: m68k Classic Mac OS
 * Type: Control Panel Device (cdev)
 * Creator: time
 */

#ifndef DATETIME_CDEV_H
#define DATETIME_CDEV_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/* DateTime Control Panel - Standalone Implementation */

/* Forward declarations and local type definitions */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* MenuHandle is in MenuTypes.h */

/* Complete EventRecord definition */
/* EventRecord is in EventManager/EventTypes.h */

/* Event types */

/* Item types */

/* Key constants */

/* Control Panel Message Types */

/* Date Format Types */

/* Time Format Types */

/* Dialog Item IDs */

/* Control Panel Device Record */
              /* Control panel message type */
    Boolean setup;              /* Setup flag */
    char padding1;              /* Alignment padding */
    Rect *rect;                 /* Control panel rectangle */
    short procID;               /* Procedure ID */
    short refCon;               /* Reference constant */
    Handle storage;             /* Private storage handle */
} CDevRecord, *CDevPtr;

/* Date & Time Preferences */
           /* 'time' signature */
    short version;              /* Preferences version */
    short dateFormat;           /* Date display format */
    short timeFormat;           /* Time display format */
    Boolean suppressLeadingZero; /* Suppress leading zero flag */
    Boolean showSeconds;        /* Show seconds flag */
    unsigned long currentDate;  /* Current date (Mac format) */
    unsigned long currentTime;  /* Current time cache */
    short menuChoice;           /* Selected menu choice */
    short editField;            /* Currently active edit field */
    Boolean modified;           /* Settings modified flag */
    char padding[7];            /* Reserved space */
} DateTimePrefs, *DateTimePrefsPtr;

/* Date & Time Edit Record */
                 /* Year being edited */
    short month;                /* Month being edited */
    short day;                  /* Day being edited */
    short hour;                 /* Hour being edited */
    short minute;               /* Minute being edited */
    short second;               /* Second being edited */
    Boolean isPM;               /* PM indicator */
    Boolean valid;              /* Values valid flag */
    short activeField;          /* Active edit field */
    unsigned long originalDateTime; /* Original date/time */
    unsigned long tempDateTime;     /* Working date/time */
} DateTimeEditRec, *DateTimeEditPtr;

/* Function Prototypes - Remove pascal calling convention for C compatibility */
OSErr DateTimeMain(short message, Boolean setup, Rect *rect,
                  short procID, short refCon, Handle storage);
void InitDateTimeDialog(DialogPtr dialog);
Boolean HandleDateTimeItem(DialogPtr dialog, short item);
void DrawDateTimeControls(DialogPtr dialog);
Boolean FilterDateTimeEvent(DialogPtr dialog, EventRecord *event, short *item);

/* Utility Functions */
static void SetupDateTimePrefs(DateTimePrefsPtr prefs);
static void UpdateDateTimeDisplay(DialogPtr dialog, DateTimePrefsPtr prefs);
static void HandleDateFormatButton(DialogPtr dialog, DateTimePrefsPtr prefs);
static void HandleTimeFormatButton(DialogPtr dialog, DateTimePrefsPtr prefs);
static void ConvertToDateTimeRec(unsigned long dateTime, DateTimeEditPtr editRec);
static unsigned long ConvertFromDateTimeRec(DateTimeEditPtr editRec);
static void ValidateDateTimeValues(DateTimeEditPtr editRec);

#endif /* DATETIME_CDEV_H */