/*
 * STFileIO.c - SimpleText File I/O Operations
 *
 * Handles file reading/writing and file dialogs
 */

#include <string.h>
#include "Apps/SimpleText.h"
#include "MemoryMgr/MemoryManager.h"
#include "FS/vfs.h"

extern void serial_printf(const char* fmt, ...);

/* Read file into document */
Boolean STIO_ReadFile(STDocument* doc, const char* path)
{
    Handle textHandle = NULL;
    SInt32 fileSize;
    char* buffer = NULL;
    Boolean result = false;

    ST_Log("STIO_ReadFile: %s", path);

    if (!doc || !path) {
        return false;
    }

    /* For now, use sample text if VFS not available */
    /* In real implementation, would use VFS_Open/VFS_Read */
    const char* sampleText =
        "Welcome to SimpleText!\n"
        "\n"
        "This is a simple text editor for System 7.1.\n"
        "You can:\n"
        "- Type and edit text\n"
        "- Use standard keyboard shortcuts\n"
        "- Save and open files\n"
        "- Change fonts and styles\n"
        "\n"
        "Enjoy using SimpleText!";

    fileSize = strlen(sampleText);

    /* Allocate handle for text */
    textHandle = NewHandle(fileSize);
    if (!textHandle) {
        ST_Log("STIO_ReadFile: Failed to allocate memory");
        return false;
    }

    /* Copy text to handle */
    HLock(textHandle);
    memcpy(*textHandle, sampleText, fileSize);
    HUnlock(textHandle);

    /* Set text in TextEdit */
    if (doc->hTE) {
        /* Clear existing text */
        TESetSelect(0, 32767, doc->hTE);
        TEDelete(doc->hTE);

        /* Insert new text */
        HLock(textHandle);
        TEInsert(*textHandle, fileSize, doc->hTE);
        HUnlock(textHandle);

        /* Reset selection to start */
        TESetSelect(0, 0, doc->hTE);

        /* Update document info */
        strcpy(doc->filePath, path);

        /* Extract filename from path - manual search for last '/' */
        const char* lastSlash = NULL;
        const char* p = path;
        while (*p) {
            if (*p == '/') {
                lastSlash = p;
            }
            p++;
        }

        if (lastSlash) {
            int nameLen = strlen(lastSlash + 1);
            if (nameLen > 255) nameLen = 255;
            doc->fileName[0] = nameLen;
            memcpy(&doc->fileName[1], lastSlash + 1, nameLen);
        } else {
            int nameLen = strlen(path);
            if (nameLen > 255) nameLen = 255;
            doc->fileName[0] = nameLen;
            memcpy(&doc->fileName[1], path, nameLen);
        }

        doc->untitled = false;
        doc->dirty = false;
        doc->lastSaveLen = fileSize;

        result = true;
    }

    /* Cleanup */
    if (textHandle) {
        DisposeHandle(textHandle);
    }

    return result;
}

/* Write document to file */
Boolean STIO_WriteFile(STDocument* doc, const char* path)
{
    CharsHandle textHandle;
    SInt32 textLen;
    Boolean result = false;

    ST_Log("STIO_WriteFile: %s", path);

    if (!doc || !doc->hTE || !path) {
        return false;
    }

    /* Get text from TextEdit */
    textHandle = TEGetText(doc->hTE);
    if (!textHandle) {
        return false;
    }

    textLen = (*doc->hTE)->teLength;

    /* For now, just log that we would save */
    /* In real implementation, would use VFS_Create/VFS_Write */
    ST_Log("Would save %d bytes to %s", textLen, path);

    /* Update document info */
    strcpy(doc->filePath, path);

    /* Extract filename from path - manual search for last '/' */
    const char* lastSlash = NULL;
    const char* p = path;
    while (*p) {
        if (*p == '/') {
            lastSlash = p;
        }
        p++;
    }

    if (lastSlash) {
        int nameLen = strlen(lastSlash + 1);
        if (nameLen > 255) nameLen = 255;
        doc->fileName[0] = nameLen;
        memcpy(&doc->fileName[1], lastSlash + 1, nameLen);
    } else {
        int nameLen = strlen(path);
        if (nameLen > 255) nameLen = 255;
        doc->fileName[0] = nameLen;
        memcpy(&doc->fileName[1], path, nameLen);
    }

    doc->untitled = false;
    doc->dirty = false;
    doc->lastSaveLen = textLen;

    /* Set file info */
    STIO_SetFileInfo(path, 'TEXT', 'ttxt');

    result = true;

    return result;
}

/* Save dialog - returns path in pathOut */
Boolean STIO_SaveDialog(STDocument* doc, char* pathOut)
{
    ST_Log("STIO_SaveDialog");

    if (!doc || !pathOut) {
        return false;
    }

    /* For now, return a test path */
    if (doc->untitled) {
        strcpy(pathOut, "/Documents/Untitled.txt");
    } else if (doc->filePath[0]) {
        strcpy(pathOut, doc->filePath);
    } else {
        strcpy(pathOut, "/Documents/Document.txt");
    }

    ST_Log("Save dialog would return: %s", pathOut);

    /* In real implementation, would show StandardPutFile dialog */
    return true;
}

/* Open dialog - returns path in pathOut */
Boolean STIO_OpenDialog(char* pathOut)
{
    ST_Log("STIO_OpenDialog");

    if (!pathOut) {
        return false;
    }

    /* For now, return a test path */
    strcpy(pathOut, "/Documents/Sample.txt");

    ST_Log("Open dialog would return: %s", pathOut);

    /* In real implementation, would show StandardGetFile dialog */
    return true;
}

/* Set file type and creator codes */
void STIO_SetFileInfo(const char* path, OSType type, OSType creator)
{
    ST_Log("STIO_SetFileInfo: %s type='%.4s' creator='%.4s'",
           path, (char*)&type, (char*)&creator);

    /* In real implementation, would use File Manager to set file info */
    /* For now, just log the operation */
}