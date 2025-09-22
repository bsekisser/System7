/* HFS File Operations Implementation */
#include "../../include/FS/hfs_file.h"
#include "../../include/FS/hfs_endian.h"
#include <stdlib.h>
#include <string.h>

/* Serial debug output */
extern void serial_printf(const char* fmt, ...);

/* Find file record in catalog by ID */
static bool find_file_record(HFS_Catalog* cat, FileID id, HFS_CatFileRec* fileRec) {
    /* This is a simplified version - in reality we'd search the B-tree directly */

    /* Build a thread record key to find the file */
    /* For now, do a linear search through catalog */

    typedef struct {
        FileID target;
        HFS_CatFileRec* result;
        bool found;
    } FindContext;

    FindContext ctx = { .target = id, .result = fileRec, .found = false };

    /* Callback for B-tree iteration */
    bool find_callback(void* keyPtr, uint16_t keyLen,
                      void* dataPtr, uint16_t dataLen,
                      void* context) {
        FindContext* ctx = (FindContext*)context;
        uint16_t recordType = be16_read(dataPtr);

        if (recordType == kHFS_FileRecord) {
            HFS_CatFileRec* rec = (HFS_CatFileRec*)dataPtr;
            uint32_t fileID = be32_read(&rec->fileID);

            if (fileID == ctx->target) {
                memcpy(ctx->result, rec, sizeof(HFS_CatFileRec));
                ctx->found = true;
                return false;  /* Stop iteration */
            }
        }

        return true;  /* Continue */
    }

    HFS_BT_IterateLeaves(&cat->bt, find_callback, &ctx);
    return ctx.found;
}

/* Read data from file extents */
static bool read_from_extents(HFS_Volume* vol, const HFS_Extent* extents,
                             uint32_t fileSize, uint32_t offset,
                             void* buffer, uint32_t length,
                             uint32_t* bytesRead) {
    if (!vol || !extents || !buffer || !bytesRead) return false;

    *bytesRead = 0;

    /* Limit read to file size */
    if (offset >= fileSize) return true;  /* EOF */
    if (offset + length > fileSize) {
        length = fileSize - offset;
    }

    uint32_t currentOffset = 0;
    uint32_t remaining = length;
    uint8_t* dst = (uint8_t*)buffer;

    /* Read from first 3 extents */
    for (int i = 0; i < 3 && remaining > 0; i++) {
        if (extents[i].blockCount == 0) break;

        uint32_t extentBytes = extents[i].blockCount * vol->alBlkSize;

        /* Skip this extent if we're past it */
        if (offset >= currentOffset + extentBytes) {
            currentOffset += extentBytes;
            continue;
        }

        /* Calculate read position within this extent */
        uint32_t extentOffset = offset - currentOffset;
        uint32_t toRead = extentBytes - extentOffset;
        if (toRead > remaining) toRead = remaining;

        /* Calculate allocation block and offset within it */
        uint32_t startAllocBlock = extents[i].startBlock + (extentOffset / vol->alBlkSize);
        uint32_t allocOffset = extentOffset % vol->alBlkSize;
        uint32_t blocksToRead = (toRead + allocOffset + vol->alBlkSize - 1) / vol->alBlkSize;

        /* Allocate temporary buffer for block-aligned read */
        uint8_t* tempBuf = malloc(blocksToRead * vol->alBlkSize);
        if (!tempBuf) return false;

        /* Read the allocation blocks */
        if (!HFS_ReadAllocBlocks(vol, startAllocBlock, blocksToRead, tempBuf)) {
            free(tempBuf);
            return false;
        }

        /* Copy requested portion to destination */
        memcpy(dst, tempBuf + allocOffset, toRead);
        free(tempBuf);

        dst += toRead;
        offset += toRead;
        remaining -= toRead;
        *bytesRead += toRead;
        currentOffset += extentBytes;
    }

    /* TODO: Handle overflow extents for large files */
    if (remaining > 0 && *bytesRead == 0) {
        serial_printf("HFS File: Need overflow extents (not implemented)\n");
    }

    return true;
}

HFSFile* HFS_FileOpen(HFS_Catalog* cat, FileID id, bool resourceFork) {
    if (!cat || id < HFS_FIRST_CNID) return NULL;

    /* Find the file record */
    HFS_CatFileRec fileRec;
    if (!find_file_record(cat, id, &fileRec)) {
        serial_printf("HFS File: File ID %u not found\n", id);
        return NULL;
    }

    /* Allocate file handle */
    HFSFile* file = (HFSFile*)malloc(sizeof(HFSFile));
    if (!file) return NULL;

    memset(file, 0, sizeof(HFSFile));
    file->vol = cat->vol;
    file->id = id;
    file->isResource = resourceFork;

    /* Copy file info */
    file->dataSize = be32_read(&fileRec.dataLogicalSize);
    file->rsrcSize = be32_read(&fileRec.rsrcLogicalSize);

    /* Copy extents (converting from big-endian) */
    for (int i = 0; i < 3; i++) {
        file->dataExtents[i].startBlock = be16_read(&fileRec.dataExtents[i].startBlock);
        file->dataExtents[i].blockCount = be16_read(&fileRec.dataExtents[i].blockCount);
        file->rsrcExtents[i].startBlock = be16_read(&fileRec.rsrcExtents[i].startBlock);
        file->rsrcExtents[i].blockCount = be16_read(&fileRec.rsrcExtents[i].blockCount);
    }

    file->position = 0;

    serial_printf("HFS File: Opened file ID %u (%s fork, %u bytes)\n",
                  id, resourceFork ? "rsrc" : "data",
                  resourceFork ? file->rsrcSize : file->dataSize);

    return file;
}

HFSFile* HFS_FileOpenByPath(HFS_Catalog* cat, const char* path, bool resourceFork) {
    if (!cat || !path) return NULL;

    /* TODO: Implement path parsing without strtok */
    /* For now, just return NULL - path-based opening not critical for initial testing */
    serial_printf("HFS File: Path-based opening not yet implemented\n");
    return NULL;
}

void HFS_FileClose(HFSFile* file) {
    if (file) {
        free(file);
    }
}

bool HFS_FileRead(HFSFile* file, void* buffer, uint32_t length, uint32_t* bytesRead) {
    if (!file || !buffer || !bytesRead) return false;

    const HFS_Extent* extents = file->isResource ? file->rsrcExtents : file->dataExtents;
    uint32_t fileSize = file->isResource ? file->rsrcSize : file->dataSize;

    bool result = read_from_extents(file->vol, extents, fileSize,
                                   file->position, buffer, length, bytesRead);

    if (result) {
        file->position += *bytesRead;
    }

    return result;
}

bool HFS_FileSeek(HFSFile* file, uint32_t position) {
    if (!file) return false;

    uint32_t fileSize = file->isResource ? file->rsrcSize : file->dataSize;
    if (position > fileSize) position = fileSize;

    file->position = position;
    return true;
}

uint32_t HFS_FileGetSize(HFSFile* file) {
    if (!file) return 0;
    return file->isResource ? file->rsrcSize : file->dataSize;
}

uint32_t HFS_FileTell(HFSFile* file) {
    if (!file) return 0;
    return file->position;
}