/*
 * OSUtilsTraps.h - Operating System Utilities Trap Declarations
 *
 * Public API for OS utility trap implementations and tick management.
 */

#ifndef OSUTILS_TRAPS_H
#define OSUTILS_TRAPS_H

#include "SystemTypes.h"
#include "CPU/CPUBackend.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Trap Numbers
 */
#define TRAP_TickCount  0xA975  /* Get current tick count */

/*
 * OSUtils_InitializeTraps - Register OS utility traps
 *
 * @param backend CPU backend interface
 * @param as CPU address space
 * @return noErr on success, error code on failure
 */
OSErr OSUtils_InitializeTraps(ICPUBackend* backend, CPUAddressSpace as);

/*
 * OSUtils_IncrementTicks - Increment system tick count
 *
 * Should be called from timer ISR at ~60Hz.
 * Updates low memory global and provides rate-limited logging.
 */
void OSUtils_IncrementTicks(void);

/*
 * Trap Handlers (for CPU backend)
 */
OSErr Trap_TickCount(void* context, CPUAddr* pc, CPUAddr* registers);

#ifdef __cplusplus
}
#endif

#endif /* OSUTILS_TRAPS_H */
