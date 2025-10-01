/************************************************************
 *
 * TextEdit.h
 * System 7.1 Portable - TextEdit Manager
 *
 * Main TextEdit API - Comprehensive text editing functionality
 * for System 7.1 applications. Provides complete compatibility
 * with Mac OS TextEdit Toolbox Manager.
 *
 * Copyright (c) 2024 System7.1-Portable Project
 * Derived from ROM analysis: TextEdit Manager 1985-1992
 *
 ************************************************************/

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "SystemTypes.h"

#ifndef __QUICKDRAW_H__
#include "QuickDraw/QuickDraw.h"
#endif

#ifndef __MEMORY_H__
#endif

#ifndef __TYPES_H__
#include "SystemTypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
 * TextEdit Constants
 ************************************************************/

/* Justification (word alignment) styles */

/* Set/Replace style modes */

/* TESetStyle/TEContinuousStyle modes */

/* Offsets into TEDispatchRec */

/* Selectors for TECustomHook */

/* Feature or bit definitions for TEFeatureFlag */

/* Action for TEFeatureFlag interface */

/* Constants for identifying FindWord callers */

/* Constants for identifying DoText selectors */

/************************************************************
 * TextEdit Data Types
 ************************************************************/

/* Forward declarations */

/* Handle is defined in MacTypes.h */

/* Hook procedure types */

/* Text data types */

/* Handle is defined in MacTypes.h */

/* Main TextEdit Record */

/* Style run structure */

/* Style table element */

/* Handle is defined in MacTypes.h */

/* Line height element */

/* Handle is defined in MacTypes.h */

/* Scrap style element */

/* Scrap style record */

/* Handle is defined in MacTypes.h */

/* Null style record */

/* Handle is defined in MacTypes.h */

/* TextEdit style record */

/* Handle is defined in MacTypes.h */

/* Text style */

/* Style is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/************************************************************
 * Core TextEdit Functions
 ************************************************************/

/* Initialization */
pascal void TEInit(void);

/* TextEdit Record Management */
pascal TEHandle TENew(const Rect *destRect, const Rect *viewRect);
pascal TEHandle TEStyleNew(const Rect *destRect, const Rect *viewRect);
pascal TEHandle TEStylNew(const Rect *destRect, const Rect *viewRect); /* synonym */
pascal void TEDispose(TEHandle hTE);

/* Text Access */
pascal void TESetText(const void *text, long length, TEHandle hTE);
pascal CharsHandle TEGetText(TEHandle hTE);

/* Selection Management */
pascal void TESetSelect(long selStart, long selEnd, TEHandle hTE);
pascal void TEClick(Point pt, Boolean fExtend, TEHandle h);
pascal short TEGetOffset(Point pt, TEHandle hTE);
pascal Point TEGetPoint(short offset, TEHandle hTE);

/* Activation */
pascal void TEActivate(TEHandle hTE);
pascal void TEDeactivate(TEHandle hTE);
pascal void TEIdle(TEHandle hTE);

/* Text Input */
pascal void TEKey(short key, TEHandle hTE);
pascal void TEInsert(const void *text, long length, TEHandle hTE);
pascal void TEDelete(TEHandle hTE);

/* Clipboard Operations */
pascal void TECut(TEHandle hTE);
pascal void TECopy(TEHandle hTE);
pascal void TEPaste(TEHandle hTE);

/* Display and Scrolling */
pascal void TEUpdate(const Rect *rUpdate, TEHandle hTE);
pascal void TEScroll(short dh, short dv, TEHandle hTE);
pascal void TESelView(TEHandle hTE);
pascal void TEPinScroll(short dh, short dv, TEHandle hTE);
pascal void TEAutoView(Boolean fAuto, TEHandle hTE);

/* Text Formatting */
pascal void TESetAlignment(short just, TEHandle hTE);
pascal void TESetJust(short just, TEHandle hTE);
pascal void TECalText(TEHandle hTE);
pascal void TETextBox(const void *text, long length, const Rect *box, short just);
pascal void TextBox(const void *text, long length, const Rect *box, short just);

/* Style Management */
pascal void TESetStyleHandle(TEStyleHandle theHandle, TEHandle hTE);
pascal void SetStyleHandle(TEStyleHandle theHandle, TEHandle hTE); /* synonym */
pascal void SetStylHandle(TEStyleHandle theHandle, TEHandle hTE);  /* synonym */
pascal TEStyleHandle TEGetStyleHandle(TEHandle hTE);
pascal TEStyleHandle GetStyleHandle(TEHandle hTE); /* synonym */
pascal TEStyleHandle GetStylHandle(TEHandle hTE);  /* synonym */

pascal void TEGetStyle(short offset, TextStyle *theStyle, short *lineHeight,
                      short *fontAscent, TEHandle hTE);
pascal void TESetStyle(short mode, const TextStyle *newStyle, Boolean fRedraw,
                      TEHandle hTE);
pascal void TEReplaceStyle(short mode, const TextStyle *oldStyle,
                          const TextStyle *newStyle, Boolean fRedraw, TEHandle hTE);
pascal Boolean TEContinuousStyle(short *mode, TextStyle *aStyle, TEHandle hTE);

/* Style Scrap Operations */
pascal StScrpHandle TEGetStyleScrapHandle(TEHandle hTE);
pascal StScrpHandle GetStyleScrap(TEHandle hTE); /* synonym */
pascal StScrpHandle GetStylScrap(TEHandle hTE);  /* synonym */
pascal void TEStyleInsert(const void *text, long length, StScrpHandle hST,
                         TEHandle hTE);
pascal void TEStylInsert(const void *text, long length, StScrpHandle hST,
                        TEHandle hTE); /* synonym */
pascal void TEStylePaste(TEHandle hTE);
pascal void TEStylPaste(TEHandle hTE); /* synonym */
pascal void TEUseStyleScrap(long rangeStart, long rangeEnd, StScrpHandle newStyles,
                           Boolean fRedraw, TEHandle hTE);
pascal void SetStyleScrap(long rangeStart, long rangeEnd, StScrpHandle newStyles,
                         Boolean redraw, TEHandle hTE); /* synonym */
pascal void SetStylScrap(long rangeStart, long rangeEnd, StScrpHandle newStyles,
                        Boolean redraw, TEHandle hTE);  /* synonym */

/* Advanced Functions */
pascal long TEGetHeight(long endLine, long startLine, TEHandle hTE);
pascal long TENumStyles(long rangeStart, long rangeEnd, TEHandle hTE);
pascal void TECustomHook(TEIntHook which, ProcPtr *addr, TEHandle hTE);
pascal short TEFeatureFlag(short feature, short action, TEHandle hTE);

/* Hook Functions */
pascal void TESetClickLoop(ClikLoopProcPtr clikProc, TEHandle hTE);
pascal void SetClikLoop(ClikLoopProcPtr clikProc, TEHandle hTE); /* synonym */
pascal void TESetWordBreak(WordBreakProcPtr wBrkProc, TEHandle hTE);
pascal void SetWordBreak(WordBreakProcPtr wBrkProc, TEHandle hTE); /* synonym */

/* Scrap Functions */
#define TEScrapHandle() (* (Handle*) 0xAB4)
/* Note: TEGetScrapLength is implemented as function in ScrapManager.h */
/* #define TEGetScrapLength() ((long) * (unsigned short *) 0x0AB0) */
#define TEGetScrapLen() ((long) * (unsigned short *) 0x0AB0)
pascal void TESetScrapLength(long length);
pascal void TESetScrapLen(long length);
pascal OSErr TEFromScrap(void);
pascal OSErr TEToScrap(void);

/* C interface helper */
void teclick(Point *pt, Boolean fExtend, TEHandle h);

/************************************************************
 * Internal/Platform Abstraction Functions
 ************************************************************/

/* Modern platform integration */
OSErr TEInitPlatform(void);
void TECleanupPlatform(void);

/* Unicode/multi-byte character support */
OSErr TESetTextEncoding(TEHandle hTE, TextEncoding encoding);
TextEncoding TEGetTextEncoding(TEHandle hTE);

/* Modern input method support */
OSErr TESetInputMethod(TEHandle hTE, Boolean useModernIM);
Boolean TEGetInputMethod(TEHandle hTE);

/* Accessibility support */
OSErr TESetAccessibilityEnabled(TEHandle hTE, Boolean enabled);
Boolean TEGetAccessibilityEnabled(TEHandle hTE);

#ifdef __cplusplus
}
#endif

#endif /* __TEXTEDIT_H__ */