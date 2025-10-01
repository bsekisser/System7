#ifndef MICROSECOND_TIMER_H
#define MICROSECOND_TIMER_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

OSErr AddWideTime      (UnsignedWide a, UnsignedWide b, UnsignedWide *out);
OSErr SubtractWideTime (UnsignedWide a, UnsignedWide b, UnsignedWide *out);

OSErr MicrosecondDelay (UInt32 microseconds);
OSErr NanosecondDelay  (uint64_t nanoseconds);

OSErr StartPerformanceTimer(UnsignedWide *start);
OSErr EndPerformanceTimer  (UnsignedWide start, UnsignedWide *elapsed);

#ifdef __cplusplus
}
#endif
#endif /* MICROSECOND_TIMER_H */
