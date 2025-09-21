#ifndef ALARMCLOCK_H
#define ALARMCLOCK_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/*
 * AlarmClock.h - Alarm Clock Desk Accessory
 *
 * Provides time display and alarm functionality. Shows current time and date,
 * allows setting alarms, and provides notification when alarms trigger.
 * Integrates with system time and notification services.
 *
 * Based on Apple's Alarm Clock DA from System 7.1
 */

#include "DeskAccessory.h"

/* Alarm Clock Constants */
#define ALARM_VERSION           0x0100      /* Alarm Clock version 1.0 */
#define MAX_ALARMS              10          /* Maximum number of alarms */
#define ALARM_NAME_LENGTH       32          /* Maximum alarm name length */
#define TIME_FORMAT_LENGTH      32          /* Time format string length */

/* Time Display Modes */

/* Date Display Modes */

/* Alarm Types */

/* Alarm States */

/* Notification Types */

/* Time Structure */

/* Date Structure */

/* Date/Time Structure */

/* Alarm Structure */

/* Alarm Clock State */

/* Alarm Clock Functions */

/**
 * Initialize Alarm Clock
 * @param clock Pointer to alarm clock structure
 * @return 0 on success, negative on error
 */
int AlarmClock_Initialize(AlarmClock *clock);

/**
 * Shutdown Alarm Clock
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_Shutdown(AlarmClock *clock);

/**
 * Update current time
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_UpdateTime(AlarmClock *clock);

/**
 * Check for triggered alarms
 * @param clock Pointer to alarm clock structure
 * @return Number of triggered alarms
 */
int AlarmClock_CheckAlarms(AlarmClock *clock);

/* Time Display Functions */

/**
 * Set time display mode
 * @param clock Pointer to alarm clock structure
 * @param mode Time display mode
 */
void AlarmClock_SetTimeMode(AlarmClock *clock, TimeDisplayMode mode);

/**
 * Set date display mode
 * @param clock Pointer to alarm clock structure
 * @param mode Date display mode
 */
void AlarmClock_SetDateMode(AlarmClock *clock, DateDisplayMode mode);

/**
 * Format time for display
 * @param clock Pointer to alarm clock structure
 * @param time Time to format
 * @param buffer Buffer for formatted string
 * @param bufferSize Size of buffer
 */
void AlarmClock_FormatTime(AlarmClock *clock, const AlarmTime *time,
                           char *buffer, int bufferSize);

/**
 * Format date for display
 * @param clock Pointer to alarm clock structure
 * @param date Date to format
 * @param buffer Buffer for formatted string
 * @param bufferSize Size of buffer
 */
void AlarmClock_FormatDate(AlarmClock *clock, const AlarmDate *date,
                           char *buffer, int bufferSize);

/**
 * Get current time string
 * @param clock Pointer to alarm clock structure
 * @return Formatted time string
 */
const char *AlarmClock_GetTimeString(AlarmClock *clock);

/**
 * Get current date string
 * @param clock Pointer to alarm clock structure
 * @return Formatted date string
 */
const char *AlarmClock_GetDateString(AlarmClock *clock);

/* Alarm Management Functions */

/**
 * Create new alarm
 * @param clock Pointer to alarm clock structure
 * @param name Alarm name
 * @param triggerTime When alarm should trigger
 * @param type Alarm type
 * @return Pointer to new alarm or NULL on error
 */
Alarm *AlarmClock_CreateAlarm(AlarmClock *clock, const char *name,
                              const DateTime *triggerTime, AlarmType type);

/**
 * Delete alarm
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return 0 on success, negative on error
 */
int AlarmClock_DeleteAlarm(AlarmClock *clock, SInt16 alarmID);

/**
 * Enable alarm
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return 0 on success, negative on error
 */
int AlarmClock_EnableAlarm(AlarmClock *clock, SInt16 alarmID);

/**
 * Disable alarm
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return 0 on success, negative on error
 */
int AlarmClock_DisableAlarm(AlarmClock *clock, SInt16 alarmID);

/**
 * Get alarm by ID
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return Pointer to alarm or NULL if not found
 */
Alarm *AlarmClock_GetAlarm(AlarmClock *clock, SInt16 alarmID);

/**
 * Get next alarm to trigger
 * @param clock Pointer to alarm clock structure
 * @return Pointer to next alarm or NULL if none
 */
Alarm *AlarmClock_GetNextAlarm(AlarmClock *clock);

/**
 * Snooze alarm
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return 0 on success, negative on error
 */
int AlarmClock_SnoozeAlarm(AlarmClock *clock, SInt16 alarmID);

/**
 * Dismiss alarm
 * @param clock Pointer to alarm clock structure
 * @param alarmID Alarm ID
 * @return 0 on success, negative on error
 */
int AlarmClock_DismissAlarm(AlarmClock *clock, SInt16 alarmID);

/* Date/Time Utility Functions */

/**
 * Get current system time
 * @param dateTime Pointer to date/time structure
 */
void AlarmClock_GetCurrentTime(DateTime *dateTime);

/**
 * Convert timestamp to date/time
 * @param timestamp Unix timestamp
 * @param dateTime Pointer to date/time structure
 */
void AlarmClock_TimestampToDateTime(SInt32 timestamp, DateTime *dateTime);

/**
 * Convert date/time to timestamp
 * @param dateTime Pointer to date/time structure
 * @return Unix timestamp
 */
SInt32 AlarmClock_DateTimeToTimestamp(const DateTime *dateTime);

/**
 * Add time interval to date/time
 * @param dateTime Pointer to date/time structure
 * @param days Days to add
 * @param hours Hours to add
 * @param minutes Minutes to add
 * @param seconds Seconds to add
 */
void AlarmClock_AddInterval(DateTime *dateTime, SInt16 days, SInt16 hours,
                            SInt16 minutes, SInt16 seconds);

/**
 * Calculate next trigger time for repeating alarm
 * @param alarm Pointer to alarm
 * @param currentTime Current time
 * @param nextTime Pointer to next trigger time (output)
 * @return 0 on success, negative on error
 */
int AlarmClock_CalculateNextTrigger(Alarm *alarm, const DateTime *currentTime,
                                    DateTime *nextTime);

/**
 * Check if date/time matches alarm pattern
 * @param alarm Pointer to alarm
 * @param dateTime Date/time to check
 * @return true if matches
 */
Boolean AlarmClock_MatchesPattern(Alarm *alarm, const DateTime *dateTime);

/* Notification Functions */

/**
 * Trigger alarm notification
 * @param clock Pointer to alarm clock structure
 * @param alarm Pointer to alarm
 * @return 0 on success, negative on error
 */
int AlarmClock_TriggerNotification(AlarmClock *clock, Alarm *alarm);

/**
 * Play alarm sound
 * @param soundName Sound file name
 * @param volume Volume (0-100)
 * @return 0 on success, negative on error
 */
int AlarmClock_PlaySound(const char *soundName, SInt16 volume);

/**
 * Show alarm dialog
 * @param clock Pointer to alarm clock structure
 * @param alarm Pointer to alarm
 * @return User response (snooze, dismiss, etc.)
 */
int AlarmClock_ShowAlarmDialog(AlarmClock *clock, Alarm *alarm);

/**
 * Flash menu bar
 * @param duration Flash duration in milliseconds
 */
void AlarmClock_FlashMenuBar(int duration);

/* Display Functions */

/**
 * Draw alarm clock window
 * @param clock Pointer to alarm clock structure
 * @param updateRect Rectangle to update or NULL for all
 */
void AlarmClock_Draw(AlarmClock *clock, const Rect *updateRect);

/**
 * Draw time display
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_DrawTime(AlarmClock *clock);

/**
 * Draw date display
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_DrawDate(AlarmClock *clock);

/**
 * Draw alarm indicator
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_DrawAlarmIndicator(AlarmClock *clock);

/* Settings Functions */

/**
 * Load settings from preferences
 * @param clock Pointer to alarm clock structure
 * @return 0 on success, negative on error
 */
int AlarmClock_LoadSettings(AlarmClock *clock);

/**
 * Save settings to preferences
 * @param clock Pointer to alarm clock structure
 * @return 0 on success, negative on error
 */
int AlarmClock_SaveSettings(AlarmClock *clock);

/**
 * Reset to default settings
 * @param clock Pointer to alarm clock structure
 */
void AlarmClock_ResetSettings(AlarmClock *clock);

/* Desk Accessory Integration */

/**
 * Register Alarm Clock as a desk accessory
 * @return 0 on success, negative on error
 */
int AlarmClock_RegisterDA(void);

/**
 * Create Alarm Clock DA instance
 * @return Pointer to DA instance or NULL on error
 */
DeskAccessory *AlarmClock_CreateDA(void);

/* Alarm Clock Error Codes */
#define ALARM_ERR_NONE              0       /* No error */
#define ALARM_ERR_INVALID_TIME      -1      /* Invalid time */
#define ALARM_ERR_INVALID_DATE      -2      /* Invalid date */
#define ALARM_ERR_ALARM_NOT_FOUND   -3      /* Alarm not found */
#define ALARM_ERR_TOO_MANY_ALARMS   -4      /* Too many alarms */
#define ALARM_ERR_SOUND_ERROR       -5      /* Sound playback error */
#define ALARM_ERR_NOTIFICATION_ERROR -6     /* Notification error */
#define ALARM_ERR_TIME_FORMAT       -7      /* Time format error */

#endif /* ALARMCLOCK_H */