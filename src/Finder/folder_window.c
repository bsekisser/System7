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
#include "Finder/finder.h"
#include "FS/vfs.h"
#include "FS/hfs_types.h"
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
extern void GetMouse(Point* pt);
extern volatile UInt8 gCurrentButtons;
extern short FindWindow(Point thePoint, WindowPtr* theWindow);
extern bool VFS_Delete(VRefNum vref, FileID id);

/* Drag threshold for distinguishing clicks from drags */
#define kDragThreshold 4

/* Folder item representation with file system integration */
typedef struct FolderItem {
    char name[256];
    Boolean isFolder;      /* true = folder, false = document/app */
    Point position;        /* position in window (for icon view) */
    FileID fileID;         /* File system ID (CNID) */
    DirID parentID;        /* Parent directory ID */
    uint32_t type;         /* File type (OSType) */
    uint32_t creator;      /* Creator code (OSType) */
} FolderItem;

/* Folder window state (per window) */
typedef struct FolderWindowState {
    FolderItem* items;     /* Array of items in this folder */
    short itemCount;       /* Number of items */
    short selectedIndex;   /* Currently selected item (-1 = none) */
    Boolean isDragging;    /* Currently dragging an item */
    UInt32 lastClickTime;  /* For double-click detection */
    Point lastClickPos;    /* Last click position */
    short lastClickIndex;  /* Index of last clicked item */
    VRefNum vref;          /* Volume reference for this folder */
    DirID currentDir;      /* Directory ID being displayed */
    Point dragStartGlobal; /* Global coordinates where drag started */
    short draggingIndex;   /* Index of item being dragged (-1 = none) */
} FolderWindowState;

/* Global folder window states (indexed by window pointer for now) */
#define MAX_FOLDER_WINDOWS 16
static struct {
    WindowPtr window;
    FolderWindowState state;
} gFolderWindows[MAX_FOLDER_WINDOWS];

/* Get or create state for a folder window */
FolderWindowState* GetFolderState(WindowPtr w);
void InitializeFolderContents(WindowPtr w, Boolean isTrash);
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
FolderWindowState* GetFolderState(WindowPtr w) {
    extern void serial_printf(const char* fmt, ...);
    serial_printf("GetFolderState: ENTRY\n");
    if (!w) {
        serial_printf("GetFolderState: w is NULL\n");
        return NULL;
    }

    /* Search for existing slot */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == w) {
            serial_printf("GetFolderState: Found existing slot %d\n", i);
            return &gFolderWindows[i].state;
        }
    }

    /* Find empty slot */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == NULL) {
            serial_printf("GetFolderState: Creating new slot %d, refCon=0x%08x\n", i, (unsigned int)w->refCon);
            gFolderWindows[i].window = w;
            gFolderWindows[i].state.items = NULL;
            gFolderWindows[i].state.itemCount = 0;
            gFolderWindows[i].state.selectedIndex = -1;
            gFolderWindows[i].state.isDragging = false;
            gFolderWindows[i].state.lastClickTime = 0;
            gFolderWindows[i].state.lastClickPos.h = 0;
            gFolderWindows[i].state.lastClickPos.v = 0;
            gFolderWindows[i].state.lastClickIndex = -1;
            gFolderWindows[i].state.vref = 0;
            gFolderWindows[i].state.currentDir = 0;
            gFolderWindows[i].state.dragStartGlobal.h = 0;
            gFolderWindows[i].state.dragStartGlobal.v = 0;
            gFolderWindows[i].state.draggingIndex = -1;

            /* Initialize folder contents */
            serial_printf("GetFolderState: About to call InitializeFolderContents\n");
            InitializeFolderContents(w, (w->refCon == 'TRSH'));
            serial_printf("GetFolderState: InitializeFolderContents returned\n");
            return &gFolderWindows[i].state;
        }
    }

    serial_printf("GetFolderState: No slots available!\n");
    return NULL;  /* No slots available */
}

/* Initialize folder contents from VFS */
void InitializeFolderContents(WindowPtr w, Boolean isTrash) {
    extern void serial_printf(const char* fmt, ...);
    FolderWindowState* state = NULL;

    serial_printf("InitializeFolderContents: ENTRY, w=0x%08x isTrash=%d\n", (unsigned int)w, (int)isTrash);

    /* Find the state we just created */
    for (int i = 0; i < MAX_FOLDER_WINDOWS; i++) {
        if (gFolderWindows[i].window == w) {
            state = &gFolderWindows[i].state;
            break;
        }
    }

    if (!state) {
        return;
    }

    serial_printf("InitializeFolderContents: state found, getting boot vref\n");

    /* Get boot volume reference */
    VRefNum vref = VFS_GetBootVRef();
    state->vref = vref;

    if (isTrash) {
        /* Trash folder - for now, keep empty
         * TODO: Get trash directory ID and enumerate it */
        state->currentDir = 0;  /* Special trash ID (to be defined) */
        state->itemCount = 0;
        state->items = NULL;
        serial_printf("InitializeFolderContents: trash folder empty\n");
    } else {
        /* Volume root - enumerate actual file system contents */
        state->currentDir = 2;  /* HFS root directory CNID is always 2 */

        /* Enumerate directory contents using VFS */
        #define MAX_ITEMS 128
        CatEntry entries[MAX_ITEMS];
        int count = 0;

        serial_printf("InitializeFolderContents: calling VFS_Enumerate\n");

        if (!VFS_Enumerate(vref, state->currentDir, entries, MAX_ITEMS, &count)) {
            serial_printf("InitializeFolderContents: VFS_Enumerate failed\n");
            state->itemCount = 0;
            state->items = NULL;
            return;
        }

        serial_printf("InitializeFolderContents: VFS_Enumerate OK, count=%d\n", count);

        if (count == 0) {
            state->itemCount = 0;
            state->items = NULL;
            return;
        }

        /* Allocate item array */
        state->items = (FolderItem*)malloc(sizeof(FolderItem) * count);
        if (!state->items) {
            serial_printf("FW: malloc failed for %d items\n", count);
            state->itemCount = 0;
            return;
        }

        state->itemCount = count;

        /* Convert CatEntry to FolderItem and lay out in grid
         * Grid: 3 columns, spacing 100px horizontal, 90px vertical
         * Start at (80, 30) for margins */
        const int startX = 80;
        const int startY = 30;
        const int colSpacing = 100;
        const int rowHeight = 90;
        const int maxCols = 3;

        for (int i = 0; i < count; i++) {
            /* Copy name (ensure null termination) */
            size_t nameLen = strlen(entries[i].name);
            if (nameLen >= 256) nameLen = 255;
            memcpy(state->items[i].name, entries[i].name, nameLen);
            state->items[i].name[nameLen] = '\0';

            /* File system metadata */
            state->items[i].isFolder = (entries[i].kind == kNodeDir);
            state->items[i].fileID = entries[i].id;
            state->items[i].parentID = entries[i].parent;
            state->items[i].type = entries[i].type;
            state->items[i].creator = entries[i].creator;

            /* Calculate grid position */
            int col = i % maxCols;
            int row = i / maxCols;
            state->items[i].position.h = startX + (col * colSpacing);
            state->items[i].position.v = startY + (row * rowHeight);

            serial_printf("FW: Item %d: '%s' %s id=%d pos=(%d,%d)\n",
                         i, state->items[i].name,
                         state->items[i].isFolder ? "DIR" : "FILE",
                         state->items[i].fileID,
                         state->items[i].position.h, state->items[i].position.v);
        }

        serial_printf("FW: Initialized %d items from VFS\n", count);
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

        /* Label rect (text centered, with padding)
         * Label baseline is at iconTop + 40, background extends from (baseline - 12) to (baseline + 2) */
        int textWidth, textHeight;
        IconLabel_Measure(state->items[i].name, &textWidth, &textHeight);

        Rect labelRect;
        int centerX = state->items[i].position.h + 16;
        labelRect.left = centerX - (textWidth / 2) - 2;
        labelRect.top = state->items[i].position.v + 28;  /* 40 - 12 = 28 */
        labelRect.right = centerX + (textWidth / 2) + 2;
        labelRect.bottom = state->items[i].position.v + 42;  /* 40 + 2 = 42 */

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

/* Track folder item drag - detects drag threshold and sets drag state
 * Returns true if drag was started, false if click (button released before threshold)
 */
static Boolean TrackFolderItemDrag(WindowPtr w, FolderWindowState* state, short itemIndex, Point startGlobal) {
    if (!w || !state || itemIndex < 0 || itemIndex >= state->itemCount) {
        return false;
    }

    serial_printf("FW: TrackFolderItemDrag: item %d '%s' from global (%d,%d)\n",
                 itemIndex, state->items[itemIndex].name, startGlobal.h, startGlobal.v);

    /* Ensure window port is set for GetMouse calls */
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort((GrafPtr)w);

    /* Wait for drag threshold or button release */
    Point last = startGlobal, cur;
    extern void ProcessModernInput(void);

    while ((gCurrentButtons & 1) != 0) {
        ProcessModernInput();  /* Update gCurrentButtons */
        GetMouse(&cur);
        serial_printf("FW: Threshold check - GetMouse returned cur=(%d,%d)\n", cur.h, cur.v);
        LocalToGlobal(&cur);  /* Convert to global for comparison */
        serial_printf("FW: Threshold check - After LocalToGlobal cur=(%d,%d)\n", cur.h, cur.v);

        /* Check if we've exceeded drag threshold */
        int dx = cur.h - startGlobal.h;
        int dy = cur.v - startGlobal.v;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;

        if (dx >= kDragThreshold || dy >= kDragThreshold) {
            /* Threshold exceeded - start drag */
            serial_printf("FW: Drag threshold exceeded: delta=(%d,%d)\n",
                         cur.h - startGlobal.h, cur.v - startGlobal.v);

            state->isDragging = true;
            state->draggingIndex = itemIndex;
            state->dragStartGlobal = startGlobal;

            serial_printf("FW: DRAG STARTED: item='%s' fileID=%d vref=%d dir=%d\n",
                         state->items[itemIndex].name,
                         state->items[itemIndex].fileID,
                         state->vref,
                         state->currentDir);

            /* Calculate ghost rect from item position (32x32 icon) */
            FolderItem* item = &state->items[itemIndex];

            serial_printf("FW: Item '%s' position (local) = (%d,%d)\n",
                         item->name, item->position.h, item->position.v);
            serial_printf("FW: Window bounds (global) = (%d,%d,%d,%d)\n",
                         w->port.portBits.bounds.left, w->port.portBits.bounds.top,
                         w->port.portBits.bounds.right, w->port.portBits.bounds.bottom);

            Rect ghost;
            ghost.left = item->position.h;
            ghost.top = item->position.v;
            ghost.right = ghost.left + 32;
            ghost.bottom = ghost.top + 32;

            serial_printf("FW: Ghost rect BEFORE LocalToGlobal (local) = (%d,%d,%d,%d)\n",
                         ghost.left, ghost.top, ghost.right, ghost.bottom);

            /* Convert to global coordinates */
            Point topLeft, botRight;
            topLeft.h = ghost.left;
            topLeft.v = ghost.top;
            botRight.h = ghost.right;
            botRight.v = ghost.bottom;

            serial_printf("FW: topLeft BEFORE L2G: h=%d v=%d\n", topLeft.h, topLeft.v);
            serial_printf("FW: portBits.bounds: left=%d top=%d\n",
                         w->port.portBits.bounds.left, w->port.portBits.bounds.top);

            LocalToGlobal(&topLeft);
            LocalToGlobal(&botRight);

            serial_printf("FW: topLeft AFTER L2G: h=%d v=%d\n", topLeft.h, topLeft.v);

            ghost.left = topLeft.h;
            ghost.top = topLeft.v;
            ghost.right = botRight.h;
            ghost.bottom = botRight.v;

            serial_printf("FW: Ghost rect AFTER LocalToGlobal (global) X=%d-%d Y=%d-%d\n",
                         ghost.left, ghost.right, ghost.top, ghost.bottom);

            /* Add padding like desktop (left -20, right +20, bottom +16) */
            ghost.left -= 20;
            ghost.right += 20;
            ghost.bottom += 16;

            serial_printf("FW: Ghost rect with padding (global) X=%d-%d Y=%d-%d\n",
                         ghost.left, ghost.right, ghost.top, ghost.bottom);

            /* Switch to screen port for XOR ghost drawing - we need GetMouse to return global coords */
            GrafPort screenDragPort;
            OpenPort(&screenDragPort);
            screenDragPort.portBits = qd.screenBits;
            screenDragPort.portRect = qd.screenBits.bounds;
            SetPort(&screenDragPort);
            ClipRect(&qd.screenBits.bounds);

            /* Show initial ghost */
            extern void Desktop_GhostShowAt(const Rect* r);
            Desktop_GhostShowAt(&ghost);
            serial_printf("FW: Ghost visible, entering drag loop\n");

            /* Track drag with visual feedback */
            Point lastPos = cur;  /* cur is already in global coords from threshold check */
            serial_printf("FW: Starting drag loop, lastPos=(%d,%d) in GLOBAL coords\n", lastPos.h, lastPos.v);
            while ((gCurrentButtons & 1) != 0) {
                ProcessModernInput();  /* Update gCurrentButtons */
                GetMouse(&cur);
                /* g_mousePos is in window-local coords, need to convert using WINDOW port */
                SetPort((GrafPtr)w);  /* Temporarily switch back to window port for LocalToGlobal */
                LocalToGlobal(&cur);
                SetPort(&screenDragPort);  /* Switch back to screen port for drawing */
                serial_printf("FW: GetMouse+LocalToGlobal returned cur=(%d,%d)\n", cur.h, cur.v);

                /* Update ghost position if mouse moved */
                if (cur.h != lastPos.h || cur.v != lastPos.v) {
                    extern void OffsetRect(Rect* r, short dh, short dv);
                    serial_printf("FW: Mouse moved, offsetting ghost by (%d,%d)\n",
                                 cur.h - lastPos.h, cur.v - lastPos.v);
                    OffsetRect(&ghost, cur.h - lastPos.h, cur.v - lastPos.v);
                    Desktop_GhostShowAt(&ghost);
                    lastPos = cur;
                }
            }

            /* Erase ghost before processing drop */
            extern void Desktop_GhostEraseIf(void);
            Desktop_GhostEraseIf();

            /* Restore port */
            SetPort((GrafPtr)w);

            serial_printf("FW: Drag ended at global (%d,%d)\n", cur.h, cur.v);

            /* Phase 4: Check for drop-to-trash first */
            if (Desktop_IsOverTrash(cur)) {
                serial_printf("FW: DROP TO TRASH detected - deleting '%s' (fileID=%d, vref=%d)\n",
                             item->name, item->fileID, state->vref);

                bool deleted = VFS_Delete(state->vref, item->fileID);

                if (deleted) {
                    serial_printf("FW: File deleted successfully from VFS\n");

                    /* Remove item from folder window display by shifting array */
                    for (int i = itemIndex; i < state->itemCount - 1; i++) {
                        state->items[i] = state->items[i + 1];
                    }
                    state->itemCount--;

                    /* Clear selection if we deleted the selected item */
                    if (state->selectedIndex == itemIndex) {
                        state->selectedIndex = -1;
                    } else if (state->selectedIndex > itemIndex) {
                        state->selectedIndex--;  /* Adjust index after shift */
                    }

                    serial_printf("FW: Item removed from display, new itemCount=%d\n", state->itemCount);

                    /* Request window redraw */
                    PostEvent(updateEvt, (UInt32)w);
                } else {
                    serial_printf("FW: ERROR: VFS_Delete failed\n");
                }
            } else {
                /* Phase 3: Check for drop-to-desktop and create alias */
                WindowPtr hitWindow = NULL;
                short partCode = FindWindow(cur, &hitWindow);

                if (partCode == inDesk) {
                    /* Dropped on desktop - create alias */
                    Boolean isFolder = item->isFolder;

                    serial_printf("FW: DROP TO DESKTOP detected - creating alias for '%s'\n", item->name);

                    OSErr err = Desktop_AddAliasIcon(item->name, cur, item->fileID,
                                                    state->vref, isFolder);

                    if (err == noErr) {
                        serial_printf("FW: Alias created successfully on desktop\n");
                    } else {
                        serial_printf("FW: ERROR: Desktop_AddAliasIcon failed with error %d\n", err);
                    }
                } else {
                    serial_printf("FW: Dropped over window (partCode=%d) - no action\n", partCode);
                }
            }

            /* Clear drag state */
            state->isDragging = false;
            state->draggingIndex = -1;

            SetPort(savePort);
            return true;  /* Drag occurred */
        }
    }

    /* Button released before threshold - treat as click */
    serial_printf("FW: Button released before threshold - treating as click\n");
    SetPort(savePort);
    return false;
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
        /* SINGLE-CLICK: Track for potential drag, or select if just a click */
        serial_printf("FW: single-click on icon %d, tracking for drag...\n", hitIndex);

        /* Call drag tracking - this will wait for threshold or button release */
        Boolean wasDrag = TrackFolderItemDrag(w, state, hitIndex, ev->where);

        if (!wasDrag) {
            /* No drag occurred - treat as normal click/selection */
            short oldSel = state->selectedIndex;
            state->selectedIndex = hitIndex;
            state->lastClickIndex = hitIndex;
            state->lastClickTime = currentTime;

            serial_printf("FW: select %d -> %d\n", oldSel, hitIndex);

            /* Post update to redraw selection */
            PostEvent(updateEvt, (UInt32)w);
        } else {
            /* Drag occurred - selection/timing was handled by drag tracking */
            serial_printf("FW: drag completed, skipping normal selection\n");
        }
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

    /* Draw white background for content area
     * NOTE: portBits.bounds is already set to content area top (not window top)
     * by Platform_InitializeWindowPort (gFrame.top + kTitle + kSeparator)
     * So portRect local coord 0 maps to content area top, not title bar! */
    Rect contentRect = w->port.portRect;
    EraseRect(&contentRect);
    serial_printf("FW: Erased content area\n");

    /* If trash is empty, draw empty message */
    if (isTrash && state && state->itemCount == 0) {
        serial_printf("FW: Drawing empty trash message\n");
        MoveTo(10, 30);
        DrawText("Trash is empty", 0, 14);
        MoveTo(10, 50);
        DrawText("Drag items here to delete them", 0, 30);
    }
    /* If we have state, draw icons with selection highlighting */
    else if (state && state->items) {
        /* Convert window port coordinates to global screen coordinates for icon drawing */
        Point globalOrigin;
        globalOrigin.h = w->port.portBits.bounds.left;
        globalOrigin.v = w->port.portBits.bounds.top;

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
    /* Use lowercase hex to match runtime exactly */
    return (w->refCon == 0x4449534b || w->refCon == 0x54525348);  /* 'DISK' or 'TRSH' */
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