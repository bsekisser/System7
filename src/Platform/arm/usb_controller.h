/*
 * USB Controller Abstraction Layer
 * Provides unified interface for both XHCI (Pi 4/5) and DWCOTG (Pi 3)
 */

#ifndef ARM_USB_CONTROLLER_H
#define ARM_USB_CONTROLLER_H

#include <stdint.h>

/* ===== Raspberry Pi Model Detection ===== */
typedef enum {
    PI_MODEL_UNKNOWN = 0,
    PI_MODEL_3       = 3,
    PI_MODEL_4       = 4,
    PI_MODEL_5       = 5,
} rpi_model_t;

/* ===== Public API ===== */

/* Detect Raspberry Pi model */
rpi_model_t usb_detect_rpi_model(void);

/* Initialize USB controller (auto-selects XHCI or DWCOTG) */
int usb_controller_init(void);

/* Enumerate USB devices */
int usb_controller_enumerate(void);

/* Find HID keyboard */
int usb_find_keyboard(void *kb_info);

/* Find HID mouse */
int usb_find_mouse(void *mouse_info);

/* Poll keyboard */
int usb_poll_keyboard(uint8_t *key_code, uint8_t *modifiers);

/* Poll mouse */
int usb_poll_mouse(int8_t *dx, int8_t *dy, uint8_t *buttons);

/* Get device count */
uint32_t usb_device_count(void);

/* Shutdown USB controller */
void usb_controller_shutdown(void);

#endif /* ARM_USB_CONTROLLER_H */
