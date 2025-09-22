/* Virtual File System Interface */
#pragma once
#include "hfs_types.h"
#include <stdbool.h>

/* File handle for VFS operations */
typedef struct VFSFile VFSFile;

/* Initialize the VFS */
bool VFS_Init(void);

/* Shutdown the VFS */
void VFS_Shutdown(void);

/* Mount the boot volume */
bool VFS_MountBootVolume(const char* volName);

/* Get volume info */
bool VFS_GetVolumeInfo(VRefNum vref, VolumeControlBlock* vcb);

/* Get boot volume reference */
VRefNum VFS_GetBootVRef(void);

/* Directory operations */
bool VFS_Enumerate(VRefNum vref, DirID dir, CatEntry* entries, int maxEntries, int* count);
bool VFS_Lookup(VRefNum vref, DirID dir, const char* name, CatEntry* entry);
bool VFS_GetByID(VRefNum vref, FileID id, CatEntry* entry);

/* File operations */
VFSFile* VFS_OpenFile(VRefNum vref, FileID id, bool resourceFork);
VFSFile* VFS_OpenByPath(VRefNum vref, const char* path, bool resourceFork);
void VFS_CloseFile(VFSFile* file);
bool VFS_ReadFile(VFSFile* file, void* buffer, uint32_t length, uint32_t* bytesRead);
bool VFS_SeekFile(VFSFile* file, uint32_t position);
uint32_t VFS_GetFileSize(VFSFile* file);
uint32_t VFS_GetFilePosition(VFSFile* file);

/* Future write operations (stubs for now) */
bool VFS_CreateFolder(VRefNum vref, DirID parent, const char* name, DirID* newID);
bool VFS_CreateFile(VRefNum vref, DirID parent, const char* name,
                   uint32_t type, uint32_t creator, FileID* newID);
bool VFS_Rename(VRefNum vref, FileID id, const char* newName);
bool VFS_Delete(VRefNum vref, FileID id);