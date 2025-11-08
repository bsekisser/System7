/*
 * System 7.1 Stubs for linking
 *
 * [WM-050] Naming clarification: SYS71_PROVIDE_FINDER_TOOLBOX
 *
 * When SYS71_PROVIDE_FINDER_TOOLBOX is defined (=1), this means:
 *   DO NOT provide Toolbox stubs; real implementations exist and should be used.
 *
 * Stubs wrapped in #if !defined(SYS71_STUBS_DISABLED) are excluded when
 * SYS71_PROVIDE_FINDER_TOOLBOX is defined, ensuring single source of truth per symbol.
 *
 * When the flag is NOT defined (bootstrap builds), stubs are included to
 * satisfy linker requirements until real implementations are integrated.
 */

/* Lock stub switch: single knob to disable all quarantined stubs */
#ifdef SYS71_PROVIDE_FINDER_TOOLBOX
#define SYS71_STUBS_DISABLED 1
#endif
/* Disable quarantined stubs by default now that real modules exist */
#ifndef SYS71_STUBS_DISABLED
#define SYS71_STUBS_DISABLED 1
#endif

#include "../include/MacTypes.h"
#include "../include/SystemInternal.h"
#include "../include/QuickDraw/QuickDraw.h"
#include "../include/QuickDraw/QDRegions.h"
#include "../include/QuickDrawConstants.h"
#include "../include/ResourceManager.h"
#include "../include/EventManager/EventTypes.h"  /* Include EventTypes first to define activeFlag */
#include "../include/EventManager/EventManager.h"
#include "../include/WindowManager/WindowManager.h"
/* #include "../include/WindowManager/WindowManagerInternal.h" -- removed, has conflicts */
#include "../include/MenuManager/MenuManager.h"
#include "../include/DialogManager/DialogManager.h"
#include "../include/ControlManager/ControlManager.h"
#include "../include/ListManager/ListManager.h"
#include "../include/TextEdit/TextEdit.h"
#include "../include/FontManager/FontManager.h"
#include "../include/sys71_stubs.h"
#include "../include/System/SystemLogging.h"
#include "../include/MemoryMgr/MemoryManager.h"
#include "../include/FileMgr/file_manager.h"
#include "../include/Finder/finder.h"

/* DeskHook type definition if not in headers */
typedef void (*DeskHookProc)(RgnHandle invalidRgn);

/* Platform Menu System stubs */
void Platform_InitMenuSystem(void) {
    /* Platform-specific menu initialization */
}

void Platform_CleanupMenuSystem(void) {
    /* Platform-specific menu cleanup */
}

/* Resource Manager */
#ifndef ENABLE_RESOURCES
void InitResourceManager(void) {
    /* Stub implementation */
}
#endif

/* QuickDraw - InitGraf is now provided by QuickDrawCore.c */

/* Font Manager */
/* Moved to FontManagerCore.c
void InitFonts(void) {
    // Stub implementation
}
*/

/* [WM-050] Window Manager stub quarantine
 * Provenance: IM:Windows Vol I - real implementations in WindowDisplay.c, WindowEvents.c, WindowResizing.c
 * Policy: Stubs compile only if SYS71_PROVIDE_FINDER_TOOLBOX is undefined
 * Real WM always wins; no dual definitions
 */
#if !defined(SYS71_STUBS_DISABLED)

void DrawWindow(WindowPtr window) {
    /* Window chrome is drawn automatically by ShowWindow/SelectWindow */
    /* This stub exists for DialogManager compatibility */
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* Menu Manager - Most functions now provided by MenuManagerCore.c */

/* AppendMenu now implemented in MenuItems.c */

/* Dialog Manager - provided by DialogManagerCore.c */

/* Control Manager */
void InitControlManager_Sys7(void) {
    /* Stub implementation */
}

/* List Manager */
void InitListManager(void) {
    /* Initialize List Manager dialog-list registry */
    extern void InitRegistryIfNeeded(void);

    /* Set up internal registry for dialog-attached lists */
    InitRegistryIfNeeded();

    /* List Manager is now ready for LNew() calls */
}

/* Event Manager */
SInt16 InitEvents(SInt16 numEvents) {
    return 0;  /* Success */
}

/* Removed DISABLED GetNextEvent stub (real implementation lives in EventManager) */

/* Removed DISABLED PostEvent stub (real implementation lives in EventManager) */

/* GenerateSystemEvent now provided by EventManager/event_manager.c */
/* Forward declaration for compatibility */
extern void GenerateSystemEvent(short eventType, int message, Point where, short modifiers);

/* SystemTask provided by DeskManagerCore.c */

/* External functions we use */

/* ExpandMem stubs for SystemInit */
void ExpandMemInit(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInit: Initializing expanded memory\n");
    /* Set up expanded memory globals if needed */
}
void ExpandMemInitKeyboard(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInitKeyboard: Initializing keyboard expanded memory\n");
    /* Initialize keyboard-specific memory areas */
}
void ExpandMemSetAppleTalkInactive(void) {
    SYSTEM_LOG_DEBUG("ExpandMemSetAppleTalkInactive: Disabling AppleTalk\n");
    /* Mark AppleTalk as inactive in expanded memory */
}
void SetAutoDecompression(Boolean enable) {
    SYSTEM_LOG_DEBUG("SetAutoDecompression: %s\n", enable ? "Enabled" : "Disabled");
    /* Set resource decompression flag */
}
void ResourceManager_SetDecompressionCacheSize(Size size) {
    SYSTEM_LOG_DEBUG("ResourceManager_SetDecompressionCacheSize: Setting cache to %lu bytes\n", (unsigned long)size);
    /* Configure decompression cache */
}
void InstallDecompressHook(DecompressHookProc proc) {
    SYSTEM_LOG_DEBUG("InstallDecompressHook: Installing hook at %p\n", proc);
    /* Install custom decompression routine */
}
void ExpandMemInstallDecompressor(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInstallDecompressor: Installing default decompressor\n");
    /* Install default resource decompressor */
}
void ExpandMemCleanup(void) {
    SYSTEM_LOG_DEBUG("ExpandMemCleanup: Cleaning up expanded memory\n");
    /* Release expanded memory resources */
}
void ExpandMemDump(void) {
    SYSTEM_LOG_DEBUG("ExpandMemDump: Dumping expanded memory state\n");
    /* Debug dump of expanded memory contents */
}
Boolean ExpandMemValidate(void) { return true; }

/* Serial stubs */
#include <stdarg.h>

/* serial_printf moved to System71StdLib.c */

/* Finder InitializeFinder provided by finder_main.c */

/* QuickDraw globals - defined in main.c */
extern QDGlobals qd;

/* Window manager globals */
WindowPtr g_firstWindow = NULL;  /* Head of window chain */

void FinderEventLoop(void) {
    /* Stub implementation */
}

/* Additional Finder support functions */
/* Removed DISABLED FlushEvents stub (real implementation lives in EventManager) */

/* TEInit now implemented in TextEditCore.c */

/* Window Manager functions - All real implementations in WindowManager/ directory:
 * InitWindows - WindowManagerCore.c
 * NewWindow - WindowManagerCore.c
 * DisposeWindow - WindowManagerCore.c
 * MoveWindow - WindowDragging.c
 * CloseWindow - WindowManagerCore.c
 * ShowWindow - WindowDisplay.c
 * SelectWindow - WindowDisplay.c
 * FrontWindow - WindowDisplay.c
 * DrawGrowIcon - WindowDisplay.c
 * FindWindow - WindowEvents.c
 * DragWindow - WindowDragging.c
 * SetWTitle - WindowManagerCore.c
 */


/* Functions provided by other components:
 * ShowErrorDialog - finder_main.c
 * CleanUpDesktop - desktop_manager.c
 * DrawDesktop - desktop_manager.c
 */

/* System stubs */
long sysconf(int name) { return -1; }

/* Resource Manager functions provided by Memory Manager */

/* HandleKeyDown moved to Finder/finder_main.c */

/* ResolveAliasFile moved to Finder/alias_manager.c */

#ifndef ENABLE_RESOURCES
void ReleaseResource(Handle theResource) {
    /* Stub */
}
#endif

/* NewAlias moved to Finder/alias_manager.c */

/* Memory Manager functions provided by MemoryManager.c */

OSErr FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag) {
    return noErr;
}

/* File Manager stubs - Core functions now implemented in FileManager.c */

OSErr FSpCreate(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag) {
    return noErr;
}

OSErr FSpOpenDF(const FSSpec* spec, SInt16 permission, SInt16* refNum) {
    if (refNum) *refNum = 1;
    return noErr;
}

OSErr FSpOpenResFile(const FSSpec* spec, SInt16 permission) {
    return 1; /* Return fake resource file ref */
}

OSErr FSpDelete(const FSSpec* spec) {
    return noErr;
}

OSErr FSpDirDelete(const FSSpec* spec) {
    return noErr;
}

OSErr FSpCatMove(const FSSpec* source, const FSSpec* dest) {
    return noErr;
}


OSErr PBHGetVInfoSync(void *paramBlock) {
    HParamBlockRec* pb = (HParamBlockRec*)paramBlock;
    if (pb) {
        pb->u.volumeParam.ioVAlBlkSiz = 512;
        pb->u.volumeParam.ioVNmAlBlks = 800;  /* Simulate 400K disk */
    }
    return noErr;
}


OSErr SetEOF(short refNum, long logEOF) {
    return noErr;
}

/* Resource Manager stubs */
/* NewHandle and DisposeHandle provided by Memory Manager */
/* GetResource provided by simple_resource_manager.c */

#ifndef ENABLE_RESOURCES
Handle Get1Resource(ResType theType, SInt16 theID) {
    extern Handle GetResource(ResType type, short id);
    return GetResource(theType, theID);
}

void AddResource(Handle theData, ResType theType, SInt16 theID, ConstStr255Param name) {
    /* Stub */
}

void RemoveResource(Handle theResource) {
    /* Stub */
}

void WriteResource(Handle theResource) {
    /* Stub */
}

void CloseResFile(SInt16 refNum) {
    /* Stub */
}

OSErr ResError(void) {
    return noErr;
}
#endif

void AddResMenu(MenuHandle theMenu, ResType theType) {
    /* Stub - would add resources to menu */
}

/* InsertFontResMenu moved to MenuManager/MenuItems.c */

/* Memory Manager functions provided by Memory Manager */
/* MemError moved to System71StdLib.c */

/* BlockMoveData moved to System71StdLib.c */

/* Finder-specific stubs */
/* FindFolder moved to Finder/finder_main.c */

/* GenerateUniqueTrashName moved to Finder/trash_folder.c */

OSErr InitializeWindowManager(void) {
    /* Initialize the Window Manager */
    extern void InitWindows(void);

    /* Set up Window Manager port, desktop pattern, and internal structures */
    InitWindows();

    return noErr;
}

/* InitializeTrashFolder provided by trash_folder.c */

OSErr HandleGetInfo(void) {
    return noErr;
}

/* ShowFind and FindAgain implemented in src/Finder/Find.c */

OSErr ShowAboutFinder(void) {
    /* Show About Finder dialog */
    extern short Alert(short alertID, void* filterProc);

    /* Display About box with simple message */
    /* Alert ID 128 is typically used for About boxes */
    /* For now, we'll use a generic alert since we don't have resources */

    extern void ParamText(ConstStr255Param param0, ConstStr255Param param1,
                         ConstStr255Param param2, ConstStr255Param param3);
    extern short NoteAlert(short alertID, void* filterProc);

    /* Set up message text */
    const unsigned char aboutMsg[] = "\pSystem 7.1 Finder";
    const unsigned char versionMsg[] = "\pVersion 7.1";
    const unsigned char emptyMsg[] = "\p";

    ParamText(aboutMsg, versionMsg, emptyMsg, emptyMsg);

    /* Show note alert (info icon) */
    NoteAlert(128, NULL);

    return noErr;
}

/* HandleContentClick moved to Finder/finder_main.c */

OSErr HandleGrowWindow(WindowPtr window, EventRecord* event) {
    if (!window || !event) {
        return paramErr;
    }

    /* Call GrowWindow to let user resize the window */
    extern long GrowWindow(WindowPtr theWindow, Point startPt, const Rect* bBox);
    extern void SizeWindow(WindowPtr theWindow, SInt16 w, SInt16 h, Boolean fUpdate);
    extern SInt16 HiWord(long x);
    extern SInt16 LoWord(long x);

    /* Set size constraints (minimum 80x80, maximum screen size) */
    Rect sizeRect = {80, 80, 480, 640};

    /* Track window resizing */
    long newSize = GrowWindow(window, event->where, &sizeRect);

    /* If user changed size, apply it */
    if (newSize != 0) {
        SInt16 newWidth = LoWord(newSize);
        SInt16 newHeight = HiWord(newSize);
        SizeWindow(window, newWidth, newHeight, true);
    }

    return noErr;
}

/* CloseFinderWindow moved to Finder/finder_main.c */
/* TrackGoAway, TrackBox, ZoomWindow - real implementations in WindowManager */
/* DoUpdate moved to Finder/finder_main.c */

void DoActivate(WindowPtr window, Boolean activate) {
    if (!window) return;

    /* Hilite or unhilite all controls in the window */
    extern void HiliteControl(ControlHandle theControl, SInt16 hiliteState);

    ControlHandle control = window->controlList;
    while (control) {
        /* Hilite value: 0 = active, 255 = inactive */
        SInt16 hiliteState = activate ? 0 : 255;
        HiliteControl(control, hiliteState);

        /* Move to next control */
        control = (*control)->nextControl;
    }

    /* Redraw controls to show new hilite state */
    extern void DrawControls(WindowPtr theWindow);
    DrawControls(window);
}

void DoBackgroundTasks(void) {
    /* Perform idle-time system tasks */
    extern void SystemTask(void);

    /* Call SystemTask to handle desk accessories and other background processing */
    SystemTask();

    /* Could add other background tasks here:
     * - Check for disk insertions
     * - Update network status
     * - Perform deferred cleanup
     */
}

/* WaitNextEvent now implemented in EventManager/event_manager.c */

/* Removed DISABLED EventAvail stub (real implementation lives in EventManager) */

/* Menu and Window functions provided by their respective managers:
 * MenuSelect - MenuSelection.c
 * SystemClick - DeskManagerCore.c
 * HiliteMenu - MenuManagerCore.c
 * FrontWindow - WindowDisplay.c
 */

OSErr ShowConfirmDialog(StringPtr message, Boolean* confirmed) {
    if (!confirmed) {
        return paramErr;
    }

    /* Show confirmation dialog with OK and Cancel buttons */
    extern void ParamText(ConstStr255Param param0, ConstStr255Param param1,
                         ConstStr255Param param2, ConstStr255Param param3);
    extern short CautionAlert(short alertID, void* filterProc);

    const unsigned char emptyMsg[] = "\p";

    /* Set the message text */
    if (message && message[0] > 0) {
        ParamText(message, emptyMsg, emptyMsg, emptyMsg);
    }

    /* Show caution alert (returns 1 for OK, 2 for Cancel) */
    short result = CautionAlert(128, NULL);

    /* OK button = 1, Cancel button = 2 */
    *confirmed = (result == 1);

    return noErr;
}

OSErr CloseAllWindows(void) {
    /* Close all windows from front to back */
    extern WindowPtr FrontWindow(void);
    extern void CloseWindow(WindowPtr theWindow);

    WindowPtr window;
    while ((window = FrontWindow()) != NULL) {
        /* Close the front window */
        CloseWindow(window);

        /* Safety check to prevent infinite loop if CloseWindow fails */
        WindowPtr checkWindow = FrontWindow();
        if (checkWindow == window) {
            /* Window didn't close, abort to prevent hang */
            break;
        }
    }

    return noErr;
}

OSErr CleanUpSelection(WindowPtr window) {
    if (!window) {
        return paramErr;
    }

    /* Arrange selected icons in a grid pattern */
    /* In a full implementation, this would:
     * 1. Find all selected icons in the window
     * 2. Calculate grid positions based on icon size
     * 3. Move icons to grid positions
     * 4. Update icon positions in window data
     */

    /* For now, just invalidate the window to trigger redraw */
    extern void InvalRect(const Rect* badRect);

    Rect windowRect = window->portRect;
    InvalRect(&windowRect);

    return noErr;
}

OSErr CleanUpBy(WindowPtr window, SInt16 sortType) {
    if (!window) {
        return paramErr;
    }

    /* Arrange all icons by specified sort order and snap to grid */
    /* Sort types from finder.h:
     * kCleanUpByName = 0
     * kCleanUpByDate = 1
     * kCleanUpBySize = 2
     * kCleanUpByKind = 3
     * kCleanUpByLabel = 4
     */

    /* In a full implementation, this would:
     * 1. Get all icons in the window
     * 2. Sort icons by the specified criteria
     * 3. Arrange in grid pattern from top-left
     * 4. Update icon positions in window data
     */

    /* For now, just invalidate the window to trigger redraw */
    extern void InvalRect(const Rect* badRect);

    Rect windowRect = window->portRect;
    InvalRect(&windowRect);

    (void)sortType; /* Unused for now */

    return noErr;
}

/* CleanUpWindow moved to Finder/finder_main.c */

/* ParamText provided by DialogManagerCore.c */

/* Alert, StopAlert, NoteAlert, CautionAlert now provided by AlertDialogs.c */

/* TickCount() now implemented in TimeManager/TimeBase.c */

/* GetMenuItemText now implemented in MenuItems.c */

/* OpenDeskAcc provided by DeskManagerCore.c */

/* HiWord and LoWord moved to System71StdLib.c */

#if !defined(SYS71_STUBS_DISABLED)
void InvalRect(const Rect* badRect) {
    /* Stub */
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

OSErr ScanDirectoryForDesktopEntries(SInt16 vRefNum, SInt32 dirID, SInt16 databaseRefNum) {
    return noErr;
}

/* QuickDraw Region functions - All real implementations in QuickDraw/Regions.c:
 * NewRgn, DisposeRgn, RectRgn, SetRectRgn, CopyRgn, SetEmptyRgn
 */

/* Standard library functions moved to System71StdLib.c:
 * sprintf, __assert_fail, strlen, abs
 * HiWord, LoWord, BlockMoveData - moved to System71StdLib.c
 */

/* Minimal math functions for -nostdlib build */
double atan2(double y, double x) {
    /* Very basic approximation - just return 0 for now */
    return 0.0;
}

double cos(double x) {
    /* Very basic approximation - just return 1 for now */
    return 1.0;
}

double sin(double x) {
    /* Very basic approximation - just return 0 for now */
    return 0.0;
}

/* 64-bit division for -nostdlib build */
long long __divdi3(long long a, long long b) {
    /* Simple implementation for positive numbers */
    if (b == 0) return 0;
    if (a < 0 || b < 0) return 0;  /* Don't handle negative for now */

    long long quotient = 0;
    long long remainder = a;

    while (remainder >= b) {
        remainder -= b;
        quotient++;
    }

    return quotient;
}

/* Standard library minimal implementations */
#include <stddef.h>

/* Memory and string functions moved to System71StdLib.c:
 * memcpy, memset, memcmp, memmove, strncpy, snprintf
 *
 * Memory allocation handled by Memory Manager:
 * malloc, calloc, realloc, free
 */

/* External globals from main.c */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* External QuickDraw globals */
extern QDGlobals qd;
extern void* framebuffer;

/* WM_Update, WM_InvalidateDisplay, SetDeskHook, and g_deskHook moved to WindowManager/WindowDisplay.c */

/* [WM-050] Stub quarantine: real BeginUpdate/EndUpdate in WindowEvents.c */
#if !defined(SYS71_STUBS_DISABLED)
void BeginUpdate(WindowPtr theWindow) {
    /* Stub - would save port and set up clipping */
}

void EndUpdate(WindowPtr theWindow) {
    /* Stub - would restore port and clear update region */
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* SetDeskHook moved to WindowManager/WindowDisplay.c */

/* [WM-053] QuickDraw region drawing functions - real implementations in QuickDraw/Regions.c */
#if !defined(SYS71_STUBS_DISABLED)
void FillRgn(RgnHandle rgn, ConstPatternParam pat) {
    /* Stub - would fill region with pattern */
}

Boolean RectInRgn(const Rect *r, RgnHandle rgn) {
    /* Stub - check if rectangle intersects region */
    return true;
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* sqrt() moved to System71StdLib.c */
/* QDPlatform_DrawRegion() moved to QuickDraw/QuickDrawPlatform.c */
/* QuickDraw text stubs for About box */
/* Moved to FontManagerCore.c
void TextSize(short size) {
    // Stub - would set text size in current GrafPort
}
void TextFont(short font) {
    // Stub - would set text font in current GrafPort
}

void TextFace(short face) {
    // Stub - would set text face (bold, italic, etc.) in current GrafPort
}
*/


/* Alert stub for trash_folder */

void Delay(UInt32 numTicks, UInt32* finalTicks) {
    /* Wait for specified number of ticks */
    extern UInt32 TickCount(void);

    UInt32 startTicks = TickCount();
    UInt32 targetTicks = startTicks + numTicks;

    /* Wait until target tick count reached */
    while (TickCount() < targetTicks) {
        /* Busy wait - could call SystemTask() here for cooperative multitasking */
    }

    if (finalTicks) {
        *finalTicks = TickCount();
    }
}
