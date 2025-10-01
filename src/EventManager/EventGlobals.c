/*
 * EventGlobals.c - Event Manager Global Settings Implementation
 *
 * Implements classic System 7 event timing globals.
 */

#include "EventManager/EventGlobals.h"

/* Double-click timing: 36 ticks ≈ 600ms at 60Hz (classic System 7 default) */
UInt32 gDoubleTimeTicks = 36;

/* Double-click pixel slop: 6 pixels max distance */
UInt16 gDoubleClickSlop = 6;

/* Current button state (bit 0 = primary button) */
volatile UInt8 gCurrentButtons = 0;

/**
 * GetDblTime - Get current double-click time threshold
 * Classic System 7 API for retrieving double-click timing
 */
UInt32 GetDblTime(void)
{
    return gDoubleTimeTicks;
}
