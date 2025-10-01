/*
 * TimeBase.c - Time base calibration and conversion
 * Based on Inside Macintosh: Operating System Utilities (Time Manager)
 */

#include "SystemTypes.h"
#include "TimeManager/TimeBase.h"
#include "TimeManager/TimeManager.h"

/* Simple 64-bit division for 32-bit builds */
static uint64_t udiv64(uint64_t num, uint64_t den) {
    if (den == 0) return 0;
    uint64_t quot = 0;
    uint64_t bit = 1ULL << 63;
    while (bit && !(den & bit)) bit >>= 1;
    while (bit) {
        if (num >= den) {
            num -= den;
            quot |= bit;
        }
        den >>= 1;
        bit >>= 1;
    }
    return quot;
}

/* Time base state */
static struct {
    uint64_t bootCounter;
    uint64_t counterFreq;     /* Hz */
    uint64_t nsPerCount_32_32; /* 32.32 fixed point */
    uint32_t usPerCount_16_16; /* 16.16 fixed point */
    uint32_t resolutionNs;
    uint32_t overheadUs;
    Boolean  initialized;
} gTimeBase = {0};

OSErr InitTimeBase(void) {
    OSErr err = InitPlatformTimer();
    if (err != noErr) return err;
    
    /* Calibration: measure counter over ~10ms */
    uint64_t start = PlatformCounterNow();
    volatile uint32_t delay = 0;
    for (uint32_t i = 0; i < 10000000; i++) {
        delay++;
    }
    uint64_t end = PlatformCounterNow();
    uint64_t delta = end - start;
    
    /* Estimate frequency (assume ~10ms elapsed) */
    gTimeBase.counterFreq = delta * 100; /* Hz */
    if (gTimeBase.counterFreq < 1000000) {
        gTimeBase.counterFreq = 1000000; /* Min 1MHz */
    }
    
    /* Compute conversion factors using software division */
    gTimeBase.nsPerCount_32_32 = udiv64((NANOSECONDS_PER_SECOND << 32), gTimeBase.counterFreq);
    gTimeBase.usPerCount_16_16 = (uint32_t)udiv64((MICROSECONDS_PER_SECOND << 16), gTimeBase.counterFreq);

    gTimeBase.bootCounter = start;
    gTimeBase.resolutionNs = (uint32_t)udiv64(NANOSECONDS_PER_SECOND, gTimeBase.counterFreq);
    gTimeBase.overheadUs = 1; /* Estimated call overhead */
    gTimeBase.initialized = true;
    
    return noErr;
}

void ShutdownTimeBase(void) {
    gTimeBase.initialized = false;
    ShutdownPlatformTimer();
}

uint64_t GetTimerResolution(void) {
    return gTimeBase.resolutionNs;
}

UInt32 GetTimerOverhead(void) {
    return gTimeBase.overheadUs;
}

OSErr GetTimeBaseInfo(TimeBaseInfo *info) {
    if (!info) return paramErr;
    if (!gTimeBase.initialized) return tmNotActive;
    
    info->counterFrequency = gTimeBase.counterFreq;
    info->resolutionNs = gTimeBase.resolutionNs;
    info->overheadUs = gTimeBase.overheadUs;
    return noErr;
}

void Microseconds(UnsignedWide *microTickCount) {
    if (!microTickCount) return;
    if (!gTimeBase.initialized) {
        microTickCount->hi = 0;
        microTickCount->lo = 0;
        return;
    }
    
    uint64_t now = PlatformCounterNow();
    uint64_t delta = now - gTimeBase.bootCounter;
    
    /* Convert to microseconds using 16.16 fixed point */
    uint64_t us = (delta * gTimeBase.usPerCount_16_16) >> 16;
    
    microTickCount->hi = (UInt32)(us >> 32);
    microTickCount->lo = (UInt32)(us & 0xFFFFFFFF);
}

OSErr AbsoluteToNanoseconds(UnsignedWide absolute, UnsignedWide *duration) {
    if (!duration) return paramErr;
    
    uint64_t counts = ((uint64_t)absolute.hi << 32) | absolute.lo;
    uint64_t ns = (counts * gTimeBase.nsPerCount_32_32) >> 32;
    
    duration->hi = (UInt32)(ns >> 32);
    duration->lo = (UInt32)(ns & 0xFFFFFFFF);
    return noErr;
}

OSErr NanosecondsToAbsolute(UnsignedWide duration, UnsignedWide *absolute) {
    if (!absolute) return paramErr;

    uint64_t ns = ((uint64_t)duration.hi << 32) | duration.lo;
    uint64_t counts = udiv64((ns << 32), gTimeBase.nsPerCount_32_32);
    
    absolute->hi = (UInt32)(counts >> 32);
    absolute->lo = (UInt32)(counts & 0xFFFFFFFF);
    return noErr;
}
