/*
 * GestaltPriv.h - Gestalt Manager private structures
 * Based on Inside Macintosh: Operating System Utilities
 * Clean-room implementation for freestanding System 7.1
 */

#ifndef GESTALT_PRIV_H
#define GESTALT_PRIV_H

#include "SystemTypes.h"
#include "Gestalt/Gestalt.h"

/* Maximum number of Gestalt entries */
#define GESTALT_MAX_ENTRIES 128

/* Gestalt table entry */
typedef struct {
    OSType      selector;
    GestaltProc proc;
    Boolean     inUse;
} GestaltEntry;

/* Internal functions */
extern void Gestalt_Register_Builtins(void);
extern void Gestalt_SetInitBit(int bit);
/* Init bits for tracking subsystem initialization */
enum {
    kGestaltInitBit_MemoryMgr  = 0,
    kGestaltInitBit_TimeMgr    = 1,
    kGestaltInitBit_ResourceMgr = 2,
    kGestaltInitBit_EventMgr    = 3,
    kGestaltInitBit_WindowMgr   = 4,
    kGestaltInitBit_MenuMgr     = 5,
    kGestaltInitBit_ControlMgr  = 6,
    kGestaltInitBit_DialogMgr   = 7,
    kGestaltInitBit_FileMgr     = 8,
    kGestaltInitBit_ProcessMgr  = 9,
    kGestaltInitBit_Finder      = 10
};

#endif /* GESTALT_PRIV_H */
