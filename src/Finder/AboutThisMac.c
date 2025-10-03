/*
 * AboutThisMac.c - System 7-style "About This Macintosh" Window
 *
 * Implements a faithful modeless About window with memory statistics
 * and bar graph, matching classic Mac OS System 7.1 behavior.
 */

#include "SystemTypes.h"
#include <string.h>

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
extern void DrawString(const unsigned char* s);
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
extern OSErr PostEvent(short eventNum, UInt32 eventMsg);

/* External memory introspection - try to use real APIs if available */
extern Size FreeMem(void);
extern Size MaxMem(Size* grow);
extern UInt32 TotalRam(void);  /* Platform may provide this */
extern uint32_t g_total_memory_kb;  /* Actual detected RAM from multiboot2 */

/* Serial logging */
extern int serial_printf(const char* format, ...);

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
    short w = StringWidth((ConstStr255Param)s);
    MoveTo(r->left + ((r->right - r->left) - w) / 2, baseline);
    DrawString((ConstStr255Param)s);
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
 * TODO: Wire to real Memory Manager statistics when available.
 * Currently uses best-effort introspection and fallback values.
 */
static void GetMemorySnapshot(MemSnapshot* m)
{
    Size grow = 0;
    UInt32 freeBytes = 0;
    UInt32 totalBytes = 0;

    if (!m) return;

    memset(m, 0, sizeof(MemSnapshot));

    /* Get total RAM from multiboot2 detection (g_total_memory_kb is in KB) */
    totalBytes = (UInt32)g_total_memory_kb * 1024;  /* Convert KB to bytes */
    if (totalBytes == 0) {
        totalBytes = 0x01000000;  /* Fallback: 16 MB if detection failed */
    }

    /* Try FreeMem() and MaxMem() if available */
    freeBytes = (UInt32)FreeMem();
    m->largestFree = (UInt32)MaxMem(&grow);

    if (m->largestFree == 0 || m->largestFree > totalBytes) {
        /* Fallback if APIs not working */
        m->largestFree = totalBytes / 4;  /* Assume 25% free */
    }

    m->totalRAM = totalBytes;

    /* Estimate system usage (kernel, drivers, Finder heap) */
    /* TODO: Sum actual kernel data + driver buffers + Finder allocations */
    m->systemUsed = totalBytes / 8;  /* ~12.5% for system - FALLBACK */

    /* Estimate application usage */
    /* TODO: Sum all application heap allocations */
    m->appUsed = (totalBytes - m->largestFree - m->systemUsed) / 2;
    if (m->appUsed > totalBytes) m->appUsed = totalBytes / 4;

    /* Disk cache - TODO: Wire to actual cache manager when implemented */
    m->diskCache = 0;  /* Not implemented yet */

    /* Compute unused (remaining free memory) */
    if (m->systemUsed + m->appUsed + m->diskCache < m->totalRAM) {
        m->unused = m->totalRAM - m->systemUsed - m->appUsed - m->diskCache;
    } else {
        m->unused = 0;
    }

    serial_printf("AboutThisMac: Memory snapshot - Total:%lu System:%lu App:%lu Cache:%lu Free:%lu Unused:%lu\n",
                  (unsigned long)m->totalRAM, (unsigned long)m->systemUsed,
                  (unsigned long)m->appUsed, (unsigned long)m->diskCache,
                  (unsigned long)m->largestFree, (unsigned long)m->unused);
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
        serial_printf("FormatKB: %luK\n", (unsigned long)kb);
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
 */
static void DrawLabeledValue(short x, short y, const char* label, UInt32 bytes)
{
    char valueStr[32];
    unsigned char pstr[64];
    short labelLen, colonLen;

    /* Draw label */
    pstr[0] = (unsigned char)strlen(label);
    memcpy(&pstr[1], label, pstr[0]);
    MoveTo(x, y);
    DrawString(pstr);

    /* Calculate position for value (after label) */
    labelLen = pstr[0] * 6;  /* Approximate char width in Chicago 9pt */

    /* Format and draw value */
    FormatKBWithCommas(bytes, valueStr, sizeof(valueStr));
    pstr[0] = (unsigned char)strlen(valueStr);
    memcpy(&pstr[1], valueStr, pstr[0]);

    MoveTo(x + labelLen + 8, y);
    DrawString(pstr);
}

/*
 * GetSystemVersionString - Return version string
 *
 * TODO: Hook to actual build version when available
 */
static const char* GetSystemVersionString(void)
{
    return "System 7.1";
}

/*
 * AboutWindow_CreateIfNeeded - Create About window if not already open
 */
static void AboutWindow_CreateIfNeeded(void)
{
    Rect bounds;
    unsigned char title[32];

    if (sAboutWin) {
        serial_printf("AboutThisMac: Window already exists at %p\n", (void*)sAboutWin);
        return;
    }

    /* Position window centered-ish on screen (classic About box position) */
    bounds.top = 80;
    bounds.left = 120;
    bounds.bottom = bounds.top + 240;  /* Height: 240 pixels */
    bounds.right = bounds.left + 400;  /* Width: 400 pixels */

    /* Create window with movable dialog box proc, close box enabled */
    strcpy((char*)&title[1], "About This Macintosh");
    title[0] = (unsigned char)strlen((char*)&title[1]);

    sAboutWin = NewWindow(NULL, &bounds, title,
                         1,  /* visible */
                         movableDBoxProc,  /* Movable dialog box */
                         (WindowPtr)-1,  /* Behind all windows initially */
                         1,  /* Has close box */
                         kAboutRefCon);

    if (!sAboutWin) {
        serial_printf("AboutThisMac: FAILED to create window!\n");
        return;
    }

    serial_printf("AboutThisMac: Created window at %p, refCon=0x%08lX\n",
                  (void*)sAboutWin, (unsigned long)kAboutRefCon);
}

/*
 * AboutWindow_ShowOrToggle - Show About window or bring to front if open
 *
 * Public API called from Apple menu handler.
 */
void AboutWindow_ShowOrToggle(void)
{
    serial_printf("AboutThisMac: ShowOrToggle called\n");

    if (!sAboutWin) {
        AboutWindow_CreateIfNeeded();
    }

    if (!sAboutWin) {
        serial_printf("AboutThisMac: Failed to create window\n");
        return;
    }

    /* Bring to front and select */
    BringToFront(sAboutWin);
    SelectWindow(sAboutWin);

    /* Request update */
    PostEvent(updateEvt, (UInt32)sAboutWin);

    serial_printf("AboutThisMac: Window shown and brought to front\n");
}

/*
 * AboutWindow_CloseIf - Close About window if it matches
 */
void AboutWindow_CloseIf(WindowPtr w)
{
    if (!w || w != sAboutWin) {
        return;
    }

    serial_printf("AboutThisMac: Closing window\n");

    DisposeWindow(sAboutWin);
    sAboutWin = NULL;
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
    short barX, barY, barW, barH;
    short wSys, wApp, wCac, wUnu;
    UInt32 denom;
    unsigned char pstr[64];
    char valueStr[32];
    const char* versionStr;
    UInt32 currentTicks;
    extern UInt32 TickCount(void);

    if (!w || w != sAboutWin) {
        return 0;  /* false */
    }

    serial_printf("AboutThisMac: HandleUpdate called\n");

    /* Save current port and set to our window */
    GetPort(&savedPort);
    SetPort((GrafPtr)w);

    BeginUpdate(w);

    /* Optional throttle: only recompute stats every N ticks to reduce overhead */
    currentTicks = TickCount();
    if (currentTicks - sLastUpdateTicks < kAboutUpdateThrottle) {
        /* Still draw, but use cached stats - for now just note it */
        serial_printf("AboutThisMac: Throttled update (delta=%lu ticks)\n",
                     (unsigned long)(currentTicks - sLastUpdateTicks));
    }
    sLastUpdateTicks = currentTicks;

    /* Clear */
    EraseRect(&w->port.portRect);

    /* Title: "Macintosh x86" - Chicago (System), ~18 px, centered */
    Str255 title;
    TextFont(0);            /* System (Chicago) */
    TextSize(18);
    TextFace(1);            /* bold */
    ToPStr("Macintosh x86", title);
    CenterPStringInRect(title, &w->port.portRect, w->port.portRect.top + 26);

    /* Version: "System 7" - Chicago 12, normal */
    Str255 ver;
    TextSize(12);
    TextFace(0);            /* normal */
    ToPStr("System 7", ver);
    CenterPStringInRect(ver, &w->port.portRect, w->port.portRect.top + 44);

    /* Get current memory statistics */
    GetMemorySnapshot(&mem);

    /* Body text: Geneva 9 for labels/values */
    TextFont(3);           /* Geneva */
    TextSize(9);
    TextFace(0);           /* normal */

    /* Memory box */
    box.top    = w->port.portRect.top  + 60;
    box.left   = w->port.portRect.left + 20;
    box.right  = w->port.portRect.right - 20;
    box.bottom = box.top + 152;
    FrameRect(&box);

    /* "Memory" label */
    Str255 memLbl;
    ToPStr("Memory", memLbl);
    MoveTo(box.left + 8, box.top + 14);
    DrawString(memLbl);

    /* Labels left; values right aligned */
    const short leftCol  = box.left  + 10;
    const short rightCol = box.right - 10;
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
    bar.right = box.right - 10;
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

    /* Footer line (copyright) placement */
    Str255 cpy;
    TextSize(6);
    ToPStr("\xA9 2025 Kelsi Davis", cpy);
    CenterPStringInRect(cpy, &w->port.portRect, w->port.portRect.bottom - 10);

    EndUpdate(w);

    /* Restore previous port */
    SetPort(savedPort);

    serial_printf("AboutThisMac: Update complete\n");

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

    serial_printf("AboutThisMac: HandleMouseDown, part=%d\n", part);

    switch (part) {
        case inDrag:
            /* Select window before dragging (Mac OS standard behavior) */
            SelectWindow(w);
            /* Allow window to be dragged */
            dragBounds = qd.screenBits.bounds;
            InsetRect(&dragBounds, 4, 4);
            DragWindow(w, localPt, &dragBounds);
            return 1;  /* true */

        case inGoAway:
            /* Handle close box */
            if (TrackGoAway(w, localPt)) {
                AboutWindow_CloseIf(w);
            }
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
