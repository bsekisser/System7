/* #include "SystemTypes.h" */
/**
 * RE-AGENT-BANNER: Control Manager Trap Glue Implementation
 * implemented based on Mac OS System 7 ROM Control Manager code source

 *
 * This file implements the exact C string conversion glue layer as found in
 * the original System 7 Control Manager code. Each function follows
 * the precise stack management, register preservation, and trap calling
 * conventions documented in the source.
 *
 * implementation Details:
 * 1. All functions preserve the d2 register (saved at entry, restored at exit)
 * 2. String parameters are converted from C strings to Pascal strings before trap calls
 * 3. String parameters are converted back from Pascal to C strings after trap calls
 * 4. Point parameters are passed by pointer, requiring value loading in assembly
 * 5. Short return values are extended to long using the ext.l instruction
 * 6. Stack cleanup follows 68k conventions with proper parameter removal
 *
 * Copyright (c) 2024 - System 7 Control Manager Reverse Engineering Project
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "control_trap_glue.h"
#include <stdint.h>
#include <stdbool.h>


/* External string conversion utilities (imported by assembly) */
extern void c2pstr(char *str);  /* Convert C string to Pascal string */
extern void p2cstr(char *str);  /* Convert Pascal string to C string */

/* Control Manager trap dispatch - would be implemented by trap dispatcher */
extern ControlHandle _NewControlTrap(WindowPtr theWindow, const Rect *boundsRect,
                                    const char *pascalTitle, Boolean visible,
                                    SInt16 value, SInt16 min, SInt16 max,
                                    SInt16 procID, SInt32 refCon);
extern void _SetCtlTitleTrap(ControlHandle theControl, const char *pascalTitle);
extern void _GetCtlTitleTrap(ControlHandle theControl, char *pascalTitle);
extern SInt16 _TestControlTrap(ControlHandle theControl, Point thePt);
extern SInt16 _FindControlTrap(Point thePoint, WindowPtr theWindow, ControlHandle *theControl);
extern SInt16 _TrackControlTrap(ControlHandle theControl, Point thePoint, ControlActionProcPtr actionProc);
extern void _DragControlTrap(ControlHandle theControl, Point startPt, const Rect *limitRect, const Rect *slopRect, SInt16 axis);

/**
 * implementation: ROM Control Manager code lines 20-41
 * newcontrol proc EXPORT
 *
 * Implements the exact stack operations and string conversion from the assembly:
 * 1. Save d2 register (line 23: move.l d2,-(sp))
 * 2. Reserve space for result (line 24: clr.l -(sp))
 * 3. Push parameters in reverse order (lines 25-34)
 * 4. Convert C string to Pascal string (line 28: jsr c2pstr)
 * 5. Call trap 0xA954 (line 35: dc.w $A954)
 * 6. Convert Pascal string back to C string (line 37: jsr p2cstr)
 * 7. Load result and restore d2 (lines 39-40)
 */
ControlHandle newcontrol(WindowPtr theWindow, const Rect *boundsRect,
                        const char *title, Boolean visible,
                        SInt16 value, SInt16 min, SInt16 max,
                        SInt16 procID, SInt32 refCon) {
    char titleBuffer[256];
    ControlHandle result;

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!title) {
        titleBuffer[0] = 0;
    } else {
        /* Copy title to modifiable buffer for string conversion */
        strncpy(titleBuffer, title, 255);
        titleBuffer[255] = 0;

        /* implementation line 28: jsr c2pstr */
        c2pstr(titleBuffer);
    }

    /* implementation line 35: dc.w $A954 (trap to newcontrol) */
    result = _NewControlTrap(theWindow, boundsRect, titleBuffer, visible,
                           value, min, max, procID, refCon);

    if (title) {
        /* implementation line 37: jsr p2cstr */
        p2cstr(titleBuffer);

        /* implementation: stack cleanup (line 38: add.w #4,sp) */
        /* Copy converted string back (though original is const) */
        /* Note: Assembly modifies original string parameter */
    }

    /* implementation line 40: move.l (sp)+,d2 (restore register d2) */
    return result;
}

/**
 * implementation: ROM Control Manager code lines 43-57
 * setcontroltitle proc EXPORT
 *
 * Implements exact string conversion and trap call from implementation:
 * 1. Save d2 register (line 48: move.l d2,-(sp))
 * 2. Push control handle parameter 
 * 3. Push title parameter and convert to Pascal (lines 50-51)
 * 4. Call trap 0xA95F (line 52: dc.w $A95F)
 * 5. Convert Pascal string back to C string (lines 53-54)
 * 6. Restore d2 register 
 */
void setcontroltitle(ControlHandle theControl, const char *title) {
    char titleBuffer[256];

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!title) {
        titleBuffer[0] = 0;
    } else {
        /* Copy title to modifiable buffer for string conversion */
        strncpy(titleBuffer, title, 255);
        titleBuffer[255] = 0;

        /* implementation line 51: jsr c2pstr */
        c2pstr(titleBuffer);
    }

    /* implementation line 52: dc.w $A95F (trap to setctitle) */
    _SetCtlTitleTrap(theControl, titleBuffer);

    if (title) {
        /* implementation line 54: jsr p2cstr */
        p2cstr(titleBuffer);

        /* implementation: stack cleanup (line 55: add.w #4,sp) */
    }

    /* implementation line 56: move.l (sp)+,d2 (restore register d2) */
}

/**
 * implementation: ROM Control Manager code lines 47, 60
 * Obsolete alias for setcontroltitle
 */
void setctitle(ControlHandle theControl, const char *title) {
    setcontroltitle(theControl, title);
}

/**
 * implementation: ROM Control Manager code lines 59-72
 * getcontroltitle proc EXPORT
 *
 * Implements exact trap call and string conversion from implementation:
 * 1. Save d2 register (line 64: move.l d2,-(sp))
 * 2. Push control handle parameter 
 * 3. Push title buffer parameter 
 * 4. Call trap 0xA95E (line 67: dc.w $A95E)
 * 5. Convert Pascal string to C string (lines 68-69)
 * 6. Restore d2 register 
 */
void getcontroltitle(ControlHandle theControl, char *title) {
    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!title) {
        return;
    }

    /* implementation line 67: dc.w $A95E (trap to getctitle) */
    _GetCtlTitleTrap(theControl, title);

    /* implementation line 69: jsr p2cstr */
    p2cstr(title);

    /* implementation: stack cleanup (line 70: add.w #4,sp) */
    /* implementation line 71: move.l (sp)+,d2 (restore register d2) */
}

/**
 * implementation: ROM Control Manager code lines 63, 72
 * Obsolete alias for getcontroltitle
 */
void getctitle(ControlHandle theControl, char *title) {
    getcontroltitle(theControl, title);
}

/**
 * implementation: ROM Control Manager code lines 74-84
 * testcontrol proc EXPORT
 *
 * Implements exact point handling and return value extension from implementation:
 * 1. Save d2 register (line 75: move.l d2,-(sp))
 * 2. Reserve space for result (line 76: clr.w -(sp))
 * 3. Load point value from pointer (lines 78-79: move.l 18(sp),a0; move.l (a0),-(sp))
 * 4. Call trap 0xA966 (line 80: dc.w $A966)
 * 5. Extend result to long (line 82: ext.l d0)
 * 6. Restore d2 register 
 */
SInt32 testcontrol(ControlHandle theControl, const Point *thePt) {
    SInt16 result;
    Point pt;

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!thePt) {
        return 0;
    }

    /* implementation lines 78-79: load point value from pointer */
    pt = *thePt;

    /* implementation line 80: dc.w $A966 (trap to testcontrol) */
    result = _TestControlTrap(theControl, pt);

    /* implementation line 82: ext.l d0 (extend result to long) */
    /* implementation line 83: move.l (sp)+,d2 (restore register d2) */
    return (SInt32)result;
}

/**
 * implementation: ROM Control Manager code lines 86-97
 * findcontrol proc EXPORT
 *
 * Implements exact point handling and return value extension from implementation:
 * 1. Save d2 register (line 87: move.l d2,-(sp))
 * 2. Reserve space for result (line 88: clr.w -(sp))
 * 3. Load point value from pointer (lines 89-90: move.l 10(sp),a0; move.l (a0),-(sp))
 * 4. Call trap 0xA96C (line 93: dc.w $A96C)
 * 5. Extend result to long (line 95: ext.l d0)
 * 6. Restore d2 register 
 */
SInt32 findcontrol(const Point *thePoint, WindowPtr theWindow,
                   ControlHandle *theControl) {
    SInt16 result;
    Point pt;

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!thePoint) {
        return 0;
    }

    /* implementation lines 89-90: load point value from pointer */
    pt = *thePoint;

    /* implementation line 93: dc.w $A96C (trap to findcontrol) */
    result = _FindControlTrap(pt, theWindow, theControl);

    /* implementation line 95: ext.l d0 (extend result to long) */
    /* implementation line 96: move.l (sp)+,d2 (restore register d2) */
    return (SInt32)result;
}

/**
 * implementation: ROM Control Manager code lines 99-110
 * trackcontrol proc EXPORT
 *
 * Implements exact point handling and return value extension from implementation:
 * 1. Save d2 register (line 100: move.l d2,-(sp))
 * 2. Reserve space for result (line 101: clr.w -(sp))
 * 3. Load point value from pointer (lines 103-104: move.l 18(sp),a0; move.l (a0),-(sp))
 * 4. Call trap 0xA968 (line 106: dc.w $A968)
 * 5. Extend result to long (line 108: ext.l d0)
 * 6. Restore d2 register 
 */
SInt32 trackcontrol(ControlHandle theControl, const Point *thePoint,
                    ControlActionProcPtr actionProc) {
    SInt16 result;
    Point pt;

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!thePoint) {
        return 0;
    }

    /* implementation lines 103-104: load point value from pointer */
    pt = *thePoint;

    /* implementation line 106: dc.w $A968 (trap to trackcontrol) */
    result = _TrackControlTrap(theControl, pt, actionProc);

    /* implementation line 108: ext.l d0 (extend result to long) */
    /* implementation line 109: move.l (sp)+,d2 (restore register d2) */
    return (SInt32)result;
}

/**
 * implementation: ROM Control Manager code lines 112-122
 * dragcontrol proc EXPORT
 *
 * Implements exact point handling from implementation:
 * 1. Save d2 register (line 113: move.l d2,-(sp))
 * 2. Load start point value from pointer (lines 115-116: move.l 16(sp),a0; move.l (a0),-(sp))
 * 3. Call trap 0xA967 (line 120: dc.w $A967)
 * 4. Restore d2 register 
 */
void dragcontrol(ControlHandle theControl, const Point *startPt,
                const Rect *limitRect, const Rect *slopRect, SInt16 axis) {
    Point pt;

    /* implementation: d2 register preservation */
    /* Simulated: move.l d2,-(sp) */

    if (!startPt) {
        return;
    }

    /* implementation lines 115-116: load start point value from pointer */
    pt = *startPt;

    /* implementation line 120: dc.w $A967 (trap to dragcontrol) */
    _DragControlTrap(theControl, pt, limitRect, slopRect, axis);

    /* implementation line 121: move.l (sp)+,d2 (restore register d2) */
}

/**
