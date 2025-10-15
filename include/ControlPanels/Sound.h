/**
 * @file Sound.h
 * @brief Lightweight Sound control panel window.
 */

#ifndef CONTROL_PANELS_SOUND_H
#define CONTROL_PANELS_SOUND_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void SoundPanel_Open(void);
void SoundPanel_Close(void);
Boolean SoundPanel_IsOpen(void);
WindowPtr SoundPanel_GetWindow(void);
Boolean SoundPanel_HandleEvent(EventRecord *event);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_PANELS_SOUND_H */
