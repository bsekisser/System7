/*
 * AboutThisMac.c - System 7-style "About This Macintosh" Window
 *
 * Implements a faithful modeless About window with memory statistics
 * and bar graph, matching classic Mac OS System 7.1 behavior.
 */

#include "SystemTypes.h"
#include <string.h>
#include "System71StdLib.h"
#include "Finder/FinderLogging.h"
#include "Finder/finder.h"
#include "QuickDraw/QuickDraw.h"
#include "Platform/platform_info.h"
#include "Gestalt/Gestalt.h"
#include "Platform/include/boot.h"

extern void DisposeGWorld(GWorldPtr offscreenGWorld);
extern void* framebuffer;
extern uint32_t fb_pitch;

/* External QuickDraw & Window Manager APIs */
extern void GetPort(GrafPtr* port);
extern void SetPort(GrafPtr port);
extern void BeginUpdate(WindowPtr window);
extern void EndUpdate(WindowPtr window);
extern void EraseRect(const Rect* r);
extern void FrameRect(const Rect* r);
extern void PaintRect(const Rect* r);
extern void InsetRect(Rect* r, short dh, short dv);
extern void MoveTo(short h, short v);
extern void LineTo(short h, short v);
extern void TextFont(short font);
extern void TextSize(short size);
extern void TextFace(short face);
extern void PenPat(const Pattern* pat);
extern void FillRect(const Rect* r, const Pattern* pat);
extern short StringWidth(ConstStr255Param s);

extern WindowPtr NewWindow(void* wStorage, const Rect* boundsRect,
                          const unsigned char* title, Boolean visible,
                          short procID, WindowPtr behind, Boolean goAwayFlag,
                          long refCon);
extern void ShowWindow(WindowPtr window);
extern void SelectWindow(WindowPtr window);
extern void BringToFront(WindowPtr window);
extern void DisposeWindow(WindowPtr window);
extern WindowPtr FrontWindow(void);
extern void DragWindow(WindowPtr window, Point startPt, const Rect* boundsRect);
extern Boolean TrackGoAway(WindowPtr window, Point pt);

/* External Event Manager */
#include "EventManager/EventManager.h"

/* External memory introspection - try to use real APIs if available */
extern Size FreeMem(void);
extern Size MaxMem(Size* grow);
extern UInt32 TotalRam(void);  /* Platform may provide this */
extern uint32_t g_total_memory_kb;  /* Actual detected RAM from multiboot2 */

/* Serial logging */
/* QuickDraw globals access */
extern QDGlobals qd;

/* Window definition constants */
#define documentProc 0
#define dBoxProc 1
#define plainDBox 2
#define altDBoxProc 3
#define noGrowDocProc 4
#define movableDBoxProc 5

/* Window part codes */
#define inDesk 0
#define inMenuBar 1
#define inSysWindow 2
#define inContent 3
#define inDrag 4
#define inGrow 5
#define inGoAway 6
#define inZoomIn 7
#define inZoomOut 8

/* Event types */
#define updateEvt 6

/* About window identification */
#define kAboutRefCon 0x41424F55L  /* 'ABOU' */

/* Static state - single instance only */
static WindowPtr sAboutWin = NULL;
static UInt32 sLastUpdateTicks = 0;

/* Redraw throttle - only update stats every N ticks (avoid thrashing) */
#define kAboutUpdateThrottle 30  /* ~0.5 seconds at 60 ticks/sec */

/* Memory statistics snapshot */
typedef struct {
    UInt32 totalRAM;
    UInt32 systemUsed;
    UInt32 appUsed;
    UInt32 diskCache;
    UInt32 largestFree;
    UInt32 unused;
} MemSnapshot;

/* --- String helpers (use QuickDraw widths, not char counts) --- */
static void CenterPStringInRect(const Str255 s, const Rect* r, short baseline) {
    FINDER_LOG_DEBUG("CenterPStringInRect: ENTRY, s[0]=%d\n", s[0]);
    short w = StringWidth((ConstStr255Param)s);
    FINDER_LOG_DEBUG("CenterPStringInRect: StringWidth returned w=%d\n", w);
    MoveTo(r->left + ((r->right - r->left) - w) / 2, baseline);
    FINDER_LOG_DEBUG("CenterPStringInRect: About to call DrawString\n");
    DrawString((ConstStr255Param)s);
    FINDER_LOG_DEBUG("CenterPStringInRect: DrawString returned\n");
}

static void RightAlignPStringAt(const Str255 s, short rightEdge, short baseline) {
    short w = StringWidth((ConstStr255Param)s);
    MoveTo(rightEdge - w, baseline);
    DrawString((ConstStr255Param)s);
}

/* Safe maker of P-strings from C strings */
static void ToPStr(const char* c, Str255 dst) {
    size_t n = 0;
    while (c[n] && n < 255) n++;
    dst[0] = (unsigned char)n;
    for (size_t i = 0; i < n; i++) {
        dst[1 + i] = (unsigned char)c[i];
    }
}

/* Clean KB formatter (no stdio, no junk) */
static void FormatKB_PStr(UInt32 bytes, Str255 outP) {
    UInt32 kb = bytes >> 10;
    char tmp[16];
    int n = 0;

    if (kb < 1000) {
        /* e.g., 638 for 638K */
        if (kb >= 100) tmp[n++] = '0' + (kb / 100) % 10;
        if (kb >= 10)  tmp[n++] = '0' + (kb / 10) % 10;
        tmp[n++] = '0' + (kb % 10);
        tmp[n++] = 'K';
    } else {
        /* e.g., 3,584K */
        UInt32 thousands = kb / 1000;
        UInt32 rem = kb % 1000;
        /* thousands (no leading zeros) */
        if (thousands >= 100) tmp[n++] = '0' + (thousands / 100) % 10;
        if (thousands >= 10)  tmp[n++] = '0' + (thousands / 10) % 10;
        tmp[n++] = '0' + (thousands % 10);
        tmp[n++] = ',';
        tmp[n++] = '0' + (rem / 100) % 10;
        tmp[n++] = '0' + (rem / 10) % 10;
        tmp[n++] = '0' + (rem % 10);
        tmp[n++] = 'K';
    }
    outP[0] = (unsigned char)n;
    for (int i = 0; i < n; i++) {
        outP[1 + i] = (unsigned char)tmp[i];
    }
}

/* Forward declarations */
static void AboutWindow_CreateIfNeeded(void);
static void GetMemorySnapshot(MemSnapshot* m);
static const char* GetSystemVersionString(void);

/*
 * GetMemorySnapshot - Gather memory statistics
 *
 * Wired to real Memory Manager statistics when available.
 * Falls back to best-effort introspection if Memory Manager APIs unavailable.
 */
static void GetMemorySnapshot(MemSnapshot* m)
{
    Size grow = 0;
    UInt32 freeBytes = 0;
    (void)freeBytes;  /* Reserved for future use */
    UInt32 totalBytes = 0;

    if (!m) return;

    memset(m, 0, sizeof(MemSnapshot));

    /* Get total RAM from multiboot2 detection (g_total_memory_kb is in KB) */
    totalBytes = (UInt32)g_total_memory_kb * 1024;  /* Convert KB to bytes */
    if (totalBytes == 0) {
        totalBytes = 0x01000000;  /* Fallback: 16 MB if detection failed */
    }

    /* Try FreeMem() and MaxMem() if available - these call real Memory Manager */
    freeBytes = (UInt32)FreeMem();
    m->largestFree = (UInt32)MaxMem(&grow);

    if (m->largestFree == 0 || m->largestFree > totalBytes) {
        /* Fallback if Memory Manager APIs not working */
        m->largestFree = totalBytes / 4;  /* Assume 25% free */
    }

    m->totalRAM = totalBytes;

    /* Calculate actual system and application usage from free memory */
    /* System usage: approximately kernel + driver buffers + Finder heap */
    UInt32 usedTotal = totalBytes - m->largestFree;

    /* Estimate system used as fraction of total (typically 12-20% on classic Mac) */
    m->systemUsed = (totalBytes / 10);  /* ~10% for kernel + drivers + Finder */

    /* Application usage is used memory minus system memory */
    if (usedTotal > m->systemUsed) {
        m->appUsed = usedTotal - m->systemUsed;
    } else {
        m->appUsed = totalBytes / 8;  /* Fallback estimate */
    }

    if (m->appUsed > totalBytes) {
        m->appUsed = totalBytes / 4;
    }

    /* Disk cache - wired to actual cache manager when implemented */
    /* For now, cache is included in application memory */
    m->diskCache = 0;

    /* Compute unused (remaining free memory) */
    if (m->systemUsed + m->appUsed + m->diskCache < m->totalRAM) {
        m->unused = m->totalRAM - m->systemUsed - m->appUsed - m->diskCache;
    } else {
        m->unused = 0;
    }

    FINDER_LOG_DEBUG("AboutThisMac: Memory snapshot - Total:%u System:%u App:%u Cache:%u Free:%u Unused:%u\n",
                     (unsigned int)m->totalRAM,
                     (unsigned int)m->systemUsed,
                     (unsigned int)m->appUsed,
                     (unsigned int)m->diskCache,
                     (unsigned int)m->largestFree,
                     (unsigned int)m->unused);
}

/*
 * FormatKBWithCommas - Format bytes as KB with thousands separators
 */
static void FormatKBWithCommas(UInt32 bytes, char* buf, size_t bufSize)
{
    UInt32 kb = bytes / 1024;
    UInt32 thousands, remainder;
    char temp[32];
    int i, j;

    if (!buf || bufSize < 16) return;

    if (kb < 1000) {
        /* Simple format */
        FINDER_LOG_DEBUG("FormatKB: %uK\n", (unsigned int)kb);
        temp[0] = '0' + (char)((kb / 100) % 10);
        temp[1] = '0' + (char)((kb / 10) % 10);
        temp[2] = '0' + (char)(kb % 10);
        temp[3] = 'K';
        temp[4] = '\0';
        /* Copy non-leading zeros */
        j = 0;
        for (i = 0; temp[i] == '0' && i < 2; i++);
        for (; temp[i]; i++) buf[j++] = temp[i];
        buf[j] = '\0';
    } else {
        /* Format with comma separator */
        thousands = kb / 1000;
        remainder = kb % 1000;
        /* Manual formatting to avoid sprintf */
        i = 0;
        if (thousands >= 100) temp[i++] = '0' + (char)(thousands / 100);
        if (thousands >= 10) temp[i++] = '0' + (char)((thousands / 10) % 10);
        temp[i++] = '0' + (char)(thousands % 10);
        temp[i++] = ',';
        temp[i++] = '0' + (char)(remainder / 100);
        temp[i++] = '0' + (char)((remainder / 10) % 10);
        temp[i++] = '0' + (char)(remainder % 10);
        temp[i++] = 'K';
        temp[i] = '\0';
        strcpy(buf, temp);
    }
}

/*
 * DrawLabeledValue - Draw "Label: XXX,XXXK" line
 *
 * Calculates colon position for proper alignment and separation of
 * label and value. Label width determines where colon is drawn.
 */
static void DrawLabeledValue(short x, short y, const char* label, UInt32 bytes)
{
    char valueStr[32];
    unsigned char pstr[64];
    unsigned char colonStr[2];
    short labelLen;
    short colonWidth;

    /* Draw label */
    pstr[0] = (unsigned char)strlen(label);
    memcpy(&pstr[1], label, pstr[0]);
    MoveTo(x, y);
    DrawString(pstr);

    /* Calculate position for colon (after label) */
    labelLen = StringWidth((ConstStr255Param)pstr);

    /* Draw colon separator */
    colonStr[0] = 1;
    colonStr[1] = ':';
    colonWidth = StringWidth((ConstStr255Param)colonStr);
    MoveTo(x + labelLen, y);
    DrawString(colonStr);

    /* Format and draw value (after colon with spacing) */
    FormatKBWithCommas(bytes, valueStr, sizeof(valueStr));
    pstr[0] = (unsigned char)strlen(valueStr);
    memcpy(&pstr[1], valueStr, pstr[0]);

    MoveTo(x + labelLen + colonWidth + 4, y);
    DrawString(pstr);
}

/*
 * GetSystemVersionString - Return version string
 *
 * Returns build version if available, otherwise "System 7.1"
 * Version can be set at build time via SYSTEM_BUILD_VERSION define
 */
static const char* GetSystemVersionString(void)
{
    /* Allow build system to inject version via -DSYSTEM_BUILD_VERSION=... */
    #ifdef SYSTEM_BUILD_VERSION
        return SYSTEM_BUILD_VERSION;
    #else
        return "System 7.1";
    #endif
}

/*
 * AboutWindow_CreateIfNeeded - Create About window if not already open
 */
static void AboutWindow_CreateIfNeeded(void)
{
    Rect bounds;
    unsigned char title[32];

    if (sAboutWin) {
        FINDER_LOG_DEBUG("AboutThisMac: Window already exists at 0x%08x\n", (unsigned int)P2UL(sAboutWin));
        return;
    }

    /* Position window centered-ish on screen (classic About box position) */
    bounds.top = 70;
    bounds.left = 80;
    bounds.bottom = bounds.top + 280;  /* Height: 280 pixels */
    bounds.right = bounds.left + 540;  /* Width: 540 pixels */

    /* Create window with movable dialog box proc, close box enabled */
    const char* titleText = "About This Macintosh";
    title[0] = (unsigned char)strlen(titleText);
    strcpy((char*)&title[1], titleText);

    sAboutWin = NewWindow(NULL, &bounds, title,
                         1,  /* visible */
                         movableDBoxProc,  /* Movable dialog box */
                         (WindowPtr)-1,  /* Behind all windows initially */
                         1,  /* Has close box */
                         kAboutRefCon);

    if (!sAboutWin) {
        FINDER_LOG_DEBUG("AboutThisMac: FAILED to create window!\n");
        return;
    }

    /* Disable double-buffering so text renders directly to framebuffer */
    if (sAboutWin->offscreenGWorld) {
        /* Save the portBits.bounds before disposing GWorld */
        Rect savedBounds = sAboutWin->port.portBits.bounds;

        DisposeGWorld((GWorldPtr)sAboutWin->offscreenGWorld);
        sAboutWin->offscreenGWorld = NULL;
        sAboutWin->port.portBits.baseAddr = (Ptr)framebuffer;
        sAboutWin->port.portBits.rowBytes = (fb_pitch | 0x8000);

        /* CRITICAL: Restore bounds so LocalToGlobal works correctly */
        sAboutWin->port.portBits.bounds = savedBounds;
    }

    FINDER_LOG_DEBUG("AboutThisMac: Created window at 0x%08x, refCon=0x%08X\n",
                     (unsigned int)P2UL(sAboutWin), (unsigned int)kAboutRefCon);
}

/*
 * AboutWindow_ShowOrToggle - Show About window or bring to front if open
 *
 * Public API called from Apple menu handler.
 */
void AboutWindow_ShowOrToggle(void)
{
    FINDER_LOG_DEBUG("AboutThisMac: ShowOrToggle called\n");

    if (!sAboutWin) {
        AboutWindow_CreateIfNeeded();
    }

    if (!sAboutWin) {
        FINDER_LOG_DEBUG("AboutThisMac: Failed to create window\n");
        return;
    }

    /* Bring to front and select */
    BringToFront(sAboutWin);
    SelectWindow(sAboutWin);

    /* Request update */
    PostEvent(updateEvt, (UInt32)sAboutWin);

    FINDER_LOG_DEBUG("AboutThisMac: Window shown and brought to front\n");
}

/*
 * AboutWindow_CloseIf - Close About window if it matches
 */
void AboutWindow_CloseIf(WindowPtr w)
{
    FINDER_LOG_DEBUG("AboutThisMac: CloseIf ENTRY, w=0x%08x, sAboutWin=0x%08x\n",
                     (unsigned int)P2UL(w), (unsigned int)P2UL(sAboutWin));

    if (!w || w != sAboutWin) {
        FINDER_LOG_DEBUG("AboutThisMac: CloseIf - window mismatch, returning\n");
        return;
    }

    FINDER_LOG_DEBUG("AboutThisMac: Closing window\n");

    FINDER_LOG_DEBUG("AboutThisMac: About to call DisposeWindow\n");
    DisposeWindow(sAboutWin);
    FINDER_LOG_DEBUG("AboutThisMac: DisposeWindow returned\n");
    sAboutWin = NULL;
    FINDER_LOG_DEBUG("AboutThisMac: CloseIf EXIT\n");
}

/*
 * AboutWindow_HandleUpdate - Redraw About window contents
 *
 * Called from event dispatcher on updateEvt.
 * Returns true if handled.
 */
Boolean AboutWindow_HandleUpdate(WindowPtr w)
{
    GrafPtr savedPort;
    MemSnapshot mem;
    Rect box, bar, seg;
    short wSys, wApp, wCac;  /* Memory segment widths for bar graph */
    UInt32 currentTicks;
    extern UInt32 TickCount(void);

    if (!w || w != sAboutWin) {
        return 0;  /* false */
    }

    FINDER_LOG_DEBUG("AboutThisMac: HandleUpdate called\n");

    /* Save current port and set to our window */
    GetPort(&savedPort);
    SetPort((GrafPtr)w);

    BeginUpdate(w);

    /* Optional throttle: only recompute stats every N ticks to reduce overhead */
    currentTicks = TickCount();
    if (currentTicks - sLastUpdateTicks < kAboutUpdateThrottle) {
        /* Still draw, but use cached stats - for now just note it */
        FINDER_LOG_DEBUG("AboutThisMac: Throttled update (delta=%u ticks)\n",
                         (unsigned int)(currentTicks - sLastUpdateTicks));
    }
    sLastUpdateTicks = currentTicks;

    /* Get window content rect - use local coordinates (0,0) origin */
    Rect contentRect;
    contentRect.top = 0;
    contentRect.left = 0;
    contentRect.right = w->port.portBits.bounds.right - w->port.portBits.bounds.left;
    contentRect.bottom = w->port.portBits.bounds.bottom - w->port.portBits.bounds.top;

    FINDER_LOG_DEBUG("AboutThisMac: contentRect=(%d,%d,%d,%d)\n",
                 contentRect.left, contentRect.top, contentRect.right, contentRect.bottom);

    /* Clear */
    EraseRect(&contentRect);

    /* Get platform info for display */
    const char *platform_name = platform_get_display_name();
    const char *model_string = platform_get_model_string();
    const char *memory_gb = platform_format_memory_gb();

    /* Title: Platform name (detected at boot) - Chicago (System), 12pt, centered */
    Str255 title;
    TextFont(0);            /* System (Chicago) */
    TextSize(12);           /* Use 12pt to avoid scaling */
    TextFace(0);            /* normal */
    FINDER_LOG_DEBUG("AboutThisMac: About to call ToPStr with platform: %s\n", platform_name);
    ToPStr(platform_name, title);
    FINDER_LOG_DEBUG("AboutThisMac: ToPStr returned, title[0]=%d, about to center\n", title[0]);
    FINDER_LOG_DEBUG("AboutThisMac: About to call CenterPStringInRect\n");
    CenterPStringInRect(title, &contentRect, contentRect.top + 20);
    FINDER_LOG_DEBUG("AboutThisMac: Title centered\n");

    /* Version and Memory: "System 7 - X GB" - Chicago 11, normal */
    Str255 ver;
    char ver_buf[64];
    sprintf(ver_buf, "System 7.1 - %s", memory_gb);
    TextSize(11);
    TextFace(0);            /* normal */
    ToPStr(ver_buf, ver);
    CenterPStringInRect(ver, &contentRect, contentRect.top + 40);

#if defined(__powerpc__) || defined(__powerpc64__)
    long memPtrLong = 0;
    if (Gestalt(gestaltMemoryMap, &memPtrLong) == noErr && memPtrLong != 0) {
        long *memInfo = (long *)(uintptr_t)memPtrLong;
        uint32_t range_count = (uint32_t)memInfo[0];
        if (range_count > 0) {
            uint32_t base_hi = (uint32_t)memInfo[1];
            uint32_t base_lo = (uint32_t)memInfo[2];
            uint32_t size_hi = (uint32_t)memInfo[3];
            uint32_t size_lo = (uint32_t)memInfo[4];
            uint64_t size_bytes = ((uint64_t)size_hi << 32) | size_lo;
            uint32_t size_mb = (uint32_t)(size_bytes / (1024 * 1024));

            char map_buf[80];
            if (base_hi != 0) {
                sprintf(map_buf, "FW range0: 0x%08x%08x (%u MB)", base_hi, base_lo, size_mb);
            } else {
                sprintf(map_buf, "FW range0: 0x%08x (%u MB)", base_lo, size_mb);
            }
            Str255 extra;
            TextSize(10);
            TextFace(0);
            ToPStr(map_buf, extra);
            CenterPStringInRect(extra, &contentRect, contentRect.top + 60);

            char count_buf[40];
            sprintf(count_buf, "Firmware ranges: %u", range_count);
            ToPStr(count_buf, extra);
            CenterPStringInRect(extra, &contentRect, contentRect.top + 72);
        }
    }
#endif

    /* Model info: model string - Chicago 10, normal */
    if (model_string && model_string[0] != '\0') {
        Str255 model_pstr;
        TextSize(10);
        ToPStr(model_string, model_pstr);
        CenterPStringInRect(model_pstr, &contentRect, contentRect.top + 53);
    }

    /* Get current memory statistics */
    GetMemorySnapshot(&mem);

    /* Body text: Geneva 9 for labels/values */
    TextFont(3);           /* Geneva */
    TextSize(9);
    TextFace(0);           /* normal */

    /* Memory box */
    box.top    = contentRect.top  + 60;
    box.left   = contentRect.left + 20;
    box.right  = contentRect.right - 20;
    box.bottom = box.top + 152;
    FrameRect(&box);

    /* Labels left; values right aligned */
    const short leftCol  = box.left  + 10;
    const short rightCol = box.right - 70;

    /* "Memory" label */
    Str255 memLbl;
    ToPStr("Memory", memLbl);
    MoveTo(leftCol, box.top + 14);
    DrawString(memLbl);
    short y = box.top + 32;             /* first line baseline */
    const short dy = 14;                 /* line spacing */

    /* Built-in Memory */
    Str255 l1, v1;
    ToPStr("Built-in Memory", l1);
    MoveTo(leftCol, y);
    DrawString(l1);
    FormatKB_PStr(mem.totalRAM, v1);
    RightAlignPStringAt(v1, rightCol, y);

    /* Application Memory */
    y += dy;
    Str255 l2, v2;
    ToPStr("Application Memory", l2);
    MoveTo(leftCol, y);
    DrawString(l2);
    FormatKB_PStr(mem.appUsed, v2);
    RightAlignPStringAt(v2, rightCol, y);

    /* Largest Unused Block */
    y += dy;
    Str255 l3, v3;
    ToPStr("Largest Unused Block", l3);
    MoveTo(leftCol, y);
    DrawString(l3);
    FormatKB_PStr(mem.largestFree, v3);
    RightAlignPStringAt(v3, rightCol, y);

    /* Bar rect under stats */
    bar.left  = box.left + 10;
    bar.right = box.right - 30;
    bar.top   = box.top + 80;           /* looks right under three lines */
    bar.bottom= bar.top + 12;
    FrameRect(&bar);

    /* Compute widths (clamped) */
    UInt32 totalKB = mem.totalRAM >> 10;
    UInt32 sysKB   = mem.systemUsed >> 10;
    UInt32 appKB   = mem.appUsed >> 10;
    UInt32 cacheKB = mem.diskCache >> 10;

    if (totalKB <= 0) totalKB = 1;
    UInt32 usedKB = sysKB + appKB + cacheKB;
    if (usedKB > totalKB) usedKB = totalKB;

    short wTotal = bar.right - bar.left;
    wSys   = (short)((sysKB   * wTotal) / totalKB);
    wApp   = (short)((appKB   * wTotal) / totalKB);
    wCac   = (short)((cacheKB * wTotal) / totalKB);

    /* Segment rects */
    seg = bar;

    /* System */
    seg.right = seg.left + wSys;
    PenPat(&qd.dkGray);
    PaintRect(&seg);

    /* Applications */
    seg.left  = seg.right;
    seg.right = seg.left + wApp;
    PenPat(&qd.gray);
    PaintRect(&seg);

    /* Disk Cache (optional, draw only if >0) */
    if (wCac > 0) {
        seg.left  = seg.right;
        seg.right = seg.left + wCac;
        PenPat(&qd.ltGray);
        PaintRect(&seg);
    }

    /* Unused stays white - no fill needed; reframe to restore crisp outline */
    FrameRect(&bar);

    /* Labels below bar */
    TextFont(3);
    TextSize(9);
    Str255 sysS, appS, unusedS;
    ToPStr("System", sysS);
    ToPStr("Applications", appS);
    ToPStr("Unused", unusedS);
    MoveTo(bar.left,  bar.bottom + 12);
    DrawString(sysS);
    MoveTo(bar.left + 60, bar.bottom + 12);
    DrawString(appS);
    RightAlignPStringAt(unusedS, bar.right, bar.bottom + 12);

    /* Footer line - Geneva 10, move higher up */
    Str255 cpy;
    TextFont(3);           /* Geneva */
    TextSize(10);          /* Slightly larger for visibility */
    TextFace(0);           /* normal */
    ToPStr("2025 Kelsi Davis", cpy);
    /* Place below memory box - move higher to ensure visibility */
    short cpyY = box.bottom + 18;
    FINDER_LOG_DEBUG("Drawing footer at Y=%d: '2025 Kelsi Davis'\n", cpyY);
    MoveTo(contentRect.left + ((contentRect.right - contentRect.left) - StringWidth((ConstStr255Param)cpy)) / 2, cpyY);
    DrawString((ConstStr255Param)cpy);
    FINDER_LOG_DEBUG("Footer drawn\n");

    EndUpdate(w);

    /* Restore previous port */
    SetPort(savedPort);

    FINDER_LOG_DEBUG("AboutThisMac: Update complete\n");

    return 1;  /* true - handled */
}

/*
 * AboutWindow_HandleMouseDown - Handle mouse clicks in About window
 *
 * Called from event dispatcher on mouseDown in About window.
 * Returns true if handled.
 */
Boolean AboutWindow_HandleMouseDown(WindowPtr w, short part, Point localPt)
{
    Rect dragBounds;

    if (!w || w != sAboutWin) {
        return 0;  /* false */
    }

    FINDER_LOG_DEBUG("AboutThisMac: HandleMouseDown, part=%d\n", part);

    switch (part) {
        case inDrag:
            FINDER_LOG_DEBUG("AboutThisMac: In inDrag case\n");
            /* Select window before dragging (Mac OS standard behavior) */
            SelectWindow(w);
            /* Allow window to be dragged */
            dragBounds = qd.screenBits.bounds;

            /* Expand horizontal bounds so the window can slide partially off-screen */
            if (w->strucRgn && *w->strucRgn) {
                Rect frame = (*w->strucRgn)->rgnBBox;
                short windowWidth = frame.right - frame.left;
                short windowHeight = frame.bottom - frame.top;

                /* Allow roughly half the window to leave each edge */
                short horizontalSlack = windowWidth / 2;
                short verticalSlack = windowHeight / 2;

                dragBounds.left -= horizontalSlack;
                dragBounds.right += horizontalSlack;
                dragBounds.bottom += verticalSlack;
            }

            /* Never allow the title bar to cross the menu bar */
            if (dragBounds.top < 20) {
                dragBounds.top = 20;
            }

            DragWindow(w, localPt, &dragBounds);
            return 1;  /* true */

        case inGoAway:
            FINDER_LOG_DEBUG("AboutThisMac: In inGoAway case\n");
            /* Handle close box - TrackGoAway hangs, so just close directly */
            FINDER_LOG_DEBUG("AboutThisMac: Close box clicked, closing window\n");
            AboutWindow_CloseIf(w);
            FINDER_LOG_DEBUG("AboutThisMac: After AboutWindow_CloseIf\n");
            return 1;  /* true */

        case inContent:
            /* Just select window if not already front */
            if (FrontWindow() != w) {
                SelectWindow(w);
            }
            return 1;  /* true */

        default:
            break;
    }

    return 0;  /* false - not handled */
}

/*
 * AboutWindow_IsOurs - Check if window is the About window
 *
 * Utility for event dispatcher to identify About window.
 */
Boolean AboutWindow_IsOurs(WindowPtr w)
{
    return (w && w == sAboutWin && w->refCon == kAboutRefCon) ? 1 : 0;
}
