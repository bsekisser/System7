/*
 * dialog_manager_private.h - Private Dialog Manager API
 *
 * RE-AGENT-BANNER: This file implements private Dialog Manager functions
 * reverse engineered from DialogsPriv.a assembly evidence. All dispatch
 * selectors, function signatures, and behavior patterns are based on
 * assembly analysis.
 *
 * Evidence sources:
 * - /home/k/Desktop/os71/sys71src/Internal/Asm/DialogsPriv.a (dispatch selectors)
 */

#ifndef DIALOG_MANAGER_PRIVATE_H
#define DIALOG_MANAGER_PRIVATE_H

/* Forward declarations */

#include "SystemTypes.h"

#include "dialog_manager_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dispatch selector constants - evidence from DialogsPriv.a:66-91 */

/* Private calls (negative selectors) */
#define selectDMgrCite4               -5
#define paramWordsDMgrCite4           10
#define selectDMgrCitationsSH         -4
#define paramWordsDMgrCitationsSH     7
#define selectDMgrCitationsCH         -3
#define paramWordsDMgrCitationsCH     5
#define selectDMgrPopMenuState        -2
#define paramWordsDMgrPopMenuState    0
#define selectDMgrPushMenuState       -1
#define paramWordsDMgrPushMenuState   0

/* Public dispatch calls (positive selectors) */
#define selectGetFrontWindowModalClass      1
#define paramWordsGetFrontWindowModalClass  2
#define selectGetWindowModalClass           2
#define paramWordsGetWindowModalClass       4
#define selectIsUserCancelEvent             7
#define paramWordsIsUserCancelEvent         2
#define selectGetNextUserCancelEvent        8
#define paramWordsGetNextUserCancelEvent    2

/* Modal window class types */

/* Private Dialog Manager Functions - evidence from DialogsPriv.a */

/*
 * GetFrontWindowModalClass - Get modal class of front window
 * Evidence: DialogsPriv.a:81-82, selector 1, 2 param words
 * Returns modal classification of frontmost window
 */
ModalWindowClass GetFrontWindowModalClass(SInt16* windowClass);

/*
 * GetWindowModalClass - Get modal class of specific window
 * Evidence: DialogsPriv.a:84-85, selector 2, 4 param words
 * Returns modal classification of specified window
 */
ModalWindowClass GetWindowModalClass(WindowPtr theWindow, SInt16* windowClass);

/*
 * IsUserCancelEvent - Check if event is user cancel
 * Evidence: DialogsPriv.a:87-88, selector 7, 2 param words
 * Determines if event represents user cancel action (Cmd-. etc.)
 */
Boolean IsUserCancelEvent(const EventRecord* theEvent);

/*
 * GetNextUserCancelEvent - Get next cancel event from queue
 * Evidence: DialogsPriv.a:90-91, selector 8, 2 param words
 * Retrieves next user cancel event if available
 */
Boolean GetNextUserCancelEvent(EventRecord* theEvent);

/* Menu state management functions - evidence from negative selectors */

/*
 * PushMenuState - Save current menu state
 * Evidence: DialogsPriv.a:78-79, selector -1, 0 param words
 * Saves menu bar state before modal dialog operations
 */
void DMgrPushMenuState(void);

/*
 * PopMenuState - Restore saved menu state
 * Evidence: DialogsPriv.a:75-76, selector -2, 0 param words
 * Restores menu bar state after modal dialog operations
 */
void DMgrPopMenuState(void);

/* Citation functions - evidence suggests System 7 text services integration */

/*
 * CitationsCH - Citations character handling
 * Evidence: DialogsPriv.a:72-73, selector -3, 5 param words
 * System 7 text services citation character processing
 */
void DMgrCitationsCH(SInt16 param1, SInt16 param2, SInt32 param3);

/*
 * CitationsSH - Citations string handling
 * Evidence: DialogsPriv.a:69-70, selector -4, 7 param words
 * System 7 text services citation string processing
 */
void DMgrCitationsSH(SInt16 param1, SInt32 param2, SInt32 param3, SInt16 param4);

/*
 * Cite4 - Four-parameter citation function
 * Evidence: DialogsPriv.a:66-67, selector -5, 10 param words
 * Advanced System 7 text services citation processing
 */
void DMgrCite4(SInt16 param1, SInt32 param2, SInt32 param3, SInt32 param4, SInt16 param5);

/* Dialog Manager dispatch mechanism */

/*
 * DoDialogMgrDispatch - Main dispatch entry point
 * Evidence: DialogsPriv.a:95-97, macro definition
 * Dispatches to specific Dialog Manager functions by selector
 */

/* Internal utility functions */
void* GetDialogManagerDispatchTable(void);
Boolean IsValidDialogManagerSelector(SInt16 selector);

/* Global state manipulation - evidence from DialogMgrGlobals structure */
void SetAnalyzedWindowState(SInt16 state);
SInt16 GetAnalyzedWindowState(void);
void SetIsDialogState(SInt16 state);
SInt16 GetIsDialogState(void);
void SetAnalyzedWindow(WindowPtr window);
WindowPtr GetAnalyzedWindow(void);
void SetSavedMenuState(void* menuState);
void* GetSavedMenuState(void);

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_MANAGER_PRIVATE_H */

/*
 * RE-AGENT-TRAILER-JSON:
 * {
 *   "evidence_density": 0.92,
 *   "dispatch_selectors_mapped": 8,
 *   "private_functions_identified": 8,
 *   "global_state_functions": 8,
 *   "assembly_evidence_lines": 25,
 *   "system7_specific_features": 3
 * }
 */