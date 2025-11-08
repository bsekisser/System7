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
    /* Platform-specific menu initialization
     * Called by InitMenus() during Menu Manager initialization
     *
     * In a full implementation, this would:
     * - Initialize platform-specific menu rendering state
     * - Set up menu bar cursor resources
     * - Allocate platform-specific menu caches
     * - Register menu-related event handlers
     * - Initialize platform menu tracking structures
     *
     * For the kernel environment, menu rendering is handled by
     * MenuDisplay.c and platform_stubs.c, so this is a no-op.
     */
}

void Platform_CleanupMenuSystem(void) {
    /* Platform-specific menu cleanup
     * Called by CleanupMenus() during Menu Manager shutdown
     *
     * In a full implementation, this would:
     * - Release platform-specific menu rendering resources
     * - Dispose of cursor resources
     * - Free platform menu caches
     * - Unregister menu event handlers
     * - Clean up platform tracking structures
     *
     * For the kernel environment, cleanup is handled by the
     * Menu Manager core, so this is a no-op.
     */
}

void Platform_EraseMenuBar(void) {
    /* Platform-specific menu bar erasing
     *
     * In a full implementation, this would:
     * - Erase the menu bar area on screen (typically top 20 pixels)
     * - Fill with desktop pattern or white background
     * - Called before redrawing the menu bar
     * - May invalidate menu bar region for update
     *
     * For the kernel environment, menu bar erasing is handled
     * by MenuDisplay.c which directly manipulates the framebuffer,
     * so this is a no-op.
     */
}

/* Resource Manager */
#ifndef ENABLE_RESOURCES
void InitResourceManager(void) {
    /* Initialize the Resource Manager */
    /* This sets up the resource chain and system resource file */

    /* In a full implementation, this would:
     * 1. Initialize the resource file chain (empty list)
     * 2. Open the System resource file
     * 3. Set up resource map data structures
     * 4. Initialize resource cache
     * 5. Set the current resource file to the System file
     */

    /* For simple builds without resource support, this is a no-op */
    /* Resource Manager is ready for GetResource calls */
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
/* DrawWindow removed - implemented in WindowManager/WindowDisplay.c:919
 * Window chrome is drawn by real implementation, not this disabled stub */

/* Menu Manager - Most functions now provided by MenuManagerCore.c */

/* AppendMenu now implemented in MenuItems.c */

/* Dialog Manager - provided by DialogManagerCore.c */

/* Control Manager */
/* InitControlManager_Sys7 now implemented in ControlManager/control_manager_sys7.c */

/* List Manager */
void InitListManager(void) {
    /* Initialize List Manager for displaying scrollable lists
     *
     * The List Manager provides utilities for creating and managing
     * scrollable lists in dialogs and windows. It handles:
     * - List creation and disposal (LNew, LDispose)
     * - Cell selection and highlighting
     * - Scrolling and updating
     * - Drawing list contents
     * - Mouse tracking in lists
     *
     * Used extensively in:
     * - Standard File dialogs (file/folder lists)
     * - Font selection dialogs
     * - Color picker lists
     * - Custom application lists
     *
     * This initialization:
     * 1. Sets up internal dialog-list registry via InitRegistryIfNeeded()
     * 2. Prepares data structures for LNew() calls
     * 3. Initializes list manager globals
     *
     * After this call, applications can use LNew() to create lists.
     */
    extern void InitRegistryIfNeeded(void);

    /* Set up internal registry for dialog-attached lists */
    InitRegistryIfNeeded();

    /* List Manager is now ready for LNew() calls */
}

/* Event Manager */
/* InitEvents now implemented in EventManager/EventManagerCore.c */

/* Removed DISABLED GetNextEvent stub (real implementation lives in EventManager) */

/* Removed DISABLED PostEvent stub (real implementation lives in EventManager) */

/* GenerateSystemEvent now provided by EventManager/event_manager.c */
/* Forward declaration for compatibility */
extern void GenerateSystemEvent(short eventType, int message, Point where, short modifiers);

/* SystemTask provided by DeskManagerCore.c */

/* External functions we use */

/* ExpandMem stubs for SystemInit
 * These functions manage the Expanded Memory structure introduced in System 7.
 * ExpandMem contains global state and handles that don't fit in the original
 * 256-byte low memory area. It's allocated in the system heap during boot. */

void ExpandMemInit(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInit: Initializing expanded memory\n");
    /* In a full implementation, this would:
     * 1. Allocate ExpandMem handle in system heap (typically ~512 bytes)
     * 2. Initialize ExpandMem structure with magic number 'expm'
     * 3. Set up pointers to low-memory globals
     * 4. Initialize expansion slots and device structures
     * 5. Store handle in low-memory global at 0x02B6 (ExpandMemRec)
     *
     * For kernel environment, expanded memory is managed elsewhere */
}

void ExpandMemInitKeyboard(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInitKeyboard: Initializing keyboard expanded memory\n");
    /* In a full implementation, this would:
     * 1. Set up keyboard state in ExpandMem
     * 2. Initialize keyboard repeat rate (ExpandMem+0x1A)
     * 3. Set keyboard type (ExpandMem+0x1C)
     * 4. Configure dead key state for international keyboards
     *
     * For kernel environment, keyboard state is in EventManager globals */
}

void ExpandMemSetAppleTalkInactive(void) {
    SYSTEM_LOG_DEBUG("ExpandMemSetAppleTalkInactive: Disabling AppleTalk\n");
    /* In a full implementation, this would:
     * 1. Clear AppleTalk active flag in ExpandMem+0x4E
     * 2. Mark AppleTalk driver as inactive
     * 3. Prevent AppleTalk from loading at system startup
     *
     * For kernel environment without networking, this is a no-op */
}
void SetAutoDecompression(Boolean enable) {
    SYSTEM_LOG_DEBUG("SetAutoDecompression: %s\n", enable ? "Enabled" : "Disabled");
    /* In a full implementation, this would:
     * 1. Set resource decompression flag in ExpandMem
     * 2. Control automatic decompression of compressed resources
     * 3. Used by Resource Manager when loading 'dcmp' resources
     *
     * For kernel environment, resources are not compressed */
    (void)enable;
}

void ResourceManager_SetDecompressionCacheSize(Size size) {
    SYSTEM_LOG_DEBUG("ResourceManager_SetDecompressionCacheSize: Setting cache to %lu bytes\n", (unsigned long)size);
    /* In a full implementation, this would:
     * 1. Allocate decompression cache in system heap
     * 2. Configure cache size for decompressed resource data
     * 3. Improve performance for frequently-accessed compressed resources
     *
     * For kernel environment, no compression cache needed */
    (void)size;
}

void InstallDecompressHook(DecompressHookProc proc) {
    SYSTEM_LOG_DEBUG("InstallDecompressHook: Installing hook at %p\n", proc);
    /* In a full implementation, this would:
     * 1. Store decompression hook pointer in ExpandMem
     * 2. Allow custom decompression algorithms
     * 3. Called by Resource Manager for 'dcmp' resources
     *
     * For kernel environment, no custom decompression */
    (void)proc;
}

void ExpandMemInstallDecompressor(void) {
    SYSTEM_LOG_DEBUG("ExpandMemInstallDecompressor: Installing default decompressor\n");
    /* In a full implementation, this would:
     * 1. Install default resource decompressor hook
     * 2. Support standard System 7 compression algorithms
     * 3. Register with Resource Manager for automatic decompression
     *
     * For kernel environment, resources are not compressed */
}

void ExpandMemCleanup(void) {
    SYSTEM_LOG_DEBUG("ExpandMemCleanup: Cleaning up expanded memory\n");
    /* In a full implementation, this would:
     * 1. Dispose ExpandMem handle from system heap
     * 2. Clear low-memory global at 0x02B6
     * 3. Release any expansion slot structures
     * 4. Called during system shutdown
     *
     * For kernel environment, cleanup handled by memory manager */
}

void ExpandMemDump(void) {
    SYSTEM_LOG_DEBUG("ExpandMemDump: Dumping expanded memory state\n");
    /* Debug function to dump ExpandMem contents
     * In a full implementation, this would:
     * 1. Print ExpandMem structure contents to debugger
     * 2. Show magic number, size, version
     * 3. Display keyboard state, AppleTalk flags, etc.
     * 4. Useful for debugging system initialization issues
     *
     * For kernel environment, this is a no-op */
}
Boolean ExpandMemValidate(void) {
    /* Validate expanded memory structure integrity */

    /* In a full implementation, this would:
     * 1. Check expanded memory magic number/signature
     * 2. Verify critical low-memory globals are non-null
     * 3. Validate memory pointers are within valid ranges
     * 4. Check for memory corruption indicators
     */

    /* For now, always return valid */
    /* Real implementation would perform sanity checks */
    return true;
}

/* Serial stubs */
#include <stdarg.h>

/* serial_printf moved to System71StdLib.c */

/* Finder InitializeFinder provided by finder_main.c */

/* QuickDraw globals - defined in main.c */
extern QDGlobals qd;

/* Window manager globals */
WindowPtr g_firstWindow = NULL;  /* Head of window chain */

void FinderEventLoop(void) {
    /* Main Finder event processing loop */
    /* This processes events and dispatches to appropriate handlers */

    extern Boolean WaitNextEvent(SInt16 eventMask, EventRecord* theEvent,
                                 UInt32 sleep, RgnHandle mouseRgn);
    extern void DoBackgroundTasks(void);

    EventRecord event;
    Boolean gotEvent;

    /* Event loop runs until quit */
    while (true) {
        /* Wait for next event with background processing */
        gotEvent = WaitNextEvent(everyEvent, &event, 30, NULL);

        if (gotEvent) {
            /* Dispatch event to appropriate handler */
            /* In full implementation, would call:
             * - HandleMouseDown, HandleKeyDown, HandleUpdate
             * - HandleActivate, HandleOSEvent, etc.
             */
        }

        /* Perform background tasks during idle time */
        DoBackgroundTasks();
    }
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
long sysconf(int name) {
    /* POSIX system configuration query function
     * Returns runtime configuration limits and options */

    /* Common sysconf constants (from POSIX unistd.h) */
    #define _SC_PAGESIZE 1
    #define _SC_CLK_TCK 2
    #define _SC_OPEN_MAX 4
    #define _SC_NPROCESSORS_ONLN 58

    switch (name) {
        case _SC_PAGESIZE:
            /* Memory page size in bytes */
            return 4096;

        case _SC_CLK_TCK:
            /* Clock ticks per second for times() function */
            return 60;  /* 60 Hz for Mac System 7 */

        case _SC_OPEN_MAX:
            /* Maximum number of open files per process */
            return 256;

        case _SC_NPROCESSORS_ONLN:
            /* Number of online processors */
            return 1;  /* Single-processor system */

        default:
            /* Unknown configuration parameter */
            return -1;
    }
}

/* Resource Manager functions provided by Memory Manager */

/* HandleKeyDown moved to Finder/finder_main.c */

/* ResolveAliasFile moved to Finder/alias_manager.c */

#ifndef ENABLE_RESOURCES
void ReleaseResource(Handle theResource) {
    if (!theResource) return;

    /* Release a resource handle loaded from resource fork */
    /* Marks the resource as purgeable so Memory Manager can free it if needed */

    extern void HPurge(Handle h);

    /* Make the resource purgeable */
    HPurge(theResource);

    /* In a full implementation, this would also:
     * - Update resource map to mark resource as not loaded
     * - Clear any resource attributes set during loading
     * - Allow the handle to be purged by Memory Manager
     */
}
#endif

/* NewAlias moved to Finder/alias_manager.c */

/* Memory Manager functions provided by MemoryManager.c */

OSErr FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag) {
    if (!spec) {
        return paramErr;
    }

    /* Create a new resource file with empty resource fork */

    /* In a full implementation, this would:
     * 1. Create the file using FSpCreate with specified creator/type
     * 2. Create an empty resource fork
     * 3. Write an empty resource map to the resource fork
     * 4. Close the file
     */

    /* For now, just call FSpCreate to create the data fork */
    OSErr err = FSpCreate(spec, creator, fileType, scriptTag);
    if (err != noErr) {
        return err;
    }

    /* Resource fork initialization would happen here in full implementation */

    return noErr;
}

/* File Manager stubs - Core functions now implemented in FileManager.c */

OSErr FSpCreate(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag) {
    if (!spec) {
        return paramErr;
    }

    /* Create a new file with specified creator and type */

    /* In a full implementation, this would:
     * 1. Validate the FSSpec (volume and parent directory exist)
     * 2. Check if file already exists (return dupFNErr if so)
     * 3. Create directory entry with name, creator, type
     * 4. Set file dates (creation, modification)
     * 5. Initialize empty data fork
     */

    /* For now, return success */
    /* Real implementation would call File Manager PBHCreate */
    (void)creator;
    (void)fileType;
    (void)scriptTag;

    return noErr;
}

OSErr FSpOpenDF(const FSSpec* spec, SInt16 permission, SInt16* refNum) {
    if (!spec || !refNum) {
        return paramErr;
    }

    /* Open the data fork of a file */

    /* In a full implementation, this would:
     * 1. Validate the FSSpec points to an existing file
     * 2. Check permission flags (read, write, read/write)
     * 3. Allocate a file control block (FCB)
     * 4. Open the data fork via File Manager
     * 5. Return the file reference number
     */

    /* Return a fake file reference number for now */
    *refNum = 1;

    (void)permission;

    return noErr;
}

OSErr FSpOpenResFile(const FSSpec* spec, SInt16 permission) {
    if (!spec) {
        return -1; /* Invalid resource file reference */
    }

    /* Open the resource fork of a file */

    /* In a full implementation, this would:
     * 1. Validate the FSSpec points to an existing file
     * 2. Open the resource fork
     * 3. Read and parse the resource map
     * 4. Add the file to the resource file chain
     * 5. Return the resource file reference number
     */

    /* Permission values:
     * fsCurPerm (0) - read/write based on file permissions
     * fsRdPerm (1) - read-only
     * fsWrPerm (2) - write-only
     * fsRdWrPerm (3) - read/write
     */

    /* Return fake resource file reference number */
    (void)permission;
    return 1;
}

OSErr FSpDelete(const FSSpec* spec) {
    if (!spec) {
        return paramErr;
    }

    /* Delete a file or empty directory */

    /* In a full implementation, this would:
     * 1. Validate the FSSpec points to an existing file or directory
     * 2. Check if item is a directory - if so, verify it's empty
     * 3. Remove the item from its parent directory
     * 4. Release disk space allocated to the item
     * 5. Update the volume's free space count
     */

    /* For now, return success */
    return noErr;
}

OSErr FSpDirDelete(const FSSpec* spec) {
    if (!spec) {
        return paramErr;
    }

    /* Delete a directory and all its contents recursively */

    /* In a full implementation, this would:
     * 1. Validate the FSSpec points to a directory
     * 2. Recursively delete all files and subdirectories
     * 3. Delete the directory itself after emptying
     * 4. Return appropriate errors for locked or busy files
     */

    /* This is more dangerous than FSpDelete which requires empty directory */
    /* For now, return success */
    return noErr;
}

OSErr FSpCatMove(const FSSpec* source, const FSSpec* dest) {
    if (!source || !dest) {
        return paramErr;
    }

    /* Move or rename a file or directory */

    /* In a full implementation, this would:
     * 1. Validate source exists and dest doesn't exist
     * 2. Check if moving to different volume (requires copy+delete)
     * 3. If same volume, update directory entry
     * 4. If different volume, copy data then delete source
     * 5. Preserve file metadata (creator, type, dates)
     */

    /* For now, return success */
    return noErr;
}


OSErr PBHGetVInfoSync(void *paramBlock) {
    if (!paramBlock) {
        return paramErr;
    }

    HParamBlockRec* pb = (HParamBlockRec*)paramBlock;

    /* Get volume information synchronously */

    /* In a full implementation, this would:
     * 1. Validate the volume reference number
     * 2. Retrieve actual volume parameters from the volume control block
     * 3. Fill in allocation block size, total blocks, free blocks
     * 4. Set volume name, creation date, modification date
     * 5. Return volume attributes (locked, ejectable, etc.)
     */

    /* Fill in simulated volume info for 400K floppy disk */
    pb->u.volumeParam.ioVAlBlkSiz = 512;     /* Allocation block size in bytes */
    pb->u.volumeParam.ioVNmAlBlks = 800;     /* Total allocation blocks (400K) */
    pb->u.volumeParam.ioVFrBlk = 400;        /* Free blocks (50% free) */

    return noErr;
}


OSErr SetEOF(short refNum, long logEOF) {
    if (refNum <= 0) {
        return paramErr;
    }

    if (logEOF < 0) {
        return paramErr;
    }

    /* Set the logical end-of-file for an open file */

    /* In a full implementation, this would:
     * 1. Validate the file reference number
     * 2. If growing file, allocate additional disk blocks
     * 3. If shrinking file, release unused disk blocks
     * 4. Update the file control block with new EOF
     * 5. Update directory entry if needed
     */

    /* For now, return success */
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
    if (!theData) return;

    /* Add a resource to the current resource file */
    /* This adds the resource to the resource map but doesn't write to disk yet */

    /* In a full implementation, this would:
     * 1. Get the current resource file reference
     * 2. Add an entry to the resource map with type, ID, and name
     * 3. Associate the handle with the resource entry
     * 4. Mark the resource file as modified
     * 5. The actual data is written when UpdateResFile or CloseResFile is called
     */

    /* For now, just mark the handle as a resource */
    extern void HNoPurge(Handle h);
    HNoPurge(theData); /* Make resource non-purgeable */

    (void)theType;
    (void)theID;
    (void)name;
}

void RemoveResource(Handle theResource) {
    if (!theResource) return;

    /* Remove a resource from the resource map */
    /* This detaches the resource from its resource file but doesn't dispose the handle */

    /* In a full implementation, this would:
     * 1. Find the resource in the resource map
     * 2. Remove it from the map
     * 3. Mark the resource file as modified
     * 4. The handle remains valid but is no longer a resource
     */

    /* Make the handle purgeable since it's no longer a resource */
    extern void HPurge(Handle h);
    HPurge(theResource);
}

void WriteResource(Handle theResource) {
    if (!theResource) return;

    /* Write changed resource data to the resource file */
    /* This updates the resource in the resource fork */

    /* In a full implementation, this would:
     * 1. Find the resource in the resource map
     * 2. Calculate the size and position in the resource fork
     * 3. Write the handle data to the resource file
     * 4. Update the resource map with new size/position
     * 5. Mark the resource file as modified
     */

    /* For now, just mark the handle as not purgeable to ensure data is preserved */
    extern void HNoPurge(Handle h);
    HNoPurge(theResource);
}

void CloseResFile(SInt16 refNum) {
    if (refNum <= 0) return;

    /* Close a resource file */
    /* This writes any changes and releases the resource map */

    /* In a full implementation, this would:
     * 1. Write any modified resources to disk
     * 2. Update the resource map on disk
     * 3. Close the file via File Manager
     * 4. Release the in-memory resource map
     * 5. Remove from the resource file chain
     */

    /* For now, just a no-op as we don't have full resource file management */
    (void)refNum;
}

OSErr ResError(void) {
    /* Return most recent Resource Manager error code
     *
     * In a full implementation (see ResourceMgr.c), this would:
     * 1. Return the last error code from gResMgr.resError
     * 2. Clear the error code after reading (one-time read)
     * 3. Track errors from GetResource, OpenResFile, etc.
     * 4. Return noErr if no error occurred
     *
     * Error codes include:
     * - resNotFound (-192): Resource not found
     * - mapReadErr (-199): Map inconsistent with operation
     * - resFNotFound (-193): Resource file not found
     * - addResFailed (-194): AddResource failed
     *
     * For builds without ENABLE_RESOURCES, always return noErr */
    return noErr;
}
#endif

/* AddResMenu and InsertResMenu now implemented in MenuManager/menu_stubs.c
 * - AddResMenu: Enumerates resources and appends names to menu
 * - InsertResMenu: Enumerates resources and inserts names at position */

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
    /* Show Get Info dialog for selected items in front window */
    extern WindowPtr FrontWindow(void);
    extern OSErr ShowGetInfo(FSSpec *items, short count);

    WindowPtr window = FrontWindow();
    if (!window) {
        return paramErr;
    }

    /* In a full implementation, this would:
     * 1. Get selected items from the front window
     * 2. Build an array of FSSpec for selected items
     * 3. Call ShowGetInfo with the array
     */

    /* For now, show Get Info for a dummy item */
    /* Real implementation would get selection from window */
    FSSpec dummyItem;
    dummyItem.vRefNum = 0;
    dummyItem.parID = 0;
    dummyItem.name[0] = 0;

    return ShowGetInfo(&dummyItem, 1);
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
    /* Handle window activation/deactivation events
     *
     * Called when a window gains or loses focus. Updates the visual
     * state of all controls in the window to reflect active/inactive status.
     *
     * Parameters:
     *   window - Window being activated or deactivated
     *   activate - true to activate, false to deactivate
     *
     * Activation behavior:
     * - Active windows: Controls drawn normally (hilite = 0)
     * - Inactive windows: Controls drawn dimmed (hilite = 255)
     * - Title bar appearance changes (handled by Window Manager)
     * - Scroll bars change from active to inactive appearance
     *
     * This is typically called in response to:
     * - activateEvt event with activeFlag bit set/clear
     * - User clicking on a different window
     * - Application switching (suspend/resume events)
     *
     * The function walks the window's control list and updates each
     * control's hilite state, then redraws all controls to show the
     * new visual state.
     */
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
    /* Perform idle-time system tasks during event loop
     *
     * Called from FinderEventLoop and other event loops to give time
     * to background processes when no user events are pending.
     *
     * Primary tasks:
     * 1. Call SystemTask() to update Desk Accessories
     * 2. Allow DA windows to process periodic tasks
     * 3. Update desk accessory menu items
     * 4. Service VBL (vertical blanking) tasks
     *
     * In a full implementation, additional background work includes:
     * - Check for disk insertions/ejections
     * - Update network status indicators
     * - Perform deferred memory cleanup
     * - Process asynchronous I/O completions
     * - Update AppleTalk status
     * - Service printer queues
     *
     * This is critical for cooperative multitasking - without calling
     * DoBackgroundTasks(), desk accessories and system tasks won't run.
     */
    extern void SystemTask(void);
    SystemTask();
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

/* InvalRect removed - implemented in WindowManager/WindowEvents.c:348 */

OSErr ScanDirectoryForDesktopEntries(SInt16 vRefNum, SInt32 dirID, SInt16 databaseRefNum) {
    /* Scan a directory and add file/folder entries to desktop database */

    /* In a full implementation, this would:
     * 1. Enumerate all files and folders in the directory using PBGetCatInfo
     * 2. For each file, extract creator, type, icon, and comment info
     * 3. Add entries to the desktop database file
     * 4. Update the database index for fast lookups
     * 5. Handle subdirectories recursively if needed
     */

    /* Validate parameters */
    if (vRefNum == 0 || databaseRefNum <= 0) {
        return paramErr;
    }

    /* For now, return success without scanning */
    /* Real implementation would iterate directory entries */
    (void)dirID;

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
/* BeginUpdate and EndUpdate removed - implemented in WindowManager/WindowEvents.c
 * - BeginUpdate at line 499
 * - EndUpdate at line 650
 */

/* SetDeskHook moved to WindowManager/WindowDisplay.c */

/* [WM-053] QuickDraw region drawing functions
 * Removed stubs (now disabled by default via SYS71_STUBS_DISABLED=1):
 * - FillRgn - implemented in QuickDraw/Regions.c:621
 * - RectInRgn - implemented in QuickDraw/Regions.c:557
 */

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
