/*
 * PS2Controller.h - PS/2 Keyboard and Mouse Driver Interface
 *
 * Provides input support for QEMU's emulated PS/2 devices
 */

#ifndef PS2_CONTROLLER_H
#define PS2_CONTROLLER_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the PS/2 controller and devices */
Boolean InitPS2Controller(void);

/* Poll for PS/2 input (call this regularly) */
void PollPS2Input(void);

/* Get current mouse position */
void GetMouse(Point* mouseLoc);

/* Check if mouse button is pressed */
Boolean Button(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_CONTROLLER_H */