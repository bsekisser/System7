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

/* Decode native Mac ppat format */
static bool DecodeNativePPAT(const uint8_t* data, size_t size, uint32_t outRGBA[64]) {
    extern void serial_puts(const char* str);
    serial_puts("DecodeNativePPAT: called\n");

    if (size < 0x50) {
        serial_puts("DecodeNativePPAT: too small\n");
        return false;
    }

    /* Check ppat version (0x0001) */
    uint16_t version = ReadBE16(data);
    if (version != 0x0001) {
        serial_puts("DecodeNativePPAT: wrong version\n");
        return false;
    }

    /* Get pixel depth from offset 0x40 */
    uint16_t pixelDepth = ReadBE16(data + 0x40);

    /* For now, handle 2-bit patterns (4 colors) which is common */
    if (pixelDepth == 2) {
        /* Pattern data starts at 0x4C for 8x8 patterns */
        const uint8_t* patData = data + 0x4C;

        /* Color table typically starts after pattern data */
        /* For 2-bit, we have 16 bytes of pattern data (8x8 pixels, 2 bits each) */
        const uint8_t* colorTable = data + 0x5C;

        /* Parse color table - format is typically:
           - seed (4 bytes)
           - flags (2 bytes)
           - size (2 bytes)
           - then RGB entries */
        if (size < 0x86) return false;  /* Need space for 4 colors */

        /* Skip to RGB entries (around offset 0x64) */
        const uint8_t* colors = data + 0x64;

        /* Decode pattern - 2 bits per pixel */
        for (int y = 0; y < 8; y++) {
            uint16_t row = ReadBE16(patData + y * 2);
            for (int x = 0; x < 8; x++) {
                /* Extract 2-bit color index */
                int shift = (7 - x) * 2;
                int colorIdx = (row >> shift) & 0x03;

                /* Get RGB from color table (6 bytes per entry: index(2) + RGB(6)) */
                const uint8_t* color = colors + colorIdx * 8 + 2;
                uint8_t r = color[0];
                uint8_t g = color[2];
                uint8_t b = color[4];

                outRGBA[y * 8 + x] = pack_color(r, g, b);
            }
        }
        return true;
    }

    return false;
}

/* Decode PPAT8 format into RGBA pixels */
bool DecodePPAT8(const uint8_t* p, size_t n, uint32_t outRGBA[64]) {
    /* First try native ppat format */
    if (DecodeNativePPAT(p, n, outRGBA)) {
        return true;
    }

    /* Then try our custom PPAT8 format */
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