/*
 * MenuCommands.c - Menu Command Dispatcher
 *
 * Handles menu selections and executes appropriate commands
 * for System 7.1 menu structure.
 */

#include "SystemTypes.h"
#include "System71StdLib.h"
#include "MenuManager/MenuManager.h"
#include "Finder/AboutThisMac.h"
#include "ControlPanels/DesktopPatterns.h"
#include "ControlPanels/Sound.h"
#include "ControlPanels/Mouse.h"
#include "ControlPanels/Keyboard.h"
#include "ControlPanels/ControlStrip.h"
#include "Datetime/datetime_cdev.h"
#include "Platform/Halt.h"
#include "Platform/include/io.h"

#include <string.h>

#define MENU_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleMenu, kLogLevelDebug, "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_WARN(fmt, ...)  serial_logf(kLogModuleMenu, kLogLevelWarn,  "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_INFO(fmt, ...)  serial_logf(kLogModuleMenu, kLogLevelInfo, fmt, ##__VA_ARGS__)

/* Forward declarations */
static void ShowAboutBox(void);
static void perform_power_off(void);

/* Menu IDs - Standard System 7.1 */
#define kAppleMenuID    128  /* Apple menu is ID 128 in our system */
#define kFileMenuID     129
#define kEditMenuID     130
#define kViewMenuID     131
#define kLabelMenuID    132
#define kSpecialMenuID  133

/* Apple Menu Items */
#define kAboutItem      1
#define kDeskAccItem    2  /* Desk accessories start here */

/* File Menu Items - Finder specific (System 7.1) */
#define kNewFolderItem  1
#define kOpenItem       2
#define kPrintItem      3
#define kCloseItem      4
#define kGetInfoItem    6
#define kSharingItem    7
#define kDuplicateItem  8
#define kMakeAliasItem  9
#define kPutAwayItem    10
#define kFindItem       12
#define kFindAgainItem  13

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

static void perform_power_off(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* ACPI power-off sequence for x86 hardware */
    hal_outw(0x604, 0x2000);
    hal_outb(0xB004, 0x53);
#endif
    platform_halt();
}

static void perform_restart(void) {
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile(
        "movl $0, %%esp\n\t"
        "push $0\n\t"
        "push $0\n\t"
        "lidt (%%esp)\n\t"
        "int3\n\t"
        :
        :
        : "memory"
    );
#endif
    platform_halt();
}

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
    MENU_LOG_DEBUG("DoMenuCommand: menu=%d, item=%d\n", menuID, item);

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
            MENU_LOG_WARN("Unknown menu ID: %d\n", menuID);
            break;
    }

    /* Clear menu highlighting after command */
    HiliteMenu(0);
}

/* Show About Box */
static void ShowAboutBox(void)
{
    MENU_LOG_INFO("\n");
    MENU_LOG_INFO("========================================\n");
    MENU_LOG_INFO("           System 7 Reimplementation   \n");
    MENU_LOG_INFO("========================================\n");
    MENU_LOG_INFO("Version: 7.1.0\n");
    MENU_LOG_INFO("Build: Clean room reimplementation\n");
    MENU_LOG_INFO("\n");
    MENU_LOG_INFO("A compatible implementation of classic\n");
    MENU_LOG_INFO("Macintosh system software\n");
    MENU_LOG_INFO("\n");
    MENU_LOG_INFO("Open source portable implementation\n");
    MENU_LOG_INFO("========================================\n\n");

    /* Would show a proper dialog box with this information */
    /* For now just output to serial console */
}

/* Apple Menu Handler */
static Boolean GetMenuItemCString(short menuID, short item, char *out, size_t outSize)
{
    if (!out || outSize == 0) {
        return false;
    }

    MenuHandle menu = GetMenuHandle(menuID);
    if (!menu) {
        return false;
    }

    Str255 itemText;
    GetMenuItemText(menu, item, itemText);

    size_t len = itemText[0];
    if (len >= outSize) {
        len = outSize - 1;
    }

    memcpy(out, &itemText[1], len);
    out[len] = '\0';
    return true;
}

static void HandleAppleMenu(short item)
{
    char itemName[256];
    if (!GetMenuItemCString(kAppleMenuID, item, itemName, sizeof(itemName))) {
        MENU_LOG_WARN("Apple Menu: unable to resolve item %d\n", item);
        return;
    }

    if (strcmp(itemName, "About This Macintosh") == 0) {
        MENU_LOG_DEBUG("About This Macintosh...\n");
        extern void AboutWindow_ShowOrToggle(void);
        AboutWindow_ShowOrToggle();
        return;
    }

    if (strcmp(itemName, "Desktop Patterns...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Desktop Patterns...\n");
        OpenDesktopCdev();
        return;
    }

    if (strcmp(itemName, "Date & Time...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Date & Time...\n");
        DateTimePanel_Open();
        return;
    }

    if (strcmp(itemName, "Sound...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Sound...\n");
        SoundPanel_Open();
        return;
    }

    if (strcmp(itemName, "Mouse...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Mouse...\n");
        MousePanel_Open();
        return;
    }

    if (strcmp(itemName, "Keyboard...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Keyboard...\n");
        KeyboardPanel_Open();
        return;
    }

    if (strcmp(itemName, "Control Strip...") == 0) {
        MENU_LOG_DEBUG("Apple Menu > Control Strip...\n");
        ControlStrip_Toggle();
        return;
    }

    if (strcmp(itemName, "-") == 0) {
        /* Separator */
        return;
    }

    MENU_LOG_WARN("Unknown Apple menu item: '%s' (index %d)\n", itemName, item);
}

/* File Menu Handler - Finder specific */
static void HandleFileMenu(short item)
{
    extern WindowPtr FrontWindow(void);
    extern void DrawDesktop(void);

    switch (item) {
        case kNewFolderItem: {
            MENU_LOG_INFO("File > New Folder\n");
            /* Create new folder in Finder - creates in Desktop (parent ID 2) */
            extern Boolean VFS_CreateFolder(SInt16 vref, SInt32 parent, const char* name, SInt32* newID);
            SInt32 newFolderID;
            if (VFS_CreateFolder(0, 2, "New Folder", &newFolderID)) {
                MENU_LOG_DEBUG("Created new folder with ID %d\n", (int)newFolderID);
                DrawDesktop();
            }
            break;
        }

        case kOpenItem: {
            MENU_LOG_INFO("File > Open\n");
            /* Open selected item in Finder window - or open front window's selected item */
            extern void OpenSelectedItems(void);
            OpenSelectedItems();
            break;
        }

        case kPrintItem: {
            MENU_LOG_INFO("File > Print\n");
            /* Print selected item - used for printing documents from Finder */
            MENU_LOG_DEBUG("Print not implemented for Finder\n");
            break;
        }

        case kCloseItem: {
            MENU_LOG_INFO("File > Close\n");
            /* Close current window - standard close operation */
            extern void CloseWindow(WindowPtr window);
            WindowPtr front = FrontWindow();
            if (front && front->visible) {
                MENU_LOG_DEBUG("Closing front window 0x%08x\n", (unsigned int)P2UL(front));
                CloseWindow(front);
            }
            break;
        }

        case kGetInfoItem: {
            MENU_LOG_INFO("File > Get Info\n");
            /* Show Get Info dialog for selected item(s) */
            extern void ShowGetInfoDialog(WindowPtr w);
            WindowPtr front = FrontWindow();
            if (front) {
                ShowGetInfoDialog(front);
            }
            break;
        }

        case kSharingItem: {
            MENU_LOG_INFO("File > Sharing...\n");
            /* Show Sharing settings for selected item */
            MENU_LOG_DEBUG("Sharing settings not implemented\n");
            break;
        }

        case kDuplicateItem: {
            MENU_LOG_INFO("File > Duplicate\n");
            /* Duplicate selected items in Finder */
            extern void DuplicateSelectedItems(WindowPtr w);
            WindowPtr front = FrontWindow();
            if (front) {
                DuplicateSelectedItems(front);
            }
            break;
        }

        case kMakeAliasItem: {
            MENU_LOG_INFO("File > Make Alias\n");
            /* Create alias of selected items */
            extern void MakeAliasOfSelectedItems(WindowPtr w);
            WindowPtr front = FrontWindow();
            if (front) {
                MakeAliasOfSelectedItems(front);
            }
            break;
        }

        case kPutAwayItem: {
            MENU_LOG_INFO("File > Put Away\n");
            /* Put away selected item - move to trash or eject volume */
            extern void PutAwaySelectedItems(WindowPtr w);
            WindowPtr front = FrontWindow();
            if (front) {
                PutAwaySelectedItems(front);
            }
            break;
        }

        case kFindItem: {
            MENU_LOG_INFO("File > Find...\n");
            /* Show Find dialog to search for files */
            MENU_LOG_DEBUG("Find dialog not implemented\n");
            break;
        }

        case kFindAgainItem: {
            MENU_LOG_INFO("File > Find Again\n");
            /* Repeat the last find operation */
            MENU_LOG_DEBUG("Find Again not implemented\n");
            break;
        }

        default:
            MENU_LOG_WARN("Unknown File menu item: %d\n", item);
            break;
    }
}

/* Edit Menu Handler - System 7.1 standard */
static void HandleEditMenu(short item)
{
    extern WindowPtr FrontWindow(void);

    switch (item) {
        case kUndoItem: {
            MENU_LOG_INFO("Edit > Undo\n");
            /* Undo last action - handled by active document/window */
            extern void Finder_Undo(void);
            Finder_Undo();
            break;
        }

        case kCutItem: {
            MENU_LOG_INFO("Edit > Cut\n");
            /* Cut selection to clipboard - context dependent */
            extern void Finder_Cut(void);
            Finder_Cut();
            break;
        }

        case kCopyItem: {
            MENU_LOG_INFO("Edit > Copy\n");
            /* Copy selection to clipboard */
            extern void Finder_Copy(void);
            Finder_Copy();
            break;
        }

        case kPasteItem: {
            MENU_LOG_INFO("Edit > Paste\n");
            /* Paste from clipboard */
            extern void Finder_Paste(void);
            Finder_Paste();
            break;
        }

        case kClearItem: {
            MENU_LOG_INFO("Edit > Clear\n");
            /* Clear/delete selection */
            extern void Finder_Clear(void);
            Finder_Clear();
            break;
        }

        case kSelectAllItem: {
            MENU_LOG_INFO("Edit > Select All\n");
            /* Select all items in current context */
            extern void Finder_SelectAll(void);
            Finder_SelectAll();
            break;
        }

        default:
            MENU_LOG_WARN("Unknown Edit menu item: %d\n", item);
            break;
    }
}

/* View Menu Handler - System 7.1 Finder views */
static void HandleViewMenu(short item)
{
    extern WindowPtr FrontWindow(void);
    extern void SetWindowViewMode(WindowPtr w, short viewMode);
    extern OSErr CleanUpWindow(WindowPtr w, short sortMode);

    const char* viewNames[] = {
        "by Icon",
        "by Name",
        "by Size",
        "by Kind",
        "by Label",
        "by Date",
        "Clean Up Window",
        "Clean Up Selection"
    };

    WindowPtr front = FrontWindow();

    switch (item) {
        case 1:  /* by Icon */
            MENU_LOG_INFO("View > by Icon\n");
            if (front) SetWindowViewMode(front, 1);
            break;

        case 2:  /* by Name */
            MENU_LOG_INFO("View > by Name\n");
            if (front) SetWindowViewMode(front, 2);
            break;

        case 3:  /* by Size */
            MENU_LOG_INFO("View > by Size\n");
            if (front) SetWindowViewMode(front, 3);
            break;

        case 4:  /* by Kind */
            MENU_LOG_INFO("View > by Kind\n");
            if (front) SetWindowViewMode(front, 4);
            break;

        case 5:  /* by Label */
            MENU_LOG_INFO("View > by Label\n");
            if (front) SetWindowViewMode(front, 5);
            break;

        case 6:  /* by Date */
            MENU_LOG_INFO("View > by Date\n");
            if (front) SetWindowViewMode(front, 6);
            break;

        case 7:  /* separator */
            return;

        case 8:  /* Clean Up Window */
            MENU_LOG_INFO("View > Clean Up Window\n");
            if (front) CleanUpWindow(front, 0);
            break;

        case 9:  /* Clean Up Selection */
            MENU_LOG_INFO("View > Clean Up Selection\n");
            if (front) CleanUpWindow(front, 1);
            break;

        default:
            MENU_LOG_WARN("Unknown View menu item: %d\n", item);
            break;
    }
}

/* Label Menu Handler - System 7.1 label colors */
static void HandleLabelMenu(short item)
{
    extern WindowPtr FrontWindow(void);
    extern void ApplyLabelToSelection(WindowPtr w, short labelIndex);

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

    WindowPtr front = FrontWindow();

    if (item >= 1 && item <= 8) {
        MENU_LOG_INFO("Label > %s\n", labelNames[item - 1]);
        /* Apply label to selection */
        if (front) {
            ApplyLabelToSelection(front, item - 1);
        }
    } else {
        MENU_LOG_WARN("Unknown Label menu item: %d\n", item);
    }
}

/* Special Menu Handler - System 7.1 Special menu */
static void HandleSpecialMenu(short item)
{
    extern void ArrangeDesktopIcons(void);
    extern OSErr EmptyTrash(Boolean force);

    switch (item) {
        case 1: {  /* Clean Up Desktop */
            MENU_LOG_INFO("Special > Clean Up Desktop\n");
            ArrangeDesktopIcons();
            break;
        }

        case 2: {  /* Empty Trash */
            MENU_LOG_INFO("Special > Empty Trash\n");
            OSErr err = EmptyTrash(false);
            if (err == noErr) {
                MENU_LOG_DEBUG("Trash emptied successfully\n");
            } else {
                MENU_LOG_WARN("Failed to empty trash (error %d)\n", err);
            }
            break;
        }

        case 3: {  /* separator */
            return;
        }

        case 4: {  /* Eject */
            MENU_LOG_INFO("Special > Eject\n");
            /* Eject removable media - platform specific */
            MENU_LOG_DEBUG("Eject: Ejecting removable media\n");
            break;
        }

        case 5: {  /* Erase Disk */
            MENU_LOG_INFO("Special > Erase Disk\n");
            /* Show erase disk confirmation dialog */
            MENU_LOG_DEBUG("Erase Disk: Confirmation dialog would appear\n");
            break;
        }

        case 6: {  /* separator */
            return;
        }

        case 7: {  /* Restart */
            MENU_LOG_INFO("Special > Restart\n");
            MENU_LOG_INFO("System restart initiated...\n");
            perform_restart();
            break;
        }

        case 8: {  /* Shut Down */
            MENU_LOG_INFO("Special > Shut Down\n");
            MENU_LOG_INFO("System shutdown initiated...\n");
            MENU_LOG_INFO("It is now safe to turn off your computer.\n");
            perform_power_off();
            break;
        }

        default:
            MENU_LOG_WARN("Unknown Special menu item: %d\n", item);
            break;
    }
}

/* ========================================================================
 * FINDER OPERATION STUBS - implementations provided by Finder module
 * ======================================================================== */

/* File Operations */
void OpenSelectedItems(void) {
    MENU_LOG_DEBUG("[STUB] OpenSelectedItems called\n");
}

void ShowGetInfoDialog(WindowPtr w) {
    MENU_LOG_DEBUG("[STUB] ShowGetInfoDialog called\n");
}

void DuplicateSelectedItems(WindowPtr w) {
    MENU_LOG_DEBUG("[STUB] DuplicateSelectedItems called\n");
}

void MakeAliasOfSelectedItems(WindowPtr w) {
    MENU_LOG_DEBUG("[STUB] MakeAliasOfSelectedItems called\n");
}

void PutAwaySelectedItems(WindowPtr w) {
    MENU_LOG_DEBUG("[STUB] PutAwaySelectedItems called\n");
}

/* Edit Operations */
void Finder_Undo(void) {
    MENU_LOG_DEBUG("[STUB] Finder_Undo called\n");
}

void Finder_Cut(void) {
    MENU_LOG_DEBUG("[STUB] Finder_Cut called\n");
}

void Finder_Copy(void) {
    MENU_LOG_DEBUG("[STUB] Finder_Copy called\n");
}

void Finder_Paste(void) {
    MENU_LOG_DEBUG("[STUB] Finder_Paste called\n");
}

void Finder_Clear(void) {
    MENU_LOG_DEBUG("[STUB] Finder_Clear called\n");
}

void Finder_SelectAll(void) {
    MENU_LOG_DEBUG("[STUB] Finder_SelectAll called\n");
}

/* View Operations */
void SetWindowViewMode(WindowPtr w, short viewMode) {
    if (!w) return;
    MENU_LOG_DEBUG("[STUB] SetWindowViewMode called with mode=%d\n", viewMode);
}

/* Label Operations */
void ApplyLabelToSelection(WindowPtr w, short labelIndex) {
    if (!w) return;
    MENU_LOG_DEBUG("[STUB] ApplyLabelToSelection called with label=%d\n", labelIndex);
}
