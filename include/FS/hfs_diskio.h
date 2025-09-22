/* HFS Disk I/O Layer */
#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Block device abstraction */
typedef struct {
    void*    data;          /* Memory-mapped image or file handle */
    uint64_t size;          /* Total size in bytes */
    uint32_t sectorSize;    /* Sector size (typically 512) */
    bool     readonly;      /* Read-only flag */
} HFS_BlockDev;

/* Initialize block device from memory buffer (for RAM disk) */
bool HFS_BD_InitMemory(HFS_BlockDev* bd, void* buffer, uint64_t size);

/* Initialize block device from file path */
bool HFS_BD_InitFile(HFS_BlockDev* bd, const char* path, bool readonly);

/* Read from block device */
bool HFS_BD_Read(HFS_BlockDev* bd, uint64_t offset, void* buffer, uint32_t length);

/* Write to block device (if not readonly) */
bool HFS_BD_Write(HFS_BlockDev* bd, uint64_t offset, const void* buffer, uint32_t length);

/* Close block device */
void HFS_BD_Close(HFS_BlockDev* bd);

/* Read a sector */
bool HFS_BD_ReadSector(HFS_BlockDev* bd, uint32_t sector, void* buffer);