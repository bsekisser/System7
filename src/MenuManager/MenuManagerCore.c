#include <stdlib.h>
#include <string.h>
/*
 * MenuManagerCore.c - Core Menu Manager Implementation
 *
 * This file implements the core Menu Manager functionality including
 * menu creation, disposal, menu bar management, and the fundamental
 * menu operations. This is THE FINAL CRITICAL COMPONENT that completes
 * the essential Mac OS interface for System 7.1 compatibility.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis 7.1 Menu Manager
 */

#include "../include/MacTypes.h"
#include "../include/QuickDraw/QuickDraw.h"
#include "../include/QuickDrawConstants.h"

#include "../include/MenuManager/MenuManager.h"
#include "../include/MenuManager/MenuTypes.h"

/* QuickDraw globals */
extern QDGlobals qd;


/* ============================================================================
 * Menu Manager Types and Structures
 * ============================================================================ */

/* Menu list entry */
typedef struct MenuListEntry {
    short menuID;
    short menuLeft;
    short menuWidth;
} MenuListEntry;

/* Define struct Menu to match MenuHandle definition in SystemTypes.h */
struct Menu {
    SInt16   menuID;
    SInt16   menuWidth;
    SInt16   menuHeight;
    Handle   menuProc;
    SInt32   enableFlags;
    Str255   menuData;
};

/* Menu bar list structure */
typedef struct MenuBarList {
    short numMenus;
    short totalWidth;
    short lastRight;
    short mbResID;
    MenuListEntry menus[1];  /* Variable length array */
} MenuBarList;

/* Menu error codes */
#define menuInvalidErr -150
#define hierMenu -1

/* ============================================================================
 * Menu Manager State Structure
 * ============================================================================ */

typedef struct MenuManagerState {
    Boolean initialized;
    Boolean menuBarVisible;
    Handle menuBar;
    Handle menuList;
    short hiliteMenu;
    Boolean menuBarInvalid;
    short currentMenuBar;
    short menuBarHeight;
    Handle menuColorTable;
    short menuFlash;
    long lastMenuChoice;
    Boolean trackingMenu;
    MenuHandle currentMenu;
    short currentItem;
    void* platformData;
} MenuManagerState;

/* ============================================================================
 * Global Menu Manager State
 * ============================================================================ */

static MenuManagerState* gMenuMgrState = NULL;
static Boolean gMenuMgrInitialized = false;

/* Menu bar constants */
#define menuBarStdHeight 20
#define kMenuBarHeight 20

/* Low memory globals (for compatibility) */
static short gMBarHeight = 20;          /* Menu bar height */
static Handle gMenuList = NULL;         /* Current menu list */
static MCTableHandle gMCTable = NULL;   /* Menu color table */
static short gMenuFlash = 3;            /* Flash count */
static long gLastMenuChoice = 0;        /* Last menu choice */

/* Simple menu handle tracking */
#define MAX_MENUS 32
static struct {
    short menuID;
    MenuHandle handle;
} gMenuHandles[MAX_MENUS];
static int gNumMenuHandles = 0;

/* Internal function prototypes */
static MenuManagerState* AllocateMenuManagerState(void);
static void InitializeMenuManagerState(MenuManagerState* state);
static void DisposeMenuManagerState(MenuManagerState* state);
static OSErr ValidateMenuHandle(MenuHandle theMenu);
static OSErr ValidateMenuID(short menuID);
static MenuHandle FindMenuInList(short menuID);
static void UpdateMenuBarLayout(void);
static void InvalidateMenuBar(void);

/* Platform function prototypes (implemented elsewhere) */
extern void Platform_InitMenuSystem(void);
extern void Platform_CleanupMenuSystem(void);
extern void Platform_DrawMenuBar(void);
extern void Platform_EraseMenuBar(void);

/* ============================================================================
 * Menu Manager Initialization and Cleanup
 * ============================================================================ */

/*
 * InitMenus - Initialize the Menu Manager
 *
 * This MUST be called before any other Menu Manager functions.
 * Sets up internal structures, initializes the menu bar, and prepares
 * the Menu Manager for use.
 */
void InitMenus(void)
{
    if (gMenuMgrInitialized) {
        return; /* Already initialized */
    }

    /* Allocate and initialize global state */
    gMenuMgrState = AllocateMenuManagerState();
    if (gMenuMgrState == NULL) {
        /* Critical failure - cannot continue */
        return;
    }

    InitializeMenuManagerState(gMenuMgrState);

    /* Initialize platform-specific menu system */
    Platform_InitMenuSystem();

    /* Set up standard menu bar */
    gMBarHeight = menuBarStdHeight;
    gMenuList = NULL;
    gMCTable = NULL;
    gMenuFlash = 3;
    gLastMenuChoice = 0;

    /* Mark as initialized */
    gMenuMgrInitialized = true;
    gMenuMgrState->initialized = true;

    /* Menu Manager initialized successfully */
}

/*
 * CleanupMenus - Clean up Menu Manager resources
 */
void CleanupMenus(void)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    /* Clean up platform-specific resources */
    Platform_CleanupMenuSystem();

    /* Dispose of menu color table */
    if (gMCTable != NULL) {
        DisposeMCInfo(gMCTable);
        gMCTable = NULL;
    }

    /* Clear menu list */
    if (gMenuList != NULL) {
        free(gMenuList);
        gMenuList = NULL;
    }

    /* Dispose of global state */
    if (gMenuMgrState != NULL) {
        DisposeMenuManagerState(gMenuMgrState);
        gMenuMgrState = NULL;
    }

    gMenuMgrInitialized = false;
}

/* ============================================================================
 * Menu Bar Management
 * ============================================================================ */

/*
 * GetMenuBar - Get current menu list
 */
Handle GetMenuBar(void)
{
    if (!gMenuMgrInitialized) {
        return NULL;
    }

    return gMenuList;
}

/*
 * GetNewMBar - Create menu list from MBAR resource
 */
Handle GetNewMBar(short menuBarID)
{
    if (!gMenuMgrInitialized) {
        return NULL;
    }

    /* TODO: Implement resource loading */
    /* For now, create an empty menu list */
    MenuBarList* menuBar = (MenuBarList*)malloc(sizeof(MenuBarList));
    if (menuBar == NULL) {
        return NULL;
    }

    menuBar->numMenus = 0;
    menuBar->totalWidth = 0;
    menuBar->lastRight = 0;
    menuBar->mbResID = menuBarID;

    return (Handle)menuBar;
}

/*
 * SetMenuBar - Set current menu list
 */
void SetMenuBar(Handle menuList)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    /* Dispose of old menu list if it exists */
    if (gMenuList != NULL && gMenuList != menuList) {
        free(gMenuList);
    }

    gMenuList = menuList;
    gMenuMgrState->menuList = menuList;

    /* Update menu bar display */
    UpdateMenuBarLayout();
    InvalidateMenuBar();
}

/*
 * ClearMenuBar - Remove all menus from menu bar
 */
void ClearMenuBar(void)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    if (gMenuList != NULL) {
        free(gMenuList);
        gMenuList = NULL;
        gMenuMgrState->menuList = NULL;
    }

    /* Clear menu bar display */
    InvalidateMenuBar();
}

/* Classic Macintosh Apple logo icon (16x16) */
/* This is a properly designed Apple logo for the menu bar */
static const uint8_t g_appleIconData[32] = {
    /* Row 0  */    0x00, 0x00,  /* ................ */
    /* Row 1  */    0x01, 0x80,  /* .......##....... */
    /* Row 2  */    0x01, 0x80,  /* .......##....... */
    /* Row 3  */    0x03, 0x00,  /* ......##........ */
    /* Row 4  */    0x07, 0x00,  /* .....###........ */
    /* Row 5  */    0x1F, 0xE0,  /* ...########..... */
    /* Row 6  */    0x3F, 0xF0,  /* ..##########.... */
    /* Row 7  */    0x7F, 0xF8,  /* .############... */
    /* Row 8  */    0x7F, 0xF8,  /* .############... */
    /* Row 9  */    0x7F, 0xF8,  /* .############... */
    /* Row 10 */    0x7F, 0xF8,  /* .############... */
    /* Row 11 */    0x7F, 0xF8,  /* .############... */
    /* Row 12 */    0x7F, 0xF8,  /* .############... */
    /* Row 13 */    0x3F, 0xF0,  /* ..##########.... */
    /* Row 14 */    0x1F, 0xE0,  /* ...########..... */
    /* Row 15 */    0x00, 0x00   /* ................ */
};

/* Draw Apple icon at specified position */
static void DrawAppleIcon(short x, short y) {
    extern void* framebuffer;
    extern uint32_t fb_width;
    extern uint32_t fb_height;
    extern uint32_t fb_pitch;
    extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

    /* serial_printf("DrawAppleIcon: x=%d, y=%d, fb=%p\n", x, y, framebuffer); */

    if (!framebuffer) {
        /* serial_printf("DrawAppleIcon: No framebuffer!\n"); */
        return;
    }

    uint32_t *fb = (uint32_t*)framebuffer;

    /* Classic rainbow Apple logo colors (top to bottom) */
    /* The classic 6-color Apple logo uses these colors in horizontal stripes */
    uint32_t colors[6] = {
        pack_color(97, 190, 61),    /* Green */
        pack_color(251, 186, 18),   /* Yellow */
        pack_color(255, 117, 20),   /* Orange */
        pack_color(232, 40, 42),    /* Red */
        pack_color(148, 36, 146),   /* Purple */
        pack_color(29, 169, 226)    /* Blue */
    };

    /* Draw 16x16 Apple icon with rainbow colors */
    for (int row = 0; row < 16; row++) {
        if (y + row >= fb_height) break;

        uint16_t row_data = (g_appleIconData[row*2] << 8) | g_appleIconData[row*2 + 1];

        /* Determine which color stripe this row belongs to */
        /* Divide 16 rows into 6 color bands */
        int color_index;
        if (row < 3) color_index = 0;      /* Green: rows 0-2 */
        else if (row < 6) color_index = 1; /* Yellow: rows 3-5 */
        else if (row < 8) color_index = 2; /* Orange: rows 6-7 */
        else if (row < 10) color_index = 3; /* Red: rows 8-9 */
        else if (row < 13) color_index = 4; /* Purple: rows 10-12 */
        else color_index = 5;               /* Blue: rows 13-15 */

        for (int col = 0; col < 16; col++) {
            if (x + col >= fb_width) break;

            if (row_data & (1 << (15 - col))) {
                int fb_offset = (y + row) * (fb_pitch / 4) + (x + col);
                fb[fb_offset] = colors[color_index];
            }
        }
    }
}

/*
 * DrawMenuBar - Redraw the menu bar
 */
void DrawMenuBar(void)
{
    extern void serial_puts(const char* str);
    serial_puts("DrawMenuBar called\n");

    if (!gMenuMgrInitialized) {
        serial_puts("DrawMenuBar: MenuManager not initialized\n");
        return;
    }

    GrafPtr savePort;
    GetPort(&savePort);

    /* Make sure we have a valid port to draw to */
    if (qd.thePort) {
        SetPort(qd.thePort);  /* Draw to screen port */
        serial_puts("DrawMenuBar: SetPort called with qd.thePort\n");
    } else {
        serial_puts("DrawMenuBar: qd.thePort is NULL!\n");
    }

    /* Menu bar rectangle */
    Rect menuBarRect;
    SetRect(&menuBarRect, 0, 0, qd.screenBits.bounds.right, 20);

    /* Clear menu bar area with white */
    ClipRect(&menuBarRect);
    FillRect(&menuBarRect, &qd.white);  /* White background */

    /* Draw bottom line */
    MoveTo(0, 19);
    LineTo(qd.screenBits.bounds.right - 1, 19);

    /* Draw menu titles */
    short x = 10;
    serial_puts("DrawMenuBar: Checking menu state...\n");
    if (gMenuMgrState) {
        serial_puts("DrawMenuBar: gMenuMgrState exists\n");
        if (gMenuMgrState->menuBar) {
            MenuBarList* menuBar = (MenuBarList*)gMenuMgrState->menuBar;
            serial_puts("DrawMenuBar: menuBar exists\n");
            /* serial_printf might not be working, use serial_puts for now */

            for (int i = 0; i < menuBar->numMenus; i++) {
                serial_puts("DrawMenuBar: Processing menu\n");
                MenuHandle menu = GetMenuHandle(menuBar->menus[i].menuID);
                if (menu) {
                    serial_puts("DrawMenuBar: Menu handle found\n");

                    /* Debug: show MenuInfo offsets */
                    MenuInfo* mptr = *menu;
                    serial_printf("  MenuID: %d\n", mptr->menuID);
                    serial_printf("  sizeof(MenuInfo): %d\n", sizeof(MenuInfo));
                    serial_printf("  offsetof menuData: %d\n", ((char*)&(mptr->menuData) - (char*)mptr));

                    /* Get menu title */
                    unsigned char titleLen = (**menu).menuData[0];
                    serial_printf("DrawMenuBar: Got title length: %d (0x%02x)\n", titleLen, titleLen);

                    /* Also show first few bytes of menuData for debugging */
                    serial_printf("  menuData[0-7]: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                  (**menu).menuData[0], (**menu).menuData[1],
                                  (**menu).menuData[2], (**menu).menuData[3],
                                  (**menu).menuData[4], (**menu).menuData[5],
                                  (**menu).menuData[6], (**menu).menuData[7]);

                    /* Check if title is at wrong offset */
                    unsigned char alt_len = (**menu).menuData[1];
                    serial_printf("  Alt titleLen at [1]: %d\n", alt_len);
                    if (titleLen > 0 && titleLen < 50) { /* Sanity check title length */
                        /* Check if this is the Apple menu - only draw icon for ID 128 */
                        /* ID 1 appears to be a duplicate that should be ignored */
                        if (mptr->menuID == 128) {
                            /* Draw Apple icon instead of text */
                            /* serial_printf("Drawing Apple icon for menu %d at x=%d\n", mptr->menuID, x); */
                            DrawAppleIcon(x, 2);
                            x += 20;  /* Icon width + spacing */
                        } else if (mptr->menuID == 1) {
                            /* Skip menu ID 1 - it's a duplicate Apple menu */
                            /* serial_printf("Skipping duplicate Apple menu (ID 1)\n"); */
                            continue;
                        } else {
                            /* Draw normal text title */
                            MoveTo(x, 13);  /* Centered vertically in 20px menu bar */
                            serial_puts("Calling DrawText with title...\n");
                            DrawText((char*)&(**menu).menuData[1], 0, titleLen);
                            serial_puts("DrawText returned\n");
                            x += StringWidth(&(**menu).menuData) + 20;
                        }
                    }
                }
            }
        } else {
            serial_puts("DrawMenuBar: menuBar is NULL\n");
        }
    } else {
        serial_puts("DrawMenuBar: gMenuMgrState is NULL\n");
        /* Draw default Apple menu if no menus installed */
        MoveTo(x, 13);  /* Centered vertically */
        DrawAppleIcon(x, 2);  /* Draw Apple icon at top of menu bar */
    }

    SetPort(savePort);
    gMenuMgrState->menuBarInvalid = false;
}

/*
 * InvalMenuBar - Mark menu bar as needing redraw
 */
void InvalMenuBar(void)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    InvalidateMenuBar();
}

/*
 * HiliteMenu - Highlight a menu title
 */
void HiliteMenu(short menuID)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    gMenuMgrState->hiliteMenu = menuID;

    /* Update display */
    DrawMenuBar();
}

/* GetMBarHeight is defined as a macro in MenuManager.h */

/* ============================================================================
 * Menu Creation and Management
 * ============================================================================ */

/*
 * NewMenu - Create a new menu
 */
MenuHandle NewMenu(short menuID, ConstStr255Param menuTitle)
{
    MenuHandle theMenu;
    MenuInfo* menuPtr;
    size_t titleLen;

    if (!gMenuMgrInitialized) {
        return NULL;
    }

    if (ValidateMenuID(menuID) != 0) {
        return NULL;
    }

    /* Check if menu ID already exists */
    if (FindMenuInList(menuID) != NULL) {
        return NULL; /* Menu ID already in use */
    }

    /* Allocate menu handle */
    theMenu = (MenuHandle)malloc(sizeof(MenuInfo*));
    if (theMenu == NULL) {
        return NULL;
    }

    /* Allocate menu info */
    menuPtr = (MenuInfo*)malloc(sizeof(MenuInfo));
    if (menuPtr == NULL) {
        free(theMenu);
        return NULL;
    }

    /* Initialize menu info */
    memset(menuPtr, 0, sizeof(MenuInfo));
    *theMenu = menuPtr;

    menuPtr->menuID = menuID;
    menuPtr->menuWidth = 0;  /* Will be calculated */
    menuPtr->menuHeight = 0; /* Will be calculated */
    menuPtr->menuProc = NULL; /* Standard text menu */
    menuPtr->enableFlags = 0xFFFFFFFF; /* All items enabled initially */

    /* Copy menu title */
    titleLen = menuTitle[0];
    if (titleLen > 255) titleLen = 255;
    menuPtr->menuData[0] = (unsigned char)titleLen;
    memcpy(&menuPtr->menuData[1], &menuTitle[1], titleLen);
    /* Clear rest of menuData to prevent garbage */
    memset(&menuPtr->menuData[1 + titleLen], 0, 255 - titleLen);

    /* Created menu */

    /* Add to tracking array */
    if (gNumMenuHandles < MAX_MENUS) {
        gMenuHandles[gNumMenuHandles].menuID = menuID;
        gMenuHandles[gNumMenuHandles].handle = theMenu;
        gNumMenuHandles++;
    }

    return theMenu;
}

/*
 * GetMenu - Load menu from MENU resource
 */
MenuHandle GetMenu(short resourceID)
{
    if (!gMenuMgrInitialized) {
        return NULL;
    }

    /* TODO: Implement resource loading */
    /* For now, create a simple menu */
    unsigned char title[256];
    snprintf((char*)&title[1], 255, "Menu %d", resourceID);
    title[0] = strlen((char*)&title[1]);

    return NewMenu(resourceID, title);
}

/*
 * DisposeMenu - Dispose of a menu
 */
void DisposeMenu(MenuHandle theMenu)
{
    if (!gMenuMgrInitialized || theMenu == NULL) {
        return;
    }

    if (ValidateMenuHandle(theMenu) != 0) {
        return;
    }

    /* Remove from menu bar if present */
    DeleteMenu((*theMenu)->menuID);

    /* Free menu data */
    if (*theMenu != NULL) {
        free(*theMenu);
    }
    free(theMenu);
}

/*
 * InsertMenu - Add menu to menu bar
 */
void InsertMenu(MenuHandle theMenu, short beforeID)
{
    MenuBarList* menuBar;
    int insertIndex;

    if (!gMenuMgrInitialized || theMenu == NULL) {
        return;
    }

    if (ValidateMenuHandle(theMenu) != 0) {
        return;
    }

    /* Create menu list if it doesn't exist */
    if (gMenuList == NULL) {
        gMenuList = (Handle)malloc(sizeof(MenuBarList));
        if (gMenuList == NULL) {
            return;
        }
        menuBar = (MenuBarList*)gMenuList;
        menuBar->numMenus = 0;
        menuBar->totalWidth = 0;
        menuBar->lastRight = 0;
        menuBar->mbResID = 0;
    } else {
        menuBar = (MenuBarList*)gMenuList;
    }

    /* Make sure menu bar is set in state */
    if (gMenuMgrState) {
        gMenuMgrState->menuBar = gMenuList;
    }

    /* Find insertion point */
    insertIndex = menuBar->numMenus; /* Default to end */
    if (beforeID != 0 && beforeID != hierMenu) {
        for (int i = 0; i < menuBar->numMenus; i++) {
            if (menuBar->menus[i].menuID == beforeID) {
                insertIndex = i;
                break;
            }
        }
    }

    /* Expand menu list */
    size_t newSize = sizeof(MenuBarList) + (menuBar->numMenus) * sizeof(MenuListEntry);
    gMenuList = (Handle)realloc(gMenuList, newSize);
    if (gMenuList == NULL) {
        return;
    }
    menuBar = (MenuBarList*)gMenuList;

    /* Shift existing menus if inserting in middle */
    if (insertIndex < menuBar->numMenus) {
        memmove(&menuBar->menus[insertIndex + 1], &menuBar->menus[insertIndex],
                (menuBar->numMenus - insertIndex) * sizeof(MenuListEntry));
    }

    /* Insert new menu */
    menuBar->menus[insertIndex].menuID = (*theMenu)->menuID;
    menuBar->menus[insertIndex].menuLeft = 0; /* Will be calculated */
    menuBar->menus[insertIndex].menuWidth = 0; /* Will be calculated */
    menuBar->numMenus++;

    /* Update layout and display */
    UpdateMenuBarLayout();
    InvalidateMenuBar();

    /* Inserted menu */
}

/*
 * DeleteMenu - Remove menu from menu bar
 */
void DeleteMenu(short menuID)
{
    MenuBarList* menuBar;
    int menuIndex = -1;

    if (!gMenuMgrInitialized || gMenuList == NULL) {
        return;
    }

    menuBar = (MenuBarList*)gMenuList;

    /* Find menu in list */
    for (int i = 0; i < menuBar->numMenus; i++) {
        if (menuBar->menus[i].menuID == menuID) {
            menuIndex = i;
            break;
        }
    }

    if (menuIndex == -1) {
        return; /* Menu not found */
    }

    /* Remove menu from list */
    if (menuIndex < menuBar->numMenus - 1) {
        memmove(&menuBar->menus[menuIndex], &menuBar->menus[menuIndex + 1],
                (menuBar->numMenus - menuIndex - 1) * sizeof(MenuListEntry));
    }
    menuBar->numMenus--;

    /* Update layout and display */
    UpdateMenuBarLayout();
    InvalidateMenuBar();

    /* Deleted menu */
}

/*
 * GetMenuHandle - Find menu by ID
 */
MenuHandle GetMenuHandle(short menuID)
{
    if (!gMenuMgrInitialized) {
        return NULL;
    }

    return FindMenuInList(menuID);
}

/* ============================================================================
 * Menu Flash and Feedback
 * ============================================================================ */

/*
 * FlashMenuBar - Flash menu bar for feedback
 */
void FlashMenuBar(short menuID)
{
    if (!gMenuMgrInitialized) {
        return;
    }

    /* Flash the specified menu or entire menu bar */
    for (int i = 0; i < gMenuFlash; i++) {
        /* TODO: Implement flashing animation */
        /* Flash menu bar */
    }
}

/*
 * SetMenuFlash - Set menu flash count
 */
void SetMenuFlash(short count)
{
    gMenuFlash = count;
}

/* ============================================================================
 * Menu Manager State Access
 * ============================================================================ */

/*
 * GetMenuManagerState - Get global Menu Manager state
 */
MenuManagerState* GetMenuManagerState(void)
{
    return gMenuMgrState;
}

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

/*
 * AllocateMenuManagerState - Allocate Menu Manager state
 */
static MenuManagerState* AllocateMenuManagerState(void)
{
    MenuManagerState* state = (MenuManagerState*)malloc(sizeof(MenuManagerState));
    if (state == NULL) {
        return NULL;
    }

    memset(state, 0, sizeof(MenuManagerState));
    return state;
}

/*
 * InitializeMenuManagerState - Initialize Menu Manager state
 */
static void InitializeMenuManagerState(MenuManagerState* state)
{
    if (state == NULL) {
        return;
    }

    state->menuList = NULL;
    state->currentMenuBar = 0;
    state->hiliteMenu = 0;
    state->menuBarHeight = menuBarStdHeight;
    state->menuBarVisible = true;
    state->menuBarInvalid = true;
    state->menuColorTable = NULL;
    state->menuFlash = 3;
    state->lastMenuChoice = 0;
    state->trackingMenu = false;
    state->currentMenu = NULL;
    state->currentItem = 0;
    state->platformData = NULL;
    state->initialized = false;
}

/*
 * DisposeMenuManagerState - Dispose Menu Manager state
 */
static void DisposeMenuManagerState(MenuManagerState* state)
{
    if (state == NULL) {
        return;
    }

    /* Clean up any allocated resources */
    if (state->menuColorTable != NULL) {
        free(state->menuColorTable);
    }

    if (state->platformData != NULL) {
        free(state->platformData);
    }

    free(state);
}

/*
 * ValidateMenuHandle - Validate menu handle
 */
static OSErr ValidateMenuHandle(MenuHandle theMenu)
{
    if (theMenu == NULL || *theMenu == NULL) {
        return menuInvalidErr;
    }

    if ((*theMenu)->menuID == 0) {
        return menuInvalidErr;
    }

    return 0;
}

/*
 * ValidateMenuID - Validate menu ID
 */
static OSErr ValidateMenuID(short menuID)
{
    if (menuID == 0) {
        return menuInvalidErr;
    }

    return 0;
}

/*
 * FindMenuInList - Find menu by ID in current menu list
 */
static MenuHandle FindMenuInList(short menuID)
{
    /* Look up in tracking array */
    for (int i = 0; i < gNumMenuHandles; i++) {
        if (gMenuHandles[i].menuID == menuID) {
            return gMenuHandles[i].handle;
        }
    }
    return NULL;
}

/*
 * UpdateMenuBarLayout - Update menu bar layout
 */
static void UpdateMenuBarLayout(void)
{
    MenuBarList* menuBar;
    short currentLeft = 0;

    if (gMenuList == NULL) {
        return;
    }

    menuBar = (MenuBarList*)gMenuList;

    /* Calculate positions for all menus */
    for (int i = 0; i < menuBar->numMenus; i++) {
        /* TODO: Calculate actual menu width based on title */
        short menuWidth = 80; /* Default width */

        menuBar->menus[i].menuLeft = currentLeft;
        menuBar->menus[i].menuWidth = menuWidth;
        currentLeft += menuWidth;
    }

    menuBar->totalWidth = currentLeft;
    menuBar->lastRight = currentLeft;
}

/*
 * InvalidateMenuBar - Mark menu bar as needing redraw
 */
static void InvalidateMenuBar(void)
{
    if (gMenuMgrState != NULL) {
        gMenuMgrState->menuBarInvalid = true;
    }
}

/* ============================================================================
 * Menu Color Support
 * ============================================================================ */

/*
 * GetMCInfo - Get menu color table
 */
MCTableHandle GetMCInfo(void)
{
    return gMCTable;
}

/*
 * SetMCInfo - Set menu color table
 */
void SetMCInfo(MCTableHandle menuCTbl)
{
    if (gMCTable != NULL && gMCTable != menuCTbl) {
        DisposeMCInfo(gMCTable);
    }

    gMCTable = menuCTbl;
    if (gMenuMgrState != NULL) {
        gMenuMgrState->menuColorTable = menuCTbl;
    }

    /* Redraw menu bar with new colors */
    InvalidateMenuBar();
}

/*
 * DisposeMCInfo - Dispose menu color table
 */
void DisposeMCInfo(MCTableHandle menuCTbl)
{
    if (menuCTbl != NULL) {
        free(menuCTbl);
    }
}

/*
 * GetMCEntry - Get menu color entry
 */
MCEntryPtr GetMCEntry(short menuID, short menuItem)
{
    /* TODO: Implement menu color lookup */
    return NULL;
}

/*
 * SetMCEntries - Set multiple menu color entries
 */
void SetMCEntries(short numEntries, MCTablePtr menuCEntries)
{
    /* TODO: Implement menu color entry setting */
}

/*
 * DeleteMCEntries - Delete menu color entries
 */
void DeleteMCEntries(short menuID, short menuItem)
{
    /* TODO: Implement menu color entry deletion */
}
