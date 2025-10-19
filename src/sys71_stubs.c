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
    /* Stub implementation */
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

/* Window stubs for functions not yet implemented elsewhere */
#if !defined(SYS71_STUBS_DISABLED)
void InitWindows(void) {
    SYSTEM_LOG_DEBUG("InitWindows: Initializing window manager\n");

    /* Set up window list chain */
    extern WindowPtr g_firstWindow;
    g_firstWindow = NULL;

    /* Initialize desktop port */
    extern QDGlobals qd;
    if (qd.thePort == NULL) {
        /* Create default desktop port */
        SYSTEM_LOG_DEBUG("InitWindows: Creating default desktop port\n");
    }
}

#ifdef SYS71_PROVIDE_FINDER_TOOLBOX
/* NewWindow stub - this old stub is DISABLED when SYS71_PROVIDE_FINDER_TOOLBOX=1 */
/* The real implementation is in WindowManagerCore.c */
#else
WindowPtr NewWindow(void* storage, const Rect* boundsRect, ConstStr255Param title,
                    Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag,
                    long refCon) {
    SYSTEM_LOG_DEBUG("NewWindow (STUB): Creating window procID=%d visible=%d\n", procID, visible);

    /* Allocate window structure if no storage provided */
    WindowPtr window;
    if (storage) {
        window = (WindowPtr)storage;
    } else {
        /* Allocate from heap - use multiple static windows for now */
        static WindowRecord windows[10];  /* Support up to 10 windows */
        static int nextWindow = 0;

        if (nextWindow >= 10) {
            SYSTEM_LOG_DEBUG("NewWindow: Out of window slots!\n");
            return NULL;
        }

        window = (WindowPtr)&windows[nextWindow++];
        memset(window, 0, sizeof(WindowRecord));
    }

    /* Initialize window fields */
    GrafPort* port = (GrafPort*)window;
    if (boundsRect) {
        port->portRect = *boundsRect;
    } else {
        /* Default window size */
        port->portRect.left = 100;
        port->portRect.top = 100;
        port->portRect.right = 500;
        port->portRect.bottom = 400;
    }

    window->visible = visible;
    window->windowKind = procID;  /* Use procID as window kind */
    window->hilited = false;
    window->goAwayFlag = goAwayFlag;
    window->refCon = refCon;

    /* Copy title if provided */
    if (title && title[0] > 0) {
        /* Would copy to window's title storage */
        SYSTEM_LOG_DEBUG("NewWindow: Title length = %d\n", title[0]);
    }

    /* Insert into window chain */
    extern WindowPtr g_firstWindow;
    if (behind == (WindowPtr)-1L) {
        /* Add to end of chain */
        WindowPtr last = g_firstWindow;
        if (last) {
            while (last->nextWindow) {
                last = last->nextWindow;
            }
            last->nextWindow = window;
        } else {
            g_firstWindow = window;
        }
        window->nextWindow = NULL;
    } else if (behind == NULL) {
        /* Add to front */
        window->nextWindow = g_firstWindow;
        g_firstWindow = window;
    } else {
        /* Insert after specific window */
        window->nextWindow = behind->nextWindow;
        behind->nextWindow = window;
    }

    /* Set up graf port fields */
    port->portBits.baseAddr = NULL;  /* Would point to window's pixels */
    port->portBits.rowBytes = 0;
    port->portBits.bounds = port->portRect;

    SYSTEM_LOG_DEBUG("NewWindow: Created window at %p\n", window);
    return window;
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

void DisposeWindow(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("DisposeWindow: NULL window\n");
        return;
    }

    SYSTEM_LOG_DEBUG("DisposeWindow: Disposing window at %p\n", window);

    /* Remove from window chain */
    extern WindowPtr g_firstWindow;
    if (g_firstWindow == window) {
        g_firstWindow = window->nextWindow;
    } else {
        WindowPtr prev = g_firstWindow;
        while (prev && prev->nextWindow != window) {
            prev = prev->nextWindow;
        }
        if (prev) {
            prev->nextWindow = window->nextWindow;
        }
    }

    /* Free window storage - would normally free memory */
    /* For now just mark it invalid */
    window->windowKind = 0;
}
/* DragWindow is implemented in src/WindowManager/WindowDragging.c */

void MoveWindow(WindowPtr window, short h, short v, Boolean front) {
    if (!window) return;

    /* CRITICAL: portRect should ALWAYS stay in LOCAL coordinates (0,0,width,height)!
     * Only portBits.bounds changes to map local coords to global screen position.
     *
     * IMPORTANT: h,v are WINDOW FRAME coords, but portBits.bounds must map to CONTENT area!
     * Content area starts 21px down from frame (20px title + 1px separator) */
    const short kBorder = 1, kTitle = 20, kSeparator = 1;

    GrafPort* port = (GrafPort*)window;
    short width = port->portRect.right - port->portRect.left;
    short height = port->portRect.bottom - port->portRect.top;

    /* portRect stays local - DO NOT modify it */

    /* portBits.bounds maps local (0,0) to global content area position */
    port->portBits.bounds.left = h + kBorder;
    port->portBits.bounds.top = v + kTitle + kSeparator;
    port->portBits.bounds.right = h + kBorder + width;
    port->portBits.bounds.bottom = v + kTitle + kSeparator + height;

    if (front) {
        SelectWindow(window);
    }
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

#if !defined(SYS71_STUBS_DISABLED)
void CloseWindow(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("CloseWindow: NULL window\n");
        return;
    }

    SYSTEM_LOG_DEBUG("CloseWindow: Closing window at %p\n", window);

    /* Hide the window */
    window->visible = false;

    /* Generate close event if needed */
    /* DisposeWindow handles actual disposal */
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

#if !defined(SYS71_STUBS_DISABLED)
void ShowWindow(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("ShowWindow: NULL window\n");
        return;
    }

    SYSTEM_LOG_DEBUG("ShowWindow (sys71_stubs.c): Showing window at %p\n", window);

    window->visible = true;

    /* Call PaintOne to actually draw the window */
    extern void PaintOne(WindowPtr window, RgnHandle clobberedRgn);
    extern void CalcVis(WindowPtr window);
    extern void CalcVisBehind(WindowPtr startWindow, RgnHandle clobberedRgn);

    CalcVis(window);
    PaintOne(window, NULL);
    CalcVisBehind(window->nextWindow, window->strucRgn);
}
/* HideWindow stub removed - real implementation in WindowManager/WindowDisplay.c */
/* void HideWindow(WindowPtr window) { ... } */
void SelectWindow(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("SelectWindow: NULL window\n");
        return;
    }

    SYSTEM_LOG_DEBUG("SelectWindow: Selecting window at %p\n", window);

    /* Move window to front of chain */
    extern WindowPtr g_firstWindow;
    if (g_firstWindow != window) {
        /* Remove from current position */
        WindowPtr prev = g_firstWindow;
        while (prev && prev->nextWindow != window) {
            prev = prev->nextWindow;
        }
        if (prev) {
            prev->nextWindow = window->nextWindow;
        }

        /* Insert at front */
        window->nextWindow = g_firstWindow;
        g_firstWindow = window;
    }

    /* Set as active window */
    window->hilited = true;
}
WindowPtr FrontWindow(void) {
    extern WindowPtr g_firstWindow;

    /* Find first visible window */
    WindowPtr window = g_firstWindow;
    while (window) {
        if (window->visible) {
            return window;
        }
        window = window->nextWindow;
    }

    return NULL;
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */
/* SetWTitle moved to WindowManager/WindowManagerCore.c */
#if !defined(SYS71_STUBS_DISABLED)
void DrawGrowIcon(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("DrawGrowIcon: NULL window\n");
        return;
    }

    if (!window->visible) {
        return;
    }

    SYSTEM_LOG_DEBUG("DrawGrowIcon: Drawing grow icon for window at %p\n", window);

    /* Draw grow icon in bottom-right corner */
    GrafPort* port = (GrafPort*)window;
    Rect growRect;
    growRect.right = port->portRect.right;
    growRect.bottom = port->portRect.bottom;
    growRect.left = growRect.right - 15;
    growRect.top = growRect.bottom - 15;

    /* Would draw grow box pattern here */
    extern void FrameRect(const Rect* r);
    FrameRect(&growRect);
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */
void WM_UpdateWindowVisibility(WindowPtr window) {
    if (!window) {
        SYSTEM_LOG_DEBUG("WM_UpdateWindowVisibility: NULL window\n");
        return;
    }

    SYSTEM_LOG_DEBUG("WM_UpdateWindowVisibility: Updating visibility for window at %p\n", window);

    /* Update window visibility state */
    if (window->visible) {
        /* Ensure window is drawn */
        extern void InvalRect(const Rect* rect);
        GrafPort* port = (GrafPort*)window;
        InvalRect(&port->portRect);
    }
}

#if !defined(SYS71_STUBS_DISABLED)
short FindWindow(Point thePt, WindowPtr *window) {

    if (window) {
        *window = NULL;
    }

    /* Check if click is in menu bar (top 20 pixels) */
    if (thePt.v >= 0 && thePt.v < 20) {
        SYSTEM_LOG_DEBUG("FindWindow: Click in menu bar at v=%d\n", thePt.v);
        return inMenuBar;
    }

    /* Search through visible windows from front to back */
    extern WindowPtr FrontWindow(void);

    WindowPtr currentWindow = FrontWindow();

    while (currentWindow) {
        if (!currentWindow->visible) {
            currentWindow = currentWindow->nextWindow;
            continue;
        }

        /* Get the window structure region (frame + title bar) */
        if (currentWindow->strucRgn) {
            /* Use Point-in-Region test */
            extern Boolean PtInRgn(Point pt, RgnHandle rgn);
            if (PtInRgn(thePt, currentWindow->strucRgn)) {
                *window = currentWindow;

                /* Determine which part of the window was hit */
                Rect frame = currentWindow->port.portRect;

                /* Title bar area: top to top+20 */
                if (thePt.v >= frame.top && thePt.v < frame.top + 20) {
                    /* Check for close box (top-left) */
                    if (thePt.h >= frame.left + 4 && thePt.h < frame.left + 18 &&
                        thePt.v >= frame.top + 4 && thePt.v < frame.top + 18) {
                        return inGoAway;
                    }

                    /* Check for zoom box (top-right) */
                    if (currentWindow->spareFlag) {  /* spareFlag indicates zoom enabled */
                        if (thePt.h >= frame.right - 20 && thePt.h < frame.right - 4 &&
                            thePt.v >= frame.top + 4 && thePt.v < frame.top + 16) {
                            return inZoomIn;  /* Could be inZoomOut if at other position */
                        }
                    }

                    /* Otherwise it's the drag area (title bar) */
                    return inDrag;
                }

                /* Grow box area: bottom-right corner 16x16 */
                if (currentWindow->windowKind >= 0) {  /* Document windows have grow box */
                    if (thePt.h >= frame.right - 16 && thePt.h <= frame.right &&
                        thePt.v >= frame.bottom - 16 && thePt.v <= frame.bottom) {
                        return inGrow;
                    }
                }

                /* Content area: everything else inside the window */
                if (thePt.h > frame.left && thePt.h < frame.right &&
                    thePt.v > frame.top + 20 && thePt.v < frame.bottom) {
                    return inContent;
                }

                /* Must be in frame/structure area but not specifically recognized */
                return inContent;
            }
        }

        currentWindow = currentWindow->nextWindow;
    }

    /* No window hit, check desktop */
    return inDesk;
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* DragWindow is implemented in WindowManager/WindowDragging.c
void DragWindow(WindowPtr window, Point startPt, const Rect* limitRect) {
    // Stub - would handle window dragging
}
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

OSErr ResolveAliasFile(const FSSpec* spec, FSSpec* target, Boolean* wasAliased, Boolean* wasFolder) {
    if (target) *target = *spec;
    if (wasAliased) *wasAliased = false;
    if (wasFolder) *wasFolder = false;
    return noErr;
}

#ifndef ENABLE_RESOURCES
void ReleaseResource(Handle theResource) {
    /* Stub */
}
#endif

OSErr NewAlias(const FSSpec* fromFile, const FSSpec* target, AliasHandle* alias) {
    if (alias) *alias = (AliasHandle)NewHandle(sizeof(AliasRecord));
    return noErr;
}

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
OSErr FindFolder(SInt16 vRefNum, OSType folderType, Boolean createFolder, SInt16* foundVRefNum, SInt32* foundDirID) {
    if (foundVRefNum) *foundVRefNum = vRefNum;
    if (foundDirID) *foundDirID = 2;  /* Root directory */
    return noErr;
}

OSErr GenerateUniqueTrashName(Str255 baseName, Str255 uniqueName) {
    if (uniqueName && baseName) {
        BlockMoveData(baseName, uniqueName, baseName[0] + 1);
    }
    return noErr;
}

OSErr InitializeWindowManager(void) {
    return noErr;
}

/* InitializeTrashFolder provided by trash_folder.c */

OSErr HandleGetInfo(void) {
    return noErr;
}

/* ShowFind and FindAgain implemented in src/Finder/Find.c */

OSErr ShowAboutFinder(void) {
    return noErr;
}

/* HandleContentClick moved to Finder/finder_main.c */

OSErr HandleGrowWindow(WindowPtr window, EventRecord* event) {
    return noErr;
}

/* CloseFinderWindow moved to Finder/finder_main.c */

#if !defined(SYS71_STUBS_DISABLED)
Boolean TrackGoAway(WindowPtr window, Point pt) {
    return false;
}

Boolean TrackBox(WindowPtr window, Point pt, SInt16 part) {
    return false;
}

void ZoomWindow(WindowPtr window, SInt16 part, Boolean front) {
    /* Stub */
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

/* DoUpdate moved to Finder/finder_main.c */

void DoActivate(WindowPtr window, Boolean activate) {
    /* Stub */
}

void DoBackgroundTasks(void) {
    /* Stub */
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
    if (confirmed) *confirmed = true;  /* Always confirm for testing */
    return noErr;
}

OSErr CloseAllWindows(void) {
    return noErr;
}

OSErr CleanUpSelection(WindowPtr window) {
    return noErr;
}

OSErr CleanUpBy(WindowPtr window, SInt16 sortType) {
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

/* [WM-053] QuickDraw Region stubs - only for bootstrap, real implementations in QuickDraw/Regions.c */
/* NewRgn and DisposeRgn moved to QuickDraw/Regions.c - removed to fix multiple definition linker errors */
#if !defined(SYS71_STUBS_DISABLED)

void RectRgn(RgnHandle rgn, const Rect* r) {
    if (rgn && r && *rgn) {
        (*rgn)->rgnBBox = *r;
    }
}

void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom) {
    if (rgn && *rgn) {
        SetRect(&(*rgn)->rgnBBox, left, top, right, bottom);
        (*rgn)->rgnSize = sizeof(Region);
    }
}

void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn) {
    if (srcRgn && dstRgn && *srcRgn && *dstRgn) {
        **dstRgn = **srcRgn;
    }
}

void SetEmptyRgn(RgnHandle rgn) {
    if (rgn && *rgn) {
        SetRect(&(*rgn)->rgnBBox, 0, 0, 0, 0);
        (*rgn)->rgnSize = sizeof(Region);
    }
}
#endif /* !SYS71_PROVIDE_FINDER_TOOLBOX */

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
    /* Minimal delay stub - just return current tick count */
    if (finalTicks) {
        extern UInt32 TickCount(void);
        *finalTicks = TickCount();
    }
}
