/*
 * StandardFilePackage.h
 * System 7.1 Portable Standard File Package Implementation
 *
 * Implements Mac OS Standard File Package (Pack 3) for file dialog functionality.
 * Critical for applications that need to open/save files through standard dialogs.
 * Provides platform-independent file selection interface.
 */

#ifndef __STANDARD_FILE_PACKAGE_H__
#define __STANDARD_FILE_PACKAGE_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "PackageTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Standard File selector constants */
#define sfSelPutFile        1
#define sfSelGetFile        2
#define sfSelPPutFile       3
#define sfSelPGetFile       4
#define sfSelStandardPut    5
#define sfSelStandardGet    6
#define sfSelCustomPut      7
#define sfSelCustomGet      8

/* Dialog ID constants */
#define putDlgID            -3999
#define getDlgID            -4000
#define sfPutDialogID       -6043
#define sfGetDialogID       -6042

/* Dialog item constants */
#define putSave             1
#define putCancel           2
#define putEject            5
#define putDrive            6
#define putName             7

#define getOpen             1
#define getCancel           3
#define getEject            5
#define getDrive            6
#define getNmList           7
#define getScroll           8

/* System 7.0+ dialog items */
#define sfItemOpenButton        1
#define sfItemCancelButton      2
#define sfItemBalloonHelp       3
#define sfItemVolumeUser        4
#define sfItemEjectButton       5
#define sfItemDesktopButton     6
#define sfItemFileListUser      7
#define sfItemPopUpMenuUser     8
#define sfItemDividerLinePict   9
#define sfItemFileNameTextEdit  10
#define sfItemPromptStaticText  11
#define sfItemNewFolderUser     12

/* Hook event constants */
#define sfHookFirstCall         -1
#define sfHookCharOffset        0x1000
#define sfHookNullEvent         100
#define sfHookRebuildList       101
#define sfHookFolderPopUp       102
#define sfHookOpenFolder        103
#define sfHookOpenAlias         104
#define sfHookGoToDesktop       105
#define sfHookGoToAliasTarget   106
#define sfHookGoToParent        107
#define sfHookGoToNextDrive     108
#define sfHookGoToPrevDrive     109
#define sfHookChangeSelection   110
#define sfHookSetActiveOffset   200
#define sfHookLastCall          -2

/* Dialog reference constants */
#define sfMainDialogRefCon      'stdf'
#define sfNewFolderDialogRefCon 'nfdr'
#define sfReplaceDialogRefCon   'rplc'
#define sfStatWarnDialogRefCon  'stat'
#define sfLockWarnDialogRefCon  'lock'
#define sfErrorDialogRefCon     'err '

/* File types */

/* Standard File reply structures */

/* Filter and hook procedure types */

/* Enhanced procedure types with user data */

/* File dialog configuration */

/* Standard File Package API Functions */

/* Package initialization and management */
SInt32 InitStandardFilePackage(void);
void CleanupStandardFilePackage(void);
SInt32 StandardFileDispatch(SInt16 selector, void *params);

/* Classic Standard File functions */
void SFPutFile(Point where, ConstStr255Param prompt, ConstStr255Param origName,
               DlgHookProcPtr dlgHook, SFReply *reply);

void SFGetFile(Point where, ConstStr255Param prompt, FileFilterProcPtr fileFilter,
               SInt16 numTypes, SFTypeList typeList, DlgHookProcPtr dlgHook, SFReply *reply);

void SFPPutFile(Point where, ConstStr255Param prompt, ConstStr255Param origName,
                DlgHookProcPtr dlgHook, SFReply *reply, SInt16 dlgID, ModalFilterProcPtr filterProc);

void SFPGetFile(Point where, ConstStr255Param prompt, FileFilterProcPtr fileFilter,
                SInt16 numTypes, SFTypeList typeList, DlgHookProcPtr dlgHook, SFReply *reply,
                SInt16 dlgID, ModalFilterProcPtr filterProc);

/* System 7.0+ Standard File functions */
void StandardPutFile(ConstStr255Param prompt, ConstStr255Param defaultName, StandardFileReply *reply);

void StandardGetFile(FileFilterProcPtr fileFilter, SInt16 numTypes, SFTypeList typeList,
                     StandardFileReply *reply);

void CustomPutFile(ConstStr255Param prompt, ConstStr255Param defaultName, StandardFileReply *reply,
                   SInt16 dlgID, Point where, DlgHookYDProcPtr dlgHook,
                   ModalFilterYDProcPtr filterProc, SInt16 *activeList,
                   ActivateYDProcPtr activateProc, void *yourDataPtr);

void CustomGetFile(FileFilterYDProcPtr fileFilter, SInt16 numTypes, SFTypeList typeList,
                   StandardFileReply *reply, SInt16 dlgID, Point where, DlgHookYDProcPtr dlgHook,
                   ModalFilterYDProcPtr filterProc, SInt16 *activeList,
                   ActivateYDProcPtr activateProc, void *yourDataPtr);

/* C-style interface functions */
void sfputfile(Point *where, char *prompt, char *origName, DlgHookProcPtr dlgHook, SFReply *reply);
void sfgetfile(Point *where, char *prompt, FileFilterProcPtr fileFilter, SInt16 numTypes,
               SFTypeList typeList, DlgHookProcPtr dlgHook, SFReply *reply);
void sfpputfile(Point *where, char *prompt, char *origName, DlgHookProcPtr dlgHook, SFReply *reply,
                SInt16 dlgID, ModalFilterProcPtr filterProc);
void sfpgetfile(Point *where, char *prompt, FileFilterProcPtr fileFilter, SInt16 numTypes,
                SFTypeList typeList, DlgHookProcPtr dlgHook, SFReply *reply,
                SInt16 dlgID, ModalFilterProcPtr filterProc);

/* Platform-independent file dialog interface */

/* File system navigation */

/* Directory navigation functions */
OSErr SF_GetDirectoryListing(const FSSpec *directory, DirectoryListing *listing);
void SF_FreeDirectoryListing(DirectoryListing *listing);
OSErr SF_ChangeDirectory(FSSpec *currentDir, const char *newPath);
OSErr SF_GetParentDirectory(const FSSpec *currentDir, FSSpec *parentDir);
Boolean SF_IsValidFilename(const char *filename);
OSErr SF_CreateNewFolder(const FSSpec *parentDir, const char *folderName, FSSpec *newFolder);

/* File type and filter management */
Boolean SF_FileMatchesFilter(const FileInfo *fileInfo, const SFTypeList typeList, SInt16 numTypes);
void SF_BuildTypeFilter(const SFTypeList typeList, SInt16 numTypes, char *filterString, int maxLen);
OSErr SF_GetFileInfo(const FSSpec *fileSpec, FileInfo *fileInfo);
OSErr SF_SetFileInfo(const FSSpec *fileSpec, const FileInfo *fileInfo);

/* Dialog management */

SFDialogState *SF_CreateDialog(const SFDialogConfig *config, Boolean isOpen);
void SF_DestroyDialog(SFDialogState *dialogState);
Boolean SF_RunDialog(SFDialogState *dialogState, StandardFileReply *reply);
void SF_UpdateFileList(SFDialogState *dialogState);
void SF_HandleDialogEvent(SFDialogState *dialogState, const EventRecord *event);

/* Volume and drive management */

SInt16 SF_GetVolumeList(VolumeInfo *volumes, SInt16 maxVolumes);
OSErr SF_EjectVolume(SInt16 vRefNum);
OSErr SF_MountVolume(void);
void SF_GoToDesktop(FSSpec *currentDir);
OSErr SF_ResolveAlias(const FSSpec *aliasFile, FSSpec *target);

/* Configuration and preferences */
void SF_SetDefaultDirectory(const FSSpec *directory);
void SF_GetDefaultDirectory(FSSpec *directory);
void SF_SetFileDialogPreferences(SInt32 preferences);
SInt32 SF_GetFileDialogPreferences(void);
void SF_SetPlatformFileDialogs(const PlatformFileDialogs *dialogs);

/* Error handling */
const char *SF_GetErrorString(OSErr error);
void SF_SetErrorHandler(void (*handler)(OSErr error, const char *message));

/* Utility functions */
void SF_FSSpecToPath(const FSSpec *spec, char *path, int maxLen);
OSErr SF_PathToFSSpec(const char *path, FSSpec *spec);
Boolean SF_FSSpecEqual(const FSSpec *spec1, const FSSpec *spec2);
OSErr SF_GetFullPath(const FSSpec *spec, char *fullPath, int maxLen);

/* Thread safety */
void SF_LockFileDialogs(void);
void SF_UnlockFileDialogs(void);

/* Memory management */
void *SF_AllocMem(size_t size);
void SF_FreeMem(void *ptr);
void *SF_ReallocMem(void *ptr, size_t newSize);

#ifdef __cplusplus
}
#endif

#endif /* __STANDARD_FILE_PACKAGE_H__ */
