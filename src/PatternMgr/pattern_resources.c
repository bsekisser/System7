/*
 * RE-AGENT-BANNER
 * Pattern Resources Implementation
 * System 7.1 Pattern Resource Loading
 *
 * Loads 'PAT ' and 'ppat' resources from resource files.
 * Classic 'PAT ' data is exactly 8 bytes (one row per byte, bit7 leftmost).
 * PixPat resources are more complex and stored as opaque handles.
 */

#include "PatternMgr/pattern_resources.h"
#include "ResourceMgr/resource_manager.h"
#include <string.h>
#include <stdlib.h>

/*
 * Helper to extract 8 bytes from resource data
 * Some resource forks wrap the pattern data; we look for an 8‑byte block.
 */
static bool find8(const uint8_t *p, int32_t n, const uint8_t **out) {
    if (n < 8) return false;
    /* Fast path: if size is 8 or more, use first 8 bytes */
    if (n >= 8) {
        *out = p;
        return true;
    }
    return false;
}

/* External function from pattern_data.c */
extern const uint8_t* GetBuiltInPatternData(int16_t patternID);

bool LoadPATResource(int16_t id, Pattern *outPat) {
    if (!outPat) return false;

    /* First try to get from resource manager */
    Handle h = GetResource(kPatternResourceType, id);
    if (!h) {
        /* Fall back to built-in patterns */
        const uint8_t* builtInData = GetBuiltInPatternData(id);
        if (builtInData) {
            memset(outPat, 0, sizeof(*outPat));
            memcpy(outPat->pat, builtInData, 8);
            return true;
        }
        return false;
    }

    HLock(h);
    const uint8_t *p = (const uint8_t *)*h;
    Size sz = GetHandleSize(h);

    const uint8_t *block = NULL;
    if (sz == 8) {
        /* Perfect match - pattern is exactly 8 bytes */
        block = p;
    } else if (sz >= 8) {
        /* Use first 8 bytes */
        find8(p, sz, &block);
    }

    if (!block) {
        HUnlock(h);
        ReleaseResource(h);
        return false;
    }

    memset(outPat, 0, sizeof(*outPat));
    memcpy(outPat->pat, block, 8);

    HUnlock(h);
    ReleaseResource(h);
    return true;
}

Handle LoadPPATResource(int16_t id) {
    Handle h = GetResource(kPixPatternResourceType, id);
    if (!h) return NULL;

    /* Return a duplicate the caller owns; leave original in the resource map */
    Size sz = GetHandleSize(h);
    Handle dup = NewHandle(sz);
    if (!dup) {
        ReleaseResource(h);
        return NULL;
    }

    HLock(h);
    HLock(dup);
    memcpy(*dup, *h, sz);
    HUnlock(dup);
    HUnlock(h);
    ReleaseResource(h);

    return dup;
}