/*
 * ARM64 Generic Timer Interface
 * ARMv8-A Architecture Timer
 */

#ifndef ARM64_TIMER_H
#define ARM64_TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* Initialize the ARM Generic Timer */
void timer_init(void);

/* Get timer frequency in Hz */
uint64_t timer_get_freq(void);

/* Get current counter value (raw ticks) */
uint64_t timer_get_ticks(void);

/* Get current time in microseconds since boot */
uint64_t timer_get_usec(void);

/* Get current time in milliseconds since boot */
uint64_t timer_get_msec(void);

/* Busy-wait delay functions */
void timer_usleep(uint64_t usec);
void timer_msleep(uint64_t msec);

/* Timer interrupt functions */
bool timer_set_timeout(uint64_t usec);
void timer_disable(void);
bool timer_is_pending(void);
void timer_ack(void);

#endif /* ARM64_TIMER_H */
