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
extern Handle NewHandleClear(Size byteCount);
extern void DisposeHandle(Handle h);
extern void HLock(Handle h);
extern void HUnlock(Handle h);
extern void CenterDialogOnScreen(DialogPtr dlg);
extern void InvalRect(const Rect* r);

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

/* Built-in fallback alert specifications (System 7-style layout) */
typedef struct {
    Rect bounds;      /* dialog frame in global coords */
    SInt16 defItem;   /* 1-based default button */
    SInt16 cancelItem;/* 1-based cancel button (0=none) */
    SInt16 icon;      /* 0=none, 1=stop, 2=note, 3=caution */
    SInt16 ditlId;    /* pseudo id for fallback DITL */
} BuiltInAlertSpec;

static const BuiltInAlertSpec kFallbackStop    = {{160, 180, 320, 460}, 1, 0, 1, 9001};
static const BuiltInAlertSpec kFallbackNote    = {{160, 180, 320, 460}, 1, 0, 2, 9002};
static const BuiltInAlertSpec kFallbackCaution = {{160, 180, 320, 460}, 1, 0, 3, 9003};
static const BuiltInAlertSpec kFallbackGeneric = {{160, 180, 320, 460}, 1, 2, 0, 9004};

/* Private function prototypes */
static SInt16 RunAlertDialog(SInt16 alertID, ModalFilterProcPtr filterProc, SInt16 alertType);
static DialogPtr CreateAlertDialogFromTemplate(const AlertTemplate* alertTemplate);
static void PlayAlertSoundForStage(SInt16 alertType, SInt16 stage);
static void PositionAlertDialog(DialogPtr alertDialog);
static void SubstituteParameters(unsigned char* text);
static void DrawAlertIcon(DialogPtr alertDialog, SInt16 iconType);
static OSErr BuildFallbackDLOG(const BuiltInAlertSpec* spec, DialogTemplate** outDLOG);
static OSErr BuildFallbackDITL(SInt16 pseudoId, SInt16 iconKind, Handle* outDITL);
static Boolean LoadAlertWithFallback(SInt16 alertID, SInt16 alertType,
                                     DialogTemplate** outDLOG, Handle* outDITL,
                                     SInt16* outDefItem, SInt16* outCancelItem,
                                     SInt16* outIconKind);

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

static OSErr BuildFallbackDLOG(const BuiltInAlertSpec* spec, DialogTemplate** outDLOG)
{
    DialogTemplate* t;

    if (!spec || !outDLOG) {
        return -50; /* paramErr */
    }

    t = (DialogTemplate*)malloc(sizeof(DialogTemplate));
    if (!t) {
        return -108; /* memFullErr */
    }

    memset(t, 0, sizeof(DialogTemplate));
    t->boundsRect = spec->bounds;  /* already global; DM will position/center */
    t->procID     = 1;             /* modal */
    t->visible    = false;
    t->goAwayFlag = false;
    t->refCon     = spec->icon;    /* Store icon kind in refCon for drawing */
    t->itemsID    = spec->ditlId;
    t->title[0]   = 5;             /* Pascal string length */
    memcpy(&t->title[1], "Alert", 5);

    *outDLOG = t;
    return noErr;
}

static OSErr BuildFallbackDITL(SInt16 pseudoId, SInt16 iconKind, Handle* outDITL)
{
    SInt16 n;
    unsigned char* p;
    Handle h;
    SInt16 i;
    static const unsigned char okText[] = "\002OK";
    static const unsigned char cancelText[] = "\006Cancel";
    static const unsigned char msgText[] = "\034Alert message will appear here.";

    if (!outDITL) {
        return -50; /* paramErr */
    }

    /* Determine item count: icon(1), text(2), OK(3), Cancel(4 if generic) */
    n = (pseudoId == 9004) ? 4 : 3;

    /* Build minimal DITL in binary format that ParseDITL can read */
    /* Format: count-1 (2 bytes), then for each item: placeholder(4), rect(8), type(1), data */
    h = NewHandleClear(1024);  /* Generous size for small DITL */
    if (!h) {
        return -108; /* memFullErr */
    }

    HLock(h);
    p = (unsigned char*)*h;

    /* Write count-1 */
    *p++ = 0;
    *p++ = (unsigned char)(n - 1);

    /* Item 1: Icon user item at left */
    /* Placeholder (4 bytes) */
    *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
    /* Bounds: top, left, bottom, right (each 2 bytes) */
    *p++ = 0; *p++ = 20;  /* top */
    *p++ = 0; *p++ = 20;  /* left */
    *p++ = 0; *p++ = 52;  /* bottom */
    *p++ = 0; *p++ = 52;  /* right */
    /* Type: userItem (0) + itemDisable (128) */
    *p++ = 0;  /* userItem */
    /* Data length */
    *p++ = 0;  /* No data for user item */

    /* Item 2: Static text */
    /* Placeholder */
    *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
    /* Bounds */
    *p++ = 0; *p++ = 20;  /* top */
    *p++ = 0; *p++ = 60;  /* left */
    *p++ = 0; *p++ = 96;  /* bottom */
    *p++ = 1; *p++ = 4;   /* right (260) */
    /* Type: statText (8) */
    *p++ = 8;
    /* Data: Pascal string */
    *p++ = msgText[0];  /* length */
    for (i = 1; i <= msgText[0]; i++) {
        *p++ = msgText[i];
    }

    /* Item 3: OK button */
    /* Placeholder */
    *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
    /* Bounds: bottom-right, 80×20 button */
    *p++ = 0; *p++ = 96;   /* top */
    *p++ = 0; *p++ = 180;  /* left (260-80) */
    *p++ = 0; *p++ = 116;  /* bottom */
    *p++ = 0; *p++ = 240;  /* right (260-20) */
    /* Type: ctrlItem + btnCtrl (4+0) */
    *p++ = 4;
    /* Data: Pascal string */
    *p++ = okText[0];
    for (i = 1; i <= okText[0]; i++) {
        *p++ = okText[i];
    }

    if (n == 4) {
        /* Item 4: Cancel button */
        /* Placeholder */
        *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
        /* Bounds */
        *p++ = 0; *p++ = 96;   /* top */
        *p++ = 0; *p++ = 100;  /* left (260-160) */
        *p++ = 0; *p++ = 116;  /* bottom */
        *p++ = 0; *p++ = 160;  /* right (260-100) */
        /* Type: ctrlItem + btnCtrl */
        *p++ = 4;
        /* Data: Pascal string */
        *p++ = cancelText[0];
        for (i = 1; i <= cancelText[0]; i++) {
            *p++ = cancelText[i];
        }
    }

    HUnlock(h);
    *outDITL = h;
    return noErr;
}

static Boolean LoadAlertWithFallback(SInt16 alertID, SInt16 alertType,
                                     DialogTemplate** outDLOG, Handle* outDITL,
                                     SInt16* outDefItem, SInt16* outCancelItem,
                                     SInt16* outIconKind)
{
    const BuiltInAlertSpec* spec;
    OSErr err;

    if (!outDLOG || !outDITL || !outDefItem || !outCancelItem || !outIconKind) {
        return false;
    }

    /* Map alert ID to fallback spec */
    /* Standard alert IDs: 128=generic, 129=stop, 130=note, 131=caution */
    if (alertID == 129) {
        spec = &kFallbackStop;
    } else if (alertID == 130) {
        spec = &kFallbackNote;
    } else if (alertID == 131) {
        spec = &kFallbackCaution;
    } else {
        spec = &kFallbackGeneric;
    }

    *outDefItem    = spec->defItem;
    *outCancelItem = spec->cancelItem;
    *outIconKind   = spec->icon;

    /* Build fallback DLOG and DITL */
    err = BuildFallbackDLOG(spec, outDLOG);
    if (err != noErr) {
        printf("Failed to build fallback DLOG: error %d\n", err);
        return false;
    }

    err = BuildFallbackDITL(spec->ditlId, spec->icon, outDITL);
    if (err != noErr) {
        printf("Failed to build fallback DITL: error %d\n", err);
        if (*outDLOG) {
            free(*outDLOG);
            *outDLOG = NULL;
        }
        return false;
    }

    printf("Using fallback alert template for ID %d (icon=%d, def=%d, cancel=%d)\n",
           alertID, *outIconKind, *outDefItem, *outCancelItem);

    return true;
}

static SInt16 RunAlertDialog(SInt16 alertID, ModalFilterProcPtr filterProc, SInt16 alertType)
{
    DialogPtr alertDialog = NULL;
    DialogTemplate* dlogTemplate = NULL;
    Handle ditlHandle = NULL;
    SInt16 itemHit = 1; /* Default to OK button */
    SInt16 defItem = 1, cancelItem = 0, iconKind = 0;
    static const unsigned char alertTitle[] = "\005Alert";

    if (!gAlertState.initialized) {
        printf("Error: Alert subsystem not initialized\n");
        return 1;
    }

    /* Play alert sound based on current stage */
    PlayAlertSoundForStage(alertType, gAlertState.alertStage);

    /* Load alert with fallback */
    if (!LoadAlertWithFallback(alertID, alertType, &dlogTemplate, &ditlHandle,
                               &defItem, &cancelItem, &iconKind)) {
        printf("Error: Failed to load/create alert %d\n", alertID);
        SysBeep(30);
        return 1;
    }

    /* Create the dialog window */
    alertDialog = NewDialog(NULL, &dlogTemplate->boundsRect, alertTitle,
                           false, /* Start invisible */
                           1,     /* Modal dialog proc */
                           (WindowPtr)-1, /* Behind all windows */
                           false, /* No close box */
                           iconKind,      /* Store icon kind in refCon */
                           ditlHandle);

    if (!alertDialog) {
        printf("Error: Failed to create alert dialog\n");
        if (dlogTemplate) free(dlogTemplate);
        if (ditlHandle) DisposeHandle(ditlHandle);
        return 1;
    }

    /* Set default and cancel items */
    if (defItem) SetDialogDefaultItem(alertDialog, defItem);
    if (cancelItem) SetDialogCancelItem(alertDialog, cancelItem);

    /* Center and show */
    CenterDialogOnScreen(alertDialog);
    ShowWindow((WindowPtr)alertDialog);

    /* Force initial update */
    InvalRect(&((GrafPtr)alertDialog)->portRect);

    /* Make dialog modal and run modal loop */
    BeginModalDialog(alertDialog);
    ModalDialog(filterProc, &itemHit);
    EndModalDialog(alertDialog);

    /* Dispose of the alert dialog */
    DisposeDialog(alertDialog);
    if (dlogTemplate) free(dlogTemplate);

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
