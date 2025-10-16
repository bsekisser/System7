/*
 * Hardware Detection Interface
 * Detects Raspberry Pi model using multiple methods
 */

#ifndef ARM_HARDWARE_DETECT_H
#define ARM_HARDWARE_DETECT_H

#include "usb_controller.h"

/* Detect hardware model
 * Returns pi_model_t and optionally fills model_string
 * Call once - subsequent calls return cached results
 */
rpi_model_t hardware_detect_model(char *model_string, uint32_t string_len);

/* Get cached model (no detection, just returns cached value) */
rpi_model_t hardware_get_model(void);

/* Get cached model string */
const char* hardware_get_model_string(void);

/* Print detailed hardware information */
void hardware_report_info(void);

#endif /* ARM_HARDWARE_DETECT_H */
