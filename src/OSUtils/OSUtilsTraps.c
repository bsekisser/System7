/*
 * OSUtilsTraps.c - Operating System Utilities Trap Implementations
 *
 * Implements core OS utility traps including:
 * - _TickCount (0xA975) - Get current system tick count
 * - Tick increment mechanism for 1/60s timing
 */

#include "CPU/CPUBackend.h"
#include "CPU/LowMemGlobals.h"
#include "System71StdLib.h"

/* Tick rate limiting for logging */
static UInt32 g_lastLoggedTick = 0;
#define TICK_LOG_INTERVAL 1024

/*
 * Trap_TickCount - Implements _TickCount trap (0xA975)
 *
 * Returns the current tick count from low memory global.
 * Ticks increment at ~60Hz (1/60 second intervals).
 *
 * Entry: (none)
 * Exit: D0.L = current tick count
 */
OSErr Trap_TickCount(void* context, CPUAddr* pc, CPUAddr* registers)
{
    UInt32 ticks;

    (void)context;  /* Unused */
    (void)pc;       /* Unused */

    /* Read tick count from low memory global */
    ticks = LMGetTicks();

    /* Return in D0 */
    registers[0] = ticks;  /* D0 */

    /* Rate-limited logging */
    if (ticks - g_lastLoggedTick >= TICK_LOG_INTERVAL) {
        serial_printf("[TRAP] _TickCount -> %u (0x%08X)\n", ticks, ticks);
        g_lastLoggedTick = ticks;
    }

    return noErr;
}

/*
 * OSUtils_IncrementTicks - Increment system tick count
 *
 * Should be called from a timer ISR at ~60Hz.
 * Implements rate-limited logging.
 */
void OSUtils_IncrementTicks(void)
{
    UInt32 ticks = LMGetTicks();
    ticks++;
    LMSetTicks(ticks);

    /* Rate-limited logging every 1024 ticks (~17 seconds at 60Hz) */
    if ((ticks & (TICK_LOG_INTERVAL - 1)) == 0) {
        serial_printf("[LM] Tick++ -> %u\n", ticks);
    }
}

/*
 * OSUtils_InitializeTraps - Register OS utility traps
 *
 * Registers trap handlers with the CPU backend.
 */
OSErr OSUtils_InitializeTraps(ICPUBackend* backend, CPUAddressSpace as)
{
    OSErr err;

    /* Register _TickCount trap (0xA975) */
    err = backend->InstallTrap(as, 0xA975, Trap_TickCount, NULL);
    if (err != noErr) {
        serial_printf("[OSUTILS] Failed to register _TickCount trap: %d\n", err);
        return err;
    }

    /* Initialize tick count to 0 */
    LMSetTicks(0);

    serial_printf("[OSUTILS] Traps initialized: _TickCount registered\n");
    return noErr;
}
