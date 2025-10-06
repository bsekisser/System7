#include "OSUtils/OSUtilsTraps.h"

#include "SegmentLoader/SegmentLoader.h"
#include "CPU/M68KInterp.h"
#include "CPU/LowMemGlobals.h"
#include "TimeManager/TimeManager.h"
#include "System71StdLib.h"

#define TICK_PERIOD_US 16667U  /* Roughly 60 Hz */

static struct TMTask gTickTask;
static Boolean gTickTaskInstalled = false;
static UInt32 gTickLogCounter = 0;

static void TickCount_Task(TMTask* task);
static OSErr TickCount_Trap(void* context, CPUAddr* pc, CPUAddr* registers);

static void TickCount_Task(TMTask* task)
{
    (void)task;

    UInt32 ticks = LMGetTicks();
    LMSetTicks(ticks + 1);

    if (PrimeTime(&gTickTask, TICK_PERIOD_US) != noErr) {
        if ((gTickLogCounter++ % 120) == 0) {
            serial_puts("[OSUtils] WARNING: PrimeTime failed for tick task\n");
        }
    }
}

static OSErr TickCount_Trap(void* context, CPUAddr* pc, CPUAddr* registers)
{
    (void)context;
    (void)pc;

    if (!registers) {
        return paramErr;
    }

    registers[0] = LMGetTicks();
    return noErr;
}

OSErr OSUtils_InstallTraps(SegmentLoaderContext* ctx)
{
    OSErr err;

    if (!ctx || !ctx->cpuBackend || !ctx->cpuAS) {
        return paramErr;
    }

    err = ctx->cpuBackend->InstallTrap(ctx->cpuAS, 0xA975, TickCount_Trap, NULL);
    if (err != noErr) {
        serial_printf("[OSUtils] InstallTrap(_TickCount) failed: %d\n", err);
        return err;
    }

    if (!gTickTaskInstalled) {
        memset(&gTickTask, 0, sizeof(gTickTask));
        gTickTask.tmAddr = (Ptr)TickCount_Task;
        gTickTask.qType = TM_FLAG_PERIODIC;

        err = InsTime(&gTickTask);
        if (err != noErr) {
            serial_printf("[OSUtils] InsTime failed for tick task: %d\n", err);
            return err;
        }

        err = PrimeTime(&gTickTask, TICK_PERIOD_US);
        if (err != noErr) {
            serial_printf("[OSUtils] PrimeTime failed for tick task: %d\n", err);
            RmvTime(&gTickTask);
            memset(&gTickTask, 0, sizeof(gTickTask));
            return err;
        }

        gTickTaskInstalled = true;
        gTickLogCounter = 0;
        serial_puts("[OSUtils] TickCount timer armed at ~60Hz\n");
    }

    return noErr;
}

void OSUtils_Shutdown(void)
{
    if (gTickTaskInstalled) {
        CancelTime(&gTickTask);
        RmvTime(&gTickTask);
        memset(&gTickTask, 0, sizeof(gTickTask));
        gTickTaskInstalled = false;
        serial_puts("[OSUtils] TickCount timer stopped\n");
    }
}
