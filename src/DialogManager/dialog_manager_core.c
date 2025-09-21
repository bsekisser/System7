/* #include "SystemTypes.h" */
#include <stdlib.h>
#include <string.h>
/*
 * dialog_manager_core.c - Core Dialog Manager Implementation
 *
 * RE-AGENT-BANNER: This file implements the core Dialog Manager functions
 * implemented based on Mac OS System 7.1 sources. All function
 * implementations are based on implementation and behavioral analysis.
 *
 * Evidence sources:
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "EventManager/EventTypes.h"
#include "WindowManager/WindowManager.h"
#include "DialogManager/dialog_manager_core.h"
#include "DialogManager/dialog_manager_dispatch.h"


static DialogMgrGlobals gDialogMgrGlobals = {
    .AnalyzedWindowState = 0,
    .IsDialogState = 0,
    .AnalyzedWindow = NULL,
    .SavedMenuState = NULL
};

/* Parameter text storage - evidence from implementation usage patterns */
static Str255 gParamText[4] = {{0}, {0}, {0}, {0}};

/* Dialog Manager initialization state */
static Boolean gDialogMgrInitialized = false;
static ResumeProcPtr gResumeProc = NULL;
static SoundProcPtr gSoundProc = NULL;

/*
 * String conversion utilities - evidence from implementation c2pstr/p2cstr patterns
 */
void C2PStr(char* str) {
    if (!str) return;

    /* Convert C string to Pascal string in place */
    unsigned char len = strlen(str);
    if (len > 255) len = 255;

    /* Move string data forward by 1 byte */
    memmove(str + 1, str, len);
    str[0] = len;  /* Set length byte */
}

void P2CStr(unsigned char* str) {
    if (!str) return;

    /* Convert Pascal string to C string in place */
    unsigned char len = str[0];
    if (len > 255) len = 255;

    /* Move string data back by 1 byte */
    memmove(str, str + 1, len);
    str[len] = '\0';  /* Null terminate */
}

/*
 * InitDialogs - Initialize Dialog Manager

 * implementation suggests this sets up global state
 */
void InitDialogs(ResumeProcPtr resumeProc) {
    gResumeProc = resumeProc;
    gSoundProc = NULL;

    /* Clear global state - evidence from DialogMgrGlobals structure */
    gDialogMgrGlobals.AnalyzedWindowState = 0;
    gDialogMgrGlobals.IsDialogState = 0;
    gDialogMgrGlobals.AnalyzedWindow = NULL;
    gDialogMgrGlobals.SavedMenuState = NULL;

    /* Clear parameter text */
    for (int i = 0; i < 4; i++) {
        gParamText[i][0] = 0;
    }

    gDialogMgrInitialized = true;
}

/*
 * ErrorSound - Set error sound procedure

 */
void ErrorSound(SoundProcPtr soundProc) {
    gSoundProc = soundProc;
}

/*
 * NewDialog - Create new dialog
 * Assembly shows c2pstr/p2cstr conversion for title parameter
 */
DialogPtr NewDialog(void* wStorage, const Rect* boundsRect,
                   const unsigned char* title, Boolean visible, SInt16 procID,
                   WindowPtr behind, Boolean goAwayFlag, SInt32 refCon,
                   Handle itmLstHndl) {
    DialogPtr dialog;

    if (!gDialogMgrInitialized) {
        return NULL;
    }

    /* Allocate dialog structure */
    if (wStorage) {
        dialog = (DialogPtr)wStorage;
    } else {
        dialog = (DialogPtr)malloc(sizeof(DialogRecord));
        if (!dialog) return NULL;
    }

    /* Initialize dialog record - evidence from DialogRecord structure */
    memset(dialog, 0, sizeof(DialogRecord));

    /* Set up dialog-specific fields */
    DialogPeek dialogPeek = (DialogPeek)dialog;
    dialogPeek->items = itmLstHndl;
    dialogPeek->textH = NULL;
    dialogPeek->editField = -1;  /* No active edit field */
    dialogPeek->editOpen = 0;
    dialogPeek->aDefItem = ok;   /* Default button is OK */

    /* Create underlying window - evidence from implementation creating window first */
    WindowPtr window = NewWindow(wStorage, boundsRect, title, visible,
                                procID, behind, goAwayFlag, refCon);
    if (!window) {
        if (!wStorage) free(dialog);
        return NULL;
    }

    /* Copy window structure into dialog - evidence from DialogRecord layout */
    memcpy(&dialogPeek->window, window, sizeof(WindowRecord));

    return dialog;
}

/*
 * NewColorDialog - Create new color dialog
 * Identical to NewDialog but uses color QuickDraw
 */
DialogPtr NewColorDialog(void* wStorage, const Rect* boundsRect,
                        const unsigned char* title, Boolean visible, SInt16 procID,
                        WindowPtr behind, Boolean goAwayFlag, SInt32 refCon,
                        Handle itmLstHndl) {
    /* Implementation identical to NewDialog for this reverse engineering */
    /* implementation shows same parameter handling with different trap vector */
    return NewDialog(wStorage, boundsRect, title, visible, procID,
                    behind, goAwayFlag, refCon, itmLstHndl);
}

/*
 * ParamText - Set parameter text for dialog substitution
 * Assembly shows c2pstr/p2cstr conversion for all 4 parameters
 */
void ParamText(const unsigned char* param0, const unsigned char* param1,
               const unsigned char* param2, const unsigned char* param3) {
    const unsigned char* params[4] = {param0, param1, param2, param3};

    /* Store parameter text - evidence from implementation storing all 4 parameters */
    for (int i = 0; i < 4; i++) {
        if (params[i]) {
            /* Copy parameter text - handle both C and Pascal strings */
            unsigned char len = strlen((const char*)params[i]);
            if (len > 255) len = 255;

            gParamText[i][0] = len;
            memcpy(&gParamText[i][1], params[i], len);
        } else {
            gParamText[i][0] = 0;  /* Empty string */
        }
    }
}

/*
 * GetDialogItemText - Get text from dialog item
 * Assembly shows p2cstr conversion for result text
 */
void GetDialogItemText(Handle item, Str255 text) {
    if (!item || !text) return;

    /* For this reverse engineering, implement basic text retrieval */
    /* Real implementation would extract text from item based on item type */
    text[0] = 0;  /* Empty string by default */

    /* TODO: Extract actual text from dialog item handle */
    /* Evidence suggests this involves examining item type and extracting */
    /* text content from control or static text item */
}

/*
 * SetDialogItemText - Set text in dialog item
 * Assembly shows c2pstr/p2cstr conversion for text parameter
 */
void SetDialogItemText(Handle item, const Str255 text) {
    if (!item || !text) return;

    /* For this reverse engineering, implement basic text setting */
    /* Real implementation would set text in item based on item type */

    /* TODO: Set actual text in dialog item handle */
    /* Evidence suggests this involves examining item type and setting */
    /* text content in control or static text item */
}

/*
 * FindDialogItem - Find dialog item at point
 * Assembly shows Point parameter handling: move.l (a0),-(sp)
 */
SInt16 FindDialogItem(DialogPtr theDialog, Point thePt) {
    if (!theDialog) return 0;

    DialogPeek dialogPeek = (DialogPeek)theDialog;
    Handle itemList = dialogPeek->items;

    if (!itemList) return 0;

    /* For this reverse engineering, implement basic hit testing */
    /* Real implementation would iterate through DITL and test each item rect */

    /* TODO: Implement actual hit testing against dialog item rectangles */
    /* Evidence from implementation suggests iterating through item list and */
    /* testing point against each item's rectangle */

    return 0;  /* No item found */
}

/*
 * StdFilterProc - Standard filter procedure
 * implementation is a trampoline that gets actual filter address
 */
Boolean StdFilterProc(DialogPtr dlg, EventRecord* evt, SInt16* itemHit) {
    if (!dlg || !evt || !itemHit) return false;

    /* For this reverse engineering, implement basic standard filtering */
    /* Real implementation calls _GetStdFilterProc to get actual filter */

    /* Handle standard dialog events - evidence from Mac documentation */
    switch (evt->what) {
        case keyDown:
        case autoKey:
            /* Handle Return/Enter as default button - evidence from behavior */
            if ((evt->message & charCodeMask) == '\r' ||
                (evt->message & charCodeMask) == 3) {  /* Enter */
                DialogPeek dialogPeek = (DialogPeek)dlg;
                *itemHit = dialogPeek->aDefItem;
                return true;
            }
            /* Handle Escape/Cmd-. as cancel - evidence from Mac standards */
            if ((evt->message & charCodeMask) == 27 ||  /* Escape */
                ((evt->message & charCodeMask) == '.' &&
                 (evt->modifiers & cmdKey))) {  /* Cmd-. */
                *itemHit = cancel;
                return true;
            }
            break;

        case mouseDown:
            /* Let dialog handle mouse clicks normally */
            return false;

        default:
            return false;
    }

    return false;
}

/*
 * GetDialogManagerGlobals - Access global state
 */
DialogMgrGlobals* GetDialogManagerGlobals(void) {
    return &gDialogMgrGlobals;
}

/*
