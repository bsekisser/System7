/*
 * MenuCommands.c - Menu Command Dispatcher
 *
 * Handles menu selections and executes appropriate commands
 * for System 7.1 menu structure.
 */

#include "SystemTypes.h"

/* Serial output functions */
extern void serial_puts(const char* str);
extern void serial_printf(const char* format, ...);
#include "MenuManager/MenuManager.h"

/* Menu IDs - Standard System 7.1 */
#define kAppleMenuID    1
#define kFileMenuID     128
#define kEditMenuID     129
#define kViewMenuID     130
#define kLabelMenuID    131
#define kSpecialMenuID  132

/* Apple Menu Items */
#define kAboutItem      1
#define kDeskAccItem    2  /* Desk accessories start here */

/* File Menu Items */
#define kNewItem        1
#define kOpenItem       2
#define kCloseItem      4
#define kSaveItem       5
#define kSaveAsItem     6
#define kPageSetupItem  8
#define kPrintItem      9
#define kQuitItem       11

/* Edit Menu Items */
#define kUndoItem       1
#define kCutItem        3
#define kCopyItem       4
#define kPasteItem      5
#define kClearItem      6
#define kSelectAllItem  8

/* View Menu Items */
#define kBySmallIcon    1
#define kByIcon         2
#define kByName         3
#define kBySize         4
#define kByKind         5
#define kByLabel        6
#define kByDate         7

/* Forward declarations */
static void HandleAppleMenu(short item);
static void HandleFileMenu(short item);
static void HandleEditMenu(short item);
static void HandleViewMenu(short item);
static void HandleLabelMenu(short item);
static void HandleSpecialMenu(short item);

/* Main menu command dispatcher */
void DoMenuCommand(short menuID, short item)
{
    serial_printf("DoMenuCommand: menu=%d, item=%d\n", menuID, item);

    switch (menuID) {
        case kAppleMenuID:
            HandleAppleMenu(item);
            break;

        case kFileMenuID:
            HandleFileMenu(item);
            break;

        case kEditMenuID:
            HandleEditMenu(item);
            break;

        case kViewMenuID:
            HandleViewMenu(item);
            break;

        case kLabelMenuID:
            HandleLabelMenu(item);
            break;

        case kSpecialMenuID:
            HandleSpecialMenu(item);
            break;

        default:
            serial_printf("Unknown menu ID: %d\n", menuID);
            break;
    }

    /* Clear menu highlighting after command */
    HiliteMenu(0);
}

/* Apple Menu Handler */
static void HandleAppleMenu(short item)
{
    switch (item) {
        case kAboutItem:
            serial_printf("About System 7.1...\n");
            /* TODO: Show about box */
            break;

        default:
            /* Desk accessories would be launched here */
            serial_printf("Launch desk accessory %d\n", item - kDeskAccItem + 1);
            break;
    }
}

/* File Menu Handler */
static void HandleFileMenu(short item)
{
    switch (item) {
        case kNewItem:
            serial_printf("File > New\n");
            /* TODO: Create new document/folder */
            break;

        case kOpenItem:
            serial_printf("File > Open...\n");
            /* TODO: Show open dialog */
            break;

        case kCloseItem:
            serial_printf("File > Close\n");
            /* TODO: Close current window */
            break;

        case kSaveItem:
            serial_printf("File > Save\n");
            /* TODO: Save current document */
            break;

        case kSaveAsItem:
            serial_printf("File > Save As...\n");
            /* TODO: Show save dialog */
            break;

        case kPageSetupItem:
            serial_printf("File > Page Setup...\n");
            /* TODO: Show page setup dialog */
            break;

        case kPrintItem:
            serial_printf("File > Print...\n");
            /* TODO: Show print dialog */
            break;

        case kQuitItem:
            serial_printf("File > Quit - Shutting down...\n");
            /* TODO: Proper shutdown sequence */
            /* For now, just halt */
            __asm__ volatile("cli; hlt");
            break;

        default:
            serial_printf("Unknown File menu item: %d\n", item);
            break;
    }
}

/* Edit Menu Handler */
static void HandleEditMenu(short item)
{
    switch (item) {
        case kUndoItem:
            serial_printf("Edit > Undo\n");
            /* TODO: Undo last action */
            break;

        case kCutItem:
            serial_printf("Edit > Cut\n");
            /* TODO: Cut selection to clipboard */
            break;

        case kCopyItem:
            serial_printf("Edit > Copy\n");
            /* TODO: Copy selection to clipboard */
            break;

        case kPasteItem:
            serial_printf("Edit > Paste\n");
            /* TODO: Paste from clipboard */
            break;

        case kClearItem:
            serial_printf("Edit > Clear\n");
            /* TODO: Clear selection */
            break;

        case kSelectAllItem:
            serial_printf("Edit > Select All\n");
            /* TODO: Select all items */
            break;

        default:
            serial_printf("Unknown Edit menu item: %d\n", item);
            break;
    }
}

/* View Menu Handler */
static void HandleViewMenu(short item)
{
    const char* viewNames[] = {
        "by Small Icon",
        "by Icon",
        "by Name",
        "by Size",
        "by Kind",
        "by Label",
        "by Date"
    };

    if (item >= kBySmallIcon && item <= kByDate) {
        serial_printf("View > %s\n", viewNames[item - 1]);
        /* TODO: Change view mode */
    } else {
        serial_printf("Unknown View menu item: %d\n", item);
    }
}

/* Label Menu Handler */
static void HandleLabelMenu(short item)
{
    const char* labelNames[] = {
        "None",
        "Essential",
        "Hot",
        "In Progress",
        "Cool",
        "Personal",
        "Project 1",
        "Project 2"
    };

    if (item >= 1 && item <= 8) {
        serial_printf("Label > %s\n", labelNames[item - 1]);
        /* TODO: Apply label to selection */
    } else {
        serial_printf("Unknown Label menu item: %d\n", item);
    }
}

/* Special Menu Handler */
static void HandleSpecialMenu(short item)
{
    switch (item) {
        case 1:
            serial_printf("Special > Clean Up\n");
            /* TODO: Clean up desktop/window */
            break;

        case 2:
            serial_printf("Special > Empty Trash\n");
            /* TODO: Empty trash */
            break;

        case 3:
            serial_printf("Special > Eject\n");
            /* TODO: Eject selected disk */
            break;

        case 4:
            serial_printf("Special > Erase Disk...\n");
            /* TODO: Show erase disk dialog */
            break;

        case 5:
            serial_printf("Special > Restart\n");
            /* TODO: System restart */
            break;

        case 6:
            serial_printf("Special > Shut Down\n");
            /* TODO: System shutdown */
            break;

        default:
            serial_printf("Unknown Special menu item: %d\n", item);
            break;
    }
}