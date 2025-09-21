/*
 * RE-AGENT-BANNER
 * PRAM Desktop Preferences Header
 * System 7.1 Desktop Pattern Persistence
 *
 * Provides PRAM-style persistent storage for desktop patterns.
 * In System 7, this would use Parameter RAM; we use a simple file.
 */

#pragma once

#include <stdbool.h>
#include "PatternMgr/pattern_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Load desktop preference from persistent storage */
bool PRAM_LoadDesktopPref(DesktopPref *out);

/* Save desktop preference to persistent storage */
bool PRAM_SaveDesktopPref(const DesktopPref *in);

#ifdef __cplusplus
}
#endif