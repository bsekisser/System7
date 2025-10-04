/*
 * AlertDialogs.c - Alert Dialog Implementation
 *
 * This module provides the alert dialog functionality faithful to
 * Mac System 7.1, including Alert, StopAlert, NoteAlert, and CautionAlert.
 */

#include <stdlib.h>
#include <string.h>
#define printf(...) serial_printf(__VA_ARGS__)

#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DialogManager/AlertDialogs.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/ModalDialogs.h"
#include "DialogManager/DialogResources.h"
#include "DialogManager/DialogItems.h"

/* External dependencies */
extern void SysBeep(SInt16 duration);
extern UInt32 TickCount(void);
extern void ShowWindow(WindowPtr window);

/* Global alert state */
static struct {
    Boolean initialized;
    SInt16 alertStage;
    unsigned char paramText[4][256];
    SInt16 alertSounds[4];  /* Stop, Note, Caution, (reserved) */
    SInt16 alertIcons[4];
    Boolean useNativeAlerts;
    SInt16 alertPosition;
} gAlertState = {0};

/* Private function prototypes */
static SInt16 RunAlertDialog(SInt16 alertID, ModalFilterProcPtr filterProc, SInt16 alertType);
static DialogPtr CreateAlertDialogFromTemplate(const AlertTemplate* alertTemplate);
static void PlayAlertSoundForStage(SInt16 alertType, SInt16 stage);
static void PositionAlertDialog(DialogPtr alertDialog);
static void SubstituteParameters(unsigned char* text);
static void DrawAlertIcon(DialogPtr alertDialog, SInt16 iconType);

/*
 * InitAlertDialogs - Initialize alert dialog subsystem
 */
void InitAlertDialogs(void)
{
    int i;

    if (gAlertState.initialized) {
        return;
    }

    memset(&gAlertState, 0, sizeof(gAlertState));
    gAlertState.initialized = true;
    gAlertState.alertStage = 0;
    gAlertState.useNativeAlerts = false;
    gAlertState.alertPosition = 0; /* Center on main screen */

    /* Clear parameter text */
    for (i = 0; i < 4; i++) {
        gAlertState.paramText[i][0] = 0;
    }

    /* Set default alert sounds (0 = system beep) */
    gAlertState.alertSounds[0] = 0; /* Stop */
    gAlertState.alertSounds[1] = 0; /* Note */
    gAlertState.alertSounds[2] = 0; /* Caution */
    gAlertState.alertSounds[3] = 0; /* Reserved */

    /* Set default alert icons */
    gAlertState.alertIcons[0] = 0; /* Stop icon */
    gAlertState.alertIcons[1] = 1; /* Note icon */
    gAlertState.alertIcons[2] = 2; /* Caution icon */
    gAlertState.alertIcons[3] = 0; /* Reserved */

    printf("Alert dialog subsystem initialized\n");
}

/*
 * Alert - Display a generic alert dialog
 */
SInt16 Alert(SInt16 alertID, ModalFilterProcPtr filterProc)
{
    return RunAlertDialog(alertID, filterProc, 0);
}

/*
 * StopAlert - Display a stop alert dialog
 */
SInt16 StopAlert(SInt16 alertID, ModalFilterProcPtr filterProc)
{
    return RunAlertDialog(alertID, filterProc, 0);
}

/*
 * NoteAlert - Display a note alert dialog
 */
SInt16 NoteAlert(SInt16 alertID, ModalFilterProcPtr filterProc)
{
    return RunAlertDialog(alertID, filterProc, 1);
}

/*
 * CautionAlert - Display a caution alert dialog
 */
SInt16 CautionAlert(SInt16 alertID, ModalFilterProcPtr filterProc)
{
    return RunAlertDialog(alertID, filterProc, 2);
}

/*
 * GetAlertStage - Get current alert stage
 */
SInt16 GetAlertStage(void)
{
    if (!gAlertState.initialized) {
        return 0;
    }
    return gAlertState.alertStage;
}

/*
 * ResetAlertStage - Reset alert stage to 0
 */
void ResetAlertStage(void)
{
    if (!gAlertState.initialized) {
        return;
    }
    gAlertState.alertStage = 0;
    printf("Alert stage reset to 0\n");
}

/*
 * SetAlertStage - Set current alert stage
 */
void SetAlertStage(SInt16 stage)
{
    if (!gAlertState.initialized) {
        return;
    }
    if (stage >= 0 && stage <= 3) {
        gAlertState.alertStage = stage;
        printf("Alert stage set to %d\n", stage);
    }
}

/*
 * GetParamText - Get current parameter text
 */
void GetParamText(SInt16 paramIndex, unsigned char* text)
{
    if (!gAlertState.initialized || !text || paramIndex < 0 || paramIndex > 3) {
        if (text) text[0] = 0;
        return;
    }

    memcpy(text, gAlertState.paramText[paramIndex],
           gAlertState.paramText[paramIndex][0] + 1);
}

/*
 * ClearParamText - Clear all parameter text
 */
void ClearParamText(void)
{
    int i;

    if (!gAlertState.initialized) {
        return;
    }

    for (i = 0; i < 4; i++) {
        gAlertState.paramText[i][0] = 0;
    }
}

/*
 * SetAlertSound - Set sound for alert type
 */
void SetAlertSound(SInt16 alertType, SInt16 soundID)
{
    if (!gAlertState.initialized || alertType < 0 || alertType > 3) {
        return;
    }
    gAlertState.alertSounds[alertType] = soundID;
}

/*
 * GetAlertSound - Get sound for alert type
 */
SInt16 GetAlertSound(SInt16 alertType)
{
    if (!gAlertState.initialized || alertType < 0 || alertType > 3) {
        return 0;
    }
    return gAlertState.alertSounds[alertType];
}

/*
 * SetAlertPosition - Set position for alert dialogs
 */
void SetAlertPosition(SInt16 position)
{
    if (!gAlertState.initialized) {
        return;
    }
    gAlertState.alertPosition = position;
}

/*
 * SetAlertIcon - Set custom icon for alert type
 */
void SetAlertIcon(SInt16 alertType, SInt16 iconID)
{
    if (!gAlertState.initialized || alertType < 0 || alertType > 3) {
        return;
    }
    gAlertState.alertIcons[alertType] = iconID;
}

/*
 * GetAlertIcon - Get icon for alert type
 */
SInt16 GetAlertIcon(SInt16 alertType)
{
    if (!gAlertState.initialized || alertType < 0 || alertType > 3) {
        return 0;
    }
    return gAlertState.alertIcons[alertType];
}

/*
 * SetUseNativeAlerts - Enable/disable native alert dialogs
 */
void SetUseNativeAlerts(Boolean useNative)
{
    if (!gAlertState.initialized) {
        return;
    }
    gAlertState.useNativeAlerts = useNative;
}

/*
 * GetUseNativeAlerts - Check if native alerts are enabled
 */
Boolean GetUseNativeAlerts(void)
{
    return gAlertState.initialized ? gAlertState.useNativeAlerts : false;
}

/*
 * CreateAlertFromTemplate - Create alert from template
 */
DialogPtr CreateAlertFromTemplate(SInt16 alertID)
{
    AlertTemplate* alertTemplate = NULL;
    DialogPtr alertDialog = NULL;
    OSErr err;

    if (!gAlertState.initialized) {
        printf("Error: Alert subsystem not initialized\n");
        return NULL;
    }

    /* Load alert template */
    err = LoadAlertTemplate(alertID, &alertTemplate);
    if (err != noErr || !alertTemplate) {
        printf("Error: Failed to load ALRT resource %d (error %d)\n", alertID, err);
        return NULL;
    }

    /* Create dialog from template */
    alertDialog = CreateAlertDialogFromTemplate(alertTemplate);

    /* Dispose template */
    DisposeAlertTemplate(alertTemplate);

    return alertDialog;
}

/*
 * RunAlert - Run an alert dialog
 */
SInt16 RunAlert(DialogPtr alertDialog, ModalFilterProcPtr filterProc)
{
    SInt16 itemHit = 0;

    if (!alertDialog) {
        return 1; /* Default to OK */
    }

    /* Make dialog modal */
    BeginModalDialog(alertDialog);

    /* Run modal dialog loop */
    ModalDialog(filterProc, &itemHit);

    /* End modal processing */
    EndModalDialog(alertDialog);

    return itemHit;
}

/*
 * PlayAlertSound - Play sound for alert
 */
void PlayAlertSound(SInt16 alertType, SInt16 stage)
{
    if (!gAlertState.initialized) {
        return;
    }

    PlayAlertSoundForStage(alertType, stage);
}

/*
 * CleanupAlertDialogs - Cleanup alert subsystem
 */
void CleanupAlertDialogs(void)
{
    if (!gAlertState.initialized) {
        return;
    }

    gAlertState.initialized = false;
    printf("Alert dialog subsystem cleaned up\n");
}

/*
 * Private implementation functions
 */

static SInt16 RunAlertDialog(SInt16 alertID, ModalFilterProcPtr filterProc, SInt16 alertType)
{
    AlertTemplate* alertTemplate = NULL;
    DialogPtr alertDialog = NULL;
    SInt16 itemHit = 1; /* Default to OK button */
    OSErr err;

    if (!gAlertState.initialized) {
        printf("Error: Alert subsystem not initialized\n");
        return 1;
    }

    /* Load alert template from resource */
    err = LoadAlertTemplate(alertID, &alertTemplate);
    if (err != noErr || !alertTemplate) {
        printf("Error: Failed to load ALRT resource %d (error %d)\n", alertID, err);
        /* Play system beep even if resource fails */
        SysBeep(30);
        return 1;
    }

    /* Play alert sound based on current stage */
    PlayAlertSoundForStage(alertType, gAlertState.alertStage);

    /* Create alert dialog from template */
    alertDialog = CreateAlertDialogFromTemplate(alertTemplate);
    if (!alertDialog) {
        printf("Error: Failed to create alert dialog from template\n");
        DisposeAlertTemplate(alertTemplate);
        return 1;
    }

    /* Position the alert dialog */
    PositionAlertDialog(alertDialog);

    /* Draw alert icon if needed */
    DrawAlertIcon(alertDialog, alertType);

    /* Make dialog visible */
    ShowWindow((WindowPtr)alertDialog);

    /* Make dialog modal and run modal loop */
    BeginModalDialog(alertDialog);
    ModalDialog(filterProc, &itemHit);
    EndModalDialog(alertDialog);

    /* Dispose of the alert dialog */
    DisposeDialog(alertDialog);
    DisposeAlertTemplate(alertTemplate);

    /* Advance alert stage for repeated alerts */
    if (gAlertState.alertStage < 3) {
        gAlertState.alertStage++;
    }

    printf("Alert %d completed, item hit: %d, new stage: %d\n",
           alertID, itemHit, gAlertState.alertStage);

    return itemHit;
}

static DialogPtr CreateAlertDialogFromTemplate(const AlertTemplate* alertTemplate)
{
    DialogPtr alertDialog = NULL;
    Handle itemList = NULL;
    OSErr err;
    static const unsigned char alertTitle[] = "\005Alert";  /* Pascal string: length byte + "Alert" */

    if (!alertTemplate) {
        return NULL;
    }

    /* Load item list for the alert */
    err = LoadDialogItemList(alertTemplate->itemsID, &itemList);
    if (err != noErr || !itemList) {
        printf("Error: Failed to load DITL resource %d for alert (error %d)\n",
               alertTemplate->itemsID, err);
        return NULL;
    }

    /* Create the dialog window */
    /* Alert dialogs are always modal (procID = 1) */
    alertDialog = NewDialog(NULL, &alertTemplate->boundsRect, alertTitle,
                           false, /* Start invisible */
                           1,     /* Modal dialog proc */
                           (WindowPtr)-1, /* Behind all windows */
                           false, /* No close box */
                           0,     /* refCon */
                           itemList);

    if (!alertDialog) {
        DisposeDialogItemList(itemList);
        return NULL;
    }

    return alertDialog;
}

static void PlayAlertSoundForStage(SInt16 alertType, SInt16 stage)
{
    /* In System 7, different stages can have different sounds */
    /* For now, just play system beep */
    SInt16 soundID = 0;

    if (alertType >= 0 && alertType <= 3) {
        soundID = gAlertState.alertSounds[alertType];
    }

    if (soundID == 0) {
        /* Play system beep */
        SysBeep(30);
    } else {
        /* Would play custom sound from resource */
        printf("Playing alert sound ID %d for stage %d\n", soundID, stage);
        SysBeep(30);
    }
}

static void PositionAlertDialog(DialogPtr alertDialog)
{
    if (!alertDialog) {
        return;
    }

    /* In System 7, alerts are typically centered on the main screen */
    /* The position can be specified in the ALRT resource:
     * 0x0000 = use bounds as-is
     * 0x300A = center on main screen
     * 0x700A = center on deepest screen
     * 0xB00A = center over frontmost window
     */

    /* For now, we'll just use the bounds from the template */
    /* A full implementation would adjust position based on positioning code */

    printf("Positioning alert dialog at %p\n", (void*)alertDialog);
}

static void SubstituteParameters(unsigned char* text)
{
    /* This would substitute ^0, ^1, ^2, ^3 with parameter text */
    /* For now, stub */
    if (!text) {
        return;
    }
}

static void DrawAlertIcon(DialogPtr alertDialog, SInt16 iconType)
{
    if (!alertDialog) {
        return;
    }

    /* In System 7, alerts have standard icons:
     * Stop = stop sign icon
     * Note = information icon
     * Caution = exclamation point icon
     */

    /* This would draw the appropriate icon in the alert dialog */
    /* Typically in the first item (or a specific user item) */

    printf("Drawing alert icon type %d in dialog %p\n", iconType, (void*)alertDialog);
}

/* Stub implementations for additional alert functions */

void SetAlertAccessibility(Boolean enabled)
{
    printf("SetAlertAccessibility: %d\n", enabled);
}

void AnnounceAlert(const char* title, const char* message)
{
    printf("AnnounceAlert: %s - %s\n", title ? title : "", message ? message : "");
}

SInt16 ShowNativeAlert(const char* title, const char* message,
                       SInt16 buttons, SInt16 alertType)
{
    printf("ShowNativeAlert: %s - %s (buttons: %d, type: %d)\n",
           title ? title : "", message ? message : "", buttons, alertType);
    return 1; /* OK button */
}

void SetAlertTheme(const DialogTheme* theme)
{
    printf("SetAlertTheme\n");
}

void GetAlertTheme(DialogTheme* theme)
{
    printf("GetAlertTheme\n");
}

SInt16 ShowAlert(const char* title, const char* message,
                 SInt16 buttons, SInt16 alertType)
{
    printf("ShowAlert: %s - %s (buttons: %d, type: %d)\n",
           title ? title : "", message ? message : "", buttons, alertType);
    return 1; /* OK button */
}

SInt16 ShowAlertWithParams(const char* title, const char* message,
                           SInt16 buttons, SInt16 alertType,
                           const char* param0, const char* param1,
                           const char* param2, const char* param3)
{
    printf("ShowAlertWithParams: %s - %s\n", title ? title : "", message ? message : "");
    return 1; /* OK button */
}

void ProcessAlertStages(SInt16 alertType, SInt16 stage)
{
    printf("ProcessAlertStages: type=%d, stage=%d\n", alertType, stage);
}

void SubstituteParamText(char* text, size_t textSize)
{
    if (!text) {
        return;
    }
    /* Stub - would substitute ^0, ^1, ^2, ^3 with parameter text */
}
