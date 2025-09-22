/* HFS Disk I/O Implementation */
#include "../../include/FS/hfs_diskio.h"
#include "../../include/MemoryMgr/MemoryManager.h"
#include <string.h>

/* For now, we'll use memory-based implementation */
/* Later this can be extended to use real file I/O */

bool HFS_BD_InitMemory(HFS_BlockDev* bd, void* buffer, uint64_t size) {
    if (!bd || !buffer || size == 0) return false;

    bd->data = buffer;
    bd->size = size;
    bd->sectorSize = 512;
    bd->readonly = false;
    return true;
}

bool HFS_BD_InitFile(HFS_BlockDev* bd, const char* path, bool readonly) {
    /* For kernel implementation, we'll allocate from heap */
    /* In a real implementation, this would open a file */
    if (!bd) return false;

    /* Allocate a 4MB disk image from heap */
    bd->size = 4 * 1024 * 1024;
    bd->data = malloc(bd->size);
    if (!bd->data) return false;

    memset(bd->data, 0, bd->size);
    bd->sectorSize = 512;
    bd->readonly = readonly;

    /* TODO: Actually load from path if it exists */
    return true;
}

bool HFS_BD_Read(HFS_BlockDev* bd, uint64_t offset, void* buffer, uint32_t length) {
    if (!bd || !bd->data || !buffer) return false;
    if (offset + length > bd->size) return false;

    memcpy(buffer, (uint8_t*)bd->data + offset, length);
    return true;
}

bool HFS_BD_Write(HFS_BlockDev* bd, uint64_t offset, const void* buffer, uint32_t length) {
    if (!bd || !bd->data || !buffer) return false;
    if (bd->readonly) return false;
    if (offset + length > bd->size) return false;

    memcpy((uint8_t*)bd->data + offset, buffer, length);
    return true;
}

void HFS_BD_Close(HFS_BlockDev* bd) {
    if (!bd) return;

    /* If we allocated memory, free it */
    if (bd->data) {
        free(bd->data);
        bd->data = NULL;
    }
    bd->size = 0;
}

bool HFS_BD_ReadSector(HFS_BlockDev* bd, uint32_t sector, void* buffer) {
    if (!bd || !buffer) return false;

    uint64_t offset = (uint64_t)sector * bd->sectorSize;
    return HFS_BD_Read(bd, offset, buffer, bd->sectorSize);
}