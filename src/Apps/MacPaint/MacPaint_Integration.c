/*
 * MacPaint_Integration.c - System 7.1 Integration Layer
 *
 * Integrates MacPaint with System 7.1 managers:
 * - MenuManager: Load and manage application menus
 * - StandardFile: File open/save dialogs
 * - PrintManager: Print document support
 * - WindowManager: Window and document management
 * - DialogManager: Pattern/brush editor dialogs
 * - EventManager: Main event loop
 *
 * This layer bridges the portable MacPaint code with System 7.1 APIs
 */

#include "SystemTypes.h"
#include "Apps/MacPaint.h"
#include "MenuManager/MenuManager.h"
#include "WindowManager/WindowManager.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include <string.h>

/*
 * WINDOW AND DOCUMENT MANAGEMENT
 */

typedef struct {
    WindowPtr window;           /* Main paint window */
    GrafPtr port;              /* Drawing port */
    Rect paintRect;            /* Canvas area within window */
    int windowOpen;            /* Window currently displayed */
    char documentName[256];    /* Full path or title */
} DocumentWindow;

static DocumentWindow gDocWindow = {0};

/**
 * MacPaint_CreateWindow - Create main paint window
 */
OSErr MacPaint_CreateWindow(void)
{
    if (gDocWindow.windowOpen) {
        return noErr;  /* Already open */
    }

    /* TODO: Use WindowManager to create window
     * - Title from document name with dirty indicator
     * - Size: 600x800 typical
     * - Controls: scroll bars, zoom
     * - Position: centered on screen
     */

    gDocWindow.windowOpen = 1;
    return noErr;
}

/**
 * MacPaint_GetWindowPtr - Get current window pointer
 */
WindowPtr MacPaint_GetWindowPtr(void)
{
    return gDocWindow.window;
}

/**
 * MacPaint_GetPaintRect - Get canvas drawing rectangle
 */
void MacPaint_GetPaintRect(Rect *rect)
{
    if (rect) {
        *rect = gDocWindow.paintRect;
    }
}

/**
 * MacPaint_InvalidateWindow - Mark window for redraw
 */
void MacPaint_InvalidateWindow(void)
{
    if (gDocWindow.window) {
        /* TODO: Use WindowManager to invalidate/redraw
         * InvalRect(&gDocWindow.paintRect);
         */
    }
}

/**
 * MacPaint_CloseWindow - Close paint window
 */
void MacPaint_CloseWindow(void)
{
    if (gDocWindow.window) {
        /* TODO: Dispose window
         * DisposeWindow(gDocWindow.window);
         */
        gDocWindow.window = NULL;
        gDocWindow.windowOpen = 0;
    }
}

/**
 * MacPaint_UpdateWindowTitle - Update window title with document name
 */
void MacPaint_UpdateWindowTitle(void)
{
    if (!gDocWindow.window) {
        return;
    }

    const char *title = MacPaint_GetWindowTitle();
    /* TODO: Use WindowManager to set window title
     * SetWTitle(gDocWindow.window, (ConstStr255Param)title);
     */
}

/*
 * MENU MANAGER INTEGRATION
 */

typedef struct {
    MenuHandle fileMenu;       /* MENU 129 */
    MenuHandle editMenu;       /* MENU 130 */
    MenuHandle fontMenu;       /* MENU 131 */
    MenuHandle styleMenu;      /* MENU 132 */
    MenuHandle aidsMenu;       /* MENU 133 */
    int menuBarInitialized;
} MenuBarState;

static MenuBarState gMenuBar = {0};

/**
 * MacPaint_InitializeMenuBar - Load and install menus from resources
 */
OSErr MacPaint_InitializeMenuBar(void)
{
    /* TODO: Use MenuManager to load menus from resources
     * GetMenu(129) -> gMenuBar.fileMenu
     * GetMenu(130) -> gMenuBar.editMenu
     * GetMenu(131) -> gMenuBar.fontMenu
     * GetMenu(132) -> gMenuBar.styleMenu
     * GetMenu(133) -> gMenuBar.aidsMenu
     * InsertMenu(menu, 0) for each
     * DrawMenuBar()
     */

    /* Initialize menus locally if no resources */
    MacPaint_InitializeMenus();
    gMenuBar.menuBarInitialized = 1;

    return noErr;
}

/**
 * MacPaint_HandleMenuSelection - Process MenuManager menu selection
 */
void MacPaint_HandleMenuSelection(int menuID, int itemID)
{
    /* Dispatch to menu handler from MacPaint_Menus.c */
    MacPaint_HandleMenuCommand(menuID, itemID);
}

/**
 * MacPaint_AdjustMenus - Enable/disable menu items based on state
 */
void MacPaint_AdjustMenus(void)
{
    MacPaint_UpdateMenus();

    /* TODO: Update MenuManager menu items
     * For each menu item:
     * - Enable/disable based on MacPaint_IsMenuItemAvailable
     * - Set/clear checkmarks for toggles
     * - Update item text for dynamic items
     */
}

/*
 * STANDARD FILE DIALOGS
 */

/**
 * MacPaint_DoOpenDialog - Show Standard File open dialog
 * Returns filename in path, or empty string if cancelled
 */
int MacPaint_DoOpenDialog(char *path, int pathLen)
{
    if (!path || pathLen <= 0) {
        return 0;
    }

    /* TODO: Use StandardFile package to show open dialog
     * StandardGetFile(&filterProc, -1, NULL, &reply);
     * if (!reply.good) return 0;  // Cancelled
     * Convert FSSpec to pathname
     * Return 1 on success, 0 on cancel
     */

    return 0;  /* Placeholder */
}

/**
 * MacPaint_DoSaveDialog - Show Standard File save dialog
 * Returns filename in path, or empty string if cancelled
 */
int MacPaint_DoSaveDialog(char *path, int pathLen)
{
    if (!path || pathLen <= 0) {
        return 0;
    }

    /* TODO: Use StandardFile package to show save dialog
     * StandardPutFile("Save Picture As:", filename, &reply);
     * if (!reply.good) return 0;  // Cancelled
     * Convert FSSpec to pathname
     * Return 1 on success, 0 on cancel
     */

    return 0;  /* Placeholder */
}

/*
 * PRINT MANAGER INTEGRATION
 */

/**
 * MacPaint_DoPrintDialog - Show print dialog and get settings
 */
int MacPaint_DoPrintDialog(void)
{
    /* TODO: Use PrintManager to show print dialog
     * PrOpen() - open printer driver
     * PrJobDialog(...) - show job settings
     * if cancelled: return 0
     * Store print record for printing
     */

    return 0;  /* Placeholder */
}

/**
 * MacPaint_PrintDocument - Print current document
 */
OSErr MacPaint_PrintDocument(void)
{
    /* TODO: Print the paint buffer
     * PrOpenDoc() - start document
     * PrOpenPage() - start page
     * Render bitmap to printer port
     * PrClosePage() - end page
     * PrCloseDoc() - end document
     * PrClose() - close printer
     */

    return noErr;  /* Placeholder */
}

/*
 * DIALOG FRAMEWORK FOR EDITORS
 */

typedef struct {
    DialogPtr dialog;           /* Dialog window pointer */
    int itemHit;               /* Last item clicked */
    int isModal;               /* Modal vs modeless dialog */
} DialogState;

static DialogState gPatternDlg = {0};
static DialogState gBrushDlg = {0};

/**
 * MacPaint_CreatePatternEditorDialog - Create pattern editor window
 */
OSErr MacPaint_CreatePatternEditorDialog(void)
{
    if (gPatternDlg.dialog) {
        return noErr;  /* Already open */
    }

    /* TODO: Use DialogManager to create modeless dialog
     * GetNewDialog(500, NULL, (WindowPtr)-1)  // Resource ID 500
     * Set up 8x8 grid of buttons/checkboxes for pixels
     * Add OK/Cancel/Reset buttons
     * ShowWindow(dialog)
     */

    MacPaint_OpenPatternEditor();
    return noErr;
}

/**
 * MacPaint_PatternEditorEventHandler - Handle dialog events
 */
int MacPaint_PatternEditorEventHandler(int itemHit)
{
    if (!gPatternDlg.dialog) {
        return 0;
    }

    switch (itemHit) {
        case 1: /* OK button */
            /* TODO: Save pattern and close */
            MacPaint_ClosePatternEditorDialog();
            return 1;

        case 2: /* Cancel button */
            MacPaint_ClosePatternEditorDialog();
            return 1;

        case 3: /* Reset button */
            /* TODO: Restore original pattern */
            return 1;

        default:
            if (itemHit >= 4 && itemHit <= 67) {
                /* Pixel grid items (64 pixels in 8x8 grid) */
                int x = (itemHit - 4) % 8;
                int y = (itemHit - 4) / 8;
                MacPaint_PatternEditorPixelClick(x, y);
                /* TODO: Redraw pattern preview */
                return 0;  /* Continue dialog */
            }
            break;
    }

    return 0;
}

/**
 * MacPaint_ClosePatternEditorDialog - Close pattern editor
 */
void MacPaint_ClosePatternEditorDialog(void)
{
    if (gPatternDlg.dialog) {
        /* TODO: Use DialogManager to dispose dialog
         * DisposeDialog(gPatternDlg.dialog);
         */
        gPatternDlg.dialog = NULL;
    }

    MacPaint_ClosePatternEditor();
}

/**
 * MacPaint_CreateBrushEditorDialog - Create brush editor window
 */
OSErr MacPaint_CreateBrushEditorDialog(void)
{
    if (gBrushDlg.dialog) {
        return noErr;  /* Already open */
    }

    /* TODO: Use DialogManager to create modeless dialog
     * GetNewDialog(501, NULL, (WindowPtr)-1)  // Resource ID 501
     * Set up brush shape radio buttons
     * Add size slider or spinner control
     * Add OK/Cancel buttons
     * ShowWindow(dialog)
     */

    MacPaint_OpenBrushEditor();
    return noErr;
}

/**
 * MacPaint_BrushEditorEventHandler - Handle dialog events
 */
int MacPaint_BrushEditorEventHandler(int itemHit)
{
    if (!gBrushDlg.dialog) {
        return 0;
    }

    switch (itemHit) {
        case 1: /* OK button */
            MacPaint_CloseBrushEditorDialog();
            return 1;

        case 2: /* Cancel button */
            MacPaint_CloseBrushEditorDialog();
            return 1;

        default:
            /* TODO: Handle brush shape selection, size adjustment */
            return 0;
    }

    return 0;
}

/**
 * MacPaint_CloseBrushEditorDialog - Close brush editor
 */
void MacPaint_CloseBrushEditorDialog(void)
{
    if (gBrushDlg.dialog) {
        /* TODO: Use DialogManager to dispose dialog
         * DisposeDialog(gBrushDlg.dialog);
         */
        gBrushDlg.dialog = NULL;
    }

    MacPaint_CloseBrushEditor();
}

/*
 * EVENT LOOP INTEGRATION
 */

/**
 * MacPaint_RunEventLoop - Main application event loop
 * Should be called from MacPaintMain or system event manager
 */
void MacPaint_RunEventLoop(void)
{
    /* TODO: Implement main event loop
     * loop {
     *   WaitNextEvent(everyEvent, &event, 30, NULL);
     *   if event.what == mouseDown:
     *     // Determine if click in paint window, menu bar, dialog, etc
     *     if click in paint canvas:
     *       MacPaint_HandleMouseDown(x, y, modifiers)
     *   else if event.what == mouseUp:
     *     MacPaint_HandleMouseUp(x, y)
     *   else if event.what == mouseMoved:
     *     MacPaint_HandleMouseDrag(x, y)
     *   else if event.what == keyDown:
     *     MacPaint_HandleKeyDown(keyCode, modifiers)
     *   else if event.what == updateEvt:
     *     MacPaint_InvalidateWindow()
     *   else if pattern editor open:
     *     DialogSelect(&event, &dialog, &itemHit)
     *     MacPaint_PatternEditorEventHandler(itemHit)
     *   else if brush editor open:
     *     DialogSelect(&event, &dialog, &itemHit)
     *     MacPaint_BrushEditorEventHandler(itemHit)
     * }
     */
}

/*
 * DRAG AND DROP SUPPORT
 */

/**
 * MacPaint_CanAcceptDraggedFile - Check if file can be dropped on window
 */
int MacPaint_CanAcceptDraggedFile(const char *filename)
{
    /* TODO: Check if filename is valid MacPaint format
     * - Check extension (.paint, .pict, .bmp, etc)
     * - Verify file header if it's a MacPaint file
     * - Return 1 if acceptable, 0 otherwise
     */

    return 0;  /* Placeholder */
}

/**
 * MacPaint_HandleDroppedFile - Process dropped file
 */
OSErr MacPaint_HandleDroppedFile(const char *filename)
{
    /* TODO: Open dropped file
     * Check if current document needs saving
     * Load file using MacPaint_OpenDocument
     */

    return noErr;  /* Placeholder */
}

/*
 * COMMAND ROUTING AND DISPATCH
 */

/**
 * MacPaint_ExecuteMenuCommand - Execute menu command via system integration
 * This wraps the lower-level menu handlers with system cleanup
 */
OSErr MacPaint_ExecuteMenuCommand(int menuID, int itemID)
{
    /* Call the menu command handler */
    MacPaint_HandleMenuCommand(menuID, itemID);

    /* Update window title if document was modified */
    MacPaint_UpdateWindowTitle();

    /* Adjust menus for new state */
    MacPaint_AdjustMenus();

    /* Redraw window if needed */
    if (gDocWindow.windowOpen) {
        MacPaint_InvalidateWindow();
    }

    return noErr;
}

/*
 * RESOURCE LOADING
 */

/**
 * MacPaint_LoadApplicationResources - Load all application resources
 */
OSErr MacPaint_LoadApplicationResources(void)
{
    OSErr err;

    /* TODO: Load resources from application file
     * - Menus (MENU resources)
     * - Dialogs (DLOG resources)
     * - Icons (ICON resources)
     * - Patterns (PAT# resources)
     * - Cursors (CURS resources)
     * - Strings (STR resources)
     */

    return noErr;
}

/**
 * MacPaint_ReleaseApplicationResources - Unload resources
 */
void MacPaint_ReleaseApplicationResources(void)
{
    /* TODO: Release loaded resources
     * - Close resource file
     * - Flush resource cache if needed
     */
}

/*
 * INITIALIZATION AND CLEANUP
 */

/**
 * MacPaint_InitializeSystem - Set up System 7.1 integration
 */
OSErr MacPaint_InitializeSystem(void)
{
    OSErr err;

    /* Initialize window system */
    err = MacPaint_CreateWindow();
    if (err != noErr) {
        return err;
    }

    /* Load application resources */
    err = MacPaint_LoadApplicationResources();
    if (err != noErr) {
        return err;
    }

    /* Initialize menu bar */
    err = MacPaint_InitializeMenuBar();
    if (err != noErr) {
        return err;
    }

    /* Initial menu adjustment */
    MacPaint_AdjustMenus();

    return noErr;
}

/**
 * MacPaint_ShutdownSystem - Clean up System 7.1 integration
 */
void MacPaint_ShutdownSystem(void)
{
    /* Close any open dialogs */
    MacPaint_ClosePatternEditorDialog();
    MacPaint_CloseBrushEditorDialog();

    /* Close main window */
    MacPaint_CloseWindow();

    /* Release resources */
    MacPaint_ReleaseApplicationResources();
}

/*
 * STATE QUERIES FOR UI
 */

/**
 * MacPaint_IsWindowOpen - Check if main window is visible
 */
int MacPaint_IsWindowOpen(void)
{
    return gDocWindow.windowOpen;
}

/**
 * MacPaint_GetMenuBarState - Get menu bar initialization status
 */
int MacPaint_GetMenuBarState(void)
{
    return gMenuBar.menuBarInitialized;
}

/*
 * ACCESSORS FOR SYSTEM STATE
 */

/**
 * MacPaint_SetDocumentName - Update document name (for title bar, etc)
 */
void MacPaint_SetDocumentName(const char *name)
{
    if (name) {
        strncpy(gDocWindow.documentName, name, sizeof(gDocWindow.documentName) - 1);
        gDocWindow.documentName[sizeof(gDocWindow.documentName) - 1] = '\0';
        MacPaint_UpdateWindowTitle();
    }
}

/**
 * MacPaint_GetDocumentName - Get current document name
 */
const char* MacPaint_GetDocumentName(void)
{
    return gDocWindow.documentName;
}
