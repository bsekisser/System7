/* #include "SystemTypes.h" */
/*
 * RE-AGENT-BANNER
 * Desktop Manager Implementation
 *
 * Reverse-engineered from System 7 Finder.rsrc
 * Source:  3_resources/Finder.rsrc
 *
 * Evidence sources:
 * - String analysis: "Clean Up Desktop", "Rebuilding the desktop file"
 * - String analysis: "was found on the desktop", "on the desktop"
 * - Desktop database management functionality
 *
 * This module handles desktop icon positioning and desktop database management.
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include <string.h>
#include <stdlib.h>  /* For abs() */

#include "Finder/finder.h"
#include "Finder/finder_types.h"
#include "FileMgr/file_manager.h"
/* Use local headers instead of system headers */
#include "MemoryMgr/memory_manager_types.h"
#include "ResourceManager.h"
#include "PatternMgr/pattern_manager.h"
#include "FS/vfs.h"
#include "chicago_font.h"  /* For ChicagoCharInfo */
#include "Finder/Icon/icon_types.h"
#include "Finder/Icon/icon_label.h"
#include "Finder/Icon/icon_system.h"
#include "QuickDrawConstants.h"

/* HD Icon data - still needed by icon_system.c */
extern const uint8_t g_HDIcon[128];
extern const uint8_t g_HDIconMask[128];

/* Debug output */
extern void serial_printf(const char* fmt, ...);
extern void serial_puts(const char* str);

/* Chicago font data */
extern const uint8_t chicago_bitmap[];
extern GrafPtr g_currentPort;
#define CHICAGO_HEIGHT 16
#define CHICAGO_ASCENT 12
#define CHICAGO_ROW_BYTES 140


/* Desktop Database Constants */
#define kDesktopDatabaseName    "\012Desktop DB"
#define kDesktopIconSpacing     80          /* Pixel spacing between icons */
#define kDesktopMargin          20          /* Margin from screen edge */
#define kMaxDesktopIcons        256         /* Maximum icons on desktop */

/* Grid and icon sizing (System 7-ish) */
enum { kGridW = 8, kGridH = 12, kIconW = 32, kIconH = 32 };

/* Drag threshold for distinguishing clicks from drags */
#define kDragThreshold 4

/* External globals */
extern QDGlobals qd;  /* QuickDraw globals from main.c */
extern void* framebuffer;
extern uint32_t fb_width, fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* Global tracking guard for modal drag loops */
volatile Boolean gInMouseTracking = false;

/* Index of icon being dragged (-1 if none) */
static short gDraggingIconIndex = -1;

/* Global Desktop State */
static DesktopItem *gDesktopIcons = nil;   /* Array of desktop items (universal system) */
static short gDesktopIconCount = 0;         /* Number of icons on desktop */
static Boolean gDesktopNeedsCleanup = false; /* Desktop needs reorganization */
static VRefNum gBootVolumeRef = 0;          /* Boot volume reference */
static Boolean gVolumeIconVisible = false;   /* Is volume icon shown on desktop */

/* Icon selection and dragging state */
static short gSelectedIcon = -1;             /* Index of selected icon (-1 = none) */
static Boolean gDraggingIcon = false;        /* Currently dragging an icon */
static Point gDragOffset = {0, 0};           /* Offset from icon origin to mouse */
static Point gOriginalPos = {0, 0};          /* Original position before drag */
static UInt32 gLastClickTime = 0;            /* For double-click detection */
static Point gLastClickPos = {0, 0};         /* Last click position */

/* Same-icon double-click tracking (Classic Finder behavior) */
static short sLastClickIcon = -1;            /* Icon clicked last time */
static UInt32 sLastClickTicks = 0;           /* When it was clicked */

/* Forward Declarations */
static OSErr LoadDesktopDatabase(short vRefNum);
static OSErr SaveDesktopDatabase(short vRefNum);
static OSErr AllocateDesktopIcons(void);
static Point CalculateNextIconPosition(void);
static Boolean IsPositionOccupied(Point position);
static OSErr ScanDirectoryForDesktopEntries(short vRefNum, long dirID, short databaseRefNum);
static Point SnapToGrid(Point p);
static void UpdateIconRect(short iconIndex, Rect *outRect);
static short IconAtPoint(Point where);
static void GhostXOR(const Rect* r);
static void GhostEraseIf(void);
static void GhostShowAt(const Rect* r);
static void DesktopYield(void);
static void TrackIconDragSync(short iconIndex, Point startPt);

/* Public function to draw the desktop */
void DrawDesktop(void);
void DrawVolumeIcon(void);

/*
 * CleanUpDesktop - Arranges all desktop icons in a grid pattern

 */
OSErr CleanUpDesktop(void)
{
    OSErr err = noErr;
    Point currentPos;
    short iconIndex;
    short iconsPerRow;
    short currentRow = 0;
    short currentCol = 0;

    /* Calculate icons per row based on screen width */
    iconsPerRow = (qd.screenBits.bounds.right - (kDesktopMargin * 2)) / kDesktopIconSpacing;

    /* Starting position */
    currentPos.h = kDesktopMargin;
    currentPos.v = kDesktopMargin + 40; /* Leave space for menu bar */

    /* Arrange all desktop icons in grid */
    for (iconIndex = 0; iconIndex < gDesktopIconCount; iconIndex++) {
        /* Calculate grid position */
        currentPos.h = kDesktopMargin + (currentCol * kDesktopIconSpacing);
        currentPos.v = kDesktopMargin + 40 + (currentRow * kDesktopIconSpacing);

        /* Update icon position */
        gDesktopIcons[iconIndex].position = currentPos;

        /* Move to next grid position */
        currentCol++;
        if (currentCol >= iconsPerRow) {
            currentCol = 0;
            currentRow++;
        }
    }

    /* Save updated positions to desktop database */
    err = SaveDesktopDatabase(0); /* System volume */
    if (err != noErr) {
        return err;
    }

    /* Redraw desktop */
    InvalRect(&qd.screenBits.bounds);
    DrawDesktop();  /* Draw the desktop pattern */
    gDesktopNeedsCleanup = false;

    return noErr;
}

/*
 * Finder_DeskHook - Called by Window Manager to let Finder draw desktop
 * This is called during PaintBehind when the desktop needs updating
 */
static void Finder_DeskHook(RgnHandle invalidRgn)
{
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(qd.thePort);  /* Draw to screen port - FIX: qd.thePort is already a GrafPtr */

    /* Clip to the invalid region */
    SetClip(invalidRgn);

    /* Draw desktop pattern using current background pattern */
    /* EraseRgn will use the BackPat/BackColor set by Pattern Manager */
    EraseRgn(invalidRgn);

    /* Draw desktop icons in invalid region */
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (i == gDraggingIconIndex) continue;

        Rect iconRect;
        SetRect(&iconRect,
                gDesktopIcons[i].position.h,
                gDesktopIcons[i].position.v,
                gDesktopIcons[i].position.h + 32,
                gDesktopIcons[i].position.v + 32);

        if (RectInRgn(&iconRect, invalidRgn)) {
            /* Draw icon using QuickDraw */
            /* PlotIcon(&iconRect, gDesktopIcons[i].icon); */

            /* Never draw a border during regular paints. Ghost outline is handled
               exclusively by InvertIconOutline() inside TrackIconDragSync(). */
        }
    }

    /* Draw volume icon if visible (DrawVolumeIcon also draws trash) */
    if (gVolumeIconVisible) {
        DrawVolumeIcon();
    }

    SetPort(savePort);
}

/* ArrangeDesktopIcons - Arrange desktop icons in a grid */
void ArrangeDesktopIcons(void)
{
    const int gridSpacing = 80;  /* Spacing between icons */
    const int startX = 700;       /* Start from right side */
    const int startY = 50;        /* Start below menu bar */
    int currentX = startX;
    int currentY = startY;

    serial_printf("ArrangeDesktopIcons: Arranging %d icons\n", gDesktopIconCount);

    /* Arrange icons in a vertical column from top-right */
    for (int i = 0; i < gDesktopIconCount; i++) {
        gDesktopIcons[i].position.h = currentX;
        gDesktopIcons[i].position.v = currentY;

        currentY += gridSpacing;

        /* Wrap to next column if too low */
        if (currentY > 400) {
            currentY = startY;
            currentX -= gridSpacing;
        }
    }

    /* Redraw desktop to show new arrangement */
    DrawDesktop();
}

/*
 * DrawDesktop - Initial desktop drawing (called once at startup)
 * Also used to redraw desktop after menus or dialogs
 */
void DrawDesktop(void)
{
    static Boolean gInDesktopPaint = false;
    RgnHandle desktopRgn;

    /* Erase any active ghost outline before desktop redraw */
    GhostEraseIf();

    /* Re-entrancy guard: prevent recursive painting */
    if (gInDesktopPaint) {
        extern void serial_printf(const char* fmt, ...);
        serial_printf("DrawDesktop: re-entry detected, skipping to avoid freeze\n");
        return;
    }
    gInDesktopPaint = true;

    /* Register our DeskHook with Window Manager */
    SetDeskHook(Finder_DeskHook);

    /* Create a region for the desktop (excluding menu bar) */
    desktopRgn = NewRgn();
    Rect desktopRect = qd.screenBits.bounds;
    desktopRect.top = 20;  /* Start below menu bar */
    RectRgn(desktopRgn, &desktopRect);

    /* Directly call our DeskHook to paint the desktop with the proper pattern */
    Finder_DeskHook(desktopRgn);  /* Pass the desktop region to paint */

    /* NOTE: Do NOT call InvalRect here - that would cause infinite recursion!
     * The caller (PostEvent(updateEvt, 0) sites) already requested the update. */

    DisposeRgn(desktopRgn);
    gInDesktopPaint = false;
}

/*
 * RebuildDesktopFile - Rebuilds the desktop database for a volume

 */
OSErr RebuildDesktopFile(short vRefNum)
{
    OSErr err = noErr;
    short databaseRefNum;
    FSSpec databaseSpec;
    HParamBlockRec pb;

    /* Create FSSpec for desktop database */
    err = FSMakeFSSpec(vRefNum, fsRtDirID, kDesktopDatabaseName, &databaseSpec);
    if (err == fnfErr) {
        /* Database doesn't exist, create it */
        err = FSpCreate(&databaseSpec, 'DMGR', 'DTBS', smSystemScript);
        if (err != noErr && err != dupFNErr) {
            return err;
        }
    } else if (err != noErr) {
        return err;
    }

    /* Open desktop database file */
    err = FSpOpenDF(&databaseSpec, fsWrPerm, &databaseRefNum);
    if (err != noErr) {
        return err;
    }

    /* Clear existing database content */
    err = SetEOF(databaseRefNum, 0);
    if (err != noErr) {
        FSClose(databaseRefNum);
        return err;
    }

    /* Scan volume and rebuild desktop entries */
    pb.ioCompletion = nil;
    pb.ioNamePtr = nil;
    pb.ioVRefNum = vRefNum;
    pb.u.volumeParam.ioVolIndex = 0;

    err = PBHGetVInfoSync(&pb);
    if (err == noErr) {
        /* Recursively scan directories and build database */
        err = ScanDirectoryForDesktopEntries(vRefNum, fsRtDirID, databaseRefNum);
    }

    FSClose(databaseRefNum);
    return err;
}

/*
 * GetDesktopIconPosition - Gets the position of an icon on the desktop

 */
OSErr GetDesktopIconPosition(FSSpec *item, Point *position)
{
    short iconIndex;
    UInt32 itemID;

    if (item == nil || position == nil) {
        return paramErr;
    }

    /* Generate unique ID for item */
    itemID = (UInt32)item->parID ^ (UInt32)item->name[0];

    /* Search for icon in desktop array */
    for (iconIndex = 0; iconIndex < gDesktopIconCount; iconIndex++) {
        if (gDesktopIcons[iconIndex].iconID == itemID) {
            *position = gDesktopIcons[iconIndex].position;
            return noErr;
        }
    }

    /* Icon not found on desktop */
    return fnfErr;
}

/*
 * SetDesktopIconPosition - Sets the position of an icon on the desktop

 */
OSErr SetDesktopIconPosition(FSSpec *item, Point position)
{
    OSErr err;
    short iconIndex;
    UInt32 itemID;
    Boolean found = false;

    if (item == nil) {
        return paramErr;
    }

    /* Ensure desktop icons array is allocated */
    if (gDesktopIcons == nil) {
        err = AllocateDesktopIcons();
        if (err != noErr) return err;
    }

    /* Generate unique ID for item */
    itemID = (UInt32)item->parID ^ (UInt32)item->name[0];

    /* Search for existing icon */
    for (iconIndex = 0; iconIndex < gDesktopIconCount; iconIndex++) {
        if (gDesktopIcons[iconIndex].iconID == itemID) {
            gDesktopIcons[iconIndex].position = position;
            found = true;
            break;
        }
    }

    /* Add new icon if not found */
    if (!found) {
        if (gDesktopIconCount >= kMaxDesktopIcons) {
            return memFullErr;
        }

        gDesktopIcons[gDesktopIconCount].iconID = itemID;
        gDesktopIcons[gDesktopIconCount].position = position;
        gDesktopIconCount++;
    }

    /* Save updated positions */
    err = SaveDesktopDatabase(0);
    return err;
}

/*
 * InitializeDesktopDB - Initialize desktop database system

 */
OSErr InitializeDesktopDB(void)
{
    OSErr err;

    extern void serial_puts(const char* str);
    serial_puts("Desktop: InitializeDesktopDB called\n");

    /* Initialize Pattern Manager first */
    PM_Init();

    /* Try different ppat IDs to see which ones work */
    DesktopPref pref;
    serial_puts("Desktop: Testing different ppat patterns\n");

    /* Try custom PPAT8 patterns that we know have color data */
    int16_t ppat_ids[] = {304}; /* Pattern 304 - BluePixel custom pattern */
    int num_ppats = sizeof(ppat_ids) / sizeof(ppat_ids[0]);

    bool found_working = false;
    for (int i = 0; i < num_ppats; i++) {
        char msg[80];
        sprintf(msg, "Desktop: Trying ppat ID %d\n", ppat_ids[i]);
        serial_puts(msg);

        pref.usePixPat = true;
        pref.patID = 16;      /* Fallback PAT if ppat fails */
        pref.ppatID = ppat_ids[i];
        pref.backColor.red = 0xC000;
        pref.backColor.green = 0xC000;
        pref.backColor.blue = 0xC000;

        if (PM_ApplyDesktopPref(&pref)) {
            sprintf(msg, "Desktop: SUCCESS - ppat ID %d loaded!\n", ppat_ids[i]);
            serial_puts(msg);
            found_working = true;
            break;  /* Use the first one that works */
        } else {
            sprintf(msg, "Desktop: Failed ppat ID %d\n", ppat_ids[i]);
            serial_puts(msg);
        }
    }

    if (!found_working) {
        serial_puts("Desktop: No ppat patterns loaded successfully, using fallback\n");
        pref.usePixPat = false;
        pref.patID = 16;  /* Use dots pattern as fallback */
        PM_ApplyDesktopPref(&pref);
    }

    /* Allocate desktop icons array */
    err = AllocateDesktopIcons();
    if (err != noErr) return err;

    /* Load existing desktop database */
    err = LoadDesktopDatabase(0); /* System volume */
    if (err != noErr) {
        /* If load fails, keep trash as the only item (gDesktopIconCount = 1) */
        /* Don't reset to 0 - trash is already initialized at index 0 */
    }

    /* Register our DeskHook with Window Manager */
    SetDeskHook(Finder_DeskHook);

    return noErr;
}

/* Static helper functions */

/*
 * AllocateDesktopIcons - Allocate memory for desktop icons array
 */
static OSErr AllocateDesktopIcons(void)
{
    if (gDesktopIcons != NULL) {
        return noErr; /* Already allocated */
    }

    gDesktopIcons = (DesktopItem*)NewPtr(sizeof(DesktopItem) * kMaxDesktopIcons);
    if (gDesktopIcons == NULL) {
        return memFullErr;
    }

    /* Initialize trash as the first desktop item */
    gDesktopIcons[0].type = kDesktopItemTrash;
    gDesktopIcons[0].iconID = 0xFFFFFFFF;
    gDesktopIcons[0].position.h = fb_width - 100;
    gDesktopIcons[0].position.v = fb_height - 80;
    strcpy(gDesktopIcons[0].name, "Trash");
    gDesktopIcons[0].movable = false;  /* Trash stays in place */
    gDesktopIconCount = 1;  /* Start with trash */

    return noErr;
}

/*
 * CalculateNextIconPosition - Calculate next available icon position
 */
static Point CalculateNextIconPosition(void)
{
    Point pos;
    pos.h = kDesktopMargin;
    pos.v = kDesktopMargin + 40;

    /* Simple increment for now */
    if (gDesktopIconCount > 0) {
        pos.h += (gDesktopIconCount % 10) * kDesktopIconSpacing;
        pos.v += (gDesktopIconCount / 10) * kDesktopIconSpacing;
    }

    return pos;
}

/*
 * IsPositionOccupied - Check if a position is already occupied
 */
static Boolean IsPositionOccupied(Point position)
{
    short i;
    for (i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].position.h == position.h &&
            gDesktopIcons[i].position.v == position.v) {
            return true;
        }
    }
    return false;
}

/*
 * SnapToGrid - Snap position to grid (System 7 style)
 */
static Point SnapToGrid(Point p)
{
    extern QDGlobals qd;

    /* Round to nearest grid */
    p.h = ((p.h + kGridW / 2) / kGridW) * kGridW;
    p.v = ((p.v + kGridH / 2) / kGridH) * kGridH;

    /* Clamp to screen bounds (leave room for 32x32 icon) */
    if (p.h < 0) p.h = 0;
    if (p.v < 20) p.v = 20;  /* Below menu bar */
    if (p.h > qd.screenBits.bounds.right - 32) p.h = qd.screenBits.bounds.right - 32;
    if (p.v > qd.screenBits.bounds.bottom - 32) p.v = qd.screenBits.bounds.bottom - 32;

    return p;
}

/*
 * UpdateIconRect - Update icon rect from position
 */
static void UpdateIconRect(short iconIndex, Rect *outRect)
{
    if (iconIndex < 0 || iconIndex >= gDesktopIconCount) return;

    outRect->left   = gDesktopIcons[iconIndex].position.h;
    outRect->top    = gDesktopIcons[iconIndex].position.v;
    outRect->right  = gDesktopIcons[iconIndex].position.h + kIconW;
    outRect->bottom = gDesktopIcons[iconIndex].position.v + kIconH;
}

/*
 * IconAtPoint - Find icon at point (returns index, -1 if none)
 */
static short IconAtPoint(Point where)
{
    Rect iconRect, labelRect;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("IconAtPoint: checking (%d,%d), gDesktopIconCount=%d\n",
                 where.h, where.v, gDesktopIconCount);

    /* Check all desktop items */
    for (int i = 0; i < gDesktopIconCount; i++) {
        serial_printf("IconAtPoint: Checking item %d (type=%d) at (%d,%d)\n",
                     i, gDesktopIcons[i].type,
                     gDesktopIcons[i].position.h, gDesktopIcons[i].position.v);

        SetRect(&iconRect,
                gDesktopIcons[i].position.h,
                gDesktopIcons[i].position.v,
                gDesktopIcons[i].position.h + kIconW,
                gDesktopIcons[i].position.v + kIconH);

        /* Calculate label rect based on item type */
        int labelOffset = (gDesktopIcons[i].type == kDesktopItemTrash) ? 48 : kIconH;
        SetRect(&labelRect,
                gDesktopIcons[i].position.h - 20,
                gDesktopIcons[i].position.v + labelOffset,
                gDesktopIcons[i].position.h + kIconW + 20,
                gDesktopIcons[i].position.v + labelOffset + 16);

        serial_printf("IconAtPoint: Item %d rects: icon=(%d,%d,%d,%d), label=(%d,%d,%d,%d)\n",
                     i, iconRect.left, iconRect.top, iconRect.right, iconRect.bottom,
                     labelRect.left, labelRect.top, labelRect.right, labelRect.bottom);

        if (PtInRect(where, &iconRect) || PtInRect(where, &labelRect)) {
            serial_printf("IconAtPoint: HIT item %d!\n", i);
            return i;
        }
    }

    serial_printf("IconAtPoint: No icon hit, returning -1\n");
    return -1;
}

/*
 * Ghost outline state for drag tracking
 */
static Boolean sGhostActive = false;
static Rect    sGhostRect   = {0,0,0,0};

/*
 * GhostXOR - Low-level XOR rect drawing using screen port directly
 */
static void GhostXOR(const Rect* r)
{
    /* Direct XOR rectangle drawing to framebuffer for immediate visibility */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;
    extern void serial_printf(const char* fmt, ...);

    if (!framebuffer || !r) return;

    serial_printf("GhostXOR: received Rect top=%d left=%d bottom=%d right=%d\n",
                 r->top, r->left, r->bottom, r->right);

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;

    /* Clip to screen bounds */
    short left = (r->left >= 0) ? r->left : 0;
    short top = (r->top >= 0) ? r->top : 0;
    short right = (r->right <= (short)fb_width) ? r->right : fb_width;
    short bottom = (r->bottom <= (short)fb_height) ? r->bottom : fb_height;

    serial_printf("GhostXOR: drawing at X=%d-%d Y=%d-%d\n", left, right, top, bottom);

    if (left >= right || top >= bottom) return;

    /* XOR color: white (inverts on blue desktop) */
    uint32_t xor_color = 0xFFFFFFFF;

    /* Draw thick (3px) XOR rectangle frame for better visibility */
    /* Top edge (3 pixels thick) */
    for (int y = top; y < top + 3 && y < bottom; y++) {
        for (int x = left; x < right; x++) {
            fb[y * pitch + x] ^= xor_color;
        }
    }

    /* Bottom edge (3 pixels thick) */
    for (int y = bottom - 3; y < bottom && y >= top; y++) {
        for (int x = left; x < right; x++) {
            fb[y * pitch + x] ^= xor_color;
        }
    }

    /* Left edge (3 pixels thick, avoiding corners already drawn) */
    for (int y = top + 3; y < bottom - 3; y++) {
        for (int x = left; x < left + 3 && x < right; x++) {
            fb[y * pitch + x] ^= xor_color;
        }
    }

    /* Right edge (3 pixels thick, avoiding corners already drawn) */
    for (int y = top + 3; y < bottom - 3; y++) {
        for (int x = right - 3; x < right && x >= left; x++) {
            fb[y * pitch + x] ^= xor_color;
        }
    }

    /* Force immediate display update */
    extern void QDPlatform_UpdateScreen(SInt32 left, SInt32 top, SInt32 right, SInt32 bottom);
    QDPlatform_UpdateScreen(left, top, right, bottom);

    serial_printf("GhostXOR: Drew XOR rect (%d,%d,%d,%d)\n", left, top, right, bottom);
}

/*
 * GhostEraseIf - Erase ghost if active
 */
static inline void GhostEraseIf(void)
{
    if (sGhostActive) {
        GhostXOR(&sGhostRect);  /* XOR twice = erase */
        sGhostActive = false;
    }
}

/*
 * Desktop_GhostEraseIf - Public wrapper for folder windows to erase ghost safely
 */
void Desktop_GhostEraseIf(void)
{
    GhostEraseIf();
}

/*
 * Desktop_GhostShowAt - Public wrapper for folder windows to show ghost at rect
 */
void Desktop_GhostShowAt(const Rect* r)
{
    GhostShowAt(r);
}

/*
 * GhostShowAt - Show ghost at new position
 */
static inline void GhostShowAt(const Rect* r)
{
    GhostEraseIf();  /* Erase old first */
    sGhostRect = *r;
    GhostXOR(&sGhostRect);
    sGhostActive = true;
}

/*
 * DesktopYield - Keep system ticking during modal loops
 */
static void DesktopYield(void)
{
    extern void SystemTask(void);
    extern void ProcessModernInput(void);
    extern void serial_printf(const char* fmt, ...);
    static int yieldCount = 0;
    if (++yieldCount % 100 == 0) {
        serial_printf("[DesktopYield] Called %d times\n", yieldCount);
    }

    /* Don't call EventPumpYield() here - it can cause re-entrancy issues during drag.
     * ProcessModernInput() polls PS/2 input AND updates gCurrentButtons */
    ProcessModernInput();  /* Update mouse/keyboard state including gCurrentButtons */
    SystemTask();    /* Handle time manager tasks */
}

/*
 * TrackIconDragSync - System 7 style synchronous modal drag with XOR ghost
 * This runs immediately when mouseDown is received, polls StillDown() directly
 */
static void TrackIconDragSync(short iconIndex, Point startPt)
{
    Rect ghost;
    Boolean ghostOn = false;
    Boolean didDrag = false;  /* Track if icon actually moved during drag */
    Point p;
    GrafPtr savePort;

    extern void serial_printf(const char* fmt, ...);
    extern Boolean StillDown(void);
    extern void GetMouse(Point *pt);

    if (iconIndex < 0 || iconIndex >= gDesktopIconCount) return;

    serial_printf("TrackIconDragSync ENTRY: starting modal drag for icon %d\n", iconIndex);

    /* Set tracking guard to suppress queued mouse events */
    gInMouseTracking = true;

    /* Mark this icon as being dragged */
    gDraggingIconIndex = iconIndex;

    /* Prepare ghost rect */
    UpdateIconRect(iconIndex, &ghost);
    ghost.left -= 20;
    ghost.right += 20;
    ghost.bottom += 16;

    /* Threshold using current button state (PS/2/USB safe) */
    Point last = startPt, cur;
    extern volatile UInt8 gCurrentButtons;

    while ((gCurrentButtons & 1) != 0) {
        GetMouse(&cur);
        if (abs(cur.h - startPt.h) >= kDragThreshold || abs(cur.v - startPt.v) >= kDragThreshold) {
            last = cur;
            serial_printf("TrackIconDragSync: threshold exceeded, starting drag\n");
            break;  /* start drag */
        }
        DesktopYield();
    }

    if ((gCurrentButtons & 1) == 0) {  /* released before threshold */
        serial_printf("TrackIconDragSync: button released before threshold\n");
        gDraggingIconIndex = -1;
        gInMouseTracking = false;
        return;
    }

    /* Enter drag: draw XOR ghost outline following cursor */
    GetPort(&savePort);
    SetPort(qd.thePort);  /* use screen port */
    ClipRect(&qd.screenBits.bounds);
    GhostShowAt(&ghost);  /* visible immediately */
    serial_printf("TrackIconDragSync: ghost visible, entering drag loop\n");

    while ((gCurrentButtons & 1) != 0) {
        DesktopYield();  /* keeps input flowing but we don't paint desktop */
        GetMouse(&cur);

        if (cur.h | cur.v) {  /* cheap branch predictor nudge */
            if (cur.h != last.h || cur.v != last.v) {
                /* Move by per-frame delta (cur - last, not cur - start) */
                OffsetRect(&ghost, cur.h - last.h, cur.v - last.v);
                didDrag = true;  /* Icon actually moved */

                /* Constrain to desktop bounds */
                Rect desk = qd.screenBits.bounds;
                desk.top = 20;  /* Below menu bar */

                if (ghost.left < desk.left)     OffsetRect(&ghost, desk.left - ghost.left, 0);
                if (ghost.top < desk.top)       OffsetRect(&ghost, 0, desk.top - ghost.top);
                if (ghost.right > desk.right)   OffsetRect(&ghost, desk.right - ghost.right, 0);
                if (ghost.bottom > desk.bottom) OffsetRect(&ghost, 0, desk.bottom - ghost.bottom);

                GhostShowAt(&ghost);  /* erase old, draw new */
                last = cur;
            }
        }
    }

    /* Button released: erase ghost, restore port */
    GhostEraseIf();
    SetPort(savePort);
    serial_printf("TrackIconDragSync: drag complete, ghost erased\n");

    /* Determine drop target and action */
    Point dropPoint = (Point){ .h = ghost.left + 20 + 16, .v = ghost.top + 16 };  /* Icon center */
    DesktopItem* item = &gDesktopIcons[iconIndex];
    Boolean operationSucceeded = false;
    Boolean invalidDrop = false;

    /* Get modifier keys to determine action (option = alias, cmd = copy) */
    extern void GetKeys(KeyMap theKeys);
    KeyMap keys;
    GetKeys(keys);
    /* Check for option key (0x3A = option key scancode, byte 7, bit 2) */
    Boolean optionKeyDown = (keys[7] & 0x04) != 0;
    Boolean cmdKeyDown = (keys[7] & 0x80) != 0;  /* Cmd key */

    serial_printf("TrackIconDragSync: Modifiers - option=%d cmd=%d\n", optionKeyDown, cmdKeyDown);

    /* Check drop targets in priority order */
    Boolean droppedOnTrash = Desktop_IsOverTrash(dropPoint);

    if (droppedOnTrash && iconIndex != 0 && item->type != kDesktopItemVolume) {
        /* DROP TARGET: Trash - always move to trash folder with name conflict resolution */
        serial_printf("TrackIconDragSync: Dropped on trash! Moving to trash folder\n");

        extern bool Trash_MoveNode(VRefNum vref, DirID parent, FileID id);
        extern VRefNum VFS_GetBootVRef(void);

        if (item->iconID != 0xFFFFFFFF) {
            VRefNum vref = VFS_GetBootVRef();
            if (Trash_MoveNode(vref, HFS_ROOT_DIR_ID, item->iconID)) {
                serial_printf("TrackIconDragSync: Successfully moved to trash\n");
                operationSucceeded = true;

                /* Remove from desktop array */
                for (short i = iconIndex; i < gDesktopIconCount - 1; i++) {
                    gDesktopIcons[i] = gDesktopIcons[i + 1];
                }
                gDesktopIconCount--;
                gSelectedIcon = -1;
            } else {
                serial_printf("TrackIconDragSync: Trash operation failed\n");
            }
        }
    } else if (droppedOnTrash && (iconIndex == 0 || item->type == kDesktopItemVolume)) {
        /* Cannot trash the trash itself or volume icons */
        serial_printf("TrackIconDragSync: Cannot trash this item\n");
        invalidDrop = true;
    } else {
        /* DROP TARGET: Desktop or folder window */
        extern short FindWindow(Point thePoint, WindowPtr* theWindow);
        extern VRefNum VFS_GetVRefByID(FileID id);
        extern VRefNum VFS_GetBootVRef(void);
        extern bool VFS_GetParentDir(VRefNum vref, FileID id, DirID* parentDir);
        extern bool VFS_Copy(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName, FileID* newID);
        extern OSErr CreateAlias(FSSpec* target, FSSpec* aliasFile);

        WindowPtr hitWindow = NULL;
        short partCode = FindWindow(dropPoint, &hitWindow);

        /* Check if dropped on a folder window */
        Boolean droppedOnFolder = (hitWindow != NULL && partCode == inContent);
        DirID targetDir = HFS_ROOT_DIR_ID;  /* Default to desktop (root) */
        VRefNum vref = VFS_GetBootVRef();

        if (droppedOnFolder) {
            /* TODO: Get actual folder window's DirID from window refCon */
            serial_printf("TrackIconDragSync: Dropped on folder window\n");
            /* For now, treat as desktop drop */
            droppedOnFolder = false;
        }

        /* Get source volume and directory */
        VRefNum sourceVRef = VFS_GetVRefByID(item->iconID);
        DirID sourceDir = HFS_ROOT_DIR_ID;
        VFS_GetParentDir(sourceVRef, item->iconID, &sourceDir);

        /* Determine if cross-volume */
        Boolean crossVolume = (sourceVRef != vref);

        if (optionKeyDown) {
            /* Option key: create alias */
            serial_printf("TrackIconDragSync: Creating alias\n");

            FSSpec target, aliasFile;
            target.vRefNum = sourceVRef;
            target.parID = sourceDir;
            /* Copy name from item */
            int nameLen = strlen(item->name);
            if (nameLen > 31) nameLen = 31;
            memcpy(target.name, item->name, nameLen);
            target.name[nameLen] = 0;

            /* Create alias on desktop with " alias" suffix */
            aliasFile.vRefNum = vref;
            aliasFile.parID = targetDir;
            char aliasName[64];
            snprintf(aliasName, sizeof(aliasName), "%s alias", item->name);
            if (strlen(aliasName) > 31) aliasName[31] = 0;
            memcpy(aliasFile.name, aliasName, strlen(aliasName) + 1);

            if (CreateAlias(&target, &aliasFile) == noErr) {
                serial_printf("TrackIconDragSync: Alias created successfully\n");
                operationSucceeded = true;
            } else {
                serial_printf("TrackIconDragSync: Alias creation failed\n");
                invalidDrop = true;
            }
        } else if (cmdKeyDown || crossVolume) {
            /* Cmd key or cross-volume: force copy */
            serial_printf("TrackIconDragSync: Copying file (cmd=%d, crossVol=%d)\n", cmdKeyDown, crossVolume);

            char copyName[32];
            extern bool VFS_GenerateUniqueName(VRefNum vref, DirID dir, const char* base, char* out);
            VFS_GenerateUniqueName(vref, targetDir, item->name, copyName);

            FileID newID = 0;
            if (VFS_Copy(vref, sourceDir, item->iconID, targetDir, copyName, &newID)) {
                serial_printf("TrackIconDragSync: Copy succeeded, newID=%u\n", newID);
                operationSucceeded = true;
            } else {
                serial_printf("TrackIconDragSync: Copy operation failed\n");
                invalidDrop = true;
            }
        } else {
            /* No modifiers: default behavior (move on same volume or reposition desktop icon) */
            if (droppedOnFolder) {
                /* Move to folder */
                extern bool VFS_Move(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName);
                if (VFS_Move(vref, sourceDir, item->iconID, targetDir, NULL)) {
                    serial_printf("TrackIconDragSync: Moved to folder\n");
                    operationSucceeded = true;
                    /* Remove from desktop */
                    for (short i = iconIndex; i < gDesktopIconCount - 1; i++) {
                        gDesktopIcons[i] = gDesktopIcons[i + 1];
                    }
                    gDesktopIconCount--;
                    gSelectedIcon = -1;
                }
            } else if (gDesktopIcons[iconIndex].movable) {
                /* Reposition on desktop */
                Point snapped = (Point){ .h = ghost.left + 20, .v = ghost.top };
                snapped = SnapToGrid(snapped);
                gDesktopIcons[iconIndex].position = snapped;
                serial_printf("TrackIconDragSync: Repositioned to (%d,%d)\n", snapped.h, snapped.v);
                operationSucceeded = true;
            }
        }
    }

    /* Handle operation result */
    if (invalidDrop) {
        /* Invalid drop: beep and restore to original position */
        serial_printf("TrackIconDragSync: Invalid drop - would beep here\n");
        extern void SysBeep(short duration);
        SysBeep(1);  /* System beep */
        /* Icon position already unchanged for invalid drops */
    }

    /* Post updateEvt to defer desktop redraw (no reentrancy) */
    extern OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
    PostEvent(updateEvt, 0);  /* desktop repaint */
    serial_printf("TrackIconDragSync: posted updateEvt\n");

    /* If we actually dragged, clear same-icon tracking to prevent accidental open */
    if (didDrag) {
        serial_printf("TrackIconDragSync: didDrag=true, clearing sLastClickIcon\n");
        sLastClickIcon = -1;
    }

    /* Clear tracking guard and drag flag */
    serial_printf("TrackIconDragSync: clearing tracking flags\n");
    gDraggingIconIndex = -1;
    gInMouseTracking = false;

    /* Drain any queued mouseUp that may have been posted anyway */
    serial_printf("TrackIconDragSync: checking for queued mouseUp\n");
    EventRecord e;
    if (EventAvail(mUpMask, &e)) {
        serial_printf("TrackIconDragSync: draining mouseUp event\n");
        EventRecord dump;
        GetNextEvent(mUpMask, &dump);
    }
    serial_printf("TrackIconDragSync: EXIT\n");
}

/*
 * LoadDesktopDatabase - Load desktop positions from database file

 */
static OSErr LoadDesktopDatabase(short vRefNum)
{
    OSErr err;
    FSSpec databaseSpec;
    short databaseRefNum;
    long dataSize;

    /* Create FSSpec for desktop database */
    err = FSMakeFSSpec(vRefNum, fsRtDirID, kDesktopDatabaseName, &databaseSpec);
    if (err != noErr) {
        return err; /* Database doesn't exist */
    }

    /* Open database file */
    err = FSpOpenDF(&databaseSpec, fsRdPerm, &databaseRefNum);
    if (err != noErr) {
        return err;
    }

    /* Read icon count */
    dataSize = sizeof(short);
    err = FSRead(databaseRefNum, &dataSize, &gDesktopIconCount);
    if (err != noErr) {
        FSClose(databaseRefNum);
        return err;
    }

    /* Validate icon count */
    if (gDesktopIconCount > kMaxDesktopIcons) {
        gDesktopIconCount = kMaxDesktopIcons;
    }

    /* Read desktop items */
    if (gDesktopIconCount > 0) {
        dataSize = sizeof(DesktopItem) * gDesktopIconCount;
        err = FSRead(databaseRefNum, &dataSize, gDesktopIcons);
    }

    FSClose(databaseRefNum);
    return err;
}

/*
 * SaveDesktopDatabase - Save desktop positions to database file

 */
static OSErr SaveDesktopDatabase(short vRefNum)
{
    OSErr err;
    FSSpec databaseSpec;
    short databaseRefNum;
    long dataSize;

    /* Create FSSpec for desktop database */
    err = FSMakeFSSpec(vRefNum, fsRtDirID, kDesktopDatabaseName, &databaseSpec);
    if (err == fnfErr) {
        /* Create database file */
        err = FSpCreate(&databaseSpec, 'DMGR', 'DTBS', smSystemScript);
        if (err != noErr && err != dupFNErr) {
            return err;
        }
    } else if (err != noErr) {
        return err;
    }

    /* Open database file */
    err = FSpOpenDF(&databaseSpec, fsWrPerm, &databaseRefNum);
    if (err != noErr) {
        return err;
    }

    /* Write icon count */
    dataSize = sizeof(short);
    err = FSWrite(databaseRefNum, &dataSize, &gDesktopIconCount);
    if (err != noErr) {
        FSClose(databaseRefNum);
        return err;
    }

    /* Write desktop items */
    if (gDesktopIconCount > 0) {
        dataSize = sizeof(DesktopItem) * gDesktopIconCount;
        err = FSWrite(databaseRefNum, &dataSize, gDesktopIcons);
    }

    FSClose(databaseRefNum);
    return err;
}

/*
 * ScanDirectoryForDesktopEntries - Recursively scan directory for desktop database

 */
static OSErr ScanDirectoryForDesktopEntries(short vRefNum, long dirID, short databaseRefNum)
{
    OSErr err = noErr;
    CInfoPBRec pb;
    Str255 itemName;
    short itemIndex = 1;
    DesktopRecord record;
    long dataSize;

    /* Scan all items in directory */
    do {
        pb.ioCompletion = nil;
        pb.ioNamePtr = itemName;
        pb.ioVRefNum = vRefNum;
        pb.u.hFileInfo.ioDirID = dirID;
        pb.u.hFileInfo.ioFDirIndex = itemIndex;

        err = PBGetCatInfoSync(&pb);
        if (err == noErr) {
            /* Create desktop record for this item */
            record.recordType = (pb.u.hFileInfo.ioFlAttrib & ioDirMask) ? 1 : 0; /* Folder or file */
            record.fileType = pb.u.hFileInfo.ioFlFndrInfo.fdType;
            record.creator = pb.u.hFileInfo.ioFlFndrInfo.fdCreator;
            record.iconLocalID = 0; /* Will be assigned later */
            record.iconType = 0;

            /* Write record to database */
            dataSize = sizeof(DesktopRecord);
            FSWrite(databaseRefNum, &dataSize, &record);

            /* If it's a folder, recurse into it */
            if (pb.u.hFileInfo.ioFlAttrib & ioDirMask) {
                ScanDirectoryForDesktopEntries(vRefNum, pb.u.dirInfo.ioDrDirID, databaseRefNum);
            }

            itemIndex++;
        }
    } while (err == noErr);

    /* fnfErr is expected when we've processed all items */
    return (err == fnfErr) ? noErr : err;
}

/*

/*
 * InitializeVolumeIcon - Initialize boot volume icon on desktop
 * Integrates with VFS to show Macintosh HD
 */
OSErr InitializeVolumeIcon(void)
{
    OSErr err = noErr;
    VolumeControlBlock vcb;

    /* Get boot volume reference */
    gBootVolumeRef = VFS_GetBootVRef();

    /* Get volume info */
    if (!VFS_GetVolumeInfo(gBootVolumeRef, &vcb)) {
        return ioErr;
    }

    /* Ensure desktop icons array is allocated */
    if (gDesktopIcons == nil) {
        err = AllocateDesktopIcons();
        if (err != noErr) return err;
    }

    /* Add volume icon to desktop (note: trash is already at index 0) */
    if (gDesktopIconCount < kMaxDesktopIcons) {
        /* Position at top-right of desktop */
        gDesktopIcons[gDesktopIconCount].type = kDesktopItemVolume;
        gDesktopIcons[gDesktopIconCount].iconID = 0xFFFFFFFF; /* Special ID for volume */
        gDesktopIcons[gDesktopIconCount].position.h = fb_width - 100;
        gDesktopIcons[gDesktopIconCount].position.v = 60;
        strcpy(gDesktopIcons[gDesktopIconCount].name, "Macintosh HD");
        gDesktopIcons[gDesktopIconCount].movable = true;  /* Volumes can be moved */
        gDesktopIcons[gDesktopIconCount].data.volume.vRefNum = gBootVolumeRef;

        serial_printf("InitializeVolumeIcon: Added volume icon at index %d, pos=(%d,%d)\n",
                      gDesktopIconCount, fb_width - 100, 60);

        gDesktopIconCount++;
        gVolumeIconVisible = true;

        serial_printf("InitializeVolumeIcon: gDesktopIconCount now = %d\n", gDesktopIconCount);
    }

    return noErr;
}

/*
 * Desktop_AddVolumeIcon - Add a volume icon to the desktop
 * Called when a new volume is mounted
 *
 * Parameters:
 *   name - Volume name to display
 *   vref - Volume reference number
 *
 * Returns: OSErr (noErr on success)
 */
OSErr Desktop_AddVolumeIcon(const char* name, VRefNum vref) {
    extern uint32_t fb_width, fb_height;

    if (!name || gDesktopIconCount >= kMaxDesktopIcons) {
        return paramErr;
    }

    /* Ensure desktop icons array is allocated */
    if (gDesktopIcons == nil) {
        OSErr err = AllocateDesktopIcons();
        if (err != noErr) return err;
    }

    /* Calculate position (stack volumes on right side) */
    /* Count existing volume icons to calculate Y position */
    int volumeCount = 0;
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].type == kDesktopItemVolume) {
            volumeCount++;
        }
    }

    /* Add volume icon */
    DesktopItem* item = &gDesktopIcons[gDesktopIconCount];
    item->type = kDesktopItemVolume;
    item->iconID = 0xFFFFFFFF; /* Special ID for volume */
    item->position.h = fb_width - 100;
    item->position.v = 60 + (volumeCount * 80); /* Stack vertically */
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';
    item->movable = true;
    item->data.volume.vRefNum = vref;

    serial_printf("Desktop_AddVolumeIcon: Added '%s' (vRef %d) at index %d, pos=(%d,%d)\n",
                  name, vref, gDesktopIconCount, item->position.h, item->position.v);

    gDesktopIconCount++;

    /* Desktop will be redrawn on next event loop iteration */

    return noErr;
}

/*
 * Desktop_RemoveVolumeIcon - Remove a volume icon from the desktop
 * Called when a volume is unmounted
 *
 * Parameters:
 *   vref - Volume reference number to remove
 *
 * Returns: OSErr (noErr on success)
 */
OSErr Desktop_RemoveVolumeIcon(VRefNum vref) {
    if (!gDesktopIcons) {
        return paramErr;
    }

    /* Find the volume icon */
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].type == kDesktopItemVolume &&
            gDesktopIcons[i].data.volume.vRefNum == vref) {

            serial_printf("Desktop_RemoveVolumeIcon: Removing volume icon for vRef %d at index %d\n", vref, i);

            /* Shift remaining icons down */
            for (int j = i; j < gDesktopIconCount - 1; j++) {
                gDesktopIcons[j] = gDesktopIcons[j + 1];
            }

            gDesktopIconCount--;

            /* Desktop will be redrawn on next event loop iteration */

            return noErr;
        }
    }

    serial_printf("Desktop_RemoveVolumeIcon: Volume icon for vRef %d not found\n", vref);
    return fnfErr; /* Not found */
}

/*
 * Desktop_AddAliasIcon - Add an alias icon to the desktop (PUBLIC API)
 * Creates a desktop alias/shortcut pointing to a file or folder
 *
 * Parameters:
 *   name      - Display name for the alias
 *   position  - Position on desktop (global coordinates)
 *   targetID  - FileID of the target file/folder
 *   vref      - Volume reference containing the target
 *   isFolder  - true if target is a folder, false for file
 *
 * Returns: OSErr (noErr on success)
 */
OSErr Desktop_AddAliasIcon(const char* name, Point position, FileID targetID,
                           VRefNum vref, Boolean isFolder) {
    if (!name || gDesktopIconCount >= kMaxDesktopIcons) {
        return paramErr;
    }

    serial_printf("Desktop_AddAliasIcon: Creating alias '%s' at (%d,%d), targetID=%d, vref=%d\n",
                 name, position.h, position.v, targetID, vref);

    /* Add to desktop icons array */
    DesktopItem* item = &gDesktopIcons[gDesktopIconCount];

    item->type = isFolder ? kDesktopItemFolder : kDesktopItemAlias;
    item->iconID = targetID;  /* Use FileID as iconID */
    item->position = position;
    item->movable = true;  /* Aliases are movable */

    /* Copy name (ensure null termination) */
    size_t nameLen = strlen(name);
    if (nameLen >= 64) nameLen = 63;
    memcpy(item->name, name, nameLen);
    item->name[nameLen] = '\0';

    /* Store alias-specific data */
    if (isFolder) {
        item->data.folder.dirID = (long)targetID;
    } else {
        item->data.alias.targetID = (long)targetID;
    }

    gDesktopIconCount++;

    serial_printf("Desktop_AddAliasIcon: Added alias at index %d, total icons now = %d\n",
                 gDesktopIconCount - 1, gDesktopIconCount);

    /* Request desktop redraw */
    extern OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
    PostEvent(updateEvt, 0);  /* 0 = desktop update */

    return noErr;
}

/*
 * Desktop_IsOverTrash - Check if a point is over the trash icon (PUBLIC API)
 * Used for drag-and-drop trash detection
 *
 * Parameters:
 *   where - Point to test (in global coordinates)
 *
 * Returns: true if point is over trash icon or label, false otherwise
 */
Boolean Desktop_IsOverTrash(Point where) {
    /* Trash is always at index 0 */
    if (gDesktopIconCount == 0 || gDesktopIcons[0].type != kDesktopItemTrash) {
        return false;
    }

    Rect iconRect, labelRect;

    /* Calculate trash icon rect (32x32) */
    SetRect(&iconRect,
            gDesktopIcons[0].position.h,
            gDesktopIcons[0].position.v,
            gDesktopIcons[0].position.h + kIconW,
            gDesktopIcons[0].position.v + kIconH);

    /* Calculate trash label rect (trash has labelOffset=48) */
    SetRect(&labelRect,
            gDesktopIcons[0].position.h - 20,
            gDesktopIcons[0].position.v + 48,
            gDesktopIcons[0].position.h + kIconW + 20,
            gDesktopIcons[0].position.v + 48 + 16);

    return (PtInRect(where, &iconRect) || PtInRect(where, &labelRect));
}

/*
 * DrawVolumeIcon - Draw the boot volume icon on desktop using universal icon system
 */
void DrawVolumeIcon(void)
{
    static Boolean gInVolumeIconPaint = false;
    IconHandle iconHandle;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("DrawVolumeIcon: ENTRY\n");

    /* Erase any active ghost outline before icon redraw */
    GhostEraseIf();

    /* Re-entrancy guard: prevent recursive painting */
    if (gInVolumeIconPaint) {
        serial_printf("DrawVolumeIcon: re-entry detected, skipping to avoid freeze\n");
        return;
    }
    gInVolumeIconPaint = true;

    if (!gVolumeIconVisible) {
        serial_printf("DrawVolumeIcon: not visible, returning\n");
        gInVolumeIconPaint = false;
        return;
    }

    serial_printf("DrawVolumeIcon: Drawing all desktop items\n");

    /* Draw all desktop items */
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].type == kDesktopItemVolume) {
            /* Draw volume icon */
            serial_printf("DrawVolumeIcon: Drawing volume at index %d, pos=(%d,%d)\n",
                         i, gDesktopIcons[i].position.h, gDesktopIcons[i].position.v);

            iconHandle.fam = IconSys_DefaultVolume();
            iconHandle.selected = (gSelectedIcon == i);

            Icon_DrawWithLabelOffset(&iconHandle, gDesktopIcons[i].name,
                                    gDesktopIcons[i].position.h + 16,  /* Center X */
                                    gDesktopIcons[i].position.v,        /* Top Y */
                                    34,                                 /* Label offset */
                                    iconHandle.selected);
        } else if (gDesktopIcons[i].type == kDesktopItemTrash) {
            /* Draw trash icon */
            extern const IconFamily* IconSys_TrashEmpty(void);
            extern const IconFamily* IconSys_TrashFull(void);
            extern bool Trash_IsEmptyAll(void);

            IconHandle trashHandle;
            trashHandle.fam = Trash_IsEmptyAll() ? IconSys_TrashEmpty() : IconSys_TrashFull();
            trashHandle.selected = (gSelectedIcon == i);

            serial_printf("DrawVolumeIcon: Drawing trash at (%d,%d)\n",
                         gDesktopIcons[i].position.h, gDesktopIcons[i].position.v);

            Icon_DrawWithLabelOffset(&trashHandle, gDesktopIcons[i].name,
                                    gDesktopIcons[i].position.h + 16,  /* Center X */
                                    gDesktopIcons[i].position.v,        /* Top Y */
                                    48,                                 /* Label offset */
                                    trashHandle.selected);
        }
        /* Future: handle kDesktopItemFile, kDesktopItemFolder, etc. */
    }
    serial_printf("DrawVolumeIcon: about to return\n");
    gInVolumeIconPaint = false;
    return;  /* Explicit return for debugging */
}
/* DrawVolumeIcon function ends here */

/*
 * HandleDesktopClick - Check if a click is on a desktop icon and handle it
 * Returns true if click was on an icon, false otherwise
 */
Boolean HandleDesktopClick(Point clickPoint, Boolean doubleClick)
{
    short prevSelected = gSelectedIcon;
    short hitIcon;
    WindowPtr whichWindow;

    extern void serial_printf(const char* fmt, ...);
    extern short FindWindow(Point thePoint, WindowPtr* theWindow);

    serial_printf("HandleDesktopClick: click at (%d,%d), doubleClick=%d\n",
                  clickPoint.h, clickPoint.v, doubleClick);

    /* System 7 faithful: only handle if click is on desktop (not a window) */
    short part = FindWindow(clickPoint, &whichWindow);
    if (part != inDesk) {
        /* Click was on a window, not desktop - let Window Manager handle it */
        serial_printf("HandleDesktopClick: part=%d (not inDesk), ignoring\n", part);
        return false;
    }

    /* Ensure we're in screen port for global coordinate hit testing */
    extern QDGlobals qd;
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(qd.thePort);

    /* Find which icon was hit */
    hitIcon = IconAtPoint(clickPoint);

    if (hitIcon == -1) {
        /* No icon hit - deselect and clear same-icon tracking */
        if (prevSelected != -1) {
            gSelectedIcon = -1;
            /* Don't redraw here - let Window Manager handle it via DeskHook */
        }
        sLastClickIcon = -1;
        sLastClickTicks = 0;
        SetPort(savePort);
        return false;
    }

    /* Check for double-click ourselves using time threshold (ignore broken event flag) */
    extern UInt32 TickCount(void);
    extern UInt32 GetDblTime(void);
    UInt32 currentTicks = TickCount();
    UInt32 timeSinceLastClick = currentTicks - sLastClickTicks;
    Boolean isDoubleClick = (hitIcon == sLastClickIcon && timeSinceLastClick <= GetDblTime());

    serial_printf("Hit icon index %d, same=%d, dt=%d, threshold=%d, dblClick=%d\n",
                 hitIcon, (hitIcon == sLastClickIcon), (int)timeSinceLastClick,
                 (int)GetDblTime(), (int)isDoubleClick);

    /* Double-click on SAME icon: open immediately, never drag */
    if (isDoubleClick && hitIcon >= 0 && hitIcon < gDesktopIconCount) {
        serial_printf("[DBLCLK SAME ICON] Opening icon %d\n", hitIcon);
        extern WindowPtr Finder_OpenDesktopItem(Boolean isTrash, ConstStr255Param title);
        DesktopItem *it = &gDesktopIcons[hitIcon];

        /* Ensure any ghost from prior drag is erased */
        GhostEraseIf();

        if (it->type == kDesktopItemVolume) {
            /* Build Pascal string for HD title */
            static unsigned char hdTitle[256];
            strcpy((char*)&hdTitle[1], "Macintosh HD");
            hdTitle[0] = (unsigned char)strlen("Macintosh HD");
            Finder_OpenDesktopItem(false, hdTitle);
        } else if (it->type == kDesktopItemTrash) {
            /* Build Pascal string for Trash title */
            static unsigned char trashTitle[256];
            strcpy((char*)&trashTitle[1], "Trash");
            trashTitle[0] = (unsigned char)strlen("Trash");
            Finder_OpenDesktopItem(true, trashTitle);
        }

        /* DO NOT post desktop updateEvt - the window will handle its own updates */
        /* Posting updateEvt(0) would trigger DrawDesktop which paints over windows! */
        sLastClickIcon = -1;  /* Consume double-click */
        SetPort(savePort);
        return true;
    }

    /* Single click or double-click on different icon: select and arm for drag */
    if (hitIcon >= 0 && hitIcon < gDesktopIconCount) {
        gSelectedIcon = hitIcon;

        /* Update same-icon tracking for next potential double-click */
        extern UInt32 TickCount(void);
        sLastClickIcon = hitIcon;
        sLastClickTicks = TickCount();

        extern OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
        PostEvent(updateEvt, 0);  /* selection redraw deferred */

        serial_printf("Single-click: icon %d selected, sLastClickIcon=%d\n", hitIcon, sLastClickIcon);

        extern volatile UInt8 gCurrentButtons;
        if ((gCurrentButtons & 1) != 0) {  /* mouse still down? arm drag */
            serial_printf("Single-click: button still down, starting drag tracking\n");
            SetPort(savePort);
            TrackIconDragSync(hitIcon, clickPoint);
        } else {
            serial_printf("Single-click: button released, no drag\n");
            SetPort(savePort);
        }
        return true;
    }

    SetPort(savePort);

    return true;
}

/*
 * HandleVolumeDoubleClick - Open volume window on double-click
 */
OSErr HandleVolumeDoubleClick(Point clickPoint)
{
    Rect iconRect;
    WindowPtr volumeWindow;
    VolumeControlBlock vcb;

    /* Check if click is on volume icon */
    for (int i = 0; i < gDesktopIconCount; i++) {
        if (gDesktopIcons[i].type == kDesktopItemVolume) {
            SetRect(&iconRect,
                    gDesktopIcons[i].position.h,
                    gDesktopIcons[i].position.v,
                    gDesktopIcons[i].position.h + 32,
                    gDesktopIcons[i].position.v + 32);

            if (PtInRect(clickPoint, &iconRect)) {
                /* Get volume info */
                if (!VFS_GetVolumeInfo(gBootVolumeRef, &vcb)) {
                    return ioErr;
                }

                /* Open root directory window */
                /* TODO: Create folder window for root directory */
                /* serial_printf("Opening volume: %s (root ID=%d)\n", vcb.name, vcb.rootID); */

                return noErr;
            }
        }
    }

    return fnfErr;
}

/*
 * StartDragIcon - Begin dragging an icon
 */
void StartDragIcon(Point mousePt)
{
    extern void serial_printf(const char* fmt, ...);

    if (gSelectedIcon >= 0 && gSelectedIcon < gDesktopIconCount) {
        if (gDesktopIcons[gSelectedIcon].movable) {
            gDraggingIcon = true;
            serial_printf("Started dragging icon %d\n", gSelectedIcon);
        } else {
            serial_printf("Cannot drag non-movable icon %d\n", gSelectedIcon);
        }
    }
}

/*
 * DragIcon - Update icon position during drag
 */
void DragIcon(Point mousePt)
{
    if (!gDraggingIcon || gSelectedIcon < 0 || gSelectedIcon >= gDesktopIconCount) {
        return;
    }

    /* Calculate new position based on mouse and offset */
    Point newPos;
    newPos.h = mousePt.h - gDragOffset.h;
    newPos.v = mousePt.v - gDragOffset.v;

    /* Constrain to screen bounds */
    if (newPos.h < 10) newPos.h = 10;
    if (newPos.v < 30) newPos.v = 30;  /* Below menu bar */
    if (newPos.h > fb_width - 42) newPos.h = fb_width - 42;
    if (newPos.v > fb_height - 60) newPos.v = fb_height - 60;

    /* Update icon position */
    gDesktopIcons[gSelectedIcon].position = newPos;

    /* Redraw desktop to show new position */
    DrawDesktop();
    DrawVolumeIcon();
}

/*
 * EndDragIcon - Finish dragging an icon
 */
void EndDragIcon(Point mousePt)
{
    extern void serial_printf(const char* fmt, ...);

    if (!gDraggingIcon) {
        return;
    }

    gDraggingIcon = false;

    if (gSelectedIcon >= 0 && gSelectedIcon < gDesktopIconCount) {
        serial_printf("Finished dragging icon %d to (%d,%d)\n",
                     gSelectedIcon,
                     gDesktopIcons[gSelectedIcon].position.h,
                     gDesktopIcons[gSelectedIcon].position.v);

        /* Save the new position */
        SaveDesktopDatabase(0);
    }
}

/*
 * HandleDesktopDrag - Handle mouse drag on desktop
 * Returns true if dragging an icon
 */
Boolean HandleDesktopDrag(Point mousePt, Boolean buttonDown)
{
    if (gDraggingIcon && buttonDown) {
        /* Continue dragging */
        DragIcon(mousePt);
        return true;
    } else if (gDraggingIcon && !buttonDown) {
        /* End drag */
        EndDragIcon(mousePt);
        return true;
    }

    return false;
}

/*
 * SelectNextDesktopIcon - Cycle through desktop icons with Tab key
 */
void SelectNextDesktopIcon(void)
{
    extern void serial_printf(const char* fmt, ...);

    serial_printf("SelectNextDesktopIcon: current=%d, count=%d\n",
                  gSelectedIcon, gDesktopIconCount);

    if (gDesktopIconCount == 0) {
        return;
    }

    /* Advance to next icon, wrapping around */
    short prevSelected = gSelectedIcon;
    gSelectedIcon = (gSelectedIcon + 1) % gDesktopIconCount;

    serial_printf("SelectNextDesktopIcon: selected %d → %d, posting updateEvt\n",
                 prevSelected, gSelectedIcon);

    /* Post updateEvt to defer redraw to event loop (avoids re-entrancy freeze) */
    extern OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
    PostEvent(updateEvt, 0);  /* NULL window = desktop/background */
}

/*
 * OpenSelectedDesktopIcon - Open window for currently selected desktop icon
 */
void OpenSelectedDesktopIcon(void)
{
    extern void serial_printf(const char* fmt, ...);

    serial_printf("OpenSelectedDesktopIcon: selected=%d, count=%d\n",
                  gSelectedIcon, gDesktopIconCount);

    if (gSelectedIcon < 0 || gSelectedIcon >= gDesktopIconCount) {
        serial_printf("OpenSelectedDesktopIcon: No icon selected\n");
        return;
    }

    /* Check if it's the volume icon */
    if (gDesktopIcons[gSelectedIcon].type == kDesktopItemVolume) {
        serial_printf("OpenSelectedDesktopIcon: Opening volume window\n");

        /* Create a folder window for the volume */
        Rect windowBounds;
        SetRect(&windowBounds, 100, 60, 500, 360);

        WindowPtr volumeWindow = NewWindow(NULL, &windowBounds,
                                          "\014Macintosh HD",
                                          true,  /* visible */
                                          0,     /* documentProc */
                                          (WindowPtr)-1L,  /* frontmost */
                                          true,  /* goAway box */
                                          'DISK');  /* refCon to identify as disk window */

        if (volumeWindow) {
            ShowWindow(volumeWindow);
            SelectWindow(volumeWindow);

            /* Force update event */
            InvalRect(&volumeWindow->port.portRect);

            serial_printf("OpenSelectedDesktopIcon: Volume window created successfully\n");
        } else {
            serial_printf("OpenSelectedDesktopIcon: Failed to create window\n");
        }
    } else if (gDesktopIcons[gSelectedIcon].type == kDesktopItemTrash) {
        serial_printf("OpenSelectedDesktopIcon: Opening trash window\n");

        /* Create a trash window */
        Rect windowBounds;
        SetRect(&windowBounds, 200, 120, 600, 420);

        WindowPtr trashWindow = (WindowPtr)NewWindow(NULL, &windowBounds,
                                         "\005Trash",
                                         true,  /* visible */
                                         0,     /* documentProc */
                                         (WindowPtr)-1L,  /* frontmost */
                                         true,  /* goAway box */
                                         'TRSH');  /* refCon to identify as trash window */

        if (trashWindow) {
            ShowWindow(trashWindow);
            SelectWindow(trashWindow);

            /* Force update event */
            InvalRect(&trashWindow->port.portRect);

            serial_printf("OpenSelectedDesktopIcon: Trash window created successfully\n");
        } else {
            serial_printf("OpenSelectedDesktopIcon: Failed to create trash window\n");
        }
    }
}
