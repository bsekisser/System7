/*
 * PlatformTime.c - Minimal platform timer for freestanding x86
 */

#include "SystemTypes.h"
#include "TimeManager/TimeBase.h"

/* Use RDTSC on x86 */
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static uint64_t gBootCounter = 0;
static uint64_t gCounterFreq = 2000000000ULL; /* Assume 2GHz */

OSErr InitPlatformTimer(void) {
    gBootCounter = rdtsc();
    return noErr;
}

void ShutdownPlatformTimer(void) {
}

uint64_t PlatformCounterNow(void) {
    return rdtsc();
}

OSErr GetPlatformTime(UnsignedWide *timeValue) {
    uint64_t now = rdtsc();
    timeValue->hi = (UInt32)(now >> 32);
    timeValue->lo = (UInt32)(now & 0xFFFFFFFF);
    return noErr;
}
