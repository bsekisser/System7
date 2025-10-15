/**
 * @file datetime_cdev.c
 * @brief Minimal Date & Time control panel window.
 */

#include "SystemTypes.h"
#include "System71StdLib.h"

#include "Datetime/datetime_cdev.h"

#include "ControlManager/ControlManager.h"
#include "ControlManager/ControlTypes.h"
#include "MenuManager/MenuManager.h"
#include "QuickDraw/QuickDraw.h"
#include "WindowManager/WindowManager.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ----------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------------- */

#define PANEL_WIDTH   320
#define PANEL_HEIGHT  180
#define MAC_EPOCH_OFFSET 2082844800UL

typedef struct DateTimePanelState {
    Boolean    isOpen;
    Boolean    showSeconds;
    Boolean    use24Hour;
    WindowPtr  window;
    ControlHandle showSecondsCheck;
    ControlHandle use24HourCheck;
    UInt32     lastDisplayKey;
    Rect       textRect;
} DateTimePanelState;

static DateTimePanelState gPanel = {
    .isOpen = false,
    .showSeconds = false,
    .use24Hour = false,
    .window = NULL,
    .showSecondsCheck = NULL,
    .use24HourCheck = NULL,
    .lastDisplayKey = 0,
    .textRect = { 40, 20, 100, PANEL_WIDTH - 20 }
};

/* Convert a C string to a Pascal Str255. */
static void cstr_to_pstr(const char *src, Str255 dst)
{
    if (!src || !dst) {
        return;
    }

    size_t len = strlen(src);
    if (len > 255) {
        len = 255;
    }

    dst[0] = (UInt8)len;
    memcpy(&dst[1], src, len);
}

static time_t mac_to_unix(UInt32 macTime)
{
    if (macTime < MAC_EPOCH_OFFSET) {
        return 0;
    }
    return (time_t)(macTime - MAC_EPOCH_OFFSET);
}

static UInt32 current_mac_time(void)
{
    time_t now = time(NULL);
    return (UInt32)(now + MAC_EPOCH_OFFSET);
}

static UInt32 compute_display_key(UInt32 macTime)
{
    return gPanel.showSeconds ? macTime : (macTime / 60U);
}

static void draw_panel_contents(void)
{
    if (!gPanel.window) {
        return;
    }

    GrafPtr previousPort;
    GetPort(&previousPort);
    SetPort((GrafPtr)gPanel.window);

    /* Clear the text area */
    Rect drawRect = gPanel.textRect;
    EraseRect(&drawRect);

    UInt32 macTime = current_mac_time();
    gPanel.lastDisplayKey = compute_display_key(macTime);

    time_t unixTime = mac_to_unix(macTime);
    struct tm localTm;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    localtime_r(&unixTime, &localTm);
#else
    struct tm *tmp = localtime(&unixTime);
    if (tmp) {
        localTm = *tmp;
    } else {
        memset(&localTm, 0, sizeof(localTm));
    }
#endif

    char dateBuf[64];
    if (strftime(dateBuf, sizeof(dateBuf), "%A, %B %e, %Y", &localTm) == 0) {
        strcpy(dateBuf, "--");
    }

    char timeFormat[16];
    if (gPanel.use24Hour) {
        strcpy(timeFormat, gPanel.showSeconds ? "%H:%M:%S" : "%H:%M");
    } else {
        strcpy(timeFormat, gPanel.showSeconds ? "%I:%M:%S %p" : "%I:%M %p");
    }

    char timeBuf[32];
    if (strftime(timeBuf, sizeof(timeBuf), timeFormat, &localTm) == 0) {
        strcpy(timeBuf, "--");
    }

    Str255 pascal;
    MoveTo(gPanel.textRect.left, gPanel.textRect.top + 20);
    cstr_to_pstr(dateBuf, pascal);
    DrawString(pascal);

    MoveTo(gPanel.textRect.left, gPanel.textRect.top + 44);
    cstr_to_pstr(timeBuf, pascal);
    DrawString(pascal);

    SetPort(previousPort);
}

static void update_checkbox(ControlHandle ctrl, Boolean checked)
{
    if (!ctrl) {
        return;
    }
    SetControlValue(ctrl, checked ? 1 : 0);
    Draw1Control(ctrl);
}

static void ensure_controls(void)
{
    if (!gPanel.window) {
        return;
    }

    Rect checkboxRect;
    Rect portRect = gPanel.window->port.portRect;

    if (!gPanel.use24HourCheck) {
        checkboxRect.top = portRect.bottom - 60;
        checkboxRect.left = 20;
        checkboxRect.bottom = checkboxRect.top + 18;
        checkboxRect.right = portRect.right - 20;
        gPanel.use24HourCheck = NewControl(
            gPanel.window,
            &checkboxRect,
            "\01424-Hour Time",
            true,
            gPanel.use24Hour ? 1 : 0,
            0,
            1,
            checkBoxProc,
            0
        );
    } else {
        update_checkbox(gPanel.use24HourCheck, gPanel.use24Hour);
    }

    if (!gPanel.showSecondsCheck) {
        checkboxRect.top = portRect.bottom - 36;
        checkboxRect.left = 20;
        checkboxRect.bottom = checkboxRect.top + 18;
        checkboxRect.right = portRect.right - 20;
        gPanel.showSecondsCheck = NewControl(
            gPanel.window,
            &checkboxRect,
            "\014Show Seconds",
            true,
            gPanel.showSeconds ? 1 : 0,
            0,
            1,
            checkBoxProc,
            0
        );
    } else {
        update_checkbox(gPanel.showSecondsCheck, gPanel.showSeconds);
    }
}

static void destroy_controls(void)
{
    if (gPanel.use24HourCheck) {
        DisposeControl(gPanel.use24HourCheck);
        gPanel.use24HourCheck = NULL;
    }
    if (gPanel.showSecondsCheck) {
        DisposeControl(gPanel.showSecondsCheck);
        gPanel.showSecondsCheck = NULL;
    }
}

static void request_redraw(void)
{
    if (!gPanel.window) {
        return;
    }
    GrafPtr previousPort;
    GetPort(&previousPort);
    SetPort((GrafPtr)gPanel.window);
    Rect r = gPanel.textRect;
    InvalRect(&r);
    SetPort(previousPort);
}

/* ----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------- */

void DateTimePanel_Open(void)
{
    if (gPanel.isOpen && gPanel.window) {
        SelectWindow(gPanel.window);
        return;
    }

    Rect bounds;
    bounds.top = 80;
    bounds.left = 120;
    bounds.bottom = bounds.top + PANEL_HEIGHT;
    bounds.right = bounds.left + PANEL_WIDTH;

    static unsigned char title[] = {12, 'D','a','t','e',' ','&',' ','T','i','m','e'};
    gPanel.window = NewWindow(NULL, &bounds, title, true, documentProc, (WindowPtr)-1, true, 0);
    if (!gPanel.window) {
        return;
    }

    gPanel.isOpen = true;
    gPanel.lastDisplayKey = 0;

    Rect portRect = gPanel.window->port.portRect;
    gPanel.textRect.left = 20;
    gPanel.textRect.top = 40;
    gPanel.textRect.right = portRect.right - 20;
    gPanel.textRect.bottom = gPanel.textRect.top + 80;

    ensure_controls();
    draw_panel_contents();
    ShowWindow(gPanel.window);
}

void DateTimePanel_Close(void)
{
    if (!gPanel.isOpen) {
        return;
    }

    destroy_controls();

    if (gPanel.window) {
        DisposeWindow(gPanel.window);
        gPanel.window = NULL;
    }

    gPanel.isOpen = false;
}

Boolean DateTimePanel_IsOpen(void)
{
    return gPanel.isOpen;
}

WindowPtr DateTimePanel_GetWindow(void)
{
    return gPanel.window;
}

static void toggle_checkbox_state(ControlHandle ctrl, Boolean *flag)
{
    if (!ctrl || !flag) {
        return;
    }

    Boolean newValue = !(*flag);
    *flag = newValue;
    SetControlValue(ctrl, newValue ? 1 : 0);
    Draw1Control(ctrl);

    gPanel.lastDisplayKey = UINT32_MAX;
    request_redraw();
}

Boolean DateTimePanel_HandleEvent(EventRecord *event)
{
    if (!event || !gPanel.isOpen || !gPanel.window) {
        return false;
    }

    WindowPtr whichWindow = NULL;
    Point localPt;
    ControlHandle ctl;
    SInt16 partCode;

    switch (event->what) {
        case updateEvt:
            if ((WindowPtr)event->message != gPanel.window) {
                return false;
            }
            BeginUpdate(gPanel.window);
            ensure_controls();
            draw_panel_contents();
            DrawControls(gPanel.window);
            EndUpdate(gPanel.window);
            return true;

        case mouseDown:
            partCode = FindWindow(event->where, &whichWindow);
            if (whichWindow != gPanel.window) {
                return false;
            }

            switch (partCode) {
                case inGoAway:
                    if (TrackGoAway(gPanel.window, event->where)) {
                        DateTimePanel_Close();
                    }
                    return true;

                case inContent:
                    SelectWindow(gPanel.window);
                    SetPort((GrafPtr)gPanel.window);
                    localPt = event->where;
                    GlobalToLocal(&localPt);

                    partCode = FindControl(localPt, gPanel.window, &ctl);
                    if (partCode != 0 && ctl != NULL) {
                        SInt16 trackResult = TrackControl(ctl, localPt, NULL);
                        if (trackResult != 0) {
                            if (ctl == gPanel.use24HourCheck) {
                                toggle_checkbox_state(gPanel.use24HourCheck, &gPanel.use24Hour);
                            } else if (ctl == gPanel.showSecondsCheck) {
                                toggle_checkbox_state(gPanel.showSecondsCheck, &gPanel.showSeconds);
                            }
                        }
                        return true;
                    }
                    return false;

                case inDrag: {
                    Rect limit = qd.screenBits.bounds;
                    DragWindow(gPanel.window, event->where, &limit);
                    return true;
                }

                default:
                    return false;
            }

        case activateEvt:
            if ((WindowPtr)event->message == gPanel.window) {
                Boolean becomingActive = (event->modifiers & activeFlag) != 0;
                if (gPanel.use24HourCheck) {
                    HiliteControl(gPanel.use24HourCheck, becomingActive ? noHilite : inactiveHilite);
                }
                if (gPanel.showSecondsCheck) {
                    HiliteControl(gPanel.showSecondsCheck, becomingActive ? noHilite : inactiveHilite);
                }
                return true;
            }
            return false;

        default:
            return false;
    }
}

void DateTimePanel_Tick(void)
{
    if (!gPanel.isOpen || !gPanel.window) {
        return;
    }

    UInt32 now = current_mac_time();
    UInt32 key = compute_display_key(now);
    if (key != gPanel.lastDisplayKey) {
        gPanel.lastDisplayKey = key;
        request_redraw();
    }
}
