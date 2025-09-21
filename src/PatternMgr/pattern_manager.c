/*
 * RE-AGENT-BANNER
 * Pattern Manager Implementation
 * System 7.1 Desktop Pattern Management
 *
 * Manages the desktop pattern state that QuickDraw uses for erasing.
 * This mirrors the classic Mac OS architecture where the Pattern Manager
 * owns the background state and QuickDraw consumes it.
 */

#include "PatternMgr/pattern_manager.h"
#include "PatternMgr/pattern_resources.h"
#include "PatternMgr/pram_prefs.h"
#include "QuickDraw/QuickDraw.h"
#include "WindowManager/window_manager.h"
#include <string.h>
#include <stdlib.h>

/* Global Pattern Manager state */
static struct {
    bool initialized;
    bool usePixPat;
    Pattern backPat;
    Handle  backPixPat;    /* Opaque PixPat; format handled inside QuickDraw */
    RGBColor backColor;
    uint32_t colorPattern[64];  /* Decoded PPAT8 pixels */
    bool hasColorPattern;
} gPM;

/* External QuickDraw globals */
extern QDGlobals qd;

void PM_Init(void) {
    if (gPM.initialized) return;
    memset(&gPM, 0, sizeof(gPM));

    /* Default classic platinum gray */
    gPM.backColor.red   = 0xC000;
    gPM.backColor.green = 0xC000;
    gPM.backColor.blue  = 0xC000;

    /* A simple 50% stipple pattern */
    static const uint8_t dither[8] = {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
    memcpy(&gPM.backPat.pat, dither, 8);

    gPM.initialized = true;
}

/* External function to update quickdraw_impl's pattern */
extern void UpdateBackgroundPattern(const Pattern* pat);

void PM_SetBackPat(const Pattern *pat) {
    if (!pat) return;
    gPM.usePixPat = false;
    gPM.backPat = *pat;

    /* Inform QuickDraw - update the global pattern */
    BackPat(pat);
    /* Also update our local copy for EraseRect */
    UpdateBackgroundPattern(pat);
}

void PM_SetBackPixPat(Handle pixPatHandle) {
    if (!pixPatHandle) return;

    gPM.usePixPat = true;

    /* Release old handle if we have one */
    if (gPM.backPixPat) {
        DisposeHandle(gPM.backPixPat);
    }

    gPM.backPixPat = pixPatHandle; /* Caller transfers ownership to PM */

    /* Try to decode as PPAT8 */
    /* GetHandleSize is broken in our stub implementation, so use a fixed size for ppat */
    Size sz = 156;  /* Need 152 bytes for pattern 304, plus some buffer */
    HLock(pixPatHandle);
    const uint8_t* data = (const uint8_t*)*pixPatHandle;

    extern void serial_puts(const char* str);
    serial_puts("PM_SetBackPixPat: called\n");

    if (DecodePPAT8(data, sz, gPM.colorPattern)) {
        gPM.hasColorPattern = true;
        serial_puts("PM_SetBackPixPat: Successfully decoded color pattern\n");
        /* Set a neutral pattern so EraseRect knows to use color */
        Pattern neutralPat;
        memset(&neutralPat, 0xFF, sizeof(neutralPat));
        BackPat(&neutralPat);
        UpdateBackgroundPattern(&neutralPat);
    } else {
        /* Fall back to using first 8 bytes as pattern */
        gPM.hasColorPattern = false;
        serial_puts("PM_SetBackPixPat: Failed to decode, using fallback\n");
        if (sz >= 8) {
            Pattern tmpPat;
            memcpy(&tmpPat.pat, data, 8);
            BackPat(&tmpPat);
            UpdateBackgroundPattern(&tmpPat);
        }
    }

    HUnlock(pixPatHandle);
}

void PM_SetBackColor(const RGBColor *rgb) {
    if (!rgb) return;
    gPM.backColor = *rgb;
    RGBBackColor(rgb);
}

void PM_GetBackPat(Pattern *pat) {
    if (pat) *pat = gPM.backPat;
}

void PM_GetBackColor(RGBColor *rgb) {
    if (rgb) *rgb = gPM.backColor;
}

bool PM_IsPixPatActive(void) {
    return gPM.usePixPat;
}

DesktopPref PM_GetSavedDesktopPref(void) {
    DesktopPref p = {0};
    if (!PRAM_LoadDesktopPref(&p)) {
        /* Fallback defaults */
        p.usePixPat = false;
        p.patID = 16; /* kDesktopPatternID from SystemTypes.h */
        p.ppatID = 0;
        p.backColor.red = p.backColor.green = p.backColor.blue = 0xC000;
    }
    return p;
}

void PM_SaveDesktopPref(const DesktopPref *p) {
    if (!p) return;
    PRAM_SaveDesktopPref(p);
}

bool PM_ApplyDesktopPref(const DesktopPref *p) {
    extern void serial_puts(const char* str);
    serial_puts("PM_ApplyDesktopPref called\n");

    if (!p) return false;

    /* Set background color */
    PM_SetBackColor(&p->backColor);

    if (p->usePixPat) {
        serial_puts("PM: Using ppat pattern\n");
        Handle h = PM_LoadPPAT(p->ppatID);
        if (!h) {
            serial_puts("PM: Failed to load ppat!\n");
            return false;
        }
        serial_puts("PM: Loaded ppat, calling SetBackPixPat\n");
        PM_SetBackPixPat(h);
        serial_puts("PM: SetBackPixPat done\n");
    } else {
        serial_puts("PM: Using PAT pattern\n");
        Pattern pat;
        if (!PM_LoadPAT(p->patID, &pat)) {
            /* Fall back to default gray pattern */
            pat = qd.gray;
        }
        PM_SetBackPat(&pat);
    }

    /* Trigger a desktop redraw */
    Rect desktopRect;
    desktopRect.left = 0;
    desktopRect.top = 0;
    desktopRect.right = qd.screenBits.bounds.right;
    desktopRect.bottom = qd.screenBits.bounds.bottom;
    InvalRect(&desktopRect);
    serial_puts("PM: Desktop invalidated\n");

    return true;
}

bool PM_LoadPAT(int16_t id, Pattern *out) {
    if (!out) return false;
    return LoadPATResource(id, out);
}

Handle PM_LoadPPAT(int16_t id) {
    return LoadPPATResource(id);
}

/* Get color pattern data if available */
bool PM_GetColorPattern(uint32_t** patternData) {
    if (gPM.hasColorPattern && patternData) {
        *patternData = gPM.colorPattern;
        return true;
    }
    return false;
}