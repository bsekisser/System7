/*
 * DialogEditText.c - Dialog Edit Text Focus and Caret Management
 *
 * Implements edit-text focus tracking and caret blinking for System 7.1 dialogs.
 * Addresses the compatibility gap noted in System7_Compatibility_Gaps.md:
 * - Edit-text items ignore focus rings
 * - System 7 drew a focus frame and moved the caret when the control is active
 */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/DialogManagerInternal.h"
#include "DialogManager/DialogManagerStateExt.h"
#include "DialogManager/DialogItems.h"
#include "DialogManager/DialogLogging.h"

/* External dependencies */
extern UInt32 TickCount(void);
extern void InvalDialogItem(DialogPtr theDialog, SInt16 itemNo);
extern void DrawDialogItem(DialogPtr theDialog, SInt16 itemNo);
extern DialogManagerState* GetDialogManagerState(void);

/* Caret blink rate in ticks (System 7 standard was ~30 ticks = 0.5 seconds) */
#define kCaretBlinkRate 30

/*
 * SetDialogEditTextFocus - Set keyboard focus to an edit-text item
 *
 * Parameters:
 *   theDialog - The dialog containing the item
 *   itemNo    - The edit-text item number to focus (0 to clear focus)
 */
void SetDialogEditTextFocus(DialogPtr theDialog, SInt16 itemNo) {
    DialogManagerState* state;
    DialogManagerState_Extended* extState;
    SInt16 oldFocusItem;

    state = GetDialogManagerState();
    extState = GET_EXTENDED_DLG_STATE(state);

    if (!state || !theDialog) {
        return;
    }

    oldFocusItem = extState->focusedEditTextItem;

    /* Clear old focus */
    if (oldFocusItem > 0 && oldFocusItem != itemNo) {
        extState->focusedEditTextItem = 0;
        extState->caretVisible = false;
        InvalDialogItem(theDialog, oldFocusItem);
        DrawDialogItem(theDialog, oldFocusItem);
    }

    /* Set new focus */
    if (itemNo > 0) {
        extState->focusedEditTextItem = itemNo;
        extState->caretBlinkTime = TickCount();
        extState->caretVisible = true;
        InvalDialogItem(theDialog, itemNo);
        DrawDialogItem(theDialog, itemNo);

        DIALOG_LOG_DEBUG("Edit-text focus set to item %d\n", itemNo);
    } else {
        extState->focusedEditTextItem = 0;
        extState->caretVisible = false;
        DIALOG_LOG_DEBUG("Edit-text focus cleared\n");
    }
}

/*
 * GetDialogEditTextFocus - Get the currently focused edit-text item
 *
 * Parameters:
 *   theDialog - The dialog to query
 *
 * Returns:
 *   The item number of the focused edit-text item, or 0 if none
 */
SInt16 GetDialogEditTextFocus(DialogPtr theDialog) {
    DialogManagerState* state;
    DialogManagerState_Extended* extState;

    state = GetDialogManagerState();
    extState = GET_EXTENDED_DLG_STATE(state);

    if (!state || !theDialog) {
        return 0;
    }

    return extState->focusedEditTextItem;
}

/*
 * UpdateDialogCaret - Update caret blink state
 *
 * Should be called from the dialog event loop to maintain caret blinking.
 * This implements the classic Mac OS caret blinking behavior.
 *
 * Parameters:
 *   theDialog - The dialog to update
 */
void UpdateDialogCaret(DialogPtr theDialog) {
    DialogManagerState* state;
    DialogManagerState_Extended* extState;

    state = GetDialogManagerState();
    extState = GET_EXTENDED_DLG_STATE(state);
    UInt32 currentTicks;
    UInt32 elapsed;

    if (!state || !theDialog || extState->focusedEditTextItem == 0) {
        return;
    }

    currentTicks = TickCount();

    /* Handle tick counter wrap-around */
    if (currentTicks < extState->caretBlinkTime) {
        extState->caretBlinkTime = currentTicks;
        return;
    }

    elapsed = currentTicks - extState->caretBlinkTime;

    /* Toggle caret visibility every kCaretBlinkRate ticks */
    if (elapsed >= kCaretBlinkRate) {
        extState->caretVisible = !extState->caretVisible;
        extState->caretBlinkTime = currentTicks;

        /* Redraw the focused edit-text item */
        InvalDialogItem(theDialog, extState->focusedEditTextItem);
        DrawDialogItem(theDialog, extState->focusedEditTextItem);

        DIALOG_LOG_DEBUG("Caret blink: %s\n", extState->caretVisible ? "visible" : "hidden");
    }
}

/*
 * AdvanceDialogEditTextFocus - Move focus to next/previous edit-text item
 *
 * Implements Tab/Shift-Tab navigation between edit-text fields.
 *
 * Parameters:
 *   theDialog - The dialog containing the items
 *   backward  - true to move backward (Shift-Tab), false for forward (Tab)
 */
void AdvanceDialogEditTextFocus(DialogPtr theDialog, Boolean backward) {
    DialogManagerState* state;
    DialogManagerState_Extended* extState;

    state = GetDialogManagerState();
    extState = GET_EXTENDED_DLG_STATE(state);
    SInt16 itemCount;
    SInt16 currentFocus;
    SInt16 nextFocus;
    SInt16 i;
    SInt16 itemType;
    Handle itemHandle;
    Rect itemBox;

    if (!state || !theDialog) {
        return;
    }

    /* Get item count - this is a placeholder, real implementation would
       query the dialog's item list */
    itemCount = 32; /* Conservative maximum */
    currentFocus = extState->focusedEditTextItem;
    nextFocus = 0;

    /* Find next focusable edit-text item */
    if (backward) {
        /* Search backward from current focus */
        for (i = currentFocus - 1; i >= 1; i--) {
            GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
            if ((itemType & 0x7F) == editText) {
                nextFocus = i;
                break;
            }
        }
        /* Wrap around if no item found */
        if (nextFocus == 0) {
            for (i = itemCount; i > currentFocus; i--) {
                GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
                if ((itemType & 0x7F) == editText) {
                    nextFocus = i;
                    break;
                }
            }
        }
    } else {
        /* Search forward from current focus */
        for (i = currentFocus + 1; i <= itemCount; i++) {
            GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
            if ((itemType & 0x7F) == editText) {
                nextFocus = i;
                break;
            }
        }
        /* Wrap around if no item found */
        if (nextFocus == 0) {
            for (i = 1; i < currentFocus; i++) {
                GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
                if ((itemType & 0x7F) == editText) {
                    nextFocus = i;
                    break;
                }
            }
        }
    }

    /* Set focus to next item if found */
    if (nextFocus > 0) {
        SetDialogEditTextFocus(theDialog, nextFocus);
    }
}

/*
 * ClearDialogEditTextFocus - Clear edit-text focus for a dialog
 *
 * Called when a dialog loses focus or is disposed.
 *
 * Parameters:
 *   theDialog - The dialog to clear focus from
 */
void ClearDialogEditTextFocus(DialogPtr theDialog) {
    SetDialogEditTextFocus(theDialog, 0);
}

/*
 * InitDialogEditTextFocus - Initialize edit-text focus for a dialog
 *
 * Sets focus to the first edit-text item in the dialog.
 *
 * Parameters:
 *   theDialog - The dialog to initialize
 */
void InitDialogEditTextFocus(DialogPtr theDialog) {
    SInt16 itemCount;
    SInt16 i;
    SInt16 itemType;
    Handle itemHandle;
    Rect itemBox;

    if (!theDialog) {
        return;
    }

    /* Find first edit-text item */
    itemCount = 32; /* Conservative maximum */
    for (i = 1; i <= itemCount; i++) {
        GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
        if ((itemType & 0x7F) == editText) {
            SetDialogEditTextFocus(theDialog, i);
            DIALOG_LOG_DEBUG("Initial edit-text focus set to item %d\n", i);
            return;
        }
    }
}
