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

/* For pack_color */
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

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

/* Parse big-endian 16-bit value */
static uint16_t ReadBE16(const uint8_t* p) {
    return (p[0] << 8) | p[1];
}

/* Decode PPAT8 format into RGBA pixels */
bool DecodePPAT8(const uint8_t* p, size_t n, uint32_t outRGBA[64]) {
    if (n < 6+2+2+2) return false;
    if (memcmp(p, "PPAT8\0", 6) != 0) return false;

    const uint8_t* q = p + 6;
    uint16_t w = ReadBE16(q); q += 2;
    uint16_t h = ReadBE16(q); q += 2;
    uint16_t N = ReadBE16(q); q += 2;

    if (w != 8 || h != 8 || N == 0 || N > 256) return false;
    if ((size_t)(q - p) + 4*N + 64 > n) return false;

    /* Read palette */
    const uint8_t* pal = q;
    q += 4*N;
    const uint8_t* idx = q;

    /* Convert to RGBA pixels */
    for (int i = 0; i < 64; i++) {
        uint8_t k = idx[i];
        if (k >= N) return false;
        const uint8_t* c = pal + 4*k;
        /* Store as ARGB for compatibility with pack_color */
        outRGBA[i] = pack_color(c[0], c[1], c[2]);
    }

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