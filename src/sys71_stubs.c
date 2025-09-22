/* System 7.1 Stubs for linking */
#include "../include/MacTypes.h"
#include "../include/QuickDraw/QuickDraw.h"
#include "../include/QuickDraw/QDRegions.h"
#include "../include/QuickDrawConstants.h"
#include "../include/ResourceManager.h"
#include "../include/EventManager/EventTypes.h"  /* Include EventTypes first to define activeFlag */
#include "../include/EventManager/EventManager.h"
#include "../include/WindowManager/WindowManager.h"
#include "../include/WindowManager/WindowManagerInternal.h"  /* For WindowManagerState */
#include "../include/MenuManager/MenuManager.h"
#include "../include/DialogManager/DialogManager.h"
#include "../include/ControlManager/ControlManager.h"
#include "../include/ListManager/ListManager.h"
#include "../include/TextEdit/TextEdit.h"
#include "../include/FontManager/FontManager.h"

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
void InitResourceManager(void) {
    /* Stub implementation */
}

/* QuickDraw - InitGraf is now provided by QuickDrawCore.c */

void InitCursor(void) {
    /* Stub implementation */
}

/* Font Manager */
void InitFonts(void) {
    /* Stub implementation */
}

/* Window Manager functions provided by WindowManagerCore.c and WindowDisplay.c */

void DrawControls(WindowPtr window) {
    /* Stub implementation */
}

/* Menu Manager - Most functions now provided by MenuManagerCore.c */

void AppendMenu(MenuHandle menu, ConstStr255Param data) {
    /* Stub implementation */
}

/* TextEdit */
void InitTE(void) {
    /* Stub implementation */
}

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

Boolean GetNextEvent(short eventMask, EventRecord* theEvent) {
    /* Always return no event */
    return false;
}

/* SystemTask provided by DeskManagerCore.c */

SInt16 PostEvent(SInt16 eventNum, SInt32 eventMsg) {
    /* Simple event posting - just log for now */
    extern void serial_printf(const char* fmt, ...);
    serial_printf("PostEvent: type=%d msg=0x%08x\n", eventNum, eventMsg);
    return noErr;
}

/* GenerateSystemEvent - forward declaration to avoid header conflicts */
void GenerateSystemEvent(short eventType, int message, Point where, short modifiers);
void GenerateSystemEvent(short eventType, int message, Point where, short modifiers) {
    /* Generate and post a system event */
    PostEvent(eventType, message);
}

/* ExpandMem stubs for SystemInit */
void ExpandMemInit(void) {}
void ExpandMemInitKeyboard(void) {}
void ExpandMemSetAppleTalkInactive(void) {}
void SetAutoDecompression(Boolean enable) {}
void ResourceManager_SetDecompressionCacheSize(Size size) {}
void InstallDecompressHook(DecompressHookProc proc) {}
void ExpandMemInstallDecompressor(void) {}
void ExpandMemCleanup(void) {}
void ExpandMemDump(void) {}
Boolean ExpandMemValidate(void) { return true; }

/* Serial stubs */
#include <stdarg.h>

/* serial_printf moved to System71StdLib.c */

/* Finder InitializeFinder provided by finder_main.c */

/* QuickDraw globals - defined in main.c */
extern QDGlobals qd;

void FinderEventLoop(void) {
    /* Stub implementation */
}

/* Additional Finder support functions */
void FlushEvents(short whichMask, short stopMask) {
    /* Stub - would flush event queue */
}

void TEInit(void) {
    /* TextEdit initialization stub */
}

OSErr FindWindow(Point thePt, WindowPtr *window) {
    /* Stub - would find which window contains point */
    return noErr;
}

void DragWindow(WindowPtr window, Point startPt, const Rect* limitRect) {
    /* Stub - would handle window dragging */
}

/* Functions provided by other components:
 * ShowErrorDialog - finder_main.c
 * CleanUpDesktop - desktop_manager.c
 * DrawDesktop - desktop_manager.c
 */

/* System stubs */
long sysconf(int name) { return -1; }

/* Resource Manager functions provided by Memory Manager */

/* Missing function implementations */
void HandleKeyDown(EventRecord* event) {
    /* Stub */
}

OSErr ResolveAliasFile(const FSSpec* spec, FSSpec* target, Boolean* wasAliased, Boolean* wasFolder) {
    if (target) *target = *spec;
    if (wasAliased) *wasAliased = false;
    if (wasFolder) *wasFolder = false;
    return noErr;
}

void ReleaseResource(Handle theResource) {
    /* Stub */
}

OSErr NewAlias(const FSSpec* fromFile, const FSSpec* target, AliasHandle* alias) {
    if (alias) *alias = (AliasHandle)NewHandle(sizeof(AliasRecord));
    return noErr;
}

/* GetHandleSize provided by Memory Manager */

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


OSErr PBHGetVInfoSync(HParamBlockRec* pb) {
    if (pb) {
        pb->u.volumeParam.ioVAlBlkSiz = 512;
        pb->u.volumeParam.ioVNmAlBlks = 800;  /* Simulate 400K disk */
    }
    return noErr;
}


OSErr SetEOF(SInt16 refNum, SInt32 logEOF) {
    return noErr;
}

/* Resource Manager stubs */
/* NewHandle and DisposeHandle provided by Memory Manager */
/* GetResource provided by simple_resource_manager.c */

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

void AddResMenu(MenuHandle theMenu, ResType theType) {
    /* Stub - would add resources to menu */
}

/* Memory Manager functions provided by Memory Manager */
/* MemError moved to System71StdLib.c */

void BlockMoveData(const void* srcPtr, void* destPtr, Size byteCount) {
    unsigned char* d = destPtr;
    const unsigned char* s = srcPtr;
    while (byteCount--) *d++ = *s++;
}

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

OSErr ShowFind(void) {
    return noErr;
}

OSErr FindAgain(void) {
    return noErr;
}

OSErr ShowAboutFinder(void) {
    return noErr;
}

OSErr HandleContentClick(WindowPtr window, EventRecord* event) {
    return noErr;
}

OSErr HandleGrowWindow(WindowPtr window, EventRecord* event) {
    return noErr;
}

OSErr CloseFinderWindow(WindowPtr window) {
    return noErr;
}

Boolean TrackGoAway(WindowPtr window, Point pt) {
    return false;
}

Boolean TrackBox(WindowPtr window, Point pt, SInt16 part) {
    return false;
}

void ZoomWindow(WindowPtr window, SInt16 part, Boolean front) {
    /* Stub */
}

void DoUpdate(WindowPtr window) {
    /* Stub */
}

void DoActivate(WindowPtr window, Boolean activate) {
    /* Stub */
}

void DoBackgroundTasks(void) {
    /* Stub */
}

Boolean WaitNextEvent(SInt16 eventMask, EventRecord* theEvent, UInt32 sleep, RgnHandle mouseRgn) {
    static int count = 0;
    if (++count > 10) return false;  /* Exit after 10 iterations */
    return false;  /* No events */
}

/* Menu and Window functions provided by their respective managers:
 * MenuSelect - MenuSelection.c
 * SystemClick - DeskManagerCore.c
 * HiliteMenu - MenuManagerCore.c
 * FrontWindow - WindowDisplay.c
 */

OSErr ShowConfirmDialog(ConstStr255Param message, Boolean* confirmed) {
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

OSErr CleanUpWindow(WindowPtr window, SInt16 cleanupType) {
    return noErr;
}

/* ParamText provided by DialogManagerCore.c */

SInt16 Alert(SInt16 alertID, ModalFilterProcPtr filterProc) {
    return 1;  /* OK button */
}

UInt32 TickCount(void) {
    static UInt32 ticks = 0;
    return ticks++;
}

void GetMenuItemText(MenuHandle menu, SInt16 item, Str255 itemString) {
    if (itemString) {
        itemString[0] = 0;  /* Empty string */
    }
}

/* OpenDeskAcc provided by DeskManagerCore.c */

SInt16 HiWord(SInt32 x) {
    return (x >> 16) & 0xFFFF;
}

SInt16 LoWord(SInt32 x) {
    return x & 0xFFFF;
}

void InvalRect(const Rect* badRect) {
    /* Stub */
}

OSErr ScanDirectoryForDesktopEntries(SInt16 vRefNum, SInt32 dirID, SInt16 databaseRefNum) {
    return noErr;
}

/* Region Manager stubs */
RgnHandle NewRgn(void) {
    static Region dummyRegion;
    static Region* regionPtr = &dummyRegion;
    dummyRegion.rgnSize = sizeof(Region);
    SetRect(&dummyRegion.rgnBBox, 0, 0, 1024, 768);
    return (RgnHandle)&regionPtr;
}

void DisposeRgn(RgnHandle rgn) {
    /* Stub */
}

void RectRgn(RgnHandle rgn, const Rect* r) {
    if (rgn && r && *rgn) {
        (*rgn)->rgnBBox = *r;
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

/* Standard library functions moved to System71StdLib.c:
 * sprintf, __assert_fail, strlen, abs
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

/* DeskHook support */
static DeskHookProc g_deskHook = NULL;

/* External QuickDraw globals */
extern QDGlobals qd;
extern void* framebuffer;

/* Window Manager update pipeline functions */
void WM_Update(void) {
    /* Create a screen port if qd.thePort is NULL */
    static GrafPort screenPort;
    if (qd.thePort == NULL) {
        /* Initialize the screen port */
        OpenPort(&screenPort);
        screenPort.portBits = qd.screenBits;  /* Use screen bitmap */
        screenPort.portRect = qd.screenBits.bounds;
        qd.thePort = &screenPort;
    }

    /* Use QuickDraw to draw desktop */
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(qd.thePort);  /* Draw to screen port */

    /* 1. Menu Manager draws the menu bar */
    DrawMenuBar();  /* Menu Manager handles menu bar */

    /* 2. Draw desktop pattern */
    Rect desktopRect;
    SetRect(&desktopRect, 0, 20,
            qd.screenBits.bounds.right,
            qd.screenBits.bounds.bottom);
    FillRect(&desktopRect, &qd.gray);  /* Gray desktop pattern */

    /* 3. Draw all visible windows */
    extern WindowManagerState* GetWindowManagerState(void);
    WindowManagerState* wmState = GetWindowManagerState();
    if (wmState && wmState->windowList) {
        /* Draw windows from back to front */
        WindowPtr window = wmState->windowList;
        WindowPtr* windowStack = NULL;
        int windowCount = 0;

        /* Count windows and build stack */
        while (window) {
            windowCount++;
            window = window->nextWindow;
        }

        if (windowCount > 0) {
            windowStack = (WindowPtr*)malloc(windowCount * sizeof(WindowPtr));
            if (windowStack) {
                /* Fill stack */
                window = wmState->windowList;
                for (int i = 0; i < windowCount && window; i++) {
                    windowStack[i] = window;
                    window = window->nextWindow;
                }

                /* Draw windows from back to front */
                for (int i = windowCount - 1; i >= 0; i--) {
                    WindowPtr w = windowStack[i];
                    if (w && w->visible) {
                        /* Draw window frame */
                        Rect frameRect = w->port.portRect;

                        /* Draw window shadow */
                        Rect shadowRect = frameRect;
                        OffsetRect(&shadowRect, 2, 2);
                        PenPat(&qd.black);
                        FrameRect(&shadowRect);

                        /* Draw window background */
                        PenPat(&qd.white);
                        PaintRect(&frameRect);

                        /* Draw window frame */
                        PenPat(&qd.black);
                        FrameRect(&frameRect);

                        /* Draw title bar */
                        Rect titleBar = frameRect;
                        titleBar.bottom = titleBar.top + 20;

                        if (w->hilited) {
                            /* Active window - draw with stripes */
                            for (int y = titleBar.top; y < titleBar.bottom; y += 2) {
                                MoveTo(titleBar.left, y);
                                LineTo(titleBar.right - 1, y);
                            }
                        } else {
                            /* Inactive window - solid gray */
                            FillRect(&titleBar, &qd.gray);
                        }

                        /* Draw window title if present */
                        if (w->titleHandle && *(w->titleHandle)) {
                            MoveTo(titleBar.left + 25, titleBar.top + 14);
                            DrawString(*(w->titleHandle));
                        }

                        /* Draw close box if present */
                        if (w->goAwayFlag) {
                            Rect closeBox;
                            closeBox.left = frameRect.left + 2;
                            closeBox.top = frameRect.top + 2;
                            closeBox.right = closeBox.left + 16;
                            closeBox.bottom = closeBox.top + 16;
                            FrameRect(&closeBox);
                        }
                    }
                }

                free(windowStack);
            }
        }
    }

    /* 4. Call DeskHook if registered for icons */
    if (g_deskHook) {
        /* Create a region for the desktop */
        RgnHandle desktopRgn = NewRgn();
        RectRgn(desktopRgn, &desktopRect);
        g_deskHook(desktopRgn);
        DisposeRgn(desktopRgn);
    }

    /* Mouse cursor is now drawn separately in main.c for better performance */
    /* Old cursor drawing code removed to prevent double cursor issue */

    SetPort(savePort);
}

void BeginUpdate(WindowPtr theWindow) {
    /* Stub - would save port and set up clipping */
}

void EndUpdate(WindowPtr theWindow) {
    /* Stub - would restore port and clear update region */
}

void SetDeskHook(DeskHookProc proc) {
    g_deskHook = proc;
}

/* QuickDraw region drawing functions */
void FillRgn(RgnHandle rgn, ConstPatternParam pat) {
    /* Stub - would fill region with pattern */
}

Boolean RectInRgn(const Rect *r, RgnHandle rgn) {
    /* Stub - check if rectangle intersects region */
    return true;
}