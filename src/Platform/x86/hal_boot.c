/*
 * hal_boot.c - x86 Boot HAL Implementation
 *
 * Platform-specific boot initialization for x86.
 */

#include "Platform/include/boot.h"

/*
 * hal_boot_init - Initialize platform-specific boot components
 *
 * For x86, most initialization is handled in kernel_main.
 * This function can be extended to handle x86-specific early init.
 */
void hal_boot_init(void) {
    /* No early x86-specific initialization needed yet */
    /* Serial port and other hardware init happens in kernel_main */
}
