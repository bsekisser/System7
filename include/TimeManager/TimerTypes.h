/*
 * TimerTypes.h
 *
 * Timer Task and Time Data Structures for System 7.1 Time Manager
 *
 * This file defines the data structures used by the Time Manager,
 * converted from the original 68k code definitions.
 */

#ifndef TIMERTYPES_H
#define TIMERTYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Forward Declarations ===== */

/* Ptr is defined in MacTypes.h */

/* ===== Timer Procedure Types ===== */

/**
 * Timer Completion Procedure
 *
 * Called when a timer task expires. The original implementation passes:
 * - A0: Address of the service routine itself
 * - A1: Pointer to the expired TMTask structure
 *
 * In the portable version:
 * @param tmTaskPtr - Pointer to the expired TMTask structure
 */

/* ===== TMTask Structure ===== */

/**
 * Time Manager Task Record
 *
 * This structure represents a timer task, based on the original
 * TMTask record from ROM system equates. The structure supports both standard
 * and extended timer tasks.
 */

/* ===== Time Manager Private Data ===== */

/**
 * Time Manager Private Storage
 *
 * This structure contains the Time Manager's internal state,
 * based on the TimeMgrPrivate record from the original implementation.
 */

/* ===== Time Base Information ===== */

/**
 * Time Base Information Record
 *
 * Contains information about the system time base, used by
 * high-resolution timing functions.
 */

/* ===== Time Conversion Structures ===== */

/**
 * 64-bit Time Value
 *
 * Used for high-precision time measurements and microsecond counters.
 */
             /* High 32 bits */
        UInt32    lo;             /* Low 32 bits */
    } parts;
    uint64_t        value;          /* Full 64-bit value */
} UnsignedWide;

/**
 * Fixed-Point Time Value
 *
 * Used for fractional time calculations in the Time Manager.
 */

/* ===== Task State Constants ===== */

/* QType flag bits */
#define QTASK_ACTIVE_FLAG       0x8000      /* Task is currently active */
#define QTASK_EXTENDED_FLAG     0x4000      /* Extended TMTask record */

/* QType base values */
#define QTASK_TIMER_TYPE        1           /* Standard timer task type */
#define QTASK_EXTENDED_TYPE     2           /* Extended timer task type */

/* ===== Timing Constants ===== */

/* Time conversion factors */
#define MICROSECONDS_PER_SECOND     1000000
#define MILLISECONDS_PER_SECOND     1000
#define MICROSECONDS_PER_MILLISEC   1000

/* Maximum timer values */
#define MAX_MILLISECONDS_DELAY      86400000    /* ~1 day in milliseconds */
#define MAX_MICROSECONDS_DELAY      2147483647  /* Maximum positive SInt32 */

/* ===== Utility Macros ===== */

/**
 * Check if a TMTask is active
 */
#define IS_TMTASK_ACTIVE(task)      ((task)->qType & QTASK_ACTIVE_FLAG)

/**
 * Check if a TMTask is extended
 */
#define IS_TMTASK_EXTENDED(task)    ((task)->qType & QTASK_EXTENDED_FLAG)

/**
 * Set TMTask active flag
 */
#define SET_TMTASK_ACTIVE(task)     ((task)->qType |= QTASK_ACTIVE_FLAG)

/**
 * Clear TMTask active flag
 */
#define CLEAR_TMTASK_ACTIVE(task)   ((task)->qType &= ~QTASK_ACTIVE_FLAG)

/**
 * Set TMTask extended flag
 */
#define SET_TMTASK_EXTENDED(task)   ((task)->qType |= QTASK_EXTENDED_FLAG)

/**
 * Clear TMTask extended flag
 */
#define CLEAR_TMTASK_EXTENDED(task) ((task)->qType &= ~QTASK_EXTENDED_FLAG)

/**
 * Convert milliseconds to microseconds (with overflow check)
 */
#define MS_TO_US(ms)                ((ms) <= MAX_MILLISECONDS_DELAY / 1000 ? \
                                     (ms) * MICROSECONDS_PER_MILLISEC : MAX_MICROSECONDS_DELAY)

/**
 * Convert microseconds to milliseconds (with rounding)
 */
#define US_TO_MS(us)                (((us) + MICROSECONDS_PER_MILLISEC / 2) / MICROSECONDS_PER_MILLISEC)

#ifdef __cplusplus
}
#endif

#endif /* TIMERTYPES_H */