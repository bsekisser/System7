/*
 * MacPaint_EventLoop.c - Main Event Loop and Window Management
 *
 * Implements the main application event loop for MacPaint:
 * - WaitNextEvent-based event dispatching
 * - Window creation and management
 * - Mouse event tracking and tool routing
 * - Keyboard event handling
 * - Menu command processing
 * - Window update and redraw
 *
 * Integrates with System 7.1 EventManager, WindowManager, MenuManager
 */

#include "SystemTypes.h"
#include "Apps/MacPaint.h"
#include "WindowManager/WindowManager.h"
#include "MenuManager/MenuManager.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include <string.h>

/*
 * APPLICATION STATE FOR EVENT LOOP
 */

typedef struct {
    int running;                /* Application should continue running */
    int mouseDown;              /* Mouse button currently pressed */
    int lastMouseX, lastMouseY; /* Last mouse position for drag tracking */
    int dragInProgress;         /* Currently dragging (mouse down + moved) */
    WindowPtr paintWindow;      /* Main paint window */
    int windowNeedsRedraw;      /* Window should be redrawn */
    int updateRect;             /* Rect that needs updating */
} EventLoopState;

static EventLoopState gEventState = {1, 0, 0, 0, 0, 0, 0};

/* External state from MacPaint modules */
extern WindowPtr gPaintWindow;
extern int gCurrentTool;
extern int gDocDirty;

/*
 * WINDOW MANAGEMENT FOR EVENT LOOP
 */

/**
 * MacPaint_CreateMainWindow - Create the main paint canvas window
 */
OSErr MacPaint_CreateMainWindow(void)
{
    /* TODO: Use WindowManager to create window
     * - Create a standard document window
     * - Size: approximately 640×800 pixels
     * - Title: Use MacPaint_GetWindowTitle() for title with dirty indicator
     * - Attributes: document window with close box, zoom box
     * - Visible: start hidden, show after setup
     *
     * Pseudocode:
     * Rect bounds = {50, 50, 650, 850};
     * CreateNewWindow(kDocumentWindowClass, windowAttrs, &bounds, &window);
     * SetWindowTitleWithCFString(window, title);
     * SetWindowDefaultButton(window, okButton);
     * ShowWindow(window);
     */

    gEventState.paintWindow = gPaintWindow;
    gEventState.windowNeedsRedraw = 1;
    return noErr;
}

/**
 * MacPaint_DrawPaintWindow - Render paint buffer to window
 */
void MacPaint_DrawPaintWindow(void)
{
    if (!gEventState.paintWindow) {
        return;
    }

    /* TODO: Render paint buffer to window
     * - Get current GrafPort for window
     * - Set drawing parameters (mode, pen size)
     * - Use CopyBits or similar to draw bitmap
     * - Draw selection rectangle if active
     * - Draw grid if enabled
     * - Draw toolbox if visible
     */
}

/**
 * MacPaint_HandleWindowUpdate - Process window update event
 */
void MacPaint_HandleWindowUpdate(WindowPtr window)
{
    if (window == gEventState.paintWindow) {
        /* TODO: Update the paint window
         * BeginUpdate(window);
         * MacPaint_DrawPaintWindow();
         * EndUpdate(window);
         */
    }
}

/**
 * MacPaint_HandleWindowClose - Handle window close button click
 */
void MacPaint_HandleWindowClose(WindowPtr window)
{
    if (window == gEventState.paintWindow) {
        /* Check if document needs saving */
        int promptResult = MacPaint_PromptSaveChanges();
        if (promptResult == 1) {
            /* User wants to save */
            MacPaint_FileSave();
        } else if (promptResult == 2) {
            /* User cancelled */
            return;
        }

        /* Close the window */
        gEventState.paintWindow = NULL;
        gEventState.running = 0;
    }
}

/*
 * MOUSE EVENT HANDLING
 */

/**
 * MacPaint_HandleMouseDownEvent - Process mouse button press (event dispatcher)
 */
void MacPaint_HandleMouseDownEvent(int x, int y, int modifiers)
{
    gEventState.mouseDown = 1;
    gEventState.lastMouseX = x;
    gEventState.lastMouseY = y;
    gEventState.dragInProgress = 0;

    /* Route to tool handler */
    MacPaint_HandleToolMouseEvent(gCurrentTool, x, y, 1);

    /* Save undo state before drawing */
    MacPaint_SaveUndoState("Drawing");

    MacPaint_InvalidateWindowArea();
}

/**
 * MacPaint_HandleMouseDragEvent - Process mouse movement during drag (event dispatcher)
 */
void MacPaint_HandleMouseDragEvent(int x, int y)
{
    if (!gEventState.mouseDown) {
        return;
    }

    /* Detect if movement is significant (drag threshold) */
    int dx = (x > gEventState.lastMouseX) ?
        (x - gEventState.lastMouseX) : (gEventState.lastMouseX - x);
    int dy = (y > gEventState.lastMouseY) ?
        (y - gEventState.lastMouseY) : (gEventState.lastMouseY - y);

    if (!gEventState.dragInProgress && (dx > 2 || dy > 2)) {
        gEventState.dragInProgress = 1;
    }

    if (gEventState.dragInProgress) {
        gEventState.lastMouseX = x;
        gEventState.lastMouseY = y;

        /* Route to tool handler for continuous drawing */
        MacPaint_HandleToolMouseEvent(gCurrentTool, x, y, 1);

        MacPaint_InvalidateWindowArea();
    }
}

/**
 * MacPaint_HandleMouseUpEvent - Process mouse button release (event dispatcher)
 */
void MacPaint_HandleMouseUpEvent(int x, int y)
{
    if (!gEventState.mouseDown) {
        return;
    }

    gEventState.mouseDown = 0;

    /* Route to tool handler to finalize drawing */
    MacPaint_HandleToolMouseEvent(gCurrentTool, x, y, 0);

    gEventState.dragInProgress = 0;
    MacPaint_InvalidateWindowArea();
}

/*
 * KEYBOARD EVENT HANDLING
 */

/**
 * MacPaint_HandleKeyDownEvent - Process keyboard key press (event dispatcher)
 */
void MacPaint_HandleKeyDownEvent(int keyCode, int modifiers)
{
    /* Route to menu/tool keyboard handlers from MacPaint_Menus.c */
    MacPaint_HandleKeyDown(keyCode, modifiers);

    MacPaint_InvalidateWindowArea();
}

/*
 * MENU EVENT HANDLING
 */

/**
 * MacPaint_HandleMenuClickEvent - Process menu selection (event dispatcher)
 */
void MacPaint_HandleMenuClickEvent(int menuID, int itemID)
{
    if (menuID == 0) {
        return;  /* Menu was closed without selection */
    }

    /* TODO: Use MenuManager to get menu title if needed
     * MenuHandle menu = GetMenuHandle(menuID);
     * Str255 itemName;
     * GetMenuItemText(menu, itemID, itemName);
     */

    /* Execute the menu command */
    MacPaint_ExecuteMenuCommand(menuID, itemID);

    MacPaint_InvalidateWindowArea();
}

/*
 * EVENT ROUTING
 */

/**
 * MacPaint_RouteEventToWindow - Determine if event is for paint window
 */
int MacPaint_IsEventInPaintWindow(int x, int y)
{
    if (!gEventState.paintWindow) {
        return 0;
    }

    /* TODO: Use WindowManager to check if point is in window
     * Rect windowBounds;
     * GetWindowBounds(gEventState.paintWindow, kWindowContentRgn, &windowBounds);
     * return (x >= windowBounds.left && x <= windowBounds.right &&
     *         y >= windowBounds.top && y <= windowBounds.bottom);
     */

    return 1;  /* Placeholder - assume click is in paint area */
}

/*
 * MAIN EVENT LOOP
 */

/**
 * MacPaint_RunEventLoop - Main application event loop
 * Processes events until application should quit
 */
void MacPaint_RunEventLoop(void)
{
    gEventState.running = 1;

    /* TODO: Full WaitNextEvent-based event loop
     *
     * Pseudocode:
     * while (gEventState.running) {
     *     EventRecord event;
     *     event.what = nullEvent;
     *
     *     // Wait for next event (max 30 ticks per iteration)
     *     WaitNextEvent(everyEvent, &event, 30, NULL);
     *
     *     switch (event.what) {
     *
     *         case mouseDown:
     *             // Check what window/region click is in
     *             if (click in paint window):
     *                 Convert to local coordinates
     *                 MacPaint_HandleMouseDown(localX, localY, event.modifiers)
     *             else if (click in menu bar):
     *                 MenuHandle menu = MenuSelect(event.where);
     *                 itemID = LoWord(menu);
     *                 menuID = HiWord(menu);
     *                 MacPaint_HandleMenuClick(menuID, itemID)
     *             break;
     *
     *         case mouseMoved:
     *             // Only track if mouse is down
     *             if (gEventState.mouseDown):
     *                 Convert to local window coordinates
     *                 MacPaint_HandleMouseDrag(localX, localY)
     *             break;
     *
     *         case mouseUp:
     *             // Note: Some systems generate mouseUp automatically
     *             MacPaint_HandleMouseUp(event.where.h, event.where.v)
     *             break;
     *
     *         case keyDown:
     *         case autoKey:
     *             keyCode = (event.message & keyCodeMask) >> 8;
     *             MacPaint_HandleKeyDown(keyCode, event.modifiers)
     *             break;
     *
     *         case updateEvt:
     *             // Window needs redraw
     *             MacPaint_HandleWindowUpdate((WindowPtr)event.message)
     *             break;
     *
     *         case osEvt:
     *             // Suspend/resume or mouse moved events
     *             if ((event.message >> 24) == suspendResumeMessage):
     *                 if (event.message & resumeFlag):
     *                     // App is being resumed
     *                 else:
     *                     // App is being suspended
     *             break;
     *
     *         case kEventWindowClose:  // Carbon API event
     *         case closeEvt:           // Classic Event Manager
     *             MacPaint_HandleWindowClose((WindowPtr)event.message)
     *             break;
     *
     *         case quitEvent:
     *             gEventState.running = 0
     *             break;
     *     }
     *
     *     // Update menu items based on current state
     *     MacPaint_AdjustMenus()
     *
     *     // Process any pending redraws
     *     if (gEventState.windowNeedsRedraw):
     *         MacPaint_HandleWindowUpdate(gEventState.paintWindow)
     *         gEventState.windowNeedsRedraw = 0
     * }
     */

    /* Placeholder: minimal event loop for testing */
    while (gEventState.running) {
        /* TODO: Actual WaitNextEvent loop implementation */

        /* Simulate one event cycle for now */
        MacPaint_InvalidateWindow();

        /* Exit after one iteration for safety during development */
        break;
    }
}

/*
 * WINDOW REDRAW AND INVALIDATION
 */

/**
 * MacPaint_InvalidateWindowArea - Mark window area for redraw
 */
void MacPaint_InvalidateWindowArea(void)
{
    gEventState.windowNeedsRedraw = 1;

    if (gEventState.paintWindow) {
        /* TODO: Use WindowManager to invalidate window
         * Rect windowBounds;
         * GetWindowBounds(gEventState.paintWindow, kWindowContentRgn, &windowBounds);
         * InvalWindowRect(gEventState.paintWindow, &windowBounds);
         */
    }
}

/**
 * MacPaint_InvalidateRectArea - Mark specific rectangle for redraw
 */
void MacPaint_InvalidateRectArea(const Rect *rect)
{
    if (!gEventState.paintWindow || !rect) {
        return;
    }

    /* TODO: Use WindowManager to invalidate specific rect
     * InvalWindowRect(gEventState.paintWindow, rect);
     */

    gEventState.windowNeedsRedraw = 1;
}

/*
 * EVENT LOOP STATE QUERIES
 */

/**
 * MacPaint_IsMouseDown - Check if mouse button is currently pressed
 */
int MacPaint_IsMouseDown(void)
{
    return gEventState.mouseDown;
}

/**
 * MacPaint_GetLastMousePosition - Get last recorded mouse position
 */
void MacPaint_GetLastMousePosition(int *x, int *y)
{
    if (x) *x = gEventState.lastMouseX;
    if (y) *y = gEventState.lastMouseY;
}

/**
 * MacPaint_ShouldQuit - Check if application should exit
 */
int MacPaint_ShouldQuit(void)
{
    return !gEventState.running;
}

/**
 * MacPaint_RequestQuit - Signal application to quit
 */
void MacPaint_RequestQuit(void)
{
    gEventState.running = 0;
}

/*
 * SAVE PROMPTS
 */

/**
 * MacPaint_PromptSaveChanges - Ask user if they want to save
 * Returns: 0 = don't save, 1 = save, 2 = cancel
 */
int MacPaint_PromptSaveChanges(void)
{
    if (!MacPaint_IsDocumentDirty()) {
        return 0;  /* No changes, don't need to save */
    }

    /* TODO: Show alert dialog
     * AlertStdAlertParamRec params;
     * DialogItemIndex itemHit;
     * OSErr err;
     *
     * params.movable = true;
     * params.helpButton = false;
     * params.filterProc = nil;
     * params.position = kWindowCenterOnMainScreen;
     *
     * err = StandardAlert(kAlertCautionAlert,
     *                     CFSTR("Save changes to document?"),
     *                     NULL, &params, &itemHit);
     *
     * switch (itemHit) {
     *     case 1: return 1;  // Save
     *     case 2: return 0;  // Don't Save
     *     case 3: return 2;  // Cancel
     * }
     */

    return 1;  /* Placeholder: always save */
}

/*
 * TOOL AND DRAWING STATE MANAGEMENT
 */

/**
 * MacPaint_SetActiveTool - Change current tool
 */
void MacPaint_SetActiveTool(int toolID)
{
    MacPaint_SelectTool(toolID);
    MacPaint_InvalidateWindow();
}

/**
 * MacPaint_GetActiveTool - Get current tool
 */
int MacPaint_GetActiveTool(void)
{
    return gCurrentTool;
}

/*
 * IDLE TIME PROCESSING
 */

/**
 * MacPaint_ProcessIdleTime - Do background work when no events pending
 * Called periodically during event loop idle time
 */
void MacPaint_ProcessIdleTime(void)
{
    /* TODO: Background tasks
     * - Update tool cursor based on position
     * - Animate tooltips or help text
     * - Check for file system changes
     * - Perform memory cleanup
     */
}

/*
 * CLIPBOARD AND DRAG/DROP INTEGRATION
 */

/**
 * MacPaint_PasteFromSystemClipboard - Get data from Mac clipboard
 */
int MacPaint_PasteFromSystemClipboard(void)
{
    /* TODO: Use Scrap Manager
     * Handle h = NewHandle(0);
     * GetScrap(h, 'PICT', &offset);
     * if (h):
     *     MacPaint_PasteFromClipboard(0, 0)
     *     DisposeHandle(h)
     *     return 1
     * return 0
     */

    return 0;  /* Placeholder */
}

/**
 * MacPaint_CopyToSystemClipboard - Put data on Mac clipboard
 */
int MacPaint_CopyToSystemClipboard(void)
{
    /* TODO: Use Scrap Manager
     * if (MacPaint_IsSelectionActive()):
     *     ZeroScrap()
     *     // Copy selection bitmap to PICT on scrap
     *     return 1
     * return 0
     */

    return 0;  /* Placeholder */
}

/*
 * ERROR RECOVERY
 */

/**
 * MacPaint_HandleError - Display error alert and continue
 */
void MacPaint_HandleError(OSErr err, const char *context)
{
    if (err == noErr) {
        return;
    }

    /* TODO: Show error alert
     * char errorMsg[256];
     * snprintf(errorMsg, sizeof(errorMsg),
     *          "Error %d in %s", err, context);
     *
     * AlertStdAlertParamRec params = {};
     * DialogItemIndex itemHit;
     * StandardAlert(kAlertStopAlert,
     *              CFSTR(errorMsg),
     *              NULL, &params, &itemHit);
     */
}

/*
 * SHUTDOWN COORDINATION
 */

/**
 * MacPaint_PrepareForShutdown - Final preparations before exit
 */
void MacPaint_PrepareForShutdown(void)
{
    /* Check if unsaved changes */
    if (MacPaint_IsDocumentDirty()) {
        int savePrompt = MacPaint_PromptSaveChanges();
        if (savePrompt == 1) {
            MacPaint_FileSave();
        }
        /* If savePrompt == 2 (cancel), user would cancel quit,
           but for now we proceed with shutdown */
    }

    /* Close any open dialogs */
    if (MacPaint_IsPatternEditorOpen()) {
        MacPaint_ClosePatternEditorDialog();
    }
    if (MacPaint_IsBrushEditorOpen()) {
        MacPaint_CloseBrushEditorDialog();
    }

    /* Close main window */
    if (gEventState.paintWindow) {
        MacPaint_CloseWindow();
    }
}
