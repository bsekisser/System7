#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t VRefNum;
typedef uint32_t DirID;
typedef uint32_t FileID;

typedef struct {
    VRefNum vref;
    DirID   dirTrash;     // ID of the trash folder on this volume
    uint32_t itemCount;   // cached number of items (for "full/empty" icon)
    bool     exists;
} TrashInfo;

bool Trash_Init(void);                         // called at boot
bool Trash_OnVolumeMount(VRefNum vref);        // ensure per-volume Trash exists
bool Trash_OnVolumeUnmount(VRefNum vref);      // cleanup cache

bool Trash_GetDir(VRefNum vref, DirID* outDir);
bool Trash_IsEmptyAll(void);
uint32_t Trash_TotalItems(void);               // sum across volumes

// operations
bool Trash_MoveNode(VRefNum vref, DirID parent, FileID id); // move to that volume's Trash
bool Trash_EmptyAll(bool force);               // permanently deletes on ALL mounted volumes
bool Trash_OpenDesktopWindow(void);            // opens aggregated view (optional)