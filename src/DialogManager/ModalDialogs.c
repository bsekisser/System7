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
#include "DialogManager/DialogHelpers.h"
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
    DialogPtr dlg;
    EventRecord evt;
    SInt16 everyEvent = 0xFFFF;

    if (itemHit) *itemHit = 0;

    dlg = FrontDialog();
    if (!dlg) {
        serial_printf("ModalDialog: No dialog to process\n");
        return;
    }

    /* Draw once */
    UpdateDialog(dlg, ((WindowPtr)dlg)->port.clipRgn);

    serial_printf("ModalDialog: Starting modal loop for dialog %p\n", (void*)dlg);

    for (;;) {
        /* Wait for next event (use GetNextEvent if WaitNextEvent unavailable) */
        if (!GetNextEvent(everyEvent, &evt)) {
            SystemTask();
            continue;
        }

        /* Filter first (can swallow / modify events) */
        if (filterProc && (*filterProc)(dlg, &evt, itemHit)) {
            if (itemHit && *itemHit) {
                serial_printf("ModalDialog: Filter returned item %d\n", *itemHit);
                return;
            }
            continue;
        }

        /* Dialog hit-testing & dispatch */
        if (IsDialogEvent(&evt)) {
            DialogPtr hitDlg = NULL;
            SInt16 hitItem = 0;
            if (DialogSelect(&evt, &hitDlg, &hitItem)) {
                if (itemHit) *itemHit = hitItem;
                serial_printf("ModalDialog: DialogSelect returned item %d\n", hitItem);
                return;
            }
            continue;
        }

        /* Esc / Cmd-. as cancel, Return activates default */
        if (evt.what == keyDown || evt.what == autoKey) {
            unsigned char ch = (unsigned char)(evt.message & 0xFF);
            Boolean cmd = (evt.modifiers & cmdKey) != 0;

            if (cmd && ch == '.') {
                if (itemHit) *itemHit = GetDialogCancelItem(dlg);
                serial_printf("ModalDialog: Cmd-. -> cancel item %d\n", *itemHit);
                return;
            }
            if (ch == 0x1B /* ESC */) {
                if (itemHit) *itemHit = GetDialogCancelItem(dlg);
                serial_printf("ModalDialog: ESC -> cancel item %d\n", *itemHit);
                return;
            }
            if (ch == '\r' || ch == 0x03) {
                if (itemHit) *itemHit = GetDialogDefaultItem(dlg);
                serial_printf("ModalDialog: Return -> default item %d\n", *itemHit);
                return;
            }
            /* Tab focus between edit fields (optional for now) */
        }

        /* Mouse in dialog? Let DialogSelect process click tracking */
        if (evt.what == mouseDown || evt.what == mouseUp) {
            DialogPtr hitDlg = NULL;
            SInt16 hitItem = 0;
            if (DialogSelect(&evt, &hitDlg, &hitItem) && hitItem) {
                if (itemHit) *itemHit = hitItem;
                serial_printf("ModalDialog: Mouse hit item %d\n", hitItem);
                return;
            }
        }

        /* Idle update pipeline */
        if (evt.what == updateEvt) {
            BeginUpdate((WindowPtr)dlg);
            UpdateDialog(dlg, ((WindowPtr)dlg)->updateRgn);
            EndUpdate((WindowPtr)dlg);
        }
    }
}

/* IsDialogEvent and DialogSelect are now in DialogEvents.c */

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

/* ProcessModalEvent - DEPRECATED - now uses DialogSelect */
Boolean ProcessModalEvent(DialogPtr theDialog, EventRecord* theEvent, SInt16* itemHit)
{
    /* Forward to DialogSelect */
    return DialogSelect(theEvent, &theDialog, itemHit);
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
