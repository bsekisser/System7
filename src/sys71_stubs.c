/* System 7.1 Stubs for linking */
#include "../include/MacTypes.h"
#include "../include/QuickDraw/QuickDraw.h"
#include "../include/QuickDraw/QDRegions.h"
#include "../include/QuickDrawConstants.h"
#include "../include/ResourceManager.h"
#include "../include/WindowManager/WindowManager.h"
#include "../include/MenuManager/MenuManager.h"
#include "../include/EventManager/EventManager.h"
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

/* SetPort is now provided by QuickDrawCore.c */

/* Font Manager */
void InitFonts(void) {
    /* Stub implementation */
}

/* Window Manager - InitWindows is in WindowManagerCore.c */
/* NewWindow is in WindowManagerCore.c */
/* ShowWindow is in WindowDisplay.c */

/* SelectWindow is now in WindowDisplay.c */
/*
void SelectWindow(WindowPtr window) {
    // Stub until fully implemented
}
*/

void DrawControls(WindowPtr window) {
    /* Stub implementation */
}

/* DrawGrowIcon is now in WindowDisplay.c */
/*
void DrawGrowIcon(WindowPtr window) {
    // Stub implementation
}
*/

/* Menu Manager - Most functions now provided by MenuManagerCore.c */

void AppendMenu(MenuHandle menu, ConstStr255Param data) {
    /* Stub implementation */
}

/* TextEdit */
void InitTE(void) {
    /* Stub implementation */
}

/* Dialog Manager */
void InitDialogs(ResumeProcPtr resumeProc) {
    /* Stub implementation */
}

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

#ifndef DESKMANAGER_INCLUDED
void SystemTask(void) {
    /* Poll PS/2 input on each system task */
    extern void PollPS2Input(void);
    PollPS2Input();
}
#endif

SInt16 PostEvent(SInt16 eventNum, SInt32 eventMsg) {
    /* Simple event posting - just log for now */
    extern void serial_printf(const char* fmt, ...);
    serial_printf("PostEvent: type=%d msg=0x%08x\n", eventNum, eventMsg);
    return noErr;
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

void serial_printf(const char* fmt, ...) {
    /* Simple implementation that only handles %d and %02x */
    extern void serial_puts(const char* str);

    const char* p = fmt;
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    int buf_idx = 0;

    while (*p && buf_idx < 250) {
        if (*p == '%' && *(p+1)) {
            p++;
            if (*p == 'd') {
                /* Decimal number */
                int val = va_arg(args, int);
                char num[12];
                int idx = 0;
                if (val < 0) {
                    buffer[buf_idx++] = '-';
                    val = -val;
                }
                if (val == 0) {
                    buffer[buf_idx++] = '0';
                } else {
                    while (val > 0) {
                        num[idx++] = '0' + (val % 10);
                        val /= 10;
                    }
                    while (idx > 0) {
                        buffer[buf_idx++] = num[--idx];
                    }
                }
            } else if (*p == 'u') {
                /* Unsigned decimal number */
                unsigned int val = va_arg(args, unsigned int);
                char num[12];
                int idx = 0;
                if (val == 0) {
                    buffer[buf_idx++] = '0';
                } else {
                    while (val > 0) {
                        num[idx++] = '0' + (val % 10);
                        val /= 10;
                    }
                    while (idx > 0) {
                        buffer[buf_idx++] = num[--idx];
                    }
                }
            } else if (*p == 'x' || (*p == '0' && *(p+1) && (*(p+1) == '4' || *(p+1) == '8') && *(p+2) == 'x')) {
                /* Hex value - %x, %04x, %08x */
                int digits = 8;  /* Default to 8 hex digits */
                if (*p == '0') {
                    if (*(p+1) == '4') {
                        digits = 4;
                        p += 2;  /* Skip "04" */
                    } else if (*(p+1) == '8') {
                        digits = 8;
                        p += 2;  /* Skip "08" */
                    }
                }
                unsigned int val = va_arg(args, unsigned int);
                const char* hex = "0123456789abcdef";
                for (int i = digits - 1; i >= 0; i--) {
                    buffer[buf_idx++] = hex[(val >> (i*4)) & 0xF];
                }
            } else if (*p == '0' && *(p+1) == '2' && *(p+2) == 'x') {
                /* Hex byte (legacy, keep for compatibility) */
                p += 2;
                unsigned int val = va_arg(args, unsigned int) & 0xFF;
                const char* hex = "0123456789abcdef";
                buffer[buf_idx++] = hex[(val >> 4) & 0xF];
                buffer[buf_idx++] = hex[val & 0xF];
            } else if (*p == 's') {
                /* String */
                const char* str = va_arg(args, const char*);
                if (str) {
                    while (*str && buf_idx < 250) {
                        buffer[buf_idx++] = *str++;
                    }
                } else {
                    buffer[buf_idx++] = '(';
                    buffer[buf_idx++] = 'n';
                    buffer[buf_idx++] = 'u';
                    buffer[buf_idx++] = 'l';
                    buffer[buf_idx++] = 'l';
                    buffer[buf_idx++] = ')';
                }
            } else if (*p == 'p') {
                /* Pointer */
                void* ptr = va_arg(args, void*);
                buffer[buf_idx++] = '0';
                buffer[buf_idx++] = 'x';
                unsigned int val = (unsigned int)ptr;
                const char* hex = "0123456789abcdef";
                for (int i = 7; i >= 0; i--) {
                    buffer[buf_idx++] = hex[(val >> (i*4)) & 0xF];
                }
            } else {
                buffer[buf_idx++] = '%';
                buffer[buf_idx++] = *p;
            }
        } else {
            buffer[buf_idx++] = *p;
        }
        p++;
    }

    buffer[buf_idx] = '\0';
    serial_puts(buffer);
    va_end(args);
}

/* Finder InitializeFinder is now provided by finder_main.c */

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

/* ShowErrorDialog is now provided by finder_main.c */
/* CleanUpDesktop is now provided by desktop_manager.c */
/* DrawDesktop is now provided by desktop_manager.c */

/* Stub DrawDesktop for testing only - not used anymore */
#if 0
void DrawDesktop_Stub(void) {
    extern QDGlobals qd;
    extern GrafPtr g_currentPort;
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;

    /* Add debug output */
    serial_puts("DrawDesktop: g_currentPort=");
    serial_print_hex((uint32_t)g_currentPort);
    serial_puts(" qd.screenBits.baseAddr=");
    serial_print_hex((uint32_t)qd.screenBits.baseAddr);
    serial_puts(" framebuffer=");
    serial_print_hex((uint32_t)framebuffer);
    serial_puts("\n");
    serial_puts("  fb_width=");
    serial_print_hex(fb_width);
    serial_puts(" fb_height=");
    serial_print_hex(fb_height);
    serial_puts(" fb_pitch=");
    serial_print_hex(fb_pitch);
    serial_puts("\n");

    /* Show actual framebuffer format */
    extern uint8_t fb_red_pos, fb_red_size;
    extern uint8_t fb_green_pos, fb_green_size;
    extern uint8_t fb_blue_pos, fb_blue_size;
    serial_puts("  FB format: R[pos=");
    serial_print_hex(fb_red_pos);
    serial_puts(" size=");
    serial_print_hex(fb_red_size);
    serial_puts("] G[pos=");
    serial_print_hex(fb_green_pos);
    serial_puts(" size=");
    serial_print_hex(fb_green_size);
    serial_puts("] B[pos=");
    serial_print_hex(fb_blue_pos);
    serial_puts(" size=");
    serial_print_hex(fb_blue_size);
    serial_puts("]\n");

    /* Test pixel writing */
    uint32_t white = pack_color(255, 255, 255);
    uint32_t black = pack_color(0, 0, 0);
    uint32_t red = pack_color(255, 0, 0);
    serial_puts("  Colors: white=");
    serial_print_hex(white);
    serial_puts(" black=");
    serial_print_hex(black);
    serial_puts(" red=");
    serial_print_hex(red);
    serial_puts("\n");

    /* Draw the Finder desktop */
    if (framebuffer) {
        volatile uint32_t* fb = (volatile uint32_t*)framebuffer;

        serial_puts("  Drawing Finder desktop at framebuffer ");
        serial_print_hex((uint32_t)framebuffer);
        serial_puts("\n");

        /* Clear entire screen with desktop pattern */
        for (int y = 0; y < fb_height; y++) {
            for (int x = 0; x < fb_width; x++) {
                uint32_t* pixel = (uint32_t*)((uint8_t*)fb + y * fb_pitch + x * 4);

                /* Classic Mac desktop pattern */
                int patY = y % 2;
                int patX = x % 2;

                if ((patY + patX) % 2 == 0) {
                    *pixel = 0x00FFFFFF;  /* White */
                } else {
                    *pixel = 0x00C0C0C0;  /* Light gray */
                }
            }
        }

        /* Draw white menu bar */
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < fb_width; x++) {
                uint32_t* pixel = (uint32_t*)((uint8_t*)fb + y * fb_pitch + x * 4);
                *pixel = 0x00FFFFFF;  /* White */
            }
        }

        /* Draw menu bar bottom line */
        for (int x = 0; x < fb_width; x++) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)fb + 19 * fb_pitch + x * 4);
            *pixel = 0x00000000;  /* Black */
        }

        serial_puts("DrawDesktop: Direct framebuffer drawing complete\n");
    }
}
#endif

#if 0  /* Orphaned window creation code */
    if (window1) {
        /* Draw the window */
        ShowWindow(window1);
        /* Draw window frame and content */
        SetPort(&window1->port);
        EraseRect(&window1->port.portRect);
        FrameRect(&window1->port.portRect);
        /* Draw title bar */
        Rect titleBar;
        SetRect(&titleBar, window1->port.portRect.left, window1->port.portRect.top,
                window1->port.portRect.right, window1->port.portRect.top + 20);
        for (int y = titleBar.top; y < titleBar.bottom; y += 2) {
            MoveTo(titleBar.left, y);
            LineTo(titleBar.right - 1, y);
        }
    }

    /* Create "System Folder" window */
    SetRect(&windowBounds, 400, 100, 650, 350);
    window2 = NewWindow(NULL, &windowBounds, "\pSystem Folder", true,
                       documentProc, (WindowPtr)-1L, true, 0);

    if (window2) {
        ShowWindow(window2);
        /* Draw window frame and content */
        SetPort(&window2->port);
        EraseRect(&window2->port.portRect);
        FrameRect(&window2->port.portRect);
        /* Draw title bar */
        Rect titleBar;
        SetRect(&titleBar, window2->port.portRect.left, window2->port.portRect.top,
                window2->port.portRect.right, window2->port.portRect.top + 20);
        for (int y = titleBar.top; y < titleBar.bottom; y += 2) {
            MoveTo(titleBar.left, y);
            LineTo(titleBar.right - 1, y);
        }
    }
#endif  /* End of orphaned window creation code */

#if 0  /* Orphaned trash drawing code */
    /* Draw trash can icon placeholder at bottom right */
    Rect trashRect;
    SetRect(&trashRect, fb_width - 100, fb_height - 100, fb_width - 50, fb_height - 50);
    EraseRect(&trashRect);
    FrameRect(&trashRect);

    /* Draw some text in the menu bar to show it's working */
    MoveTo(10, 15);
    /* Note: Text drawing needs Font Manager, just draw some lines for now */
    LineTo(50, 15);  /* Placeholder for "File" */
    MoveTo(60, 15);
    LineTo(100, 15); /* Placeholder for "Edit" */
#endif  /* End of orphaned trash drawing code */

/* System stubs */
long sysconf(int name) { return -1; }

/* Resource Manager - must come before other functions that use it */
#if 0  /* Now provided by Memory Manager */
Handle NewHandle(Size byteCount) {
    static char dummy[256];
    return (Handle)&dummy;
}
#endif

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

#if 0  /* Now provided by Memory Manager */
SInt32 GetHandleSize(Handle h) {
    return sizeof(AliasRecord);
}
#endif

OSErr FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag) {
    return noErr;
}

/* File Manager stubs */
OSErr FSMakeFSSpec(SInt16 vRefNum, SInt32 dirID, ConstStr255Param fileName, FSSpec* spec) {
    if (spec && fileName) {
        spec->vRefNum = vRefNum;
        spec->parID = dirID;
        BlockMoveData(fileName, spec->name, fileName[0] + 1);
    }
    return noErr;
}

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

OSErr PBGetCatInfoSync(CInfoPBRec* pb) {
    static int count = 0;
    if (++count > 5) return fnfErr;  /* Return no more files after 5 */
    return noErr;
}

OSErr PBSetCatInfoSync(CInfoPBRec* pb) {
    return noErr;
}

OSErr PBHGetVInfoSync(HParamBlockRec* pb) {
    if (pb) {
        pb->u.volumeParam.ioVAlBlkSiz = 512;
        pb->u.volumeParam.ioVNmAlBlks = 800;  /* Simulate 400K disk */
    }
    return noErr;
}

SInt16 FSRead(SInt16 refNum, SInt32* count, void* buffPtr) {
    if (count) *count = 0;
    return noErr;
}

SInt16 FSWrite(SInt16 refNum, SInt32* count, const void* buffPtr) {
    return noErr;
}

SInt16 FSClose(SInt16 refNum) {
    return noErr;
}

OSErr SetEOF(SInt16 refNum, SInt32 logEOF) {
    return noErr;
}

/* Resource Manager stubs */
/* NewHandle is already defined above */

#if 0  /* Now provided by Memory Manager */
void DisposeHandle(Handle h) {
    /* Stub */
}
#endif

/* GetResource is now implemented in simple_resource_manager.c */
/* Handle GetResource(ResType theType, SInt16 theID) {
    return NewHandle(4);
} */

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

/* Memory Manager stubs */
#if 0  /* Now provided by Memory Manager */
Ptr NewPtr(Size byteCount) {
    static unsigned char heap[65536];
    static size_t heap_ptr = 0;
    if (heap_ptr + byteCount > sizeof(heap)) return nil;
    Ptr ptr = (Ptr)&heap[heap_ptr];
    heap_ptr += byteCount;
    return ptr;
}
#endif

/* MemError moved to System71StdLib.c
OSErr MemError(void) {
    return noErr;
}
*/

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

/* InitializeTrashFolder is now provided by trash_folder.c */

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

/* MenuSelect now implemented in MenuSelection.c */

#ifndef DESKMANAGER_INCLUDED
void SystemClick(EventRecord* theEvent, WindowPtr window) {
    /* Stub */
}
#endif

/* HiliteMenu now provided by MenuManagerCore.c */

/* FrontWindow is now in WindowDisplay.c */
/*
WindowPtr FrontWindow(void) {
    return nil;
}
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

void ParamText(const unsigned char* param0, const unsigned char* param1, const unsigned char* param2, const unsigned char* param3) {
    /* Stub */
}

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

#ifndef DESKMANAGER_INCLUDED
SInt16 OpenDeskAcc(ConstStr255Param name) {
    return 0;
}
#endif

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

/* Standard library stubs */
int sprintf(char* str, const char* format, ...) {
    if (str) str[0] = 0;
    return 0;
}

/* Assert implementation */
void __assert_fail(const char* expr, const char* file, int line, const char* func) {
    /* In production, just ignore asserts */
}

/* strlen moved to System71StdLib.c
size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}
*/

/* Math library minimal implementations */
/* abs moved to System71StdLib.c
int abs(int n) {
    return (n < 0) ? -n : n;
}
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

/* memcpy moved to System71StdLib.c
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}
*/

/* memset moved to System71StdLib.c
void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
*/

/* memcmp moved to System71StdLib.c
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) return -1;
        if (p1[i] > p2[i]) return 1;
    }
    return 0;
}
*/

/* memmove moved to System71StdLib.c
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;

    if (d < s) {
        // Forward copy
        while (n--) *d++ = *s++;
    } else if (d > s) {
        // Backward copy
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}
*/

/* Memory allocation now handled by Memory Manager */
#if 0
void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    static unsigned char heap[65536];
    static size_t heap_ptr = 0;

    if (heap_ptr + total > sizeof(heap)) return 0;
    void* ptr = &heap[heap_ptr];
    heap_ptr += total;
    memset(ptr, 0, total);
    return ptr;
}

void* malloc(size_t size) {
    static unsigned char heap2[65536];
    static size_t heap_ptr = 0;

    if (heap_ptr + size > sizeof(heap2)) return 0;
    void* ptr = &heap2[heap_ptr];
    heap_ptr += size;
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    /* Simple implementation - just allocate new memory */
    void* newptr = malloc(size);
    if (newptr && ptr) {
        /* Copy old data (we don't know the old size, so copy min) */
        memcpy(newptr, ptr, size);
    }
    return newptr;
}

void free(void* ptr) {
    /* Simple allocator doesn't free */
}
#endif

/* strncpy moved to System71StdLib.c
char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    while (n--) *d++ = '\0';
    return dest;
}
*/

int snprintf(char* str, size_t size, const char* format, ...) {
    /* Minimal implementation - just copy format string */
    if (size == 0) return 0;
    size_t i = 0;
    while (format[i] && i < size - 1) {
        str[i] = format[i];
        i++;
    }
    str[i] = '\0';
    return i;
}

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

    /* 3. Call DeskHook if registered for icons */
    if (g_deskHook) {
        /* Create a region for the desktop */
        RgnHandle desktopRgn = NewRgn();
        RectRgn(desktopRgn, &desktopRect);
        g_deskHook(desktopRgn);
        DisposeRgn(desktopRgn);
    }

    /* 4. Draw the mouse cursor */
    /* Get mouse position from PS2Controller */
    extern struct {
        int16_t x;
        int16_t y;
        uint8_t buttons;
        uint8_t packet[3];
        uint8_t packet_index;
    } g_mouseState;

    /* Draw a simple arrow cursor at the mouse position */
    /* Arrow cursor bitmap (16x16) - classic Mac arrow */
    static const uint16_t cursorBitmap[16] = {
        0x8000, /* #............... */
        0xC000, /* ##.............. */
        0xE000, /* ###............. */
        0xF000, /* ####............ */
        0xF800, /* #####........... */
        0xFC00, /* ######.......... */
        0xFE00, /* #######......... */
        0xFF00, /* ########........ */
        0xFF80, /* #########....... */
        0xFFC0, /* ##########...... */
        0xFE00, /* #######......... */
        0xEF00, /* ###.####........ */
        0xCF00, /* ##..####........ */
        0x0780, /* .....####....... */
        0x0780, /* .....####....... */
        0x0300  /* ......##........ */
    };

    /* Draw cursor pixels directly to framebuffer */
    uint32_t* fb = (uint32_t*)framebuffer;
    int cursorX = g_mouseState.x;
    int cursorY = g_mouseState.y;

    /* Draw cursor with black outline and white fill */
    for (int y = 0; y < 16; y++) {
        if (cursorY + y >= 0 && cursorY + y < (int)fb_height) {
            uint16_t row = cursorBitmap[y];
            for (int x = 0; x < 16; x++) {
                if (cursorX + x >= 0 && cursorX + x < (int)fb_width) {
                    if (row & (0x8000 >> x)) {
                        /* Draw black pixel for cursor */
                        fb[(cursorY + y) * (fb_pitch / 4) + (cursorX + x)] = 0xFF000000;
                    }
                }
            }
        }
    }

    /* Draw white interior (smaller mask) */
    static const uint16_t cursorInterior[16] = {
        0x0000, /* ................ */
        0x4000, /* .#.............. */
        0x6000, /* .##............. */
        0x7000, /* .###............ */
        0x7800, /* .####........... */
        0x7C00, /* .#####.......... */
        0x7E00, /* .######......... */
        0x7F00, /* .#######........ */
        0x7F80, /* .########....... */
        0x7C00, /* .#####.......... */
        0x6C00, /* .##.##.......... */
        0x4600, /* .#...##......... */
        0x0600, /* .....##......... */
        0x0300, /* ......##........ */
        0x0300, /* ......##........ */
        0x0000  /* ................ */
    };

    for (int y = 0; y < 16; y++) {
        if (cursorY + y >= 0 && cursorY + y < (int)fb_height) {
            uint16_t row = cursorInterior[y];
            for (int x = 0; x < 16; x++) {
                if (cursorX + x >= 0 && cursorX + x < (int)fb_width) {
                    if (row & (0x8000 >> x)) {
                        /* Draw white pixel for cursor interior */
                        fb[(cursorY + y) * (fb_pitch / 4) + (cursorX + x)] = 0xFFFFFFFF;
                    }
                }
            }
        }
    }

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