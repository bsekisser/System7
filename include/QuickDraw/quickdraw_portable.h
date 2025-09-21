#ifndef QUICKDRAW_PORTABLE_H
#define QUICKDRAW_PORTABLE_H

#include "../SystemTypes.h"
#include "QuickDraw.h"

// Portable-specific types
typedef struct PortableContext {
    GrafPort port;
    BitMap screenBits;
    Rect bounds;
    Pattern patterns[10];
    void* buffer;
    long bufferSize;
} PortableContext;

// Battery level constants
enum {
    kBatteryEmpty = 0,
    kBatteryLow = 25,
    kBatteryMedium = 50,
    kBatteryHigh = 75,
    kBatteryFull = 100
};

// Drawing modes
enum {
    srcCopy = 0,
    srcOr = 1,
    srcXor = 2,
    srcBic = 3,
    notSrcCopy = 4,
    notSrcOr = 5,
    notSrcXor = 6,
    notSrcBic = 7
};

// Pattern definitions
extern Pattern kBatteryEmptyPat;
extern Pattern kBatteryLowPat;
extern Pattern kBatteryMedPat;
extern Pattern kBatteryHighPat;
extern Pattern kBatteryFullPat;

// Function prototypes
void InitPortableContext(PortableContext* context, const Rect* displayRect);
void DrawBatteryLevel(short level, const Rect* bounds);
void DrawCharger(const Rect* bounds);
void UpdatePortableDisplay(PortableContext* context);
void DrawStatusIndicators(PortableContext* context);

// Global port pointer
extern GrafPtr thePort;

#endif
