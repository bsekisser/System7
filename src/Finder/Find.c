/*
 * Find.c - Find File Dialog Implementation
 *
 * Provides File > Find and Find Again functionality.
 * Allows searching for files by name pattern across volumes.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "SystemTypes.h"
#include "WindowManager/WindowManager.h"
#include "QuickDraw/QuickDraw.h"
#include "FS/vfs.h"
#include "FS/hfs_types.h"
#include "Finder/FinderLogging.h"
#include "Finder/finder.h"
#include "EventManager/EventManager.h"

/* Forward declarations */
extern void MoveTo(short h, short v);
extern void DrawText(const void* textBuf, short firstByte, short byteCount);
extern void EraseRect(const Rect* r);
extern void GetPort(GrafPtr* port);
extern void SetPort(GrafPtr port);
extern void SelectWindow(WindowPtr w);
extern void BringToFront(WindowPtr w);
extern void DisposeWindow(WindowPtr w);
extern WindowPtr NewWindow(void* wStorage, const Rect* boundsRect, ConstStr255Param title,
                           Boolean visible, short procID, WindowPtr behind, Boolean goAwayFlag, long refCon);

#define kFindRefCon 0x46494E44  /* 'FIND' */

/* Global Find window state */
static WindowPtr sFindWin = NULL;
static char sSearchTerm[256] = "";
static Boolean sHasSearchTerm = false;

/*
 * Find_CreateWindow - Create Find dialog window
 */
static void Find_CreateWindow(void) {
    Rect bounds;
    unsigned char title[64];

    if (sFindWin) {
        return;
    }

    /* Position window */
    bounds.top = 100;
    bounds.left = 120;
    bounds.bottom = bounds.top + 200;  /* Height: 200 pixels */
    bounds.right = bounds.left + 400;  /* Width: 400 pixels */

    /* Create window title */
    const char* titleText = "Find";
    title[0] = (unsigned char)strlen(titleText);
    strcpy((char*)&title[1], titleText);

    sFindWin = NewWindow(NULL, &bounds, title,
                        1,  /* visible */
                        0,  /* documentProc */
                        (WindowPtr)-1,
                        1,  /* Has close box */
                        kFindRefCon);

    if (!sFindWin) {
        FINDER_LOG_DEBUG("Find: Failed to create window\n");
        return;
    }

    FINDER_LOG_DEBUG("Find: Created window at 0x%08x\n", (unsigned int)P2UL(sFindWin));
}

/*
 * ShowFind - Show Find dialog
 */
OSErr ShowFind(void) {
    FINDER_LOG_DEBUG("ShowFind: Entry\n");

    /* Create window if needed */
    if (!sFindWin) {
        Find_CreateWindow();
    }

    if (!sFindWin) {
        return memFullErr;
    }

    /* Bring to front */
    BringToFront(sFindWin);
    SelectWindow(sFindWin);

    /* Request update */
    PostEvent(updateEvt, (UInt32)sFindWin);

    FINDER_LOG_DEBUG("ShowFind: Window shown\n");
    return noErr;
}

/*
 * Find_CloseIf - Close Find window if it matches
 */
void Find_CloseIf(WindowPtr w) {
    if (!w || w != sFindWin) {
        return;
    }

    FINDER_LOG_DEBUG("Find: Closing window\n");
    DisposeWindow(sFindWin);
    sFindWin = NULL;
}

/*
 * Find_HandleUpdate - Redraw Find window contents
 */
Boolean Find_HandleUpdate(WindowPtr w) {
    GrafPtr savedPort;
    Rect contentRect;
    char buf[256];
    short y;

    if (!w || w != sFindWin) {
        return false;
    }

    FINDER_LOG_DEBUG("Find: HandleUpdate called\n");

    /* Save current port and set to our window */
    GetPort(&savedPort);
    SetPort((GrafPtr)w);

    /* Get content rect */
    contentRect = w->port.portRect;
    contentRect.top = 20;  /* Skip title bar */

    /* Clear background */
    EraseRect(&contentRect);

    /* Draw instructions */
    y = contentRect.top + 30;
    MoveTo(20, y);
    snprintf(buf, sizeof(buf), "Find File Dialog");
    DrawText(buf, 0, strlen(buf));
    y += 25;

    MoveTo(20, y);
    snprintf(buf, sizeof(buf), "Search for:");
    DrawText(buf, 0, strlen(buf));
    y += 20;

    /* Draw search term input area (placeholder) */
    MoveTo(30, y);
    if (sHasSearchTerm && sSearchTerm[0] != '\0') {
        snprintf(buf, sizeof(buf), "> %s", sSearchTerm);
    } else {
        snprintf(buf, sizeof(buf), "> (enter search term)");
    }
    DrawText(buf, 0, strlen(buf));
    y += 25;

    /* Instructions */
    MoveTo(20, y);
    snprintf(buf, sizeof(buf), "Type text to search for files by name.");
    DrawText(buf, 0, strlen(buf));
    y += 20;

    MoveTo(20, y);
    snprintf(buf, sizeof(buf), "Press Return to search.");
    DrawText(buf, 0, strlen(buf));

    /* Restore previous port */
    SetPort(savedPort);

    FINDER_LOG_DEBUG("Find: Update complete\n");
    return true;
}

/*
 * Find_IsFindWindow - Check if window is the Find window
 */
Boolean Find_IsFindWindow(WindowPtr w) {
    return (w && w == sFindWin);
}

/*
 * FindAgain - Repeat last search
 */
OSErr FindAgain(void) {
    FINDER_LOG_DEBUG("FindAgain: Entry\n");

    if (!sHasSearchTerm || sSearchTerm[0] == '\0') {
        FINDER_LOG_DEBUG("FindAgain: No previous search term\n");
        /* If no previous search, show Find dialog */
        return ShowFind();
    }

    FINDER_LOG_DEBUG("FindAgain: Repeating search for '%s'\n", sSearchTerm);

    /* TODO: Implement actual search functionality */
    /* For now, just show the Find dialog */
    return ShowFind();
}

/*
 * Find_HandleKeyPress - Handle keyboard input in Find window
 */
Boolean Find_HandleKeyPress(WindowPtr w, char key) {
    if (!w || w != sFindWin) {
        return false;
    }

    /* Handle character input */
    if (key >= 32 && key <= 126) {  /* Printable ASCII */
        size_t len = strlen(sSearchTerm);
        if (len < sizeof(sSearchTerm) - 1) {
            sSearchTerm[len] = key;
            sSearchTerm[len + 1] = '\0';
            sHasSearchTerm = true;

            /* Trigger redraw */
            PostEvent(updateEvt, (UInt32)w);

            FINDER_LOG_DEBUG("Find: Added char '%c', term now '%s'\n", key, sSearchTerm);
        }
        return true;
    }

    /* Handle backspace */
    if (key == 8 || key == 127) {  /* Backspace or Delete */
        size_t len = strlen(sSearchTerm);
        if (len > 0) {
            sSearchTerm[len - 1] = '\0';

            /* Trigger redraw */
            PostEvent(updateEvt, (UInt32)w);

            FINDER_LOG_DEBUG("Find: Backspace, term now '%s'\n", sSearchTerm);
        }
        return true;
    }

    /* Handle return key - perform search */
    if (key == 13 || key == 3) {  /* Return or Enter */
        if (sSearchTerm[0] != '\0') {
            FINDER_LOG_DEBUG("Find: Search requested for '%s'\n", sSearchTerm);
            /* TODO: Implement actual search */
            /* For now, just log it */
        }
        return true;
    }

    return false;
}
