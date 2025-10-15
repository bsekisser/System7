/**
 * @file datetime_cdev.h
 * @brief Simple Date & Time control panel window.
 *
 * Provides a lightweight replacement for the classic Date & Time control
 * panel so the portable Finder can show and toggle the current clock.
 * The implementation exposes a small API that mirrors the other
 * control-panel helpers (e.g. Desktop Patterns) so the event dispatcher
 * can route window events without special casing.
 */

#ifndef DATETIME_CDEV_H
#define DATETIME_CDEV_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void DateTimePanel_Open(void);
void DateTimePanel_Close(void);
Boolean DateTimePanel_IsOpen(void);
WindowPtr DateTimePanel_GetWindow(void);
Boolean DateTimePanel_HandleEvent(EventRecord *event);
void DateTimePanel_Tick(void);

#ifdef __cplusplus
} /* extern \"C\" */
#endif

#endif /* DATETIME_CDEV_H */
