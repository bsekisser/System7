/*
 * RE-AGENT-BANNER
 * Desktop Patterns Control Panel (cdev)
 * System 7.1 Desktop Pattern Selector
 *
 * Control Panel for selecting desktop patterns.
 * Shows a grid of available patterns and allows the user to choose one.
 * Changes are applied immediately and saved to PRAM.
 */

#include "PatternMgr/pattern_manager.h"
#include "PatternMgr/pattern_resources.h"
#include "ControlManager/ControlManager.h"
#include "WindowManager/WindowManager.h"
#include "QuickDraw/QuickDraw.h"
#include "DialogManager/DialogManager.h"
#include <string.h>
#include <stdlib.h>

/* Layout constants */
#define GRID_COLS 8
#define GRID_ROWS 4
#define CELL_W    32
#define CELL_H    32
#define CELL_PAD   8
#define WINDOW_MARGIN 16

/* External QuickDraw globals */
extern QDGlobals qd;

/* Control button type */
#define pushButProc 0

/* Global state for the control panel */
static WindowPtr gDesktopCdevWin = NULL;
static ControlHandle gOKButton = NULL;
static ControlHandle gCancelButton = NULL;
static int16_t gSelectedPatID = 0;
static Pattern gOriginalPattern;
static RGBColor gOriginalColor;
static DesktopPref gOriginalPref;

/* Forward declarations */
static void DrawPatternCell(int col, int row, int16_t patID, bool selected);
static void DrawPatternGrid(void);
static int16_t GetPatternIDAtPosition(Point pt);
static void ApplySelectedPattern(void);
static void RestoreOriginalPattern(void);

/*
 * OpenDesktopCdev - Open the Desktop Patterns control panel
 */
void OpenDesktopCdev(void) {
    if (gDesktopCdevWin != NULL) {
        /* Already open, bring to front */
        SelectWindow(gDesktopCdevWin);
        return;
    }

    /* Calculate window size */
    Rect winRect;
    winRect.top = 50;
    winRect.left = 50;
    winRect.bottom = winRect.top + 40 + (GRID_ROWS * (CELL_H + CELL_PAD)) + 60;
    winRect.right = winRect.left + (GRID_COLS * (CELL_W + CELL_PAD)) + 32;

    /* Create the window */
    static unsigned char winTitle[] = {17, 'D','e','s','k','t','o','p',' ','P','a','t','t','e','r','n','s'};
    gDesktopCdevWin = NewWindow(NULL, &winRect, winTitle,
                                 true, documentProc, (WindowPtr)-1L, true, 0);
    if (!gDesktopCdevWin) return;

    SetPort(gDesktopCdevWin);

    /* Create OK and Cancel buttons */
    Rect buttonRect;
    buttonRect.bottom = winRect.bottom - winRect.top - 20;
    buttonRect.top = buttonRect.bottom - 20;
    buttonRect.right = winRect.right - winRect.left - 20;
    buttonRect.left = buttonRect.right - 80;

    static unsigned char okTitle[] = {2, 'O','K'};
    gOKButton = NewControl(gDesktopCdevWin, &buttonRect, okTitle,
                           true, 0, 0, 1, pushButProc, 0);

    buttonRect.right = buttonRect.left - 10;
    buttonRect.left = buttonRect.right - 80;
    static unsigned char cancelTitle[] = {6, 'C','a','n','c','e','l'};
    gCancelButton = NewControl(gDesktopCdevWin, &buttonRect, cancelTitle,
                               true, 0, 0, 1, pushButProc, 0);

    /* Save current pattern so we can restore on cancel */
    gOriginalPref = PM_GetSavedDesktopPref();
    PM_GetBackPat(&gOriginalPattern);
    PM_GetBackColor(&gOriginalColor);
    gSelectedPatID = gOriginalPref.patID;

    /* Draw the window contents */
    DrawPatternGrid();
}

/*
 * CloseDesktopCdev - Close the control panel
 */
void CloseDesktopCdev(void) {
    if (gDesktopCdevWin) {
        DisposeWindow(gDesktopCdevWin);
        gDesktopCdevWin = NULL;
        gOKButton = NULL;
        gCancelButton = NULL;
    }
}

/*
 * HandleDesktopCdevEvent - Handle events for the Desktop Patterns control panel
 */
void HandleDesktopCdevEvent(EventRecord *event) {
    if (!gDesktopCdevWin) return;

    WindowPtr whichWindow;
    Point where;
    ControlHandle control;
    int16_t part;

    switch (event->what) {
        case updateEvt:
            if ((WindowPtr)event->message == gDesktopCdevWin) {
                BeginUpdate(gDesktopCdevWin);
                SetPort(gDesktopCdevWin);
                DrawPatternGrid();
                DrawControls(gDesktopCdevWin);
                EndUpdate(gDesktopCdevWin);
            }
            break;

        case mouseDown:
            part = FindWindow(event->where, &whichWindow);
            if (whichWindow == gDesktopCdevWin) {
                switch (part) {
                    case inContent:
                        SelectWindow(gDesktopCdevWin);
                        SetPort(gDesktopCdevWin);
                        where = event->where;
                        GlobalToLocal(&where);

                        /* Check if click is on a control */
                        part = FindControl(where, gDesktopCdevWin, &control);
                        if (part && control) {
                            if (TrackControl(control, where, NULL)) {
                                if (control == gOKButton) {
                                    /* Save and apply the selected pattern */
                                    ApplySelectedPattern();
                                    CloseDesktopCdev();
                                } else if (control == gCancelButton) {
                                    /* Restore original pattern and close */
                                    RestoreOriginalPattern();
                                    CloseDesktopCdev();
                                }
                            }
                        } else {
                            /* Check if click is on a pattern cell */
                            int16_t patID = GetPatternIDAtPosition(where);
                            if (patID != 0 && patID != gSelectedPatID) {
                                gSelectedPatID = patID;

                                /* Apply pattern immediately for preview */
                                Pattern pat;
                                if (PM_LoadPAT(patID, &pat)) {
                                    PM_SetBackPat(&pat);
                                }

                                /* Redraw the grid to show new selection */
                                DrawPatternGrid();
                            }
                        }
                        break;

                    case inDrag:
                        DragWindow(gDesktopCdevWin, event->where, &qd.screenBits.bounds);
                        break;

                    case inGoAway:
                        if (TrackGoAway(gDesktopCdevWin, event->where)) {
                            RestoreOriginalPattern();
                            CloseDesktopCdev();
                        }
                        break;
                }
            }
            break;
    }
}

/*
 * DrawPatternCell - Draw a single pattern cell in the grid
 */
static void DrawPatternCell(int col, int row, int16_t patID, bool selected) {
    Rect cellRect;
    cellRect.left = WINDOW_MARGIN + col * (CELL_W + CELL_PAD);
    cellRect.top = 40 + row * (CELL_H + CELL_PAD);
    cellRect.right = cellRect.left + CELL_W;
    cellRect.bottom = cellRect.top + CELL_H;

    /* Draw border */
    if (selected) {
        /* Highlight selected pattern */
        PenSize(2, 2);
        FrameRect(&cellRect);
        PenSize(1, 1);
    } else {
        FrameRect(&cellRect);
    }

    /* Fill with pattern */
    Pattern pat;
    if (LoadPATResource(patID, &pat)) {
        InsetRect(&cellRect, 1, 1);
        FillRect(&cellRect, &pat);
    }
}

/*
 * DrawPatternGrid - Draw the entire grid of patterns
 */
static void DrawPatternGrid(void) {
    if (!gDesktopCdevWin) return;

    SetPort(gDesktopCdevWin);

    /* Clear the window */
    Rect winRect;
    winRect.top = 0;
    winRect.left = 0;
    winRect.right = 400;  /* Window width */
    winRect.bottom = 300; /* Window height */
    EraseRect(&winRect);

    /* Draw title */
    MoveTo(WINDOW_MARGIN, 25);
    static unsigned char titleStr[] = {24, 'S','e','l','e','c','t',' ','D','e','s','k','t','o','p',' ','P','a','t','t','e','r','n',':'};
    DrawString(titleStr);

    /* Draw pattern grid */
    /* Start with standard pattern IDs */
    int16_t patID = 16;  /* Start with kDesktopPatternID */

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            DrawPatternCell(col, row, patID, (patID == gSelectedPatID));
            patID++;
        }
    }
}

/*
 * GetPatternIDAtPosition - Determine which pattern was clicked
 */
static int16_t GetPatternIDAtPosition(Point pt) {
    /* Check if point is in grid area */
    if (pt.h < WINDOW_MARGIN || pt.v < 40) {
        return 0;
    }

    int col = (pt.h - WINDOW_MARGIN) / (CELL_W + CELL_PAD);
    int row = (pt.v - 40) / (CELL_H + CELL_PAD);

    if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
        /* Check if click is actually within cell (not in padding) */
        int cellX = WINDOW_MARGIN + col * (CELL_W + CELL_PAD);
        int cellY = 40 + row * (CELL_H + CELL_PAD);

        if (pt.h >= cellX && pt.h < cellX + CELL_W &&
            pt.v >= cellY && pt.v < cellY + CELL_H) {
            return 16 + (row * GRID_COLS) + col;
        }
    }

    return 0;
}

/*
 * ApplySelectedPattern - Apply and save the selected pattern
 */
static void ApplySelectedPattern(void) {
    if (gSelectedPatID == 0) return;

    /* Update the preference */
    DesktopPref pref = gOriginalPref;
    pref.usePixPat = false;
    pref.patID = gSelectedPatID;

    /* Save to PRAM */
    PM_SaveDesktopPref(&pref);

    /* Apply the pattern */
    Pattern pat;
    if (PM_LoadPAT(gSelectedPatID, &pat)) {
        PM_SetBackPat(&pat);
    }
}

/*
 * RestoreOriginalPattern - Restore the original pattern (for cancel)
 */
static void RestoreOriginalPattern(void) {
    PM_SetBackPat(&gOriginalPattern);
    PM_SetBackColor(&gOriginalColor);
}