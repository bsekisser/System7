#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "FS/vfs.h"  /* Include for existing types: VRefNum, DirID, FileID, CatEntry */

/* VFS Operations for Trash support - use existing VFS types */
bool VFS_GetDirItemCount(VRefNum vref, DirID dir, uint32_t* outCount, bool recursive);
bool VFS_IsOpenByAnyProcess(FileID id);
bool VFS_IsLocked(FileID id);
bool VFS_SetFinderFlags(FileID id, uint16_t setMask, uint16_t clearMask);
bool VFS_GenerateUniqueName(VRefNum vref, DirID dir, const char* base, char* out);
bool VFS_Exists(VRefNum vref, DirID dir, const char* name);
const char* VFS_GetNameByID(VRefNum vref, DirID parent, FileID id);

/* File operations */
bool VFS_Move(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName);
bool VFS_Copy(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName, FileID* newID);
bool VFS_DeleteTree(VRefNum vref, DirID parent, FileID id);
bool VFS_EnsureHiddenFolder(VRefNum vref, const char* name, DirID* outDir);

/* Volume operations */
VRefNum VFS_GetVRefByID(FileID id);
bool VFS_GetParentDir(VRefNum vref, FileID id, DirID* parentDir);

/* Finder flags */
#define kFinderFlagHidden   0x0001
#define kFinderFlagSystem   0x0002
#define kFinderFlagLocked   0x0004