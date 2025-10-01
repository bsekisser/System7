#ifndef TIMEMANAGER_PRIV_H
#define TIMEMANAGER_PRIV_H

#include "SystemTypes.h"
#include "TimeManager/TimeManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Core exposes this to ISR */
void Core_ExpireDue(UInt64 nowUS);

/* ISR exposes this to Core */
void ProgramNextTimerInterrupt(UInt64 absDeadlineUS);

#ifdef __cplusplus
}
#endif
#endif /* TIMEMANAGER_PRIV_H */
