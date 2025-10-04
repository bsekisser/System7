/*
 * Scrap.h - Classic Mac OS Scrap Manager
 *
 * System 7.1-compatible clipboard management with multiple flavors
 * Implements the classic 68k Scrap Manager API
 */

#ifndef SCRAP_H
#define SCRAP_H

#include "SystemTypes.h"

/* Common flavor types */
#define kScrapFlavorTypeText     'TEXT'  /* Plain text */
#define kScrapFlavorTypeStyle    'styl'  /* Text style runs */
#define kScrapFlavorTypePicture  'PICT'  /* Picture data */

/* Core Scrap Manager API (System 7 authentic) */

/*
 * ZeroScrap - Clear all clipboard flavors
 * Clears all flavors, increments change count, marks dirty
 */
extern void ZeroScrap(void);

/*
 * GetScrap - Get clipboard data for specified flavor
 * If flavor exists, appends its data to hDest starting at *offset
 * Returns byte count copied; returns 0 if flavor not found
 * On success, increments *offset by bytes copied
 */
extern long GetScrap(Handle hDest, OSType theType, long* offset);

/*
 * PutScrap - Put data on clipboard with specified flavor
 * Replaces or creates the flavor; copies byteCount bytes from sourcePtr
 * Increments change count, marks dirty
 */
extern void PutScrap(long byteCount, OSType theType, const void* sourcePtr);

/*
 * LoadScrap - Load clipboard from disk
 * Loads from on-disk clipboard file if present and newer than in-memory version
 */
extern void LoadScrap(void);

/*
 * UnloadScrap - Save clipboard to disk
 * Writes the current in-memory scrap to disk if dirty
 */
extern void UnloadScrap(void);

/*
 * InfoScrap - Get clipboard change count
 * Returns change count to let callers detect updates
 */
extern long InfoScrap(void);

/* Helper functions (optional but useful) */

/*
 * ScrapHasFlavor - Check if clipboard has specified flavor
 * Returns true if the flavor exists, false otherwise
 */
extern Boolean ScrapHasFlavor(OSType theType);

/*
 * ScrapGetFlavorSize - Get size of specified flavor
 * Returns size in bytes, or 0 if flavor not found
 */
extern long ScrapGetFlavorSize(OSType theType);

/*
 * GetScrapChangeCount - Alias for InfoScrap
 * Modern name for the same functionality
 */
#define GetScrapChangeCount() InfoScrap()

#endif /* SCRAP_H */