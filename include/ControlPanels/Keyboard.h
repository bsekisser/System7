/**
 * @file Keyboard.h
 * @brief Lightweight Keyboard control panel window.
 */

#ifndef CONTROL_PANELS_KEYBOARD_H
#define CONTROL_PANELS_KEYBOARD_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void KeyboardPanel_Open(void);
void KeyboardPanel_Close(void);
Boolean KeyboardPanel_IsOpen(void);
WindowPtr KeyboardPanel_GetWindow(void);
Boolean KeyboardPanel_HandleEvent(EventRecord *event);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_PANELS_KEYBOARD_H */
