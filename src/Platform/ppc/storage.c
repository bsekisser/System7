/*
 * storage.c - PowerPC storage HAL scaffolding
 *
 * These routines will eventually wrap the chosen PowerPC storage controller.
 * They currently return unimplemented errors so higher-level code can detect
 * missing functionality.
 */

#include <string.h>

#include "Platform/include/storage.h"
#include "Errors/ErrorCodes.h"

#ifndef unimpErr
#define unimpErr (-4)
#endif

OSErr hal_storage_init(void) {
    return unimpErr;
}

OSErr hal_storage_shutdown(void) {
    return unimpErr;
}

int hal_storage_get_drive_count(void) {
    return 0;
}

OSErr hal_storage_get_drive_info(int drive_index, hal_storage_info_t* info) {
    (void)drive_index;
    if (info) {
        info->block_size = 0;
        info->block_count = 0;
    }
    return unimpErr;
}

OSErr hal_storage_read_blocks(int drive_index, uint64_t start_block, uint32_t block_count, void* buffer) {
    (void)drive_index;
    (void)start_block;
    (void)block_count;
    (void)buffer;
    return unimpErr;
}

OSErr hal_storage_write_blocks(int drive_index, uint64_t start_block, uint32_t block_count, const void* buffer) {
    (void)drive_index;
    (void)start_block;
    (void)block_count;
    (void)buffer;
    return unimpErr;
}
