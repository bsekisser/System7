/* #include "SystemTypes.h" */
#include "SystemTypes.h"
#include <string.h>
/*
 * ModalDialogs.c - Modal Dialog Management Implementation
 *
 * This module provides the modal dialog processing functionality,
 * maintaining exact Mac System 7.1 behavioral compatibility.
 */

// #include "CompatibilityFix.h" // Removed
#include "System71StdLib.h"

#include "DialogManager/ModalDialogs.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/DialogEvents.h"
#include "DialogManager/DialogItems.h"
#include "DialogManager/DialogManagerStateExt.h"
#include <stdbool.h>

/* Event constants - matching Mac System 7.1 */
#define kDialogEvent_Null         0
#define kDialogEvent_MouseDown    1
#define kDialogEvent_MouseUp      2
#define kDialogEvent_KeyDown      3
#define kDialogEvent_KeyUp        4
#define kDialogEvent_AutoKey      5
#define kDialogEvent_Update       6
#define kDialogEvent_DiskEvt      7
#define kDialogEvent_Activate     8
#define kDialogEvent_OsEvt        15

/* Key codes */
#define kDialogKey_Return         0x0D
#define kDialogKey_Enter          0x03
#define kDialogKey_Escape         0x1B
#define kDialogKey_Tab            0x09

/* Modifiers */
#define kDialogModifier_Shift     0x0200
#define kDialogModifier_Command   0x0100
#define kDialogModifier_Option    0x0800
#define kDialogModifier_Control   0x1000

/* Dialog item type constants */
#define itemTypeMask              0x7F
#define ctrlItem                  4
#define btnCtrl                   0
#define chkCtrl                   1
#define radCtrl                   2
#define resCtrl                   3
#define statText                  8
#define editText                  16
#define iconItem                  32
#define picItem                   64
#define userItem                  0
#define itemDisable               128

/* Dialog event result codes */
#define kDialogEventResult_NotHandled   0
#define kDialogEventResult_Handled      1


/* External functions that need to be linked */
extern Boolean GetNextEvent(SInt16 eventMask, EventRecord* theEvent);
extern void SystemTask(void);
extern UInt32 TickCount(void);
extern void Delay(UInt32 ticks, UInt32* finalTicks);

/* Private state for modal dialog processing */
static struct {
    Boolean            initialized;
    DialogPtr       currentModal;
    Boolean            inModalLoop;
    SInt16         modalLevel;
    WindowPtr       modalStack[16];
    UInt32        lastEventTime;
    Boolean            modalFiltersActive;
    ModalFilterProcPtr installedFilters[16];
    void*           filterUserData[16];
} gModalState = {0};

/* Function prototypes */
Boolean ProcessModalEvent(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit);
static Boolean CallModalFilter(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit);
static void UpdateModalState(DialogPtr theDialog);
static Boolean IsEventForDialog(DialogPtr theDialog, const EventRecord* theEvent);
static void HandleModalTimeout(DialogPtr theDialog, SInt16* itemHit);
static Boolean ProcessStandardModalKeys(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit);
static void FlashButtonInternal(DialogPtr theDialog, SInt16 itemNo);

/* Global Dialog Manager state access */
extern DialogManagerState* GetDialogManagerState(void);

/*
 * InitModalDialogs - Initialize modal dialog subsystem
 */
void InitModalDialogs(void)
{
    if (gModalState.initialized) {
        return;
    }

    memset(&gModalState, 0, sizeof(gModalState));
    gModalState.initialized = true;
    gModalState.currentModal = NULL;
    gModalState.inModalLoop = false;
    gModalState.modalLevel = 0;
    gModalState.lastEventTime = 0;
    gModalState.modalFiltersActive = false;

    /* Clear modal stack and filters */
    for (int i = 0; i < 16; i++) {
        gModalState.modalStack[i] = NULL;
        gModalState.installedFilters[i] = NULL;
        gModalState.filterUserData[i] = NULL;
    }

    serial_printf("Modal dialog subsystem initialized\n");
}

/*
 * ModalDialog - Run modal dialog event loop
 *
 * This is the core modal dialog function that runs the event loop until
 * the user clicks a button or performs an action that ends the dialog.
 */
void ModalDialog(ModalFilterProcPtr filterProc, SInt16* itemHit)
{
    DialogPtr theDialog;
    EventRecord theEvent;
    Boolean done = false;
    SInt16 eventMask = 0xFFFF; /* All events */
    UInt32 timeoutTime = 0;

    if (!gModalState.initialized) {
        serial_printf("Error: Modal dialog system not initialized\n");
        if (itemHit) *itemHit = 0;
        return;
    }

    /* Get the current front modal dialog */
    theDialog = GetFrontModalDialog();
    if (!theDialog) {
        serial_printf("Error: No modal dialog to process\n");
        if (itemHit) *itemHit = 0;
        return;
    }

    /* Set up modal state */
    gModalState.inModalLoop = true;
    gModalState.currentModal = theDialog;
    gModalState.lastEventTime = TickCount();

    if (itemHit) *itemHit = 0;

    serial_printf("Starting modal dialog loop for dialog at %p\n", (void*)theDialog);

    /* Main modal event loop */
    while (!done) {
        /* Check for timeout if configured */
        if (timeoutTime > 0 && TickCount() > timeoutTime) {
            HandleModalTimeout(theDialog, itemHit);
            break;
        }

        /* Get next event */
        if (GetNextEvent(eventMask, &theEvent)) {
            gModalState.lastEventTime = theEvent.when;

            /* Call custom filter first if provided */
            if (filterProc) {
                if (filterProc(theDialog, &theEvent, itemHit)) {
                    /* Filter handled the event */
                    if (itemHit && *itemHit != 0) {
                        done = true;
                    }
                    continue;
                }
            }

            /* Call installed filter if any */
            if (CallModalFilter(theDialog, &theEvent, itemHit)) {
                if (itemHit && *itemHit != 0) {
                    done = true;
                }
                continue;
            }

            /* Process the event for the dialog */
            if (ProcessModalEvent(theDialog, &theEvent, itemHit)) {
                if (itemHit && *itemHit != 0) {
                    done = true;
                }
            }
        } else {
            /* No event - give time to system and handle idle processing */
            SystemTask();
            ProcessDialogIdle(theDialog);

            /* Small delay to prevent spinning */
            UInt32 finalTicks;
            Delay(1, &finalTicks);
        }

        /* Update modal state */
        UpdateModalState(theDialog);
    }

    /* Clean up modal state */
    gModalState.inModalLoop = false;
    gModalState.currentModal = NULL;

    serial_printf("Modal dialog loop ended, item hit: %d\n", itemHit ? *itemHit : 0);
}

/*
 * IsDialogEvent - Check if an event belongs to a dialog
 */
Boolean IsDialogEvent(const EventRecord* theEvent)
{
    if (!theEvent || !gModalState.initialized) {
        return false;
    }

    /* Check if event is for the current modal dialog */
    if (gModalState.currentModal) {
        return IsEventForDialog(gModalState.currentModal, theEvent);
    }

    /* Check all dialogs in modal stack */
    for (int i = 0; i < gModalState.modalLevel; i++) {
        if (gModalState.modalStack[i]) {
            DialogPtr dialog = (DialogPtr)gModalState.modalStack[i];
            if (IsEventForDialog(dialog, theEvent)) {
                return true;
            }
        }
    }

    return false;
}

/*
 * DialogSelect - Handle dialog event
 */
Boolean DialogSelect(const EventRecord* theEvent, DialogPtr* theDialog, SInt16* itemHit)
{
    if (!theEvent || !gModalState.initialized) {
        if (theDialog) *theDialog = NULL;
        if (itemHit) *itemHit = 0;
        return false;
    }

    /* Initialize return values */
    if (theDialog) *theDialog = NULL;
    if (itemHit) *itemHit = 0;

    /* Check if this is a dialog event */
    if (!IsDialogEvent(theEvent)) {
        return false;
    }

    /* Find which dialog should handle this event */
    DialogPtr targetDialog = NULL;

    if (gModalState.currentModal && IsEventForDialog(gModalState.currentModal, theEvent)) {
        targetDialog = gModalState.currentModal;
    } else {
        /* Check modal stack */
        for (int i = gModalState.modalLevel - 1; i >= 0; i--) {
            if (gModalState.modalStack[i]) {
                DialogPtr dialog = (DialogPtr)gModalState.modalStack[i];
                if (IsEventForDialog(dialog, theEvent)) {
                    targetDialog = dialog;
                    break;
                }
            }
        }
    }

    if (!targetDialog) {
        return false;
    }

    /* Process the event for the target dialog */
    EventRecord eventCopy = *theEvent;
    SInt16 hit = 0;
    Boolean handled = ProcessModalEvent(targetDialog, &eventCopy, &hit);

    if (handled) {
        if (theDialog) *theDialog = targetDialog;
        if (itemHit) *itemHit = hit;
    }

    return handled;
}

/*
 * BeginModalDialog - Begin modal dialog processing
 */
OSErr BeginModalDialog(DialogPtr theDialog)
{
    if (!theDialog || !gModalState.initialized) {
        return -1700; /* dialogErr_InvalidDialog */
    }

    if (gModalState.modalLevel >= 16) {
        return -1706; /* dialogErr_AlreadyModal */
    }

    /* Add dialog to modal stack */
    gModalState.modalStack[gModalState.modalLevel] = (WindowPtr)theDialog;
    gModalState.modalLevel++;

    /* Set as front modal dialog */
    DialogManagerState* state = GetDialogManagerState();
    if (state) {
        (state)->globals.frontModal = theDialog;
    }

    /* Bring dialog to front */
    BringModalToFront(theDialog);

    /* Disable non-modal windows */
    DisableNonModalWindows();

    serial_printf("Began modal processing for dialog at %p (level %d)\n",
           (void*)theDialog, gModalState.modalLevel);

    return 0; /* noErr */
}

/*
 * EndModalDialog - End modal dialog processing
 */
void EndModalDialog(DialogPtr theDialog)
{
    if (!theDialog || !gModalState.initialized) {
        return;
    }

    /* Find dialog in modal stack and remove it */
    for (int i = 0; i < gModalState.modalLevel; i++) {
        if (gModalState.modalStack[i] == (WindowPtr)theDialog) {
            /* Shift remaining dialogs down */
            for (int j = i; j < gModalState.modalLevel - 1; j++) {
                gModalState.modalStack[j] = gModalState.modalStack[j + 1];
            }
            gModalState.modalStack[gModalState.modalLevel - 1] = NULL;
            gModalState.modalLevel--;
            break;
        }
    }

    /* Update front modal dialog */
    DialogManagerState* state = GetDialogManagerState();
    if (state) {
        if (gModalState.modalLevel > 0) {
            (state)->globals.frontModal = (DialogPtr)gModalState.modalStack[gModalState.modalLevel - 1];
        } else {
            (state)->globals.frontModal = NULL;
            /* Re-enable non-modal windows */
            EnableNonModalWindows();
        }
    }

    serial_printf("Ended modal processing for dialog at %p (level %d)\n",
           (void*)theDialog, gModalState.modalLevel);
}

/*
 * IsModalDialog - Check if a dialog is modal
 */
Boolean IsModalDialog(DialogPtr theDialog)
{
    if (!theDialog || !gModalState.initialized) {
        return false;
    }

    /* Check if dialog is in modal stack */
    for (int i = 0; i < gModalState.modalLevel; i++) {
        if (gModalState.modalStack[i] == (WindowPtr)theDialog) {
            return true;
        }
    }

    return false;
}

/*
 * GetFrontModalDialog - Get the front-most modal dialog
 */
DialogPtr GetFrontModalDialog(void)
{
    if (!gModalState.initialized || gModalState.modalLevel == 0) {
        return NULL;
    }

    return (DialogPtr)gModalState.modalStack[gModalState.modalLevel - 1];
}

/*
 * StandardModalFilter - Standard modal dialog filter
 */
Boolean StandardModalFilter(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!theDialog || !theEvent || !gModalState.initialized) {
        return false;
    }

    /* Handle standard modal keys */
    if (theEvent->what == kDialogEvent_KeyDown) {
        return ProcessStandardModalKeys(theDialog, theEvent, itemHit);
    }

    return false;
}

/*
 * InstallModalFilter - Install a modal filter procedure
 */
ModalFilterProcPtr InstallModalFilter(DialogPtr theDialog, ModalFilterProcPtr filterProc)
{
    if (!theDialog || !gModalState.initialized) {
        return NULL;
    }

    /* Find slot for this dialog */
    for (int i = 0; i < 16; i++) {
        if (gModalState.modalStack[i] == (WindowPtr)theDialog ||
            gModalState.installedFilters[i] == NULL) {
            ModalFilterProcPtr oldProc = gModalState.installedFilters[i];
            gModalState.installedFilters[i] = filterProc;
            gModalState.modalFiltersActive = (filterProc != NULL);
            return oldProc;
        }
    }

    return NULL;
}

/*
 * FlashButton - Flash a button briefly
 */
void FlashButton(DialogPtr theDialog, SInt16 itemNo)
{
    if (!theDialog || itemNo <= 0) {
        return;
    }

    FlashButtonInternal(theDialog, itemNo);
}

/*
 * HandleModalKeyboard - Handle keyboard input in modal dialog
 */
Boolean HandleModalKeyboard(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!theDialog || !theEvent || theEvent->what != kDialogEvent_KeyDown) {
        return false;
    }

    return ProcessStandardModalKeys(theDialog, theEvent, itemHit);
}

/*
 * HandleModalMouse - Handle mouse input in modal dialog
 */
Boolean HandleModalMouse(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!theDialog || !theEvent || theEvent->what != kDialogEvent_MouseDown) {
        return false;
    }

    /* Find which item was hit */
    SInt16 item = FindDialogItem(theDialog, theEvent->where);
    if (item > 0) {
        /* Flash the button if it's a button */
        SInt16 itemType;
        Handle itemHandle;
        Rect itemBox;
        GetDialogItem(theDialog, item, &itemType, &itemHandle, &itemBox);

        if ((itemType & itemTypeMask) == ctrlItem || (itemType & itemTypeMask) == btnCtrl) {
            FlashButtonInternal(theDialog, item);
        }

        if (itemHit) *itemHit = item;
        return true;
    }

    return false;
}

/*
 * BringModalToFront - Bring modal dialog to front
 */
void BringModalToFront(DialogPtr theDialog)
{
    if (!theDialog) {
        return;
    }

    /* In a full implementation, this would call SelectWindow or similar */
    serial_printf("Bringing modal dialog at %p to front\n", (void*)theDialog);
}

/*
 * DisableNonModalWindows - Disable non-modal windows
 */
void DisableNonModalWindows(void)
{
    /* In a full implementation, this would iterate through all windows
       and disable those that are not modal dialogs */
    serial_printf("Disabling non-modal windows\n");
}

/*
 * EnableNonModalWindows - Re-enable non-modal windows
 */
void EnableNonModalWindows(void)
{
    /* In a full implementation, this would re-enable previously disabled windows */
    serial_printf("Re-enabling non-modal windows\n");
}

/*
 * Private implementation functions
 */

Boolean ProcessModalEvent(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!theDialog || !theEvent) {
        return false;
    }

    Boolean handled = false;
    SInt16 hit = 0;

    switch (theEvent->what) {
        case kDialogEvent_MouseDown:
            handled = HandleModalMouse(theDialog, theEvent, &hit);
            break;

        case kDialogEvent_KeyDown:
        case kDialogEvent_AutoKey:
            handled = HandleModalKeyboard(theDialog, theEvent, &hit);
            break;

        case kDialogEvent_Update:
            HandleDialogUpdate(theDialog, theEvent);
            handled = true;
            break;

        case kDialogEvent_Activate:
            HandleDialogActivate(theDialog, theEvent, true);
            handled = true;
            break;

        default:
        {
            /* Let dialog events module handle other events */
            SInt16 result = ProcessDialogEvent(theDialog, theEvent, &hit);
            handled = (result != kDialogEventResult_NotHandled);
            break;
        }
    }

    if (itemHit) *itemHit = hit;
    return handled;
}

static Boolean CallModalFilter(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!gModalState.modalFiltersActive || !theDialog || !theEvent) {
        return false;
    }

    /* Find filter for this dialog */
    for (int i = 0; i < 16; i++) {
        if (gModalState.modalStack[i] == (WindowPtr)theDialog &&
            gModalState.installedFilters[i] != NULL) {
            return gModalState.installedFilters[i](theDialog, theEvent, itemHit);
        }
    }

    return false;
}

static void UpdateModalState(DialogPtr theDialog)
{
    if (!theDialog) {
        return;
    }

    /* Update dialog state as needed */
    /* This could include cursor tracking, idle processing, etc. */
}

static Boolean IsEventForDialog(DialogPtr theDialog, const EventRecord* theEvent)
{
    if (!theDialog || !theEvent) {
        return false;
    }

    /* In a full implementation, this would check if the event's window
       matches the dialog's window */
    /* For now, assume all events are for the current dialog */
    return true;
}

static void HandleModalTimeout(DialogPtr theDialog, SInt16* itemHit)
{
    if (!theDialog) {
        return;
    }

    /* Handle dialog timeout - typically activate default button */
    SInt16 defaultItem = GetDialogDefaultItem(theDialog);
    if (defaultItem > 0) {
        if (itemHit) *itemHit = defaultItem;
        FlashButtonInternal(theDialog, defaultItem);
    }

    serial_printf("Modal dialog timeout - activated default item %d\n", defaultItem);
}

static Boolean ProcessStandardModalKeys(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    if (!theDialog || !theEvent) {
        return false;
    }

    UInt8 keyCode = (theEvent->message & 0xFF);
    SInt16 modifiers = theEvent->modifiers;

    switch (keyCode) {
        case kDialogKey_Return:
        case kDialogKey_Enter:
            /* Activate default button */
            {
                SInt16 defaultItem = GetDialogDefaultItem(theDialog);
                if (defaultItem > 0) {
                    FlashButtonInternal(theDialog, defaultItem);
                    if (itemHit) *itemHit = defaultItem;
                    return true;
                }
            }
            break;

        case kDialogKey_Escape:
            /* Activate cancel button */
            {
                SInt16 cancelItem = GetDialogCancelItem(theDialog);
                if (cancelItem > 0) {
                    FlashButtonInternal(theDialog, cancelItem);
                    if (itemHit) *itemHit = cancelItem;
                    return true;
                }
            }
            break;

        case kDialogKey_Tab:
            /* Tab navigation */
            {
                Boolean backward = (modifiers & kDialogModifier_Shift) != 0;
                AdvanceDialogFocus(theDialog, backward);
                return true;
            }
            break;

        case '.':
            /* Command-. for cancel */
            if (modifiers & kDialogModifier_Command) {
                SInt16 cancelItem = GetDialogCancelItem(theDialog);
                if (cancelItem > 0) {
                    FlashButtonInternal(theDialog, cancelItem);
                    if (itemHit) *itemHit = cancelItem;
                    return true;
                }
            }
            break;
    }

    return false;
}

static void FlashButtonInternal(DialogPtr theDialog, SInt16 itemNo)
{
    if (!theDialog || itemNo <= 0) {
        return;
    }

    /* Flash the button by briefly highlighting it */
    serial_printf("Flashing button %d in dialog at %p\n", itemNo, (void*)theDialog);

    /* In a full implementation, this would:
       1. Invert the button
       2. Delay briefly
       3. Restore the button
    */
    InvalDialogItem(theDialog, itemNo);
    DrawDialogItem(theDialog, itemNo);

    /* Brief delay for visual feedback */
    UInt32 finalTicks;
    Delay(8, &finalTicks); /* About 1/8 second */

    InvalDialogItem(theDialog, itemNo);
    DrawDialogItem(theDialog, itemNo);
}

/*
 * Advanced modal dialog features
 */

void SetModalDialogTimeout(DialogPtr theDialog, UInt32 timeoutTicks, SInt16 defaultItem)
{
    /* This would be implemented with a timer system */
    serial_printf("Set modal dialog timeout: %u ticks, default item %d\n",
           timeoutTicks, defaultItem);
}

void ClearModalDialogTimeout(DialogPtr theDialog)
{
    serial_printf("Cleared modal dialog timeout\n");
}

void SetModalDialogDismissButton(DialogPtr theDialog, SInt16 itemNo)
{
    /* This would configure which button dismisses the dialog */
    serial_printf("Set dismiss button to item %d\n", itemNo);
}

SInt16 ShowNativeModal(const char* message, const char* title,
                       const char* buttons, SInt16 iconType)
{
    /* Platform-specific native modal dialog */
    serial_printf("Native modal: %s - %s (buttons: %s, icon: %d)\n",
           title, message, buttons, iconType);
    return 1; /* Default to OK */
}

void CleanupModalDialogs(void)
{
    if (!gModalState.initialized) {
        return;
    }

    /* Clean up any remaining modal state */
    gModalState.modalLevel = 0;
    gModalState.currentModal = NULL;
    gModalState.inModalLoop = false;

    serial_printf("Modal dialog subsystem cleaned up\n");
}
