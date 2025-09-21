#ifndef WINDOWMANAGERINTERNAL_H
#define WINDOWMANAGERINTERNAL_H

#include "../SystemTypes.h"
#include "WindowManager.h"

// Internal window management structures
typedef struct WindowListEntry {
    WindowPtr window;
    struct WindowListEntry* next;
    struct WindowListEntry* prev;
} WindowListEntry;

typedef struct WindowManagerState {
    GrafPtr wMgrPort;           /* Window Manager port */
    CGrafPtr wMgrCPort;         /* Color Window Manager port */
    WindowPtr windowList;       /* Head of window list */
    WindowPtr activeWindow;     /* Currently active window */
    AuxWinHandle auxWinHead;    /* Head of auxiliary window list */
    Pattern desktopPattern;     /* Desktop pattern */
    PixPatHandle desktopPixPat; /* Desktop pixel pattern (Color QD) */
    short nextWindowID;         /* Next window ID to assign */
    Boolean colorQDAvailable;   /* Color QuickDraw available */
    Boolean initialized;        /* Window Manager initialized */
    void* platformData;         /* Platform-specific data */

    /* Additional fields from the simpler version */
    WindowPtr frontWindow;
    Boolean isDragging;
    Boolean isGrowing;
    Point dragOffset;

    /* Fields referenced in InitializeWMgrPort */
    GrafPort port;              /* The actual port structure */
    WindowPtr ghostWindow;      /* Ghost window for dragging */
    Pattern deskPattern;        /* Desktop pattern */
    short menuBarHeight;        /* Height of menu bar */
    RgnHandle grayRgn;          /* Gray region (desktop minus menu bar) */
} WindowManagerState;

/* Window update flags type is defined in SystemTypes.h */

// Internal functions
void InitWindowInternals(void);
WindowListEntry* FindWindowEntry(WindowPtr window);
void UpdateWindowOrder(void);
void InvalidateWindowRegion(WindowPtr window, RgnHandle region);
WindowPtr WM_GetNextVisibleWindow(WindowPtr window);
WindowManagerState* GetWindowManagerState(void);
void WM_ScheduleWindowUpdate(WindowPtr window, WindowUpdateFlags flags);

extern WindowManagerState gWindowState;

#endif
