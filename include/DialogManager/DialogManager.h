/*
 * DialogManager.h - Macintosh System 7.1 Dialog Manager API
 *
 * This header provides the complete Dialog Manager interface for Mac System 7.1,
 * maintaining exact behavioral compatibility while providing modern platform
 * integration capabilities.
 *
 * The Dialog Manager is essential for:
 * - Modal and modeless dialog handling
 * - Alert dialogs and system notifications
 * - File dialogs (Standard File Package)
 * - Dialog item management and interaction
 * - Resource-based dialog templates (DLOG/DITL)
 * - Keyboard navigation and accessibility
 */

#ifndef DIALOG_MANAGER_H
#define DIALOG_MANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations for Mac types */
/* Rect and Point types are defined in MacTypes.h */
/* Str255 is defined in MacTypes.h */
/* OSErr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Window and Event Manager dependencies */
/* Ptr is defined in MacTypes.h */
/* EventRecord defined in EventTypes.h */

/* TextEdit dependencies */
/* Handle is defined in MacTypes.h */

/* Dialog Manager core types */
/* Ptr is defined in MacTypes.h */

/* Dialog item types and constants */

/* Standard dialog button IDs */

/* Alert icon types */

/* Dialog item list manipulation methods */

/* Stage list type for alerts */

/* Callback procedure types */

/* Dialog record - internal structure exactly matching Mac OS */
/* DialogRecord is defined in SystemTypes.h */

/* Dialog template structure for DLOG resources */

/* Ptr is defined in MacTypes.h */

/* Alert template structure for ALRT resources */

/* Ptr is defined in MacTypes.h */

/* Dialog item structure for DITL resources */

/* Dialog item list (DITL) structure */

/* Handle is defined in MacTypes.h */

/* Modal dialog window classes */

/* Extended dialog features flags */

/*
 * CORE DIALOG MANAGER API
 * These functions provide exact Mac System 7.1 Dialog Manager compatibility
 */

/* Dialog Manager initialization and cleanup */
void InitDialogs(ResumeProcPtr resumeProc);
void ErrorSound(SoundProcPtr soundProc);

/* Dialog creation and disposal */
DialogPtr NewDialog(void* wStorage, const Rect* boundsRect, const unsigned char* title,
                    Boolean visible, SInt16 procID, WindowPtr behind, Boolean goAwayFlag,
                    SInt32 refCon, Handle itmLstHndl);
DialogPtr GetNewDialog(SInt16 dialogID, void* dStorage, WindowPtr behind);
DialogPtr NewColorDialog(void* dStorage, const Rect* boundsRect, const unsigned char* title,
                        Boolean visible, SInt16 procID, WindowPtr behind, Boolean goAwayFlag,
                        SInt32 refCon, Handle items);
void CloseDialog(DialogPtr theDialog);
void DisposDialog(DialogPtr theDialog);
void DisposeDialog(DialogPtr theDialog);

/* Dialog drawing and updating */
void DrawDialog(DialogPtr theDialog);
void UpdateDialog(DialogPtr theDialog, RgnHandle updateRgn);
void UpdtDialog(DialogPtr theDialog, RgnHandle updateRgn);

/* Modal dialog processing */
void ModalDialog(ModalFilterProcPtr filterProc, SInt16* itemHit);
Boolean IsDialogEvent(const EventRecord* theEvent);
Boolean DialogSelect(const EventRecord* theEvent, DialogPtr* theDialog, SInt16* itemHit);

/* Alert dialogs */
SInt16 Alert(SInt16 alertID, ModalFilterProcPtr filterProc);
SInt16 StopAlert(SInt16 alertID, ModalFilterProcPtr filterProc);
SInt16 NoteAlert(SInt16 alertID, ModalFilterProcPtr filterProc);
SInt16 CautionAlert(SInt16 alertID, ModalFilterProcPtr filterProc);

/* Alert stage management */
SInt16 GetAlertStage(void);
void ResetAlertStage(void);
void ResetAlrtStage(void);

/* Dialog item management */
void GetDialogItem(DialogPtr theDialog, SInt16 itemNo, SInt16* itemType,
                   Handle* item, Rect* box);
void SetDialogItem(DialogPtr theDialog, SInt16 itemNo, SInt16 itemType,
                   Handle item, const Rect* box);
void HideDialogItem(DialogPtr theDialog, SInt16 itemNo);
void ShowDialogItem(DialogPtr theDialog, SInt16 itemNo);
SInt16 FindDialogItem(DialogPtr theDialog, Point thePt);

/* Dialog text management */
void GetDialogItemText(Handle item, unsigned char* text);
void SetDialogItemText(Handle item, const unsigned char* text);
void SelectDialogItemText(DialogPtr theDialog, SInt16 itemNo, SInt16 strtSel, SInt16 endSel);
void ParamText(const unsigned char* param0, const unsigned char* param1,
               const unsigned char* param2, const unsigned char* param3);

/* Dialog edit operations */
void DialogCut(DialogPtr theDialog);
void DialogCopy(DialogPtr theDialog);
void DialogPaste(DialogPtr theDialog);
void DialogDelete(DialogPtr theDialog);

/* Dialog item list manipulation */
void AppendDITL(DialogPtr theDialog, Handle theHandle, DITLMethod method);
SInt16 CountDITL(DialogPtr theDialog);
void ShortenDITL(DialogPtr theDialog, SInt16 numberItems);

/* Dialog settings */
void SetDialogFont(SInt16 fontNum);
void SetDAFont(SInt16 fontNum);

/* Standard filter procedure and extended features */
Boolean StdFilterProc(DialogPtr theDialog, EventRecord* event, SInt16* itemHit);
OSErr GetStdFilterProc(ModalFilterProcPtr* theProc);
OSErr SetDialogDefaultItem(DialogPtr theDialog, SInt16 newItem);
OSErr SetDialogCancelItem(DialogPtr theDialog, SInt16 newItem);
OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks);

/* Window modal class support */
OSErr GetFrontWindowModalClass(SInt16* modalClass);
OSErr GetWindowModalClass(WindowPtr theWindow, SInt16* modalClass);

/* User item procedures */
void SetUserItem(DialogPtr theDialog, SInt16 itemNo, UserItemProcPtr procPtr);
UserItemProcPtr GetUserItem(DialogPtr theDialog, SInt16 itemNo);

/*
 * BACKWARDS COMPATIBILITY ALIASES
 * These maintain compatibility with existing Mac code
 */
#define GetDItem        GetDialogItem
#define SetDItem        SetDialogItem
#define HideDItem       HideDialogItem
#define ShowDItem       ShowDialogItem
#define SelIText        SelectDialogItemText
#define GetIText        GetDialogItemText
#define SetIText        SetDialogItemText
#define FindDItem       FindDialogItem
#define GetAlrtStage    GetAlertStage
#define DlgCut          DialogCut
#define DlgCopy         DialogCopy
#define DlgPaste        DialogPaste
#define DlgDelete       DialogDelete
#define NewCDialog      NewColorDialog

/*
 * EXTENDED API FOR MODERN PLATFORMS
 * These functions provide additional capabilities for modern integration
 */

/* Platform integration */
void DialogManager_SetNativeDialogEnabled(Boolean enabled);
Boolean DialogManager_GetNativeDialogEnabled(void);
void DialogManager_SetAccessibilityEnabled(Boolean enabled);
Boolean DialogManager_GetAccessibilityEnabled(void);

/* High-DPI support */
void DialogManager_SetScaleFactor(float scale);
float DialogManager_GetScaleFactor(void);

/* File dialog integration */
OSErr DialogManager_ShowOpenFileDialog(const char* title, const char* defaultPath,
                                      const char* fileTypes, char* selectedPath,
                                      size_t pathSize);
OSErr DialogManager_ShowSaveFileDialog(const char* title, const char* defaultPath,
                                      const char* defaultName, char* selectedPath,
                                      size_t pathSize);

/* Color and theme support */

void DialogManager_SetTheme(const DialogTheme* theme);
void DialogManager_GetTheme(DialogTheme* theme);

/*
 * INTERNAL UTILITY FUNCTIONS
 * These are used internally but may be useful for advanced applications
 */
Handle GetDialogItemList(DialogPtr theDialog);
void SetDialogItemList(DialogPtr theDialog, Handle itemList);
SInt16 GetDialogDefaultItem(DialogPtr theDialog);
SInt16 GetDialogCancelItem(DialogPtr theDialog);
Boolean GetDialogTracksCursor(DialogPtr theDialog);

/* Dialog state queries */
Boolean IsModalDialog(DialogPtr theDialog);
Boolean IsDialogVisible(DialogPtr theDialog);
WindowPtr GetDialogWindow(DialogPtr theDialog);
DialogPtr GetWindowDialog(WindowPtr theWindow);

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_MANAGER_H */
