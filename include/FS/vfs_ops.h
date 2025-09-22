#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t VRefNum;
typedef uint32_t DirID;
typedef uint32_t FileID;

/* Catalog entry structure */
typedef struct {
    FileID id;
    char name[32];
    uint16_t flags;
    bool isFolder;
} CatEntry;

/* VFS Operations for Trash support */
bool VFS_EnsureHiddenFolder(VRefNum vref, const char* name, DirID* outDir);
bool VFS_Move(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName);
bool VFS_DeleteTree(VRefNum vref, DirID parent, FileID id);
bool VFS_Enumerate(VRefNum vref, DirID dir, CatEntry* out, int max, int* outCount);
bool VFS_GetDirItemCount(VRefNum vref, DirID dir, uint32_t* outCount, bool recursive);
bool VFS_IsOpenByAnyProcess(FileID id);
bool VFS_IsLocked(FileID id);
bool VFS_SetFinderFlags(FileID id, uint16_t setMask, uint16_t clearMask);
bool VFS_GenerateUniqueName(VRefNum vref, DirID dir, const char* base, char* out);
bool VFS_Exists(VRefNum vref, DirID dir, const char* name);
const char* VFS_GetNameByID(VRefNum vref, DirID parent, FileID id);

/* Finder flags */
#define kFinderFlagHidden   0x0001
#define kFinderFlagSystem   0x0002
#define kFinderFlagLocked   0x0004