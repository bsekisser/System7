#ifndef OSUTILS_TRAPS_H
#define OSUTILS_TRAPS_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SegmentLoaderContext SegmentLoaderContext;

/* Install OSUtils-related traps (e.g., _TickCount) for the provided loader */
OSErr OSUtils_InstallTraps(SegmentLoaderContext* ctx);

/* Shutdown any OSUtils background services */
void OSUtils_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* OSUTILS_TRAPS_H */
