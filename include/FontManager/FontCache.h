/*
 * FontCache.h - Font Strike Cache Management
 *
 * Efficient LRU caching for font strikes
 * Optimized for System 7.1 memory constraints
 */

#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include "SystemTypes.h"
#include "FontTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cache configuration */
#define FM_MAX_CACHE_SIZE      (256 * 1024)  /* 256KB max cache */
#define FM_MAX_STRIKES         16            /* Max cached strikes */
#define FM_CACHE_PURGE_RATIO   0.75f        /* Purge when 75% full */

/* Strike cache key */
typedef struct StrikeCacheKey {
    short   fontID;
    short   fontSize;
    Style   face;
    Boolean fractEnable;
} StrikeCacheKey;

/* Cached strike entry */
typedef struct CachedStrike {
    StrikeCacheKey      key;
    struct FontStrike*  strike;
    UInt32             memorySize;
    UInt32             lastUsed;      /* TickCount of last use */
    UInt32             useCount;      /* Number of uses */
    struct CachedStrike* next;
    struct CachedStrike* prev;
} CachedStrike;

/* Cache statistics */
typedef struct CacheStats {
    UInt32  totalMemory;    /* Total memory used */
    UInt32  strikeCount;    /* Number of cached strikes */
    UInt32  hitCount;       /* Cache hits */
    UInt32  missCount;      /* Cache misses */
    float   hitRate;        /* Hit rate percentage */
} CacheStats;

/* Cache initialization */
OSErr FM_InitCache(void);
void FM_ShutdownCache(void);

/* Strike management */
struct FontStrike* FM_GetCachedStrike(short fontID, short size, Style face);
OSErr FM_CacheStrike(short fontID, short size, Style face,
                     struct FontStrike* strike);
void FM_RemoveStrike(short fontID, short size, Style face);

/* Cache operations */
void FM_FlushCache(void);
void FM_PurgeOldStrikes(UInt32 ageThreshold);
void FM_CompactCache(void);

/* Statistics */
void FM_GetCacheStats(CacheStats* stats);
void FM_ResetCacheStats(void);

/* Memory management */
UInt32 FM_GetCacheMemoryUsage(void);
Boolean FM_CanAllocateStrike(UInt32 strikeSize);
void FM_OptimizeCacheMemory(void);

/* Debugging */
void FM_DumpCacheInfo(void);
void FM_ValidateCache(void);

#ifdef __cplusplus
}
#endif

#endif /* FONT_CACHE_H */