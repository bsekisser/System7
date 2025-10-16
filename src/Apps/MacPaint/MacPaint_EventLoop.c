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
#include "EventManager/EventManager.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include <string.h>

/*
 * EVENT TYPE CONSTANTS (System 7.1 compatible)
 * Local constants for event types - use local names to avoid conflicts
 */

enum {
    kEventMouseDown   = 1,
    kEventMouseUp     = 2,
    kEventKeyDown     = 3,
    kEventAutoKey     = 5,
    kEventUpdateEvt   = 6,
    kEventOSEvt       = 15,
    kEventNullEvent   = 0,
    kEventQuitEvent   = 0xFFFFFFF,
    kEventCloseEvt    = 8
};

/* MenuResult type for menu selection return */
typedef UInt32 MenuResult;

/* Suspend/resume messages */
#define kSuspendResumeMessage 0x01
#define kResumeFlag 1

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
        /* Use the rendering system's full window update
         * This handles all invalidation regions properly
         */
        MacPaint_FullWindowUpdate();

        /* Clear invalidation state after redraw */
        MacPaint_ClearInvalidState();
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

/**
 * MacPaint_HandleWindowResize - Handle window resize or bounds change
 * Called when the window is resized to recalculate invalidation regions
 */
void MacPaint_HandleWindowResize(WindowPtr window)
{
    if (window == gEventState.paintWindow) {
        /* Invalidate entire window to force complete redraw at new size
         * This call automatically recalculates invalidation region bounds
         * based on the new window dimensions
         */
        MacPaint_InvalidateWindowArea();
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

    /* Update cursor for current position */
    MacPaint_UpdateCursorPosition(x, y);

    /* Save undo state before drawing */
    MacPaint_SaveUndoState("Drawing");

    /* Route to tool handler */
    MacPaint_HandleToolMouseEvent(gCurrentTool, x, y, 1);

    /* Invalidate only paint area for efficiency */
    MacPaint_InvalidatePaintArea();
}

/**
 * MacPaint_HandleMouseDragEvent - Process mouse movement during drag (event dispatcher)
 */
void MacPaint_HandleMouseDragEvent(int x, int y)
{
    if (!gEventState.mouseDown) {
        return;
    }

    /* Update cursor position for any position changes */
    MacPaint_UpdateCursorPosition(x, y);

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

        MacPaint_InvalidatePaintArea();
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
    gEventState.dragInProgress = 0;

    /* Update cursor for current position */
    MacPaint_UpdateCursorPosition(x, y);

    /* Route to tool handler to finalize drawing */
    MacPaint_HandleToolMouseEvent(gCurrentTool, x, y, 0);

    /* Invalidate only paint area for efficiency */
    MacPaint_InvalidatePaintArea();
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

    /* Keyboard events may affect tool selection or other UI state
     * Invalidate appropriate regions based on what changed
     */
    MacPaint_InvalidatePaintArea();
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

    /* Execute the menu command */
    MacPaint_ExecuteMenuCommand(menuID, itemID);

    /* Menu commands may affect entire UI
     * Invalidate all regions to ensure consistency
     */
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
 * Processes events until application should quit using WaitNextEvent
 */
void MacPaint_RunEventLoop(void)
{
    EventRecord event;
    WindowPtr eventWindow;
    int localX, localY;
    MenuResult menuResult;
    int menuID, itemID;
    int keyCode;

    gEventState.running = 1;

    while (gEventState.running) {
        /* Wait for next event (max 30 ticks / ~500ms per iteration for responsiveness) */
        event.what = nullEvent;

        if (!WaitNextEvent(everyEvent, &event, 30, NULL)) {
            /* No event received - handle idle time processing */
            MacPaint_ProcessIdleTime();
            continue;
        }

        /* Process different event types */
        switch (event.what) {

            case kEventMouseDown:
                /* Determine which window/region the click is in */
                eventWindow = FrontWindow();

                /* Check for menu bar click */
                if (event.where.v < 20) {
                    /* Click is in menu bar region */
                    menuResult = MenuSelect(event.where);
                    menuID = HiWord(menuResult);
                    itemID = LoWord(menuResult);
                    if (menuID != 0) {
                        MacPaint_HandleMenuClickEvent(menuID, itemID);
                    }
                } else if (eventWindow == gEventState.paintWindow) {
                    /* Convert global to local window coordinates */
                    GrafPtr port = GetWindowPort(eventWindow);
                    if (port) {
                        SetPort(port);
                        localX = event.where.h - port->portRect.left;
                        localY = event.where.v - port->portRect.top;
                        MacPaint_HandleMouseDownEvent(localX, localY, event.modifiers);
                    }
                }
                break;

            case kEventMouseUp:
                /* Mouse button released */
                eventWindow = FrontWindow();
                if (eventWindow == gEventState.paintWindow) {
                    GrafPtr port = GetWindowPort(eventWindow);
                    if (port) {
                        SetPort(port);
                        localX = event.where.h - port->portRect.left;
                        localY = event.where.v - port->portRect.top;
                        MacPaint_HandleMouseUpEvent(localX, localY);
                    }
                }
                break;

            case kEventKeyDown:
            case kEventAutoKey:
                /* Extract key code from event message
                 * Format: bits 8-15 contain the key code
                 */
                keyCode = (event.message >> 8) & 0xFF;
                MacPaint_HandleKeyDownEvent(keyCode, event.modifiers);
                break;

            case kEventUpdateEvt:
                /* Window needs redraw */
                eventWindow = (WindowPtr)event.message;
                MacPaint_HandleWindowUpdate(eventWindow);
                break;

            case kEventOSEvt:
                /* Operating system event (suspend/resume) */
                /* Bit 24 indicates event type */
                if ((event.message >> 24) == kSuspendResumeMessage) {
                    if (event.message & kResumeFlag) {
                        /* Application is being resumed */
                        MacPaint_InvalidateWindowArea();
                    } else {
                        /* Application is being suspended */
                        /* Could save state or pause animations here */
                    }
                }
                break;

            case kEventCloseEvt:
                /* Window close event */
                eventWindow = (WindowPtr)event.message;
                if (eventWindow == gEventState.paintWindow) {
                    MacPaint_HandleWindowClose(eventWindow);
                }
                break;

            case kEventQuitEvent:
                /* Quit event from system */
                gEventState.running = 0;
                break;

            case kEventNullEvent:
                /* No event, but we got control back from WaitNextEvent */
                MacPaint_ProcessIdleTime();
                break;

            default:
                /* Unknown event type - just process idle time */
                MacPaint_ProcessIdleTime();
                break;
        }

        /* Update menu items to reflect current application state */
        MacPaint_AdjustMenus();
    }
}

/*
 * WINDOW REDRAW AND INVALIDATION
 */

/**
 * MacPaint_InvalidateRectArea - Mark specific rectangle for redraw
 */
void MacPaint_InvalidateRectArea(const Rect *rect)
{
    if (!gPaintWindow || !rect) {
        return;
    }

    /* Delegate to rendering module's invalidation system */
    InvalRect(rect);
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
    MacPaint_InvalidateToolArea();
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
    /* Update animated elements (marching ants, blinking cursors, etc) */
    MacPaint_UpdateAnimations();

    /* Update cursor position dynamically if mouse is over window
     * This allows cursor to change based on hover position
     */
    if (!gEventState.mouseDown) {
        int x, y;
        MacPaint_GetLastMousePosition(&x, &y);
        MacPaint_UpdateCursorPosition(x, y);
    }

    /* TODO: Additional background tasks
     * - Check for file system changes
     * - Perform memory cleanup
     * - Update status information
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
