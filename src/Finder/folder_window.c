/*
 * folder_window.c - Folder window content display with double-click support
 *
 * Displays folder contents in windows opened from desktop icons.
 * Supports single-click selection and double-click to open items.
 */

#include "SystemTypes.h"
#include "WindowManager/WindowManager.h"
#include "QuickDraw/QuickDraw.h"
#include "Finder/Icon/icon_types.h"
#include "Finder/Icon/icon_label.h"
#include "Finder/Icon/icon_system.h"
#include <string.h>
#include <stdlib.h>

extern void serial_printf(const char* fmt, ...);
extern void DrawString(const unsigned char* str);
extern void MoveTo(short h, short v);
extern void LineTo(short h, short v);
extern void FrameRect(const Rect* r);
extern void DrawText(const void* textBuf, short firstByte, short byteCount);
extern void ClipRect(const Rect* r);
extern void EraseRect(const Rect* r);
extern void GlobalToLocal(Point* pt);
extern void GetPort(GrafPtr* port);
extern void SetPort(GrafPtr port);
extern UInt32 TickCount(void);
extern UInt32 GetDblTime(void);
extern OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
extern QDGlobals qd;

/* Folder item representation (simplified for now) */
typedef struct FolderItem {
    char name[256];
    Boolean isFolder;      /* true = folder, false = document/app */
    Point position;        /* position in window (for icon view) */
} FolderItem;

/* Folder window state (per window) */
typedef struct FolderWindowState {
    FolderItem* items;     /* Array of items in this folder */
    short itemCount;       /* Number of items */
    short selectedIndex;   /* Currently selected item (-1 = none) */
    Boolean isDragging;    /* Currently dragging (reserved for future) */
    UInt32 lastClickTime;  /* For double-click detection */
    Point lastClickPos;    /* Last click position */
    short lastClickIndex;  /* Index of last clicked item */
} FolderWindowState;

/* Global folder window states (indexed by window pointer for now) */
#define MAX_FOLDER_WINDOWS 16
static struct {
    WindowPtr window;
    FolderWindowState state;
} gFolderWindows[MAX_FOLDER_WINDOWS];

/* Get or create state for a folder window */
static FolderWindowState* GetFolderState(WindowPtr w);
static void InitializeFolderContents(WindowPtr w, Boolean isTrash);
static void GhostEraseIf(void);  /* Forward declaration for ghost system */

/* Draw a simple file/folder icon */
static void DrawFileIcon(short x, short y, Boolean isFolder)
{
    extern void serial_printf(const char* fmt, ...);

    Rect iconRect;
    SetRect(&iconRect, x, y, x + 32, y + 32);

    serial_printf("[ICON] res=%d at(l)={%d,%d} port=%p\n",
                  isFolder ? 1 : 2, x, y, NULL);

    if (isFolder) {
        /* Draw folder shape - paint with gray then frame */
        extern void PaintRect(const Rect* r);
        extern void InvertRect(const Rect* r);
        PaintRect(&iconRect);  /* Fill with black */
        FrameRect(&iconRect);
        /* Tab on top */
        Rect tabRect;
        SetRect(&tabRect, x, y - 4, x + 12, y);
        PaintRect(&tabRect);  /* Fill with black */
        FrameRect(&tabRect);
    } else {
        /* Draw document shape - just frame it (white background) */
        FrameRect(&iconRect);
        /* Folded corner */
        MoveTo(x + 24, y);
        LineTo(x + 32, y + 8);
        LineTo(x + 24, y + 8);
        LineTo(x + 24, y);
    }
}

/* Draw folder window contents - Content Only, No Chrome */
void DrawFolderWindowContents(WindowPtr window, Boolean isTrash)
{
    if (!window) return;

    serial_printf("Finder: Drawing contents of '%s'\n",
                  isTrash ? "Trash" : "Macintosh HD");

    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(window);

    /* CRITICAL: Log port and coordinate mapping for debugging */
    serial_printf("DrawFolder: window=%p savePort=%p\n", window, savePort);
    serial_printf("DrawFolder: portBits.bounds(GLOBAL)=(%d,%d,%d,%d)\n",
                  window->port.portBits.bounds.left, window->port.portBits.bounds.top,
                  window->port.portBits.bounds.right, window->port.portBits.bounds.bottom);

    /* Use window's portRect which is in LOCAL (port-relative) coordinates */
    /* In Mac Toolbox, portRect should always start at (0,0) */
    Rect localBounds = window->port.portRect;
    serial_printf("DrawFolder: portRect(LOCAL)=(%d,%d,%d,%d)\n",
                  localBounds.left, localBounds.top, localBounds.right, localBounds.bottom);

    /* Calculate content area in LOCAL coordinates */
    /* Content = full port minus title bar (20px) */
    Rect contentRect;
    contentRect.left = localBounds.left;
    contentRect.top = 20;  /* Skip title bar */
    contentRect.right = localBounds.right;
    contentRect.bottom = localBounds.bottom;

    serial_printf("Finder: portRect (local) = (%d,%d,%d,%d)\n",
                  localBounds.left, localBounds.top, localBounds.right, localBounds.bottom);

    serial_printf("Finder: contentRect (local) = (%d,%d,%d,%d)\n",
                  contentRect.left, contentRect.top, contentRect.right, contentRect.bottom);

    /* Set clipping to content area to prevent drawing outside bounds */
    ClipRect(&contentRect);

    /* Fill background with white - EraseRect uses LOCAL coords, DrawPrimitive converts to GLOBAL */
    extern void EraseRect(const Rect* r);
    EraseRect(&contentRect);

    serial_printf("Finder: Erased contentRect (%d,%d,%d,%d) for white backfill\n",
                  contentRect.left, contentRect.top, contentRect.right, contentRect.bottom);

    /* Text drawing disabled until Font Manager is linked */

    if (isTrash) {
        /* Draw trash contents */
        MoveTo(contentRect.left + 10, contentRect.top + 30);
        DrawText("Trash is empty", 0, 14);
        serial_printf("[TEXT] 'Trash is empty' at(l)={%d,%d} font=%d size=%d\n",
                      contentRect.left + 10, contentRect.top + 30, 0, 9);

        MoveTo(contentRect.left + 10, contentRect.top + 50);
        DrawText("Drag items here to delete them", 0, 30);
        serial_printf("[TEXT] 'Drag items here to delete them' at(l)={%d,%d} font=%d size=%d\n",
                      contentRect.left + 10, contentRect.top + 50, 0, 9);
    } else {
        /* Draw volume contents - sample items in icon grid */
        /* Ensure margins: 80px left (room for labels), 30px top */
        short x = contentRect.left + 80;
        short y = contentRect.top + 30;
        short iconSpacing = 100;
        short rowHeight = 90;

        /* System Folder - icon 32px wide, label ~78px wide */
        DrawFileIcon(x, y, true);
        MoveTo(x - 23, y + 40);
        DrawText("System Folder", 0, 13);
        serial_printf("[TEXT] 'System Folder' at(l)={%d,%d} font=%d size=%d\n", x - 23, y + 40, 0, 9);

        x += iconSpacing;

        /* Applications folder - label ~72px wide */
        DrawFileIcon(x, y, true);
        MoveTo(x - 20, y + 40);
        DrawText("Applications", 0, 12);
        serial_printf("[TEXT] 'Applications' at(l)={%d,%d} font=%d size=%d\n", x - 20, y + 40, 0, 9);

        x += iconSpacing;

        /* Documents folder - label ~54px wide */
        DrawFileIcon(x, y, true);
        MoveTo(x - 11, y + 40);
        DrawText("Documents", 0, 9);
        serial_printf("[TEXT] 'Documents' at(l)={%d,%d} font=%d size=%d\n", x - 11, y + 40, 0, 9);

        /* Second row */
        x = contentRect.left + 80;
        y += rowHeight;

        /* SimpleText document - label ~60px wide */
        DrawFileIcon(x, y, false);
        MoveTo(x - 14, y + 40);
        DrawText("ReadMe.txt", 0, 10);
        serial_printf("[TEXT] 'ReadMe.txt' at(l)={%d,%d} font=%d size=%d\n", x - 14, y + 40, 0, 9);

        x += iconSpacing;

        /* TeachText document - label ~84px wide */
        DrawFileIcon(x, y, false);
        MoveTo(x - 26, y + 40);
        DrawText("About System 7", 0, 14);
        serial_printf("[TEXT] 'About System 7' at(l)={%d,%d} font=%d size=%d\n", x - 26, y + 40, 0, 9);

        /* Show disk space at bottom */
        MoveTo(contentRect.left + 10, contentRect.bottom - 10);
        DrawText("5 items     42.3 MB in disk     193.7 MB available", 0, 52);
        serial_printf("[TEXT] 'disk info' at(l)={%d,%d} font=%d size=%d\n",
                      contentRect.left + 10, contentRect.bottom - 10, 0, 9);
    }

    SetPort(savePort);
}

/* Helper: Find folder window state slot */
static FolderWindowState* GetFolderState(WindowPtr w) {
    if (!w) return NULL;

    /* Search for existing slot */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == w) {
            return &gFolderWindows[i].state;
        }
    }

    /* Find empty slot */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == NULL) {
            gFolderWindows[i].window = w;
            gFolderWindows[i].state.items = NULL;
            gFolderWindows[i].state.itemCount = 0;
            gFolderWindows[i].state.selectedIndex = -1;
            gFolderWindows[i].state.isDragging = false;
            gFolderWindows[i].state.lastClickTime = 0;
            gFolderWindows[i].state.lastClickPos.h = 0;
            gFolderWindows[i].state.lastClickPos.v = 0;
            gFolderWindows[i].state.lastClickIndex = -1;

            /* Initialize folder contents */
            InitializeFolderContents(w, (w->refCon == 'TRSH'));
            return &gFolderWindows[i].state;
        }
    }

    return NULL;  /* No slots available */
}

/* Initialize folder contents (creates sample items for now) */
static void InitializeFolderContents(WindowPtr w, Boolean isTrash) {
    FolderWindowState* state = NULL;

    /* Find the state we just created */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == w) {
            state = &gFolderWindows[i].state;
            break;
        }
    }

    if (!state) return;

    if (isTrash) {
        /* Trash is empty for now */
        state->itemCount = 0;
        state->items = NULL;
    } else {
        /* Sample volume contents - matches what's drawn in DrawFolderWindowContents */
        state->itemCount = 5;
        state->items = (FolderItem*)malloc(sizeof(FolderItem) * 5);

        if (state->items) {
            strcpy(state->items[0].name, "System Folder");
            state->items[0].isFolder = true;
            state->items[0].position.h = 80;
            state->items[0].position.v = 50;  /* contentRect.top + 30 */

            strcpy(state->items[1].name, "Applications");
            state->items[1].isFolder = true;
            state->items[1].position.h = 180;
            state->items[1].position.v = 50;

            strcpy(state->items[2].name, "Documents");
            state->items[2].isFolder = true;
            state->items[2].position.h = 280;
            state->items[2].position.v = 50;

            strcpy(state->items[3].name, "ReadMe.txt");
            state->items[3].isFolder = false;
            state->items[3].position.h = 80;
            state->items[3].position.v = 140;  /* Second row */

            strcpy(state->items[4].name, "About System 7");
            state->items[4].isFolder = false;
            state->items[4].position.h = 180;
            state->items[4].position.v = 140;
        }
    }
}

/* Icon hit-testing: find icon at point (local window coordinates)
 * Returns index of item or -1 if no hit.
 * Icon rect is 32x32, label is below with 20px left/right padding.
 */
static short FW_IconAtPoint(WindowPtr w, Point localPt) {
    FolderWindowState* state = GetFolderState(w);
    if (!state || !state->items) return -1;

    serial_printf("FW: hit test at local (%d,%d), itemCount=%d\n",
                 localPt.h, localPt.v, state->itemCount);

    for (short i = 0; i < state->itemCount; i++) {
        /* Icon rect (32x32 centered at position.h+16) */
        Rect iconRect;
        iconRect.left = state->items[i].position.h;
        iconRect.top = state->items[i].position.v;
        iconRect.right = state->items[i].position.h + 32;
        iconRect.bottom = state->items[i].position.v + 32;

        /* Label rect (text centered, with padding) */
        int textWidth, textHeight;
        IconLabel_Measure(state->items[i].name, &textWidth, &textHeight);

        Rect labelRect;
        int centerX = state->items[i].position.h + 16;
        labelRect.left = centerX - (textWidth / 2) - 2;
        labelRect.top = state->items[i].position.v + 27;  /* label offset from icon_label.c */
        labelRect.right = centerX + (textWidth / 2) + 2;
        labelRect.bottom = labelRect.top + 15;  /* Chicago font height */

        /* Check if point is in icon or label */
        if ((localPt.h >= iconRect.left && localPt.h < iconRect.right &&
             localPt.v >= iconRect.top && localPt.v < iconRect.bottom) ||
            (localPt.h >= labelRect.left && localPt.h < labelRect.right &&
             localPt.v >= labelRect.top && localPt.v < labelRect.bottom)) {
            serial_printf("FW: hit index %d name='%s'\n", i, state->items[i].name);
            return i;
        }
    }

    return -1;
}

/* Handle click in folder window - called from EventDispatcher
 * Point is in GLOBAL coordinates, isDoubleClick from event system
 */
Boolean HandleFolderWindowClick(WindowPtr w, EventRecord *ev, Boolean isDoubleClick) {
    if (!w || !ev) return false;

    FolderWindowState* state = GetFolderState(w);
    if (!state) return false;

    /* Convert global mouse to local window coordinates */
    Point localPt = ev->where;
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort((GrafPtr)w);
    GlobalToLocal(&localPt);

    serial_printf("FW: down at (global %d,%d) local (%d,%d) dbl=%d\n",
                 ev->where.h, ev->where.v, localPt.h, localPt.v, isDoubleClick);

    /* Hit test against icons */
    short hitIndex = FW_IconAtPoint(w, localPt);

    if (hitIndex == -1) {
        /* Clicked empty space - deselect */
        if (state->selectedIndex != -1) {
            serial_printf("FW: deselect (empty click)\n");
            state->selectedIndex = -1;
            PostEvent(updateEvt, (UInt32)w);
        }
        state->lastClickIndex = -1;
        SetPort(savePort);
        return true;
    }

    /* Clicked an icon */
    UInt32 currentTime = TickCount();
    UInt32 timeSinceLast = currentTime - state->lastClickTime;
    Boolean isSameIcon = (hitIndex == state->lastClickIndex);
    Boolean withinDblTime = (timeSinceLast <= GetDblTime());
    Boolean isRealDoubleClick = isSameIcon && withinDblTime && isDoubleClick;

    serial_printf("FW: hit index %d, same=%d, dt=%d, threshold=%d, realDbl=%d\n",
                 hitIndex, isSameIcon, (int)timeSinceLast, (int)GetDblTime(), isRealDoubleClick);

    if (isRealDoubleClick) {
        /* DOUBLE-CLICK on same icon: open it */
        serial_printf("FW: OPEN_FOLDER \"%s\"\n", state->items[hitIndex].name);

        if (state->items[hitIndex].isFolder) {
            /* Open folder: create new window (minimal implementation) */
            extern WindowPtr Finder_OpenDesktopItem(Boolean isTrash, ConstStr255Param title);

            /* Convert C string to Pascal string for window title */
            Str255 pTitle;
            size_t len = strlen(state->items[hitIndex].name);
            if (len > 255) len = 255;
            pTitle[0] = (unsigned char)len;
            memcpy(&pTitle[1], state->items[hitIndex].name, len);

            /* For now, just open a generic folder window (not the actual subfolder) */
            WindowPtr newWin = Finder_OpenDesktopItem(false, pTitle);
            if (newWin) {
                serial_printf("FW: opened new folder window %p\n", newWin);
                /* Post update for the new window and old window */
                PostEvent(updateEvt, (UInt32)newWin);
                PostEvent(updateEvt, (UInt32)w);
            }
        } else {
            /* Document/app: show "not implemented" in serial log */
            serial_printf("FW: OPEN app/doc \"%s\" not implemented\n", state->items[hitIndex].name);
        }

        /* Clear double-click tracking */
        state->lastClickIndex = -1;
        state->lastClickTime = 0;
    } else {
        /* SINGLE-CLICK: select this icon */
        short oldSel = state->selectedIndex;
        state->selectedIndex = hitIndex;
        state->lastClickIndex = hitIndex;
        state->lastClickTime = currentTime;

        serial_printf("FW: select %d -> %d\n", oldSel, hitIndex);

        /* Post update to redraw selection */
        PostEvent(updateEvt, (UInt32)w);
    }

    SetPort(savePort);
    return true;
}

/* Safe folder window drawing with ghost integration
 * Called from EventDispatcher HandleUpdate when window is a folder window
 */
void FolderWindow_Draw(WindowPtr w) {
    static Boolean gInFolderPaint = false;

    serial_printf("=== FolderWindow_Draw ENTRY === window=%p\n", w);

    if (!w) {
        serial_printf("FolderWindow_Draw: NULL window, returning\n");
        return;
    }

    serial_printf("FolderWindow_Draw: refCon=0x%08lx\n", w->refCon);

    /* Re-entrancy guard */
    if (gInFolderPaint) {
        serial_printf("FolderWindow_Draw: re-entry detected, skipping\n");
        return;
    }
    gInFolderPaint = true;

    /* Erase any ghost outline before drawing */
    GhostEraseIf();

    FolderWindowState* state = GetFolderState(w);
    Boolean isTrash = (w->refCon == 'TRSH');

    GrafPtr savePort;
    GetPort(&savePort);
    SetPort((GrafPtr)w);  /* NOTE: w is already GrafPtr, not &w */

    serial_printf("FW: updateEvt for window %p, portRect=(%d,%d,%d,%d), portBits.bounds=(%d,%d,%d,%d)\n",
                  w,
                  w->port.portRect.top, w->port.portRect.left,
                  w->port.portRect.bottom, w->port.portRect.right,
                  w->port.portBits.bounds.top, w->port.portBits.bounds.left,
                  w->port.portBits.bounds.bottom, w->port.portBits.bounds.right);

    /* Draw white background (contentRect only, skip title bar) */
    Rect contentRect = w->port.portRect;
    contentRect.top = 20;  /* Skip title bar */
    EraseRect(&contentRect);
    serial_printf("FW: Erased content area\n");

    /* If we have state, draw icons with selection highlighting */
    if (state && state->items) {
        /* Convert window port coordinates to global screen coordinates for icon drawing */
        Point globalOrigin = {.v = w->port.portBits.bounds.top, .h = w->port.portBits.bounds.left};

        serial_printf("FW: Drawing %d icons, portBounds=(%d,%d)\n",
                     state->itemCount, globalOrigin.h, globalOrigin.v);

        for (short i = 0; i < state->itemCount; i++) {
            Boolean selected = (i == state->selectedIndex);
            IconHandle iconHandle;

            if (state->items[i].isFolder) {
                iconHandle.fam = IconSys_DefaultFolder();
            } else {
                iconHandle.fam = IconSys_DefaultDoc();
            }
            iconHandle.selected = selected;

            /* Convert local position to global screen position
             * position.v is already in port-local coordinates (measured from port top)
             * globalOrigin.v is the global screen Y of the port top
             * So simple addition gives us the global coordinate
             */
            int globalX = state->items[i].position.h + globalOrigin.h;
            int globalY = state->items[i].position.v + globalOrigin.v;

            serial_printf("FW: Icon %d '%s' local=(%d,%d) global=(%d,%d)\n",
                         i, state->items[i].name,
                         state->items[i].position.h, state->items[i].position.v,
                         globalX, globalY);

            /* Draw icon with label - Icon_DrawWithLabel draws to framebuffer in global coords */
            Icon_DrawWithLabel(&iconHandle, state->items[i].name,
                              globalX + 16,  /* center X (global) */
                              globalY,       /* top Y (global) */
                              selected);
        }
    }

    SetPort(savePort);
    gInFolderPaint = false;
}

/* Check if window is a folder window (by refCon) */
Boolean IsFolderWindow(WindowPtr w) {
    if (!w) return false;
    return (w->refCon == 'DISK' || w->refCon == 'TRSH');
}

/* Update window proc for folder windows */
void FolderWindowProc(WindowPtr window, short message, long param)
{
    switch (message) {
        case 0:  /* wDraw = 0, draw content only */
            /* Draw window contents - NO CHROME! */
            serial_printf("Finder: FolderWindowProc drawing content\n");
            FolderWindow_Draw(window);
            break;

        case 1:  /* wHit = 1, handle click in content */
            /* Handle click in content */
            serial_printf("Click in folder window at (%d,%d)\n",
                         (short)(param >> 16), (short)(param & 0xFFFF));
            break;

        default:
            break;
    }
}

/*
 * GhostEraseIf stub - desktop_manager.c has the real implementation
 * This is needed because FolderWindow_Draw calls it to be safe
 */
static void GhostEraseIf(void) {
    /* Call the real ghost eraser from desktop_manager.c */
    extern void Desktop_GhostEraseIf(void);
    Desktop_GhostEraseIf();
}

/*
 * CleanupFolderWindow - Clean up state when a folder window is closed
 * This prevents crashes when the window is closed and then File > Close is used
 */
void CleanupFolderWindow(WindowPtr w) {
    if (!w) return;

    serial_printf("CleanupFolderWindow: cleaning up window %p\n", w);

    /* Find and clear this window's state */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == w) {
            serial_printf("CleanupFolderWindow: found slot %d, freeing items\n", i);

            /* Free the items array if allocated */
            if (gFolderWindows[i].state.items) {
                free(gFolderWindows[i].state.items);
            }

            /* Clear the slot */
            gFolderWindows[i].window = NULL;
            gFolderWindows[i].state.items = NULL;
            gFolderWindows[i].state.itemCount = 0;
            gFolderWindows[i].state.selectedIndex = -1;

            serial_printf("CleanupFolderWindow: slot %d cleared\n", i);
            return;
        }
    }

    serial_printf("CleanupFolderWindow: window %p not found in state table\n", w);
}