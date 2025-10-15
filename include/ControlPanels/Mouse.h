/**
 * @file Mouse.h
 * @brief Lightweight Mouse control panel window.
 */

#ifndef CONTROL_PANELS_MOUSE_H
#define CONTROL_PANELS_MOUSE_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void MousePanel_Open(void);
void MousePanel_Close(void);
Boolean MousePanel_IsOpen(void);
WindowPtr MousePanel_GetWindow(void);
Boolean MousePanel_HandleEvent(EventRecord *event);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_PANELS_MOUSE_H */
