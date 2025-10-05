/**
 * @file ControlSmoke.c
 * @brief Standard Controls smoke test
 *
 * Demonstrates buttons, checkboxes, and radio buttons in a test dialog.
 * Activated with CTRL_SMOKE_TEST=1 compile flag.
 *
 * Copyright (c) 2024 - System 7.1 Portable Toolbox Project
 * Licensed under MIT License
 */

#include "SystemTypes.h"
#include "ControlManager/ControlManager.h"
#include "ControlManager/ControlTypes.h"
#include "WindowManager/WindowManager.h"
#include "EventManager/EventManager.h"
#include "QuickDraw/QuickDraw.h"
#include "DialogManager/DialogManager.h"
#include "System71StdLib.h"

#ifdef CTRL_SMOKE_TEST

/* External functions */
extern void serial_printf(const char* fmt, ...);
extern void GlobalToLocal(Point* pt);

/* Test window and controls */
static WindowPtr gTestWindow = NULL;
static ControlHandle gOKButton = NULL;
static ControlHandle gCancelButton = NULL;
static ControlHandle gCheckbox = NULL;
static ControlHandle gRadio1 = NULL;
static ControlHandle gRadio2 = NULL;
static ControlHandle gRadio3 = NULL;

/**
 * Create smoke test window with controls
 */
void CreateControlSmokeWindow(void) {
    Rect bounds;
    Str255 title;

    if (gTestWindow) {
        return; /* Already created */
    }

    /* Create test window */
    bounds.left = 100;
    bounds.top = 100;
    bounds.right = 400;
    bounds.bottom = 300;

    title[0] = 18;
    memcpy(&title[1], "Control Smoke Test", 18);

    gTestWindow = NewWindow(NULL, &bounds, title, true, documentProc, (WindowPtr)-1, true, 0);
    if (!gTestWindow) {
        serial_printf("[CTRL SMOKE] Failed to create test window\n");
        return;
    }

    serial_printf("[CTRL SMOKE] Test window created at (%d,%d)-(%d,%d)\n",
                  bounds.left, bounds.top, bounds.right, bounds.bottom);

    /* Create OK button (default, varCode=1 for default) */
    bounds.left = 220;
    bounds.top = 250;
    bounds.right = 280;
    bounds.bottom = 270;

    title[0] = 2;
    memcpy(&title[1], "OK", 2);

    serial_printf("[CTRL SMOKE] About to create OK button with procID=%d\n", pushButProc);
    gOKButton = NewControl(gTestWindow, &bounds, title, true, 0, 0, 1, pushButProc, 1);
    serial_printf("[CTRL SMOKE] NewControl returned: %p\n", (void*)gOKButton);
    if (gOKButton) {
        serial_printf("[CTRL SMOKE] OK button created (default, varCode=1)\n");
    }

    /* Create Cancel button (varCode=2 for cancel) */
    bounds.left = 140;
    bounds.top = 250;
    bounds.right = 210;
    bounds.bottom = 270;

    title[0] = 6;
    memcpy(&title[1], "Cancel", 6);

    gCancelButton = NewControl(gTestWindow, &bounds, title, true, 0, 0, 1, pushButProc, 2);
    if (gCancelButton) {
        serial_printf("[CTRL SMOKE] Cancel button created\n");
    }

    /* Create checkbox */
    bounds.left = 20;
    bounds.top = 50;
    bounds.right = 200;
    bounds.bottom = 66;

    title[0] = 16;
    memcpy(&title[1], "Show hidden files", 16);

    gCheckbox = NewControl(gTestWindow, &bounds, title, true, 0, 0, 1, checkBoxProc, 3);
    if (gCheckbox) {
        serial_printf("[CTRL SMOKE] Checkbox created\n");
    }

    /* Create radio button group (View options) */
    bounds.left = 20;
    bounds.top = 90;
    bounds.right = 120;
    bounds.bottom = 106;

    title[0] = 5;
    memcpy(&title[1], "Icons", 5);

    gRadio1 = NewControl(gTestWindow, &bounds, title, true, 1, 0, 1, radioButProc, 4);
    if (gRadio1) {
        SetRadioGroup(gRadio1, 1); /* Group ID = 1 */
        serial_printf("[CTRL SMOKE] Radio 1 (Icons) created, group=1\n");
    }

    bounds.top = 115;
    bounds.bottom = 131;

    title[0] = 4;
    memcpy(&title[1], "List", 4);

    gRadio2 = NewControl(gTestWindow, &bounds, title, true, 0, 0, 1, radioButProc, 5);
    if (gRadio2) {
        SetRadioGroup(gRadio2, 1); /* Group ID = 1 */
        serial_printf("[CTRL SMOKE] Radio 2 (List) created, group=1\n");
    }

    bounds.top = 140;
    bounds.bottom = 156;

    title[0] = 7;
    memcpy(&title[1], "Columns", 7);

    gRadio3 = NewControl(gTestWindow, &bounds, title, true, 0, 0, 1, radioButProc, 6);
    if (gRadio3) {
        SetRadioGroup(gRadio3, 1); /* Group ID = 1 */
        serial_printf("[CTRL SMOKE] Radio 3 (Columns) created, group=1\n");
    }

    serial_printf("[CTRL SMOKE] All controls created successfully\n");

    /* Set initial keyboard focus to OK button */
    if (gOKButton) {
        extern void DM_SetKeyboardFocus(WindowPtr window, ControlHandle newFocus);
        DM_SetKeyboardFocus(gTestWindow, gOKButton);
        serial_printf("[CTRL SMOKE] Initial focus set to OK button\n");
    }

    serial_printf("[CTRL SMOKE] Try clicking buttons, checkbox, and radio buttons\n");
    serial_printf("[CTRL SMOKE] Keyboard: Tab/Shift+Tab=focus, Space=toggle, Return/Esc=activate\n");
}

/**
 * Handle click in smoke test window
 */
Boolean HandleControlSmokeClick(WindowPtr window, Point where) {
    ControlHandle control;
    SInt16 part;

    if (window != gTestWindow) {
        return false;
    }

    /* Convert to local coordinates */
    GlobalToLocal(&where);

    /* Find which control was clicked */
    part = FindControl(where, window, &control);
    if (part == 0 || !control) {
        serial_printf("[CTRL SMOKE] Click at (%d,%d) - no control hit\n", where.h, where.v);
        return true;
    }

    /* Track the control */
    part = TrackControl(control, where, NULL);
    if (part == 0) {
        serial_printf("[CTRL SMOKE] TrackControl returned 0 (mouse released outside)\n");
        return true;
    }

    /* Handle control action */
    if (control == gOKButton) {
        serial_printf("[CTRL SMOKE] OK button clicked (refCon=%d)\n",
                      (int)GetControlReference(gOKButton));
        serial_printf("[CTRL SMOKE] Checkbox value: %d\n", GetControlValue(gCheckbox));
        serial_printf("[CTRL SMOKE] Radio group values: R1=%d R2=%d R3=%d\n",
                      GetControlValue(gRadio1),
                      GetControlValue(gRadio2),
                      GetControlValue(gRadio3));
    } else if (control == gCancelButton) {
        serial_printf("[CTRL SMOKE] Cancel button clicked (refCon=%d)\n",
                      (int)GetControlReference(gCancelButton));
    } else if (control == gCheckbox) {
        SInt16 newVal = GetControlValue(gCheckbox) ? 0 : 1;
        SetControlValue(gCheckbox, newVal);
        serial_printf("[CTRL SMOKE] Checkbox toggled to %d\n", newVal);
    } else if (control == gRadio1 || control == gRadio2 || control == gRadio3) {
        SetControlValue(control, 1);
        serial_printf("[CTRL SMOKE] Radio button %d selected (refCon=%d)\n",
                      control == gRadio1 ? 1 : (control == gRadio2 ? 2 : 3),
                      (int)GetControlReference(control));
        serial_printf("[CTRL SMOKE] Radio group state: R1=%d R2=%d R3=%d\n",
                      GetControlValue(gRadio1),
                      GetControlValue(gRadio2),
                      GetControlValue(gRadio3));
    }

    return true;
}

/**
 * Handle keyboard event in smoke test window
 */
Boolean HandleControlSmokeKey(WindowPtr window, EventRecord* evt) {
    SInt16 itemHit;

    if (window != gTestWindow) {
        return false;
    }

    /* Use Dialog Manager keyboard handlers */
    if (DM_HandleDialogKey(window, evt, &itemHit)) {
        serial_printf("[CTRL SMOKE] Keyboard handled: itemHit=%d\n", itemHit);

        /* Check if OK or Cancel was activated */
        if (itemHit == 1) { /* OK button refCon */
            serial_printf("[CTRL SMOKE] OK activated via keyboard\n");
            serial_printf("[CTRL SMOKE] Final state: Checkbox=%d, Radios: R1=%d R2=%d R3=%d\n",
                          GetControlValue(gCheckbox),
                          GetControlValue(gRadio1),
                          GetControlValue(gRadio2),
                          GetControlValue(gRadio3));
        } else if (itemHit == 2) { /* Cancel button refCon */
            serial_printf("[CTRL SMOKE] Cancel activated via keyboard\n");
        }

        return true;
    }

    return false;
}

/**
 * Initialize Control smoke test
 */
void InitControlSmokeTest(void) {
    serial_printf("[CTRL SMOKE] Enabled (CTRL_SMOKE_TEST=1)\n");
    serial_printf("[CTRL SMOKE] Creating test window...\n");
    CreateControlSmokeWindow();
}

#else

/* Stubs when smoke test is disabled */
void CreateControlSmokeWindow(void) {}
Boolean HandleControlSmokeClick(WindowPtr window, Point where) {
    (void)window;
    (void)where;
    return false;
}
Boolean HandleControlSmokeKey(WindowPtr window, EventRecord* evt) {
    (void)window;
    (void)evt;
    return false;
}
void InitControlSmokeTest(void) {}

#endif /* CTRL_SMOKE_TEST */
