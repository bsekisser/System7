/*
 * StandardFileHAL_Shims.c - Hardware Abstraction Layer for Standard File Package
 *
 * Adapts StandardFile.c to the System 7.1 reimplementation's existing managers
 * (DialogManager, WindowManager, ControlManager, FileManager).
 */

#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "StandardFile/StandardFile.h"
#include "StandardFile/StandardFileHAL.h"
#include "DialogManager/DialogManager.h"
#include "WindowManager/WindowManager.h"
#include "ControlManager/ControlManager.h"
#include "EventManager/EventManager.h"
#include "QuickDraw/QuickDraw.h"

/* Debug logging */
#ifdef SF_HAL_DEBUG
#define SF_HAL_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleStandardFile, kLogLevelTrace, "[SF HAL] " fmt, ##__VA_ARGS__)
#else
#define SF_HAL_LOG_DEBUG(fmt, ...)
#endif

#define SF_HAL_LOG_INFO(fmt, ...)  serial_logf(kLogModuleStandardFile, kLogLevelInfo, "[SF HAL] " fmt, ##__VA_ARGS__)
#define SF_HAL_LOG_WARN(fmt, ...)  serial_logf(kLogModuleStandardFile, kLogLevelWarn, "[SF HAL] " fmt, ##__VA_ARGS__)

/* File list entry */
typedef struct {
    FSSpec spec;
    OSType fileType;
    Boolean isFolder;
} FileListEntry;

/* HAL state */
static Boolean gHALInitialized = false;
static FileListEntry *gFileList = NULL;
static short gFileListCount = 0;
static short gFileListCapacity = 0;
static short gSelectedIndex = -1;
static short gCurrentVRefNum = 0;
static long gCurrentDirID = 0;
static Boolean gNavigationRequested = false;

#define INITIAL_FILE_LIST_CAPACITY 100

/* Forward declaration of repopulation trigger */
extern void SF_PopulateFileList(void);

/*
 * StandardFile_HAL_Init - Initialize HAL subsystem
 */
void StandardFile_HAL_Init(void) {
    if (!gHALInitialized) {
        SF_HAL_LOG_DEBUG("StandardFile HAL: Initializing\n");

        /* Allocate file list */
        gFileListCapacity = INITIAL_FILE_LIST_CAPACITY;
        gFileList = (FileListEntry*)malloc(gFileListCapacity * sizeof(FileListEntry));
        gFileListCount = 0;
        gSelectedIndex = -1;

        gHALInitialized = true;
    }
}

/*
 * StandardFile_HAL_CreateOpenDialog - Create an open file dialog
 */
OSErr StandardFile_HAL_CreateOpenDialog(DialogPtr *outDialog, ConstStr255Param prompt) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: CreateOpenDialog prompt='%s'\n", prompt ? prompt : "(null)");

    /* Create a simple modal dialog */
    Rect bounds = {100, 100, 350, 450};
    static unsigned char title[] = {9, 'O','p','e','n',' ','F','i','l','e'};
    *outDialog = NewDialog(NULL, &bounds, title, true, dBoxProc,
                           (WindowPtr)-1, false, 0, NULL);

    if (*outDialog == NULL) {
        return memFullErr;
    }

    return noErr;
}

/*
 * StandardFile_HAL_CreateSaveDialog - Create a save file dialog
 */
OSErr StandardFile_HAL_CreateSaveDialog(DialogPtr *outDialog, ConstStr255Param prompt,
                                        ConstStr255Param defaultName) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: CreateSaveDialog prompt='%s' default='%s'\n",
           prompt ? prompt : "(null)", defaultName ? defaultName : "(null)");

    /* Create a simple modal dialog */
    Rect bounds = {100, 100, 350, 400};
    static unsigned char title[] = {9, 'S','a','v','e',' ','F','i','l','e'};
    *outDialog = NewDialog(NULL, &bounds, title, true, dBoxProc,
                           (WindowPtr)-1, false, 0, NULL);

    if (*outDialog == NULL) {
        return memFullErr;
    }

    return noErr;
}

/*
 * StandardFile_HAL_DisposeOpenDialog - Dispose of open dialog
 */
void StandardFile_HAL_DisposeOpenDialog(DialogPtr dialog) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: DisposeOpenDialog\n");
    if (dialog) {
        DisposeDialog(dialog);
    }
}

/*
 * StandardFile_HAL_DisposeSaveDialog - Dispose of save dialog
 */
void StandardFile_HAL_DisposeSaveDialog(DialogPtr dialog) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: DisposeSaveDialog\n");
    if (dialog) {
        DisposeDialog(dialog);
    }
}

/*
 * StandardFile_HAL_RunDialog - Run modal dialog and return item hit
 */
void StandardFile_HAL_RunDialog(DialogPtr dialog, short *itemHit) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: RunDialog - starting modal loop\n");

    if (!dialog || !itemHit) {
        if (itemHit) *itemHit = sfItemCancelButton;
        return;
    }

    EventRecord event;
    Boolean done = false;
    DialogPtr whichDialog;
    short item;

    /* Show the dialog window */
    ShowWindow(dialog);

    /* Modal event loop */
    while (!done) {
        if (GetNextEvent(everyEvent, &event)) {
            if (IsDialogEvent(&event)) {
                if (DialogSelect(&event, &whichDialog, &item)) {
                    if (whichDialog == dialog) {
                        SF_HAL_LOG_DEBUG("StandardFile HAL: Dialog item hit: %d\n", item);

                        /* Handle button clicks */
                        if (item == sfItemOpenButton) {
                            /* Open/Save button */
                            if (gSelectedIndex >= 0 && gSelectedIndex < gFileListCount) {
                                /* Check if it's a folder */
                                if (gFileList[gSelectedIndex].isFolder) {
                                    /* Navigate into folder - don't exit, repopulate */
                                    SF_HAL_LOG_DEBUG("StandardFile HAL: Navigating into folder\n");
                                    StandardFile_HAL_NavigateToFolder(&gFileList[gSelectedIndex].spec);
                                    /* Signal StandardFile.c to repopulate */
                                    *itemHit = sfItemOpenButton;  /* Will be handled by StandardFile.c */
                                    done = true;  /* Exit loop so StandardFile can handle navigation */
                                } else {
                                    /* File selected - exit with Open */
                                    *itemHit = sfItemOpenButton;
                                    done = true;
                                }
                            } else {
                                /* No valid selection - just log for now */
                                SF_HAL_LOG_DEBUG("StandardFile HAL: Open clicked but no valid selection\n");
                            }
                        } else if (item == sfItemCancelButton) {
                            /* Cancel button */
                            *itemHit = sfItemCancelButton;
                            done = true;
                        }
                        /* Handle other items like Eject, Desktop, etc. */
                    }
                }
            } else {
                /* Handle other event types (update, activate, etc.) */
                switch (event.what) {
                    case keyDown:
                    case autoKey: {
                        char key = event.message & charCodeMask;
                        if (event.modifiers & cmdKey) {
                            /* Command-key shortcuts */
                            if (key == '.') {
                                /* Cmd-Period = Cancel */
                                *itemHit = sfItemCancelButton;
                                done = true;
                            }
                        } else {
                            /* Regular keys */
                            if (key == '\r' || key == 0x03) {
                                /* Return/Enter = Open */
                                if (gSelectedIndex >= 0) {
                                    *itemHit = sfItemOpenButton;
                                    done = true;
                                }
                            } else if (key == 0x1B) {
                                /* Escape = Cancel */
                                *itemHit = sfItemCancelButton;
                                done = true;
                            }
                        }
                        break;
                    }
                }
            }
        }

        /* Yield to system */
        SystemTask();
    }

    SF_HAL_LOG_DEBUG("StandardFile HAL: RunDialog exiting with item %d\n", *itemHit);
}

/*
 * StandardFile_HAL_ClearFileList - Clear the file list
 */
void StandardFile_HAL_ClearFileList(DialogPtr dialog) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: ClearFileList\n");
    gFileListCount = 0;
    gSelectedIndex = -1;
}

/*
 * StandardFile_HAL_AddFileToList - Add a file to the list
 */
void StandardFile_HAL_AddFileToList(DialogPtr dialog, const FSSpec *spec, OSType fileType) {
    if (!spec) return;

    /* Check if we need to expand the list */
    if (gFileListCount >= gFileListCapacity) {
        gFileListCapacity *= 2;
        gFileList = (FileListEntry*)realloc(gFileList, gFileListCapacity * sizeof(FileListEntry));
        if (!gFileList) {
            gFileListCapacity = 0;
            gFileListCount = 0;
            return;
        }
    }

    /* Determine if this is a folder by checking with File Manager */
    CInfoPBRec pb;
    memset(&pb, 0, sizeof(pb));
    pb.ioNamePtr = (StringPtr)spec->name;
    pb.ioVRefNum = spec->vRefNum;
    pb.u.dirInfo.ioDrDirID = spec->parID;
    pb.u.hFileInfo.ioFDirIndex = 0;

    Boolean isFolder = false;
    if (PBGetCatInfoSync(&pb) == noErr) {
        isFolder = (pb.u.hFileInfo.ioFlAttrib & 0x10) != 0;  /* 0x10 = directory bit */
    }

    /* Add to list */
    gFileList[gFileListCount].spec = *spec;
    gFileList[gFileListCount].fileType = fileType;
    gFileList[gFileListCount].isFolder = isFolder;

    SF_HAL_LOG_DEBUG("StandardFile HAL: AddFileToList [%d] name='%.*s' type='%.4s' isFolder=%d\n",
           gFileListCount, spec->name[0], spec->name + 1,
           (char*)&fileType, isFolder);

    gFileListCount++;
}

/*
 * StandardFile_HAL_UpdateFileList - Refresh the file list display
 */
void StandardFile_HAL_UpdateFileList(DialogPtr dialog) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: UpdateFileList count=%d\n", gFileListCount);
    /* In a full implementation, this would redraw the list control */
}

/*
 * StandardFile_HAL_SelectFile - Select a file in the list
 */
void StandardFile_HAL_SelectFile(DialogPtr dialog, short index) {
    if (index >= 0 && index < gFileListCount) {
        gSelectedIndex = index;
        SF_HAL_LOG_DEBUG("StandardFile HAL: SelectFile index=%d name='%.*s'\n",
               index, gFileList[index].spec.name[0], gFileList[index].spec.name + 1);
    } else {
        gSelectedIndex = -1;
        SF_HAL_LOG_DEBUG("StandardFile HAL: SelectFile index=%d (invalid, cleared)\n", index);
    }
}

/*
 * StandardFile_HAL_GetSelectedFile - Get the selected file index
 */
short StandardFile_HAL_GetSelectedFile(DialogPtr dialog) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: GetSelectedFile returning %d\n", gSelectedIndex);
    return gSelectedIndex;
}

/*
 * StandardFile_HAL_SetSaveFileName - Set the save file name field
 */
void StandardFile_HAL_SetSaveFileName(DialogPtr dialog, ConstStr255Param name) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: SetSaveFileName name='%s'\n", name ? name : "(null)");
    /* Stub: would set TEHandle text */
}

/*
 * StandardFile_HAL_GetSaveFileName - Get the save file name from field
 */
void StandardFile_HAL_GetSaveFileName(DialogPtr dialog, Str255 name) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: GetSaveFileName\n");
    /* Stub: return default name */
    if (name) {
        const char *defaultName = "Untitled.txt";
        int len = strlen(defaultName);
        name[0] = len;
        memcpy(name + 1, defaultName, len);
    }
}

/*
 * StandardFile_HAL_ConfirmReplace - Ask user to confirm file replacement
 */
Boolean StandardFile_HAL_ConfirmReplace(ConstStr255Param fileName) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: ConfirmReplace file='%s'\n", fileName ? fileName : "(null)");
    /* Stub: auto-confirm for now */
    return true;
}

/*
 * StandardFile_HAL_GetDefaultLocation - Get default save/open location
 */
OSErr StandardFile_HAL_GetDefaultLocation(short *vRefNum, long *dirID) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: GetDefaultLocation\n");
    /* Stub: return root directory */
    if (vRefNum) *vRefNum = 0;
    if (dirID) *dirID = 2;  /* Root directory */
    return noErr;
}

/*
 * StandardFile_HAL_EjectVolume - Eject the current volume
 */
OSErr StandardFile_HAL_EjectVolume(short vRefNum) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: EjectVolume vRefNum=%d\n", vRefNum);
    /* Stub: not implemented */
    return noErr;
}

/*
 * StandardFile_HAL_NavigateToDesktop - Navigate to Desktop folder
 */
OSErr StandardFile_HAL_NavigateToDesktop(short *vRefNum, long *dirID) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: NavigateToDesktop\n");
    /* Stub: return desktop location */
    if (vRefNum) *vRefNum = 0;
    if (dirID) *dirID = 2;
    return noErr;
}

/* StandardFile_HAL_HandleDirPopup is now defined below after NavigateToFolder */

/*
 * StandardFile_HAL_GetNewFolderName - Prompt for new folder name
 */
Boolean StandardFile_HAL_GetNewFolderName(Str255 folderName) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: GetNewFolderName\n");
    /* Stub: return default folder name */
    if (folderName) {
        const char *defaultName = "New Folder";
        int len = strlen(defaultName);
        folderName[0] = len;
        memcpy(folderName + 1, defaultName, len);
    }
    return true;
}

/* NOTE: File Manager functions (PBGetCatInfoSync, HGetFInfo, DirCreate)
 * are provided by FileManager.c, not the HAL */

/*
 * StandardFile_HAL_NavigateToFolder - Navigate to a folder
 * Called internally when user double-clicks a folder
 */
void StandardFile_HAL_NavigateToFolder(const FSSpec *folderSpec) {
    if (!folderSpec) return;

    /* Get the dirID of this folder */
    CInfoPBRec pb;
    memset(&pb, 0, sizeof(pb));
    pb.ioNamePtr = (StringPtr)folderSpec->name;
    pb.ioVRefNum = folderSpec->vRefNum;
    pb.u.dirInfo.ioDrDirID = folderSpec->parID;
    pb.u.hFileInfo.ioFDirIndex = 0;

    if (PBGetCatInfoSync(&pb) == noErr) {
        if (pb.u.hFileInfo.ioFlAttrib & 0x10) {  /* It's a folder */
            gCurrentVRefNum = folderSpec->vRefNum;
            gCurrentDirID = pb.u.dirInfo.ioDrDirID;
            gNavigationRequested = true;
            SF_HAL_LOG_DEBUG("StandardFile HAL: Navigated to folder dirID=%ld\n", gCurrentDirID);
        }
    }
}

/*
 * StandardFile_HAL_HandleDirPopup - Handle directory popup menu
 * For now, implements simple Desktop and Parent navigation
 */
Boolean StandardFile_HAL_HandleDirPopup(DialogPtr dialog, long *selectedDir) {
    SF_HAL_LOG_DEBUG("StandardFile HAL: HandleDirPopup\n");

    /* Simple implementation: navigate to parent directory */
    if (gCurrentDirID != 2) {  /* 2 = root directory */
        CInfoPBRec pb;
        Str255 dirName;

        memset(&pb, 0, sizeof(pb));
        dirName[0] = 0;  /* Empty name to get parent info */
        pb.ioNamePtr = dirName;
        pb.ioVRefNum = gCurrentVRefNum;
        pb.u.dirInfo.ioDrDirID = gCurrentDirID;
        pb.u.hFileInfo.ioFDirIndex = -1;  /* -1 = get info about directory itself */

        if (PBGetCatInfoSync(&pb) == noErr) {
            /* Navigate to parent */
            gCurrentDirID = pb.u.dirInfo.ioDrParID;
            *selectedDir = gCurrentDirID;
            gNavigationRequested = true;
            SF_HAL_LOG_DEBUG("StandardFile HAL: Navigated to parent dirID=%ld\n", gCurrentDirID);
            return true;
        }
    }

    return false;
}
