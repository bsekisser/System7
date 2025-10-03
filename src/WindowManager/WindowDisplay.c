#include "SystemTypes.h"
#include "WindowManager/WindowManager.h"
#include "WindowManager/WindowManagerInternal.h"
#include "QuickDraw/QuickDraw.h"
#include "ControlManager/ControlTypes.h"
#include "SystemTheme.h"

/* Debug macros */
#ifdef DEBUG_WINDOW_MANAGER
    #define WM_DEBUG(...) serial_printf("WM: " __VA_ARGS__)
#else
    #define WM_DEBUG(...)
#endif

/* External functions */
extern void serial_printf(const char* fmt, ...);
extern void DrawString(const unsigned char* str);
extern void LineTo(short h, short v);
extern void MoveTo(short h, short v);
extern void FrameRect(const Rect* r);
extern void PaintRect(const Rect* r);

/* External globals */
extern QDGlobals qd;
extern void EraseRect(const Rect* r);
extern void InsetRect(Rect* r, short dh, short dv);
extern void SetRect(Rect* r, short left, short top, short right, short bottom);
extern Boolean PtInRect(Point pt, const Rect* r);
extern Boolean EqualRect(const Rect* rect1, const Rect* rect2);
extern void UnionRect(const Rect* src1, const Rect* src2, Rect* dstRect);
extern Boolean EmptyRect(const Rect* r);
extern void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn);
extern void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);
extern void OffsetRgn(RgnHandle rgn, short dh, short dv);
extern void GetPort(GrafPtr* port);
extern void SetPort(GrafPtr port);
extern void ClipRect(const Rect* r);
extern void SetClip(RgnHandle rgn);
extern void GetClip(RgnHandle rgn);
extern RgnHandle NewRgn(void);
extern void DisposeRgn(RgnHandle rgn);
extern void SetOrigin(SInt16 h, SInt16 v);

/*-----------------------------------------------------------------------*/
/* Window Display Functions                                             */
/*-----------------------------------------------------------------------*/

/* Check windows for update events (called by GetNextEvent) */
void CheckWindowsNeedingUpdate(void) {
    extern SInt16 PostEvent(SInt16 eventNum, SInt32 eventMsg);
    extern WindowPtr FrontWindow(void);
    extern Boolean EmptyRgn(RgnHandle rgn);
    extern void serial_printf(const char* fmt, ...);

    static int call_count = 0;
    call_count++;

    /* Walk all visible windows and post update events for windows with non-empty updateRgn */
    WindowPtr window = FrontWindow();

    if (call_count <= 10 || (call_count % 500) == 0) {
        serial_printf("CheckWindowsNeedingUpdate: #%d, frontWindow=0x%08x\n", call_count, (unsigned int)window);
    }

    int windowCount = 0;
    while (window) {
        windowCount++;
        Boolean hasUpdateRgn = (window->updateRgn != NULL);
        Boolean isEmpty = hasUpdateRgn ? EmptyRgn(window->updateRgn) : true;

        if (call_count <= 10 || (call_count % 500) == 0) {
            serial_printf("CheckWindowsNeedingUpdate:   Window %d: 0x%08x, visible=%d, updateRgn=0x%08x, empty=%d\n",
                         windowCount, (unsigned int)window, window->visible, (unsigned int)window->updateRgn, isEmpty);
            if (hasUpdateRgn) {
                Region* rgn = *(window->updateRgn);
                serial_printf("CheckWindowsNeedingUpdate:     updateRgn bbox=(%d,%d,%d,%d)\n",
                             rgn->rgnBBox.left, rgn->rgnBBox.top, rgn->rgnBBox.right, rgn->rgnBBox.bottom);
            }
        }

        if (window->visible && window->updateRgn && !EmptyRgn(window->updateRgn)) {
            serial_printf("CheckWindowsNeedingUpdate: Posting update event for window 0x%08x\n", (unsigned int)window);
            PostEvent(6 /* updateEvt */, (SInt32)window);
            /* Don't clear updateRgn here - BeginUpdate/EndUpdate will handle it */
        }
        window = window->nextWindow;
    }
}

/* Internal helper to draw window frame */
static void DrawWindowFrame(WindowPtr window);

/* Internal helper to draw window controls */
static void DrawWindowControls(WindowPtr window);

void PaintOne(WindowPtr window, RgnHandle clobberedRgn) {
    extern void serial_printf(const char* fmt, ...);
    serial_printf("PaintOne: ENTRY, window=%p, visible=%d\n", window, window ? window->visible : -1);

    if (!window || !window->visible) {
        serial_printf("PaintOne: Early return\n");
        return;
    }

    WM_DEBUG("PaintOne: Painting window");
    serial_printf("PaintOne: About to GetPort/SetPort\n");

    /* Save current port */
    extern void GetWMgrPort(GrafPtr* port);
    GrafPtr savePort, wmgrPort;
    GetPort(&savePort);
    GetWMgrPort(&wmgrPort);

    /* CRITICAL: Fill window with white in WMgrPort using GLOBAL strucRgn coordinates
     * This ensures we always fill the correct screen position regardless of port state */
    SetPort(wmgrPort);
    serial_printf("PaintOne: Switched to WMgr port for backfill\n");

    /* Reset clip in WMgr port */
    extern RgnHandle NewRgn(void);
    extern void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);
    extern void DisposeRgn(RgnHandle rgn);
    RgnHandle fullClipWMgr = NewRgn();
    SetRectRgn(fullClipWMgr, -32768, -32768, 32767, 32767);
    SetClip(fullClipWMgr);
    DisposeRgn(fullClipWMgr);

    /* Window backfill is handled by chrome (title bar) and content drawing */
    /* No need to fill the entire window structure here */
    serial_printf("PaintOne: Skipping window backfill (handled by chrome+content)\n");

    /* NOW draw chrome on top of backfill */
    serial_printf("PaintOne: Drawing window chrome\n");
    serial_printf("PaintOne: About to call DrawWindowFrame, window=%p\n", window);
    DrawWindowFrame(window);
    serial_printf("PaintOne: DrawWindowFrame returned\n");
    DrawWindowControls(window);
    serial_printf("PaintOne: DrawWindowControls returned\n");

    /* Test content drawing temporarily disabled until Font Manager is linked */
    serial_printf("[TEXT] Text drawing disabled - Font Manager not linked\n");

    /* Window Manager draws chrome only - content is application's job */
    /* Application must draw content via BeginUpdate/EndUpdate in update event handler */

    SetPort(savePort);
    serial_printf("PaintOne: EXIT\n");
}

void PaintBehind(WindowPtr startWindow, RgnHandle clobberedRgn) {
    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState) return;

    serial_printf("[PaintBehind] Starting, startWindow=%p\n", startWindow);

    /* Get front window - we'll repaint it last to ensure it's on top */
    WindowPtr frontWin = wmState->windowList;

    /* Find start position in window list */
    WindowPtr window = startWindow;
    if (!window) {
        window = wmState->windowList;
    }

    /* Paint windows from back to front */
    while (window) {
        if (window->visible) {
            serial_printf("[PaintBehind] Painting window %p\n", window);
            /* Paint chrome (frame and controls) */
            PaintOne(window, clobberedRgn);

            /* Invalidate content region to trigger update event for content redraw */
            if (window->contRgn) {
                serial_printf("[PaintBehind] Invalidating content for window %p\n", window);
                extern void InvalRgn(RgnHandle badRgn);
                GrafPtr savePort;
                GetPort(&savePort);
                SetPort((GrafPtr)window);
                InvalRgn(window->contRgn);
                SetPort(savePort);

                /* WORKAROUND: Directly redraw folder window content since update events may not flow */
                if (window->refCon == 'DISK' || window->refCon == 'TRSH') {
                    extern void FolderWindow_Draw(WindowPtr w);
                    serial_printf("[PaintBehind] Directly drawing folder content for window %p\n", window);
                    FolderWindow_Draw(window);
                }
            }
        }
        window = window->nextWindow;
    }

    /* Repaint front window frame last to ensure it's on top of all background content */
    if (frontWin && frontWin->visible) {
        serial_printf("[PaintBehind] Repainting front window %p frame to keep it on top\n", frontWin);
        PaintOne(frontWin, NULL);
    }

    serial_printf("[PaintBehind] Complete\n");
}

void CalcVis(WindowPtr window) {
    if (!window) return;

    WM_DEBUG("CalcVis: Calculating visible region");

    /* Start with the window's content region */
    if (window->contRgn && window->visRgn) {
        CopyRgn(window->contRgn, window->visRgn);

        /* Subtract regions of windows in front */
        WindowPtr frontWindow = FrontWindow();
        while (frontWindow && frontWindow != window) {
            if (frontWindow->visible && frontWindow->strucRgn) {
                /* Would subtract frontWindow->strucRgn from window->visRgn */
                /* For now, simplified implementation */
            }
            frontWindow = frontWindow->nextWindow;
        }
    }
}

void CalcVisBehind(WindowPtr startWindow, RgnHandle clobberedRgn) {
    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState) return;

    WM_DEBUG("CalcVisBehind: Recalculating visible regions");

    /* Find start position in window list */
    WindowPtr window = startWindow;
    if (!window) {
        window = wmState->windowList;
    }

    /* Recalculate visible regions for all windows */
    while (window) {
        CalcVis(window);
        window = window->nextWindow;
    }
}

void ClipAbove(WindowPtr window) {
    if (!window) return;

    WM_DEBUG("ClipAbove: Setting clip region");

    /* Create a region that excludes all windows above this one */
    RgnHandle clipRgn = NewRgn();
    if (clipRgn) {
        /* Start with full screen */
        SetRectRgn(clipRgn, 0, 0, 1024, 768);

        /* Subtract regions of windows in front */
        WindowPtr frontWindow = FrontWindow();
        while (frontWindow && frontWindow != window) {
            if (frontWindow->visible && frontWindow->strucRgn) {
                /* Would subtract frontWindow->strucRgn from clipRgn */
                /* For now, simplified implementation */
            }
            frontWindow = frontWindow->nextWindow;
        }

        SetClip(clipRgn);
        DisposeRgn(clipRgn);
    }
}

void SaveOld(WindowPtr window) {
    if (!window) return;

    WM_DEBUG("SaveOld: Saving window bits");

    /* Save the bits behind the window */
    /* This would typically copy screen bits to an offscreen buffer */
    /* For now, simplified implementation */
}

void DrawNew(WindowPtr window, Boolean update) {
    if (!window) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("DrawNew: ENTRY, window=%p\n", window);

    WM_DEBUG("DrawNew: Drawing window");

    /* Save current port */
    extern void GetWMgrPort(GrafPtr* port);
    GrafPtr savePort, wmgrPort;
    GetPort(&savePort);
    GetWMgrPort(&wmgrPort);

    /* Draw chrome in WMgr port */
    SetPort(wmgrPort);
    serial_printf("DrawNew: Drawing frame\n");
    DrawWindowFrame(window);
    DrawWindowControls(window);

    /* Switch to window port for content */
    SetPort((GrafPtr)window);

    if (update && window->updateRgn) {
        /* Only draw the update region */
        SetClip(window->updateRgn);
    }

    /* Content backfill is handled by application (Finder) draw code, not here */
    serial_printf("DrawNew: Content backfill handled by application draw code\n");

    SetPort(savePort);
    serial_printf("DrawNew: EXIT\n");
}

static void DrawWindowFrame(WindowPtr window) {
    serial_printf("DrawWindowFrame: ENTRY, window=%p\n", window);

    if (!window) {
        serial_printf("DrawWindowFrame: window is NULL, returning\n");
        return;
    }

    serial_printf("DrawWindowFrame: window->visible=%d\n", window->visible);
    if (!window->visible) {
        serial_printf("DrawWindowFrame: window not visible, returning\n");
        return;
    }

    /* CRITICAL: strucRgn must be set to draw the window
     * strucRgn contains GLOBAL screen coordinates
     * portRect contains LOCAL coordinates (0,0,width,height) and must NEVER be used for positioning! */
    serial_printf("DrawWindowFrame: Checking strucRgn=%p\n", window->strucRgn);
    if (!window->strucRgn) {
        serial_printf("WindowManager: DrawWindowFrame - strucRgn is NULL, cannot draw\n");
        return;
    }

    serial_printf("DrawWindowFrame: Checking *strucRgn=%p\n", *window->strucRgn);
    if (!*window->strucRgn) {
        serial_printf("WindowManager: DrawWindowFrame - *strucRgn is NULL, cannot draw\n");
        return;
    }

    extern void GetWMgrPort(GrafPtr* port);
    GrafPtr savePort, wmgrPort;
    GetPort(&savePort);
    GetWMgrPort(&wmgrPort);
    SetPort(wmgrPort);

    serial_printf("WindowManager: DrawWindowFrame START\n");

    /* Set up pen for drawing black frames */
    extern void PenNormal(void);
    extern void PenSize(short width, short height);
    static const Pattern blackPat = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    PenNormal();  /* Reset pen to normal state */
    PenPat(&blackPat);  /* Use black pattern for frames */
    PenSize(1, 1);  /* 1-pixel pen */

    /* Get window's global bounds from structure region */
    Rect frame = (*window->strucRgn)->rgnBBox;

    serial_printf("WindowManager: Frame rect (%d,%d,%d,%d)\n",
                  frame.left, frame.top, frame.right, frame.bottom);

    /* Draw frame outline using QuickDraw (not direct framebuffer) */
    FrameRect(&frame);
    serial_printf("WindowManager: Drew frame using FrameRect\n");

    /* Add 3D black highlights for depth effect */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;
    if (framebuffer) {
        uint32_t* fb = (uint32_t*)framebuffer;
        int pitch = fb_pitch / 4;
        uint32_t black = 0xFF000000;

        /* Right side highlight: 2px wide, starting 1px down from top and extending to bottom */
        int highlightStartY = frame.top + 1;
        for (int y = highlightStartY; y < frame.bottom - 1 && y < (int)fb_height; y++) {
            if (y >= 0) {
                /* Draw 2 pixels on the right side (inside the frame) */
                for (int dx = 1; dx <= 2; dx++) {
                    int x = frame.right - 1 - dx;
                    if (x >= 0 && x < (int)fb_width) {
                        fb[y * pitch + x] = black;
                    }
                }
            }
        }

        /* Bottom highlight: 2px thick */
        for (int dy = 1; dy <= 2; dy++) {
            int y = frame.bottom - 1 - dy;
            if (y >= 0 && y < (int)fb_height) {
                for (int x = frame.left + 1; x < frame.right - 3 && x < (int)fb_width; x++) {
                    if (x >= 0) {
                        fb[y * pitch + x] = black;
                    }
                }
            }
        }
    }

    /* Draw title bar BEFORE filling content area */
    serial_printf("WindowManager: About to check titleWidth=%d\n", window->titleWidth);

    if (window->titleWidth > 0) {
        serial_printf("WindowManager: titleWidth > 0, drawing title bar\n");
        /* Title bar background should be INSIDE the frame, not overlap it */
        Rect titleBar;
        titleBar.left = frame.left + 1;    /* Inset from left frame edge */
        titleBar.top = frame.top + 1;      /* Inset from top frame edge */
        titleBar.right = frame.right - 2;  /* Inset 2px from right to not overlap frame */
        titleBar.bottom = frame.top + 20;  /* Extends to separator line */

        /* Fill title bar background */
        extern void* framebuffer;
        extern uint32_t fb_width, fb_height, fb_pitch;

        if (window->hilited) {
            /* Active window: solid light grey background with darker horizontal stripes */
            if (framebuffer) {
                uint32_t* fb = (uint32_t*)framebuffer;
                int pitch = fb_pitch / 4;
                uint32_t lightGrey = 0xFFE8E8E8;  /* Solid lighter grey RGB(232,232,232) */
                uint32_t darkGrey = 0xFF808080;   /* Solid darker grey RGB(128,128,128) for stripes */

                /* Fill entire title bar with light grey */
                for (int y = titleBar.top; y < titleBar.bottom && y < (int)fb_height; y++) {
                    if (y >= 0) {
                        for (int x = titleBar.left; x < titleBar.right && x < (int)fb_width; x++) {
                            if (x >= 0) {
                                fb[y * pitch + x] = lightGrey;
                            }
                        }
                    }
                }

                /* Draw 6 evenly spaced darker horizontal stripes (every 3 pixels starting at offset 3) */
                int stripePositions[6] = {3, 6, 9, 12, 15, 18};
                for (int i = 0; i < 6; i++) {
                    int y = titleBar.top + stripePositions[i];
                    if (y >= 0 && y < (int)fb_height) {
                        for (int x = titleBar.left; x < titleBar.right && x < (int)fb_width; x++) {
                            if (x >= 0) {
                                fb[y * pitch + x] = darkGrey;
                            }
                        }
                    }
                }

                /* Draw 1px themed highlight border inside title bar */
                SystemTheme* theme = GetSystemTheme();
                RGBColor highlight = theme->highlightColor;
                /* Convert 16-bit Mac OS color to 8-bit framebuffer RGB */
                uint32_t highlightColor = 0xFF000000 | ((highlight.red >> 8) << 16) | ((highlight.green >> 8) << 8) | (highlight.blue >> 8);

                /* Top border - 1px inside */
                int y = titleBar.top;
                if (y >= 0 && y < (int)fb_height) {
                    for (int x = titleBar.left; x < titleBar.right && x < (int)fb_width; x++) {
                        if (x >= 0) fb[y * pitch + x] = highlightColor;
                    }
                }

                /* Bottom border - 1px inside (at separator line) */
                y = titleBar.bottom - 1;
                if (y >= 0 && y < (int)fb_height) {
                    for (int x = titleBar.left; x < titleBar.right && x < (int)fb_width; x++) {
                        if (x >= 0) fb[y * pitch + x] = highlightColor;
                    }
                }

                /* Left border - 1px inside */
                int x = titleBar.left;
                if (x >= 0 && x < (int)fb_width) {
                    for (y = titleBar.top; y < titleBar.bottom && y < (int)fb_height; y++) {
                        if (y >= 0) fb[y * pitch + x] = highlightColor;
                    }
                }

                /* Right border - 1px inside */
                x = titleBar.right - 1;
                if (x >= 0 && x < (int)fb_width) {
                    for (y = titleBar.top; y < titleBar.bottom && y < (int)fb_height; y++) {
                        if (y >= 0) fb[y * pitch + x] = highlightColor;
                    }
                }
            }
        } else {
            /* Inactive window: white background */
            EraseRect(&titleBar);
        }

        /* Draw System 7 close box - 14x14 at left side
         * Design: Black outline (left/top only for 3D), 1px theme highlight inside, grey fill */
        if (framebuffer) {
            int boxLeft = frame.left + 10;
            int boxTop = frame.top + 4;
            int boxSize = 14;

            uint32_t black = 0xFF000000;
            uint32_t grey = 0xFF808080;  /* Same grey as title bar stripes */
            uint32_t* fb = (uint32_t*)framebuffer;
            int pitch = fb_pitch / 4;

            /* Get theme highlight color if window is active */
            uint32_t highlightColor = grey;
            if (window->hilited) {
                SystemTheme* theme = GetSystemTheme();
                RGBColor highlight = theme->highlightColor;
                highlightColor = 0xFF000000 | ((highlight.red >> 8) << 16) | ((highlight.green >> 8) << 8) | (highlight.blue >> 8);
            }

            /* Draw black outline on LEFT and TOP only (3D effect) */
            /* Top edge */
            for (int x = boxLeft; x < boxLeft + boxSize - 1 && x < (int)fb_width; x++) {
                if (x >= 0 && boxTop >= 0 && boxTop < (int)fb_height) {
                    fb[boxTop * pitch + x] = black;
                }
            }
            /* Left edge */
            for (int y = boxTop; y < boxTop + boxSize - 1 && y < (int)fb_height; y++) {
                if (y >= 0 && boxLeft >= 0 && boxLeft < (int)fb_width) {
                    fb[y * pitch + boxLeft] = black;
                }
            }

            /* Draw 1px themed highlight border (complete box around grey) */
            /* Top highlight line */
            int y = boxTop + 1;
            if (y >= 0 && y < (int)fb_height) {
                for (int x = boxLeft + 1; x < boxLeft + boxSize - 1 && x < (int)fb_width; x++) {
                    if (x >= 0) fb[y * pitch + x] = highlightColor;
                }
            }
            /* Left highlight line */
            int x = boxLeft + 1;
            if (x >= 0 && x < (int)fb_width) {
                for (y = boxTop + 2; y < boxTop + boxSize - 2 && y < (int)fb_height; y++) {
                    if (y >= 0) fb[y * pitch + x] = highlightColor;
                }
            }
            /* Right highlight line */
            x = boxLeft + boxSize - 2;
            if (x >= 0 && x < (int)fb_width) {
                for (y = boxTop + 1; y < boxTop + boxSize - 1 && y < (int)fb_height; y++) {
                    if (y >= 0) fb[y * pitch + x] = highlightColor;
                }
            }
            /* Bottom highlight line */
            y = boxTop + boxSize - 2;
            if (y >= 0 && y < (int)fb_height) {
                for (x = boxLeft + 1; x < boxLeft + boxSize - 1 && x < (int)fb_width; x++) {
                    if (x >= 0) fb[y * pitch + x] = highlightColor;
                }
            }

            /* Fill interior with solid grey (reduced by 1px on bottom and right for shadow) */
            for (y = boxTop + 2; y < boxTop + boxSize - 3 && y < (int)fb_height; y++) {
                if (y >= 0) {
                    for (x = boxLeft + 2; x < boxLeft + boxSize - 3 && x < (int)fb_width; x++) {
                        if (x >= 0) fb[y * pitch + x] = grey;
                    }
                }
            }

            /* Draw black 3D shadow on bottom and right edges of grey */
            /* Bottom shadow */
            y = boxTop + boxSize - 3;
            if (y >= 0 && y < (int)fb_height) {
                for (x = boxLeft + 2; x < boxLeft + boxSize - 2 && x < (int)fb_width; x++) {
                    if (x >= 0) fb[y * pitch + x] = black;
                }
            }
            /* Right shadow */
            x = boxLeft + boxSize - 3;
            if (x >= 0 && x < (int)fb_width) {
                for (y = boxTop + 2; y < boxTop + boxSize - 3 && y < (int)fb_height; y++) {
                    if (y >= 0) fb[y * pitch + x] = black;
                }
            }

            /* Draw light grey separator columns on either side of close box
             * This separates the teal highlight from the dark grey horizontal stripes */
            uint32_t lightGrey = 0xFFE0E0E0;  /* Match title bar background */

            /* Left separator column - same height as black outline */
            int sepX = boxLeft - 1;
            if (sepX >= 0 && sepX < (int)fb_width) {
                for (y = boxTop; y < boxTop + boxSize - 1 && y < (int)fb_height; y++) {
                    if (y >= 0) fb[y * pitch + sepX] = lightGrey;
                }
            }

            /* Right separator column - right against the close box */
            sepX = boxLeft + boxSize - 1;
            if (sepX >= 0 && sepX < (int)fb_width) {
                for (y = boxTop; y < boxTop + boxSize - 1 && y < (int)fb_height; y++) {
                    if (y >= 0) fb[y * pitch + sepX] = lightGrey;
                }
            }
        }

        /* Draw title bar separator */
        MoveTo(frame.left, frame.top + 20);
        LineTo(frame.right - 1, frame.top + 20);

        /* Draw window title with System 7 lozenge */
        serial_printf("TITLE_DRAW: titleHandle=%p, *titleHandle=%p\n",
                     window->titleHandle, window->titleHandle ? *window->titleHandle : NULL);

        if (window->titleHandle && *window->titleHandle) {
            unsigned char* titleStr = (unsigned char*)*window->titleHandle;
            unsigned char titleLen = titleStr[0];

            serial_printf("TITLE_DRAW: titleLen=%d\n", titleLen);

            /* Basic validation: just check length is positive and not obviously corrupt */
            if (titleLen > 0 && titleLen < 128) {
                extern short StringWidth(ConstStr255Param str);
                extern void TextFace(short face);
                extern void PaintRoundRect(const Rect* r, short ovalWidth, short ovalHeight);
                extern void FrameRoundRect(const Rect* r, short ovalWidth, short ovalHeight);
                extern void InsetRect(Rect* r, short dh, short dv);

                short textWidth = StringWidth(titleStr);

                /* System 7 lozenge calculations (exact pixel metrics) */
                short barTop = frame.top;
                short barBottom = barTop + 20;
                short barMidX = (frame.left + frame.right) / 2;
                short textLeft = barMidX - textWidth / 2;
                short textBaseline = barTop + 14;

                /* Lozenge rect (before clipping) */
                Rect loz;
                loz.top = barTop + 3;
                loz.bottom = barBottom - 3;
                loz.left = textLeft - 10;
                loz.right = textLeft + textWidth + 10;

                /* Clip lozenge to avoid controls (4px clearance) */
                short ctrlPad = 4;
                short closeRight = frame.left + 4 + 14;  /* close box: 4 inset + 14 size */
                short zoomLeft = frame.right - 4 - 14;   /* zoom box (if present) */

                if (loz.left < closeRight + ctrlPad) loz.left = closeRight + ctrlPad;
                if (loz.right > zoomLeft - ctrlPad) loz.right = zoomLeft - ctrlPad;

                if (window->hilited) {
                    /* Active window: draw rectangular area behind text to cover stripes */

                    /* Fill rectangular lozenge with grey at framebuffer level */
                    if (framebuffer) {
                        uint32_t* fb = (uint32_t*)framebuffer;
                        int pitch = fb_pitch / 4;
                        uint32_t lightGrey = 0xFFE8E8E8;  /* Same as title bar background */

                        /* Simple rectangular fill to cleanly cover stripes */
                        for (int y = loz.top; y < loz.bottom; y++) {
                            if (y >= 0 && y < (int)fb_height) {
                                for (int x = loz.left; x < loz.right; x++) {
                                    if (x >= 0 && x < (int)fb_width) {
                                        fb[y * pitch + x] = lightGrey;
                                    }
                                }
                            }
                        }
                    }

                    /* Draw title text in bold black */
                    PenPat(&qd.black);
                    TextFace(1);  /* bold */
                    MoveTo(textLeft, textBaseline);
                    DrawString(titleStr);
                    TextFace(0);  /* reset to normal */
                } else {
                    /* Inactive window: no lozenge, gray text */
                    PenPat(&qd.gray);
                    TextFace(0);  /* normal */
                    MoveTo(textLeft, textBaseline);
                    DrawString(titleStr);
                    PenPat(&qd.black);  /* reset to black */
                }

                serial_printf("TITLE_DRAW: Drew title at baseline %d\n", textBaseline);
            } else {
                serial_printf("TITLE_DRAW: titleLen %d out of range\n", titleLen);
            }
        } else {
            serial_printf("TITLE_DRAW: No titleHandle or empty\n");
        }
    } else {
        serial_printf("WindowManager: titleWidth is 0, skipping title bar\n");
    }

    SetPort(savePort);
}

static void DrawWindowControls(WindowPtr window) {
    if (!window || !window->visible) return;

    /* Set up WMgr port for global coordinate drawing */
    extern void GetWMgrPort(GrafPtr* port);
    GrafPtr savePort, wmgrPort;
    GetPort(&savePort);
    GetWMgrPort(&wmgrPort);
    SetPort(wmgrPort);

    /* Set up pen for drawing black controls */
    extern void PenNormal(void);
    extern void PenSize(short width, short height);
    static const Pattern blackPat = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    PenNormal();
    PenPat(&blackPat);
    PenSize(1, 1);

    /* CRITICAL: Use global coordinates from strucRgn, not local portRect */
    Rect frame;
    if (window->strucRgn && *window->strucRgn) {
        frame = (*window->strucRgn)->rgnBBox;
    } else {
        /* Fallback to portRect if strucRgn not set */
        frame = window->port.portRect;
    }

    /* Close box is drawn in DrawWindowFrame, not here */

    /* Draw zoom box */
    if (window->spareFlag) {
        Rect zoomBox;
        SetRect(&zoomBox, frame.right - 20, frame.top + 4,
                frame.right - 8, frame.top + 16);
        FrameRect(&zoomBox);

        if (window->hilited) {
            /* Draw zoom box lines */
            Rect innerBox = zoomBox;
            InsetRect(&innerBox, 2, 2);
            FrameRect(&innerBox);
        }
    }

    /* Draw grow box */
    if (window->windowKind >= 0) {  /* Document window */
        /* Grow box in bottom-right corner */
        Rect growBox;
        SetRect(&growBox, frame.right - 16, frame.bottom - 16,
                frame.right, frame.bottom);

        /* Draw grow lines */
        MoveTo(growBox.left, growBox.bottom - 1);
        LineTo(growBox.right - 1, growBox.top);
        MoveTo(growBox.left + 4, growBox.bottom - 1);
        LineTo(growBox.right - 1, growBox.top + 4);
        MoveTo(growBox.left + 8, growBox.bottom - 1);
        LineTo(growBox.right - 1, growBox.top + 8);
    }

    /* Draw scroll bars if present */
    ControlHandle control = window->controlList;
    while (control) {
        if ((*control)->contrlVis) {
            /* Would call Draw1Control(control) */
            /* For now, simplified implementation */
        }
        control = (*control)->nextControl;
    }

    /* Restore previous port */
    SetPort(savePort);
}


/* ============================================================================
 * Main Window Drawing Function - Draws Chrome Only
 * ============================================================================ */

void DrawWindow(WindowPtr window) {
    if (!window || !window->visible) {
        serial_printf("WindowManager: DrawWindow - window NULL or not visible\n");
        return;
    }

    serial_printf("WindowManager: DrawWindow ENTRY for window '%s'\n",
                  window->titleHandle ? (char*)*window->titleHandle : "Untitled");

    /* Save current port */
    extern void GetWMgrPort(GrafPtr* port);
    GrafPtr savePort, wmgrPort;
    GetPort(&savePort);
    GetWMgrPort(&wmgrPort);

    /* Draw chrome in WMgr port */
    SetPort(wmgrPort);
    DrawWindowFrame(window);
    DrawWindowControls(window);

    /* Switch to window port for content */
    SetPort((GrafPtr)window);

    /* Fill content area - LOCAL coords */
    Rect contentRect = window->port.portRect;
    serial_printf("DrawWindow: Filling content rect (local) (%d,%d,%d,%d)\n",
        contentRect.left, contentRect.top, contentRect.right, contentRect.bottom);
    EraseRect(&contentRect);
    serial_printf("DrawWindow: Content filled\n");

    SetPort(savePort);
    serial_printf("DrawWindow: EXIT\n");
}

void DrawGrowIcon(WindowPtr window) {
    if (!window || !window->visible || window->windowKind < 0) return;

    WM_DEBUG("DrawGrowIcon: Drawing grow icon");

    GrafPtr savePort;
    GetPort(&savePort);
    SetPort((GrafPtr)window);

    /* Draw grow icon in bottom-right corner */
    Rect frame = window->port.portRect;
    Rect growBox;
    SetRect(&growBox, frame.right - 16, frame.bottom - 16,
            frame.right, frame.bottom);

    /* Clear the grow box area first */
    EraseRect(&growBox);

    /* Draw the grow lines */
    MoveTo(growBox.left, growBox.bottom - 1);
    LineTo(growBox.right - 1, growBox.top);
    MoveTo(growBox.left + 4, growBox.bottom - 1);
    LineTo(growBox.right - 1, growBox.top + 4);
    MoveTo(growBox.left + 8, growBox.bottom - 1);
    LineTo(growBox.right - 1, growBox.top + 8);

    SetPort(savePort);
}

/*-----------------------------------------------------------------------*/
/* Window Visibility Functions                                          */
/*-----------------------------------------------------------------------*/

void ShowWindow(WindowPtr window) {
    extern void serial_printf(const char* fmt, ...);
    serial_printf("ShowWindow: ENTRY, window=%p\n", window);

    if (!window || window->visible) {
        serial_printf("ShowWindow: Early return (window=%p, visible=%d)\n", window, window ? window->visible : -1);
        return;
    }

    WM_DEBUG("ShowWindow: Making window visible");
    serial_printf("ShowWindow: About to set visible=true\n");

    window->visible = true;

    /* Calculate window regions (structure and content) */
    serial_printf("ShowWindow: Calculating window regions\n");
    extern void WM_CalculateStandardWindowRegions(WindowPtr window, short varCode);
    WM_CalculateStandardWindowRegions(window, 0);

    /* Calculate visible region */
    serial_printf("ShowWindow: About to call CalcVis\n");
    CalcVis(window);

    /* Paint the window */
    serial_printf("ShowWindow: About to call PaintOne\n");
    PaintOne(window, NULL);
    serial_printf("ShowWindow: PaintOne returned\n");

    /* Invalidate content region to generate update event for application to draw content */
    if (window->contRgn) {
        serial_printf("ShowWindow: Invalidating content region to trigger update event\n");
        extern void InvalRgn(RgnHandle badRgn);
        extern void SetPort(GrafPtr port);

        /* InvalRgn operates on current port, so set port to window first */
        GrafPtr savePort;
        GetPort(&savePort);
        SetPort((GrafPtr)window);
        InvalRgn(window->contRgn);
        SetPort(savePort);
    }

    /* Recalculate regions for windows behind */
    CalcVisBehind(window->nextWindow, window->strucRgn);

    /* Don't call PaintBehind here - background windows are already painted.
     * Calling PaintBehind would cause background windows to paint over the front window. */

    serial_printf("ShowWindow: EXIT\n");
}

void HideWindow(WindowPtr window) {
    if (!window || !window->visible) return;

    WM_DEBUG("HideWindow: Hiding window");

    window->visible = false;

    /* Save the region that needs repainting */
    RgnHandle clobberedRgn = NULL;
    if (window->strucRgn) {
        clobberedRgn = NewRgn();
        if (clobberedRgn) {
            CopyRgn(window->strucRgn, clobberedRgn);
        }
    }

    /* Recalculate visible regions */
    CalcVisBehind(window->nextWindow, clobberedRgn);

    /* Repaint windows behind */
    PaintBehind(window->nextWindow, clobberedRgn);

    if (clobberedRgn) {
        DisposeRgn(clobberedRgn);
    }
}

void ShowHide(WindowPtr window, Boolean showFlag) {
    if (showFlag) {
        ShowWindow(window);
    } else {
        HideWindow(window);
    }
}

/*-----------------------------------------------------------------------*/
/* Window Highlighting Functions                                        */
/*-----------------------------------------------------------------------*/

void HiliteWindow(WindowPtr window, Boolean fHilite) {
    if (!window) return;

    if (window->hilited == fHilite) {
        serial_printf("[HILITE] Window %p already has hilite=%d, skipping\n", window, fHilite);
        return;
    }

    serial_printf("[HILITE] Window %p: changing hilite %d -> %d\n", window, window->hilited, fHilite);

    window->hilited = fHilite;

    /* Redraw the window frame to show highlight state */
    /* NOTE: DrawWindowFrame and DrawWindowControls set their own ports to WMgrPort */
    DrawWindowFrame(window);
    DrawWindowControls(window);

    serial_printf("[HILITE] Window %p: frame redrawn with hilite=%d\n", window, fHilite);
}

/*-----------------------------------------------------------------------*/
/* Window Ordering Functions                                            */
/*-----------------------------------------------------------------------*/

void BringToFront(WindowPtr window) {
    if (!window) return;

    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState) return;

    WM_DEBUG("BringToFront: Moving window to front");

    /* If already at front, just ensure it's hilited */
    if (wmState->windowList == window) {
        serial_printf("[HILITE] Window already at front, ensuring hilited\n");
        HiliteWindow(window, true);
        return;
    }

    /* Remove window from current position */
    WindowPtr prev = NULL;
    WindowPtr current = wmState->windowList;

    while (current && current != window) {
        prev = current;
        current = current->nextWindow;
    }

    if (!current) return;  /* Window not in list */

    /* Remove from list */
    if (prev) {
        prev->nextWindow = window->nextWindow;
    }

    /* Add to front */
    window->nextWindow = wmState->windowList;
    wmState->windowList = window;

    /* Update highlight state and repaint BEFORE bringing to front */
    WindowPtr prevFront = window->nextWindow;  /* Will be the previous front window */

    /* Unhighlight the window that will be demoted (if any) */
    if (prevFront) {
        serial_printf("[HILITE] Unhiliting previous front window %p\n", prevFront);
        HiliteWindow(prevFront, false);
    }

    /* Now hilite and paint the new front window */
    HiliteWindow(window, true);

    /* Recalculate visible regions */
    CalcVisBehind(window, NULL);

    /* Paint the new front window LAST so it's on top */
    PaintOne(window, NULL);
}

void SendBehind(WindowPtr window, WindowPtr behindWindow) {
    if (!window) return;

    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState) return;

    WM_DEBUG("SendBehind: Moving window behind another");

    /* Remove window from current position */
    WindowPtr prev = NULL;
    WindowPtr current = wmState->windowList;

    while (current && current != window) {
        prev = current;
        current = current->nextWindow;
    }

    if (!current) return;  /* Window not in list */

    /* Remove from list */
    if (prev) {
        prev->nextWindow = window->nextWindow;
    } else {
        wmState->windowList = window->nextWindow;
    }

    /* Insert after behindWindow */
    if (behindWindow) {
        window->nextWindow = behindWindow->nextWindow;
        behindWindow->nextWindow = window;
    } else {
        /* Send to back */
        current = wmState->windowList;
        while (current && current->nextWindow) {
            current = current->nextWindow;
        }
        if (current) {
            current->nextWindow = window;
        } else {
            wmState->windowList = window;
        }
        window->nextWindow = NULL;
    }

    /* Update highlight states */
    WindowPtr front = FrontWindow();
    current = wmState->windowList;
    while (current) {
        HiliteWindow(current, current == front);
        current = current->nextWindow;
    }

    /* Recalculate visible regions */
    CalcVisBehind(window, NULL);

    /* Repaint affected windows */
    PaintBehind(window, window->strucRgn);
}

/*-----------------------------------------------------------------------*/
/* Window Selection Functions                                           */
/*-----------------------------------------------------------------------*/

void SelectWindow(WindowPtr window) {
    if (!window) return;

    WM_DEBUG("SelectWindow: Selecting window");

    /* Bring window to front */
    BringToFront(window);

    /* Generate activate event */
    /* This would post an activateEvt to the event queue */
}

/*-----------------------------------------------------------------------*/
/* Window Query Functions                                               */
/*-----------------------------------------------------------------------*/

WindowPtr FrontWindow(void) {
    WindowManagerState* wmState = GetWindowManagerState();
    if (!wmState) {
        serial_printf("WindowManager: FrontWindow - wmState is NULL\n");
        return NULL;
    }

    static int call_count = 0;
    call_count++;

    /* Find first visible window */
    WindowPtr window = wmState->windowList;

    if (call_count <= 5 || (call_count % 5000) == 0) {
        serial_printf("WindowManager: FrontWindow #%d - list head=%p\n", call_count, window);
    }

    int count = 0;
    while (window) {
        if (call_count <= 5 || (call_count % 5000) == 0) {
            serial_printf("WindowManager: FrontWindow - checking window %p, visible=%d\n", window, window->visible);
        }
        if (window->visible) {
            if (call_count <= 5 || (call_count % 5000) == 0) {
                serial_printf("WindowManager: FrontWindow - returning visible window %p\n", window);
            }
            return window;
        }
        window = window->nextWindow;
        count++;
        if (count > 100) {
            serial_printf("WindowManager: FrontWindow - LOOP DETECTED, breaking\n");
            break;
        }
    }

    if (call_count <= 5 || (call_count % 5000) == 0) {
        serial_printf("WindowManager: FrontWindow - returning NULL (no visible window found)\n");
    }
    return NULL;
}

WindowPtr WM_FindWindowAt(Point pt) {
    WindowManagerState* wmState = GetWindowManagerState();

    WM_DEBUG("WM_FindWindowAt: Finding window at point (%d, %d)", pt.h, pt.v);

    /* Search windows from front to back */
    WindowPtr current = wmState->windowList;
    while (current) {
        if (current->visible && current->strucRgn) {
            if (Platform_PtInRgn(pt, current->strucRgn)) {
                WM_DEBUG("WM_FindWindowAt: Found window");
                return current;
            }
        }
        current = current->nextWindow;
    }

    WM_DEBUG("WM_FindWindowAt: No window found at point");
    return NULL;
}

WindowPtr WM_GetNextVisibleWindow(WindowPtr window) {
    if (window == NULL) return NULL;

    WindowPtr current = window->nextWindow;
    while (current) {
        if (current->visible) {
            return current;
        }
        current = current->nextWindow;
    }

    return NULL;
}

WindowPtr WM_GetPreviousWindow(WindowPtr window) {
    if (window == NULL) return NULL;

    WindowManagerState* wmState = GetWindowManagerState();
    WindowPtr current = wmState->windowList;

    /* Find the window that points to our window */
    while (current && current->nextWindow != window) {
        current = current->nextWindow;
    }

    return current;
}

/* FindWindow - Determine which part of the screen was clicked */
/* [WM-050] FindWindow canonical implementation in WindowEvents.c - removed incomplete duplicate */