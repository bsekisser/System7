/*
 * TimerInterrupts.c - Timer interrupt handling
 * Based on Inside Macintosh: Operating System Utilities (Time Manager)
 */

#include "SystemTypes.h"
#include "TimeManager/TimeManager.h"
#include "TimeManager/TimeManagerPriv.h"
#include "TimeManager/TimeBase.h"

/* Hardware timer state (simulated) */
static struct {
    UInt64 nextDeadlineUS;
    Boolean armed;
} gTimerState = {0};

void ProgramNextTimerInterrupt(UInt64 absDeadlineUS) {
    if (absDeadlineUS == 0) {
        /* Disarm timer */
        gTimerState.armed = false;
        gTimerState.nextDeadlineUS = 0;
    } else {
        /* Compute delta and cap to 1 second */
        UnsignedWide now;
        Microseconds(&now);
        UInt64 nowUS = ((UInt64)now.hi << 32) | now.lo;
        
        int64_t deltaUS = (int64_t)(absDeadlineUS - nowUS);
        if (deltaUS < 0) deltaUS = 0;
        if (deltaUS > MICROSECONDS_PER_SECOND) {
            deltaUS = MICROSECONDS_PER_SECOND;
        }
        
        gTimerState.nextDeadlineUS = nowUS + deltaUS;
        gTimerState.armed = true;
    }
}

void TimeManager_TimerISR(void) {
    if (!gTimerState.armed) return;
    
    UnsignedWide now;
    Microseconds(&now);
    UInt64 nowUS = ((UInt64)now.hi << 32) | now.lo;
    
    /* Check if timer expired */
    if ((int64_t)(gTimerState.nextDeadlineUS - nowUS) <= 0) {
        gTimerState.armed = false;
        Core_ExpireDue(nowUS);
    }
}
