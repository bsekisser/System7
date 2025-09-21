/************************************************************
 *
 * TextTypes.h
 * System 7.1 Portable - TextEdit Types and Data Structures
 *
 * Internal data structures and type definitions for TextEdit
 * implementation. Contains private structures and constants
 * used internally by the TextEdit Manager.
 *
 * Copyright (c) 2024 System7.1-Portable Project
 * Based on Apple Computer, Inc. TextEdit Manager 1985-1992
 *
 ************************************************************/

#ifndef __TEXTTYPES_H__
#define __TEXTTYPES_H__

#include "SystemTypes.h"

#ifndef __TYPES_H__
#include "SystemTypes.h"
#endif

#ifndef __QUICKDRAW_H__
#include "QuickDraw/QuickDraw.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
 * Internal TextEdit Constants
 ************************************************************/

/* Private offsets into TEDispatchRec */
#define newTEFlags          16    /* teFAutoScr, teFTextBuffering, teFOutlineHilite */
#define TwoByteCharBuffer   20    /* storage for buffered double-byte character */
#define lastScript          22    /* keep last script for split/single caret display */

/* Text measure overflow flag */
#define measOverFlow        0     /* text measure overflow (00000001b) */

/* Size of TEDispatchRec */
#define intDispSize         32

/* Style constants */
#define teStylSize          28    /* initial size of teStylesRec with 2 entries */
#define stBaseSize          20    /* added to fix MakeRoom bug */

/* Extended style modes */

/* Text buffer size */
#define BufferSize          50    /* max num of bytes for text buffer */

/* Defined key characters */
#define downArrowChar       0x1F
#define upArrowChar         0x1E
#define rightArrowChar      0x1D
#define leftArrowChar       0x1C
#define backspaceChar       0x08
#define returnChar          0x0D
#define tabChar             0x09

/* Internal constants */
#define lnStStartSize       2     /* stStartSize = 4; bit shift left of 2 */
#define UnboundLeft         0x8002 /* unbound left edge for rectangle */
#define UnboundRight        0x7FFE /* unbound right edge for rectangle */
#define numStyleRuns        10    /* max # for local frame; if more, allocate handle */
#define defBufSize          256   /* initial size of temp line start buffer */

/************************************************************
 * Internal Data Structures
 ************************************************************/

/* TEDispatchRec structure - internal dispatch table */
            /* End of line hook */
    ProcPtr DRAWHook;           /* Drawing hook */
    ProcPtr WIDTHHook;          /* Width calculation hook */
    ProcPtr HITTESTHook;        /* Hit testing hook */
    short newTEFlags;           /* Feature flags */
    short TwoByteCharBuffer;    /* Double-byte character buffer */
    char lastScript;            /* Last script for caret display */
    char padding1;              /* Padding */
    ProcPtr nWIDTHHook;         /* New width hook */
    ProcPtr TextWidthHook;      /* Text width hook */
} TEDispatchRec, *TEDispatchPtr, **TEDispatchHandle;

/* Format ordering frame for styled text */

/* Standard frame for text processing */

/* Shared frame for text operations */

/* Frame for recalculate lines */

/* Frame for GetCurScript */

/* Frame for FindWord */

/* Frame for outline highlighting */

/* Frame for cursor operations */

/* Frame for line breaking */

/* Frame for pixel to character conversion */

/* Frame for highlighting */

/************************************************************
 * Internal State Management
 ************************************************************/

/* TextEdit global state */

/* Platform-specific text input state */

/* Undo information */

/************************************************************
 * Internal Function Prototypes
 ************************************************************/

/* Internal utility functions */
TEDispatchHandle TECreateDispatchRec(void);
void TEDisposeDispatchRec(TEDispatchHandle hDispatch);
OSErr TEValidateRecord(TEHandle hTE);
OSErr TERecalLines(TEHandle hTE, short startLine);
void TEUpdateLineStarts(TEHandle hTE);

/* Style management internals */
OSErr TEInitStyles(TEHandle hTE);
OSErr TEDisposeStyles(TEHandle hTE);
OSErr TEStyleRunFromOffset(TEHandle hTE, long offset, StyleRun **styleRun);
short TEStyleIndexFromOffset(TEHandle hTE, long offset);

/* Text measurement and layout */
short TECharWidth(char ch, short font, short size, Style face);
short TETextWidth(Ptr textPtr, short textLen, short font, short size, Style face);
Point TEOffsetToPoint(TEHandle hTE, long offset);
long TEPointToOffset(TEHandle hTE, Point pt);

/* Selection and highlighting */
void TEHiliteRange(TEHandle hTE, long startSel, long endSel, Boolean hilite);
void TEDrawCaret(TEHandle hTE);
void TEHideCaret(TEHandle hTE);
void TEShowCaret(TEHandle hTE);

/* Platform abstraction internals */
OSErr TEInitPlatformInput(void);
void TECleanupPlatformInput(void);
OSErr TEPlatformHandleKeyEvent(TEHandle hTE, short key, long modifiers);
OSErr TEPlatformUpdateDisplay(TEHandle hTE, const Rect *updateRect);

#ifdef __cplusplus
}
#endif

#endif /* __TEXTTYPES_H__ */