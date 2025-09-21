#include "../../include/SystemTypes.h"
#include "../../include/QuickDraw/QuickDraw.h"
#include "../../include/QuickDraw/quickdraw_portable.h"

// Global port
GrafPtr thePort = NULL;

// Pattern definitions - non-const to match header
Pattern kBatteryEmptyPat = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
Pattern kBatteryLowPat = {{0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00}};
Pattern kBatteryMedPat = {{0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00}};
Pattern kBatteryHighPat = {{0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA}};
Pattern kBatteryFullPat = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

// Static patterns
static Pattern kChargingPat = {{0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18}};

void DrawBatteryLevel(short level, const Rect* bounds)
{
    if (thePort == NULL) return;
    if (bounds == NULL) return;

    // Draw battery outline
    FrameRect(bounds);

    // Draw battery terminal
    Rect terminal;
    SetRect(&terminal,
            bounds->right, bounds->top + 2,
            bounds->right + 2, bounds->bottom - 2);
    PaintRect(&terminal);

    // Fill battery based on level
    Rect fillRect;
    CopyRect(bounds, &fillRect);
    InsetRect(&fillRect, 1, 1);

    // Calculate fill width
    short fillWidth = (fillRect.right - fillRect.left) * level / 100;
    fillRect.right = fillRect.left + fillWidth;

    // Select pattern based on level
    Pattern* pat;
    if (level <= 10)
        pat = &kBatteryEmptyPat;
    else if (level <= 25)
        pat = &kBatteryLowPat;
    else if (level <= 50)
        pat = &kBatteryMedPat;
    else if (level <= 75)
        pat = &kBatteryHighPat;
    else
        pat = &kBatteryFullPat;

    // Fill with pattern
    FillRect(&fillRect, pat);
}

void DrawCharger(const Rect* bounds)
{
    if (thePort == NULL) return;
    if (bounds == NULL) return;

    // Draw lightning bolt symbol
    MoveTo((bounds->left + bounds->right) / 2, bounds->top);
    LineTo((bounds->left + bounds->right) / 2 - 2, (bounds->top + bounds->bottom) / 2);
    LineTo((bounds->left + bounds->right) / 2 + 2, (bounds->top + bounds->bottom) / 2);
    LineTo((bounds->left + bounds->right) / 2, bounds->bottom);
}

void InitPortableContext(PortableContext* context, const Rect* displayRect)
{
    if (context == NULL || displayRect == NULL) return;

    // Initialize the graphics port
    OpenPort(&context->port);

    // Set up screen bitmap
    CopyRect(displayRect, &context->bounds);
    SetPortBits(&context->screenBits);

    // Initialize patterns
    context->patterns[0] = kBatteryEmptyPat;
    context->patterns[1] = kBatteryLowPat;
    context->patterns[2] = kBatteryMedPat;
    context->patterns[3] = kBatteryHighPat;
    context->patterns[4] = kBatteryFullPat;
    context->patterns[5] = kChargingPat;
}

void UpdatePortableDisplay(PortableContext* context)
{
    if (context == NULL) return;

    // Save current port
    GrafPtr savedPort;
    GetPort(&savedPort);

    // Switch to portable context
    SetPort((GrafPtr)&context->port);

    // Update display
    InvalRect(&context->bounds);

    // Restore port
    SetPort(savedPort);
}

void DrawStatusIndicators(PortableContext* context)
{
    if (context == NULL) return;

    // Draw battery at top-right
    Rect batteryRect;
    SetRect(&batteryRect,
            context->bounds.right - 30,
            context->bounds.top + 2,
            context->bounds.right - 10,
            context->bounds.top + 10);

    DrawBatteryLevel(75, &batteryRect);

    // Draw charger indicator if connected
    Rect chargerRect;
    SetRect(&chargerRect,
            context->bounds.right - 40,
            context->bounds.top + 2,
            context->bounds.right - 32,
            context->bounds.top + 10);

    DrawCharger(&chargerRect);
}
