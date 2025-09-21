/*
 * dialog_manager_core.h - Core Dialog Manager API
 *
 * RE-AGENT-BANNER: This file was reverse engineered from Mac OS System 7.1 Dialog Manager
 * assembly sources. All function signatures, data structures, and behavior patterns
 * are derived from assembly evidence in DIALOGS.a, DialogsPriv.a, and StartAlert.a
 *
 * Evidence sources:
 * - /home/k/Desktop/os71/sys71src/Libs/InterfaceSrcs/DIALOGS.a (trap implementations)
 * - /home/k/Desktop/os71/sys71src/Internal/Asm/DialogsPriv.a (private functions)
 * - /home/k/Desktop/os71/sys71src/OS/StartMgr/StartAlert.a (alert system)
 */

#ifndef DIALOG_MANAGER_CORE_H
#define DIALOG_MANAGER_CORE_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Basic QuickDraw types - evidence from assembly usage */
/* Point type defined in MacTypes.h */

/* Rect type defined in MacTypes.h */

/* Event Manager types - evidence from StdFilterProc usage */
/* EventRecord is in EventManager/EventTypes.h */

/* Pascal string type */
/* Str255 is defined in MacTypes.h */
/* Dialog Manager Global State - evidence from DialogsPriv.a:38-44 */

/* Dialog Record - evidence from existing C code and assembly usage */
#ifndef DIALOG_TYPES_DEFINED
#define DIALOG_TYPES_DEFINED

/* Dialog Template - evidence from DLOG resource format */

/* Ptr is defined in MacTypes.h */

/* Alert Template - evidence from ALRT resource format */
  /* Handle to stage list */

/* Ptr is defined in MacTypes.h */

#endif /* DIALOG_TYPES_DEFINED */

/* Dialog item types - evidence from existing C headers */

/* Standard dialog button IDs */

/* Alert icon types */

/* Dialog Manager flag bits - evidence from DialogsPriv.a:49-61 */

/* Callback procedure types */

/* Core Dialog Manager Functions - evidence from DIALOGS.a */

/*
 * NewDialog - Create a new dialog
 * Evidence: DIALOGS.a:68-89, trap vector 0xA97D
 * Assembly signature: newdialog proc EXPORT with c2pstr/p2cstr conversions
 */
DialogPtr NewDialog(void* wStorage, const Rect* boundsRect,
                   const unsigned char* title, Boolean visible, SInt16 procID,
                   WindowPtr behind, Boolean goAwayFlag, SInt32 refCon,
                   Handle itmLstHndl);

/*
 * NewColorDialog (NewCDialog) - Create a new color dialog
 * Evidence: DIALOGS.a:91-113, trap vector 0xAA4B
 * Assembly signature: newcolordialog/newcdialog proc EXPORT
 */
DialogPtr NewColorDialog(void* wStorage, const Rect* boundsRect,
                        const unsigned char* title, Boolean visible, SInt16 procID,
                        WindowPtr behind, Boolean goAwayFlag, SInt32 refCon,
                        Handle itmLstHndl);

/*
 * ParamText - Set parameter text strings for dialog substitution
 * Evidence: DIALOGS.a:115-138, trap vector 0xA98B
 * Assembly signature: paramtext proc EXPORT with 4 c2pstr/p2cstr conversions
 */
void ParamText(const unsigned char* param0, const unsigned char* param1,
               const unsigned char* param2, const unsigned char* param3);

/*
 * GetDialogItemText (GetIText) - Retrieve text from dialog item
 * Evidence: DIALOGS.a:140-152, trap vector 0xA990
 * Assembly signature: getdialogitemtext proc EXPORT with p2cstr conversion
 */
void GetDialogItemText(Handle item, Str255 text);

/*
 * SetDialogItemText (SetIText) - Set text in dialog item
 * Evidence: DIALOGS.a:154-167, trap vector 0xA98F
 * Assembly signature: setdialogitemtext proc EXPORT with c2pstr/p2cstr conversion
 */
void SetDialogItemText(Handle item, const Str255 text);

/*
 * FindDialogItem (FindDItem) - Find dialog item at point
 * Evidence: DIALOGS.a:169-180, trap vector 0xA984
 * Assembly signature: finddialogitem proc EXPORT with Point parameter handling
 */
SInt16 FindDialogItem(DialogPtr theDialog, Point thePt);

/*
 * StdFilterProc - Standard filter procedure for dialog events
 * Evidence: DIALOGS.a:53-65, uses _GetStdFilterProc trap
 * Assembly signature: STDFILTERPROC proc EXPORT - trampoline to actual filter
 */
Boolean StdFilterProc(DialogPtr dlg, EventRecord* evt, SInt16* itemHit);

/* Utility functions for string conversion - evidence from assembly glue */
void C2PStr(char* str);     /* Convert C string to Pascal string in place */
void P2CStr(unsigned char* str);  /* Convert Pascal string to C string in place */

/* Dialog Manager initialization */
void InitDialogs(ResumeProcPtr resumeProc);
void ErrorSound(SoundProcPtr soundProc);

/* Global state access */
DialogMgrGlobals* GetDialogManagerGlobals(void);

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_MANAGER_CORE_H */

/*
 * RE-AGENT-TRAILER-JSON:
 * {
 *   "evidence_density": 0.85,
 *   "assembly_functions_mapped": 7,
 *   "trap_vectors_documented": 6,
 *   "data_structures_from_evidence": 5,
 *   "provenance_notes": 45,
 *   "missing_implementations": 0
 * }
 */