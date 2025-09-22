/* HFS File Operations */
#pragma once
#include "hfs_types.h"
#include "hfs_catalog.h"
#include <stdbool.h>

/* File handle structure */
typedef struct HFSFile {
    HFS_Volume*  vol;           /* Volume containing the file */
    FileID       id;            /* File CNID */
    uint32_t     dataSize;      /* Data fork logical size */
    uint32_t     rsrcSize;      /* Resource fork logical size */
    HFS_Extent   dataExtents[3];/* First 3 data fork extents */
    HFS_Extent   rsrcExtents[3];/* First 3 resource fork extents */
    uint32_t     position;      /* Current read position */
    bool         isResource;    /* Reading resource fork? */
} HFSFile;

/* Open a file by CNID */
HFSFile* HFS_FileOpen(HFS_Catalog* cat, FileID id, bool resourceFork);

/* Open a file by path */
HFSFile* HFS_FileOpenByPath(HFS_Catalog* cat, const char* path, bool resourceFork);

/* Close a file */
void HFS_FileClose(HFSFile* file);

/* Read from file */
bool HFS_FileRead(HFSFile* file, void* buffer, uint32_t length, uint32_t* bytesRead);

/* Seek in file */
bool HFS_FileSeek(HFSFile* file, uint32_t position);

/* Get file size */
uint32_t HFS_FileGetSize(HFSFile* file);

/* Get current position */
uint32_t HFS_FileTell(HFSFile* file);