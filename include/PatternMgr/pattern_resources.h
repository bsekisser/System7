/*
 * RE-AGENT-BANNER
 * Pattern Resources Header
 * System 7.1 Pattern Resource Loading
 *
 * Loads 'PAT ' and 'ppat' resources from resource files.
 * Mirrors the pattern resource loading in classic Mac OS.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "SystemTypes.h"
#include "ResourceMgr/resource_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kPatternResourceType  'PAT '
#define kPixPatternResourceType 'ppat'

/* Load an 8×8 1‑bit classic Pattern (8 bytes), mapped into QuickDraw Pattern */
bool LoadPATResource(int16_t id, Pattern *outPat);

/* Load a PixPat blob as a Handle (opaque to callers, interpreted by QuickDraw) */
Handle LoadPPATResource(int16_t id);

#ifdef __cplusplus
}
#endif