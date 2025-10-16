/**
 * @file ControlStrip.h
 * @brief Minimal control strip launcher palette.
 */

#ifndef CONTROL_PANELS_CONTROL_STRIP_H
#define CONTROL_PANELS_CONTROL_STRIP_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void ControlStrip_Show(void);
void ControlStrip_Hide(void);
void ControlStrip_Toggle(void);
Boolean ControlStrip_HandleEvent(EventRecord *event);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_PANELS_CONTROL_STRIP_H */
