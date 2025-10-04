/*
 * sys71_stubs.h - Function declarations for System 7.1 stubs
 *
 * This header provides declarations for stub functions defined in sys71_stubs.c
 */

#ifndef SYS71_STUBS_H
#define SYS71_STUBS_H

#include "MacTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "EventManager/EventTypes.h"
#include "WindowManager/WindowManager.h"
#include "ControlManager/ControlManager.h"
#include "TextEdit/TextEdit.h"

/* Platform Menu System */
void Platform_InitMenuSystem(void);
void Platform_CleanupMenuSystem(void);

/* TextEdit */
void InitTE(void);

/* Control Manager */
void InitControlManager_Sys7(void);

/* Expand Memory (Low Memory Globals) */
void ExpandMemInit(void);
void ExpandMemInitKeyboard(void);
void ExpandMemSetAppleTalkInactive(void);
void ExpandMemInstallDecompressor(void);
void ExpandMemCleanup(void);
void ExpandMemDump(void);
Boolean ExpandMemValidate(void);

/* Finder */
void FinderEventLoop(void);

/* Window Manager */
void WM_UpdateWindowVisibility(WindowPtr window);
OSErr InitializeWindowManager(void);

/* System */
long sysconf(int name);

/* Event Handling */
void HandleKeyDown(EventRecord* event);

/* Alias Manager */
OSErr ResolveAliasFile(const FSSpec* spec, FSSpec* target, Boolean* wasAliased, Boolean* wasFolder);
OSErr NewAlias(const FSSpec* fromFile, const FSSpec* target, AliasHandle* alias);

/* File System Spec */
OSErr FSpCreateResFile(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag);
OSErr FSpCreate(const FSSpec* spec, OSType creator, OSType fileType, SInt16 scriptTag);
OSErr FSpOpenDF(const FSSpec* spec, SInt16 permission, SInt16* refNum);
OSErr FSpOpenResFile(const FSSpec* spec, SInt16 permission);
OSErr FSpDelete(const FSSpec *spec);
OSErr FSpDirDelete(const FSSpec *spec);
OSErr FSpCatMove(const FSSpec *source, const FSSpec *dest);

/* Parameter Block */
OSErr PBHGetVInfoSync(HParamBlockRec* paramBlock);

/* File Manager */
OSErr SetEOF(SInt16 refNum, SInt32 logEOF);
OSErr FSMakeFSSpec(short vRefNum, long dirID, ConstStr255Param fileName, FSSpec *spec);
OSErr FSRead(short refNum, long *count, void *buffPtr);
OSErr FSWrite(short refNum, long *count, const void *buffPtr);
OSErr FSClose(short refNum);
OSErr PBGetCatInfoSync(CInfoPBPtr paramBlock);
OSErr PBSetCatInfoSync(CInfoPBPtr paramBlock);

/* Memory Manager */
void BlockMoveData(const void *srcPtr, void *destPtr, Size byteCount);
Ptr NewPtr(Size byteCount);
Handle NewHandle(Size byteCount);
void DisposeHandle(Handle h);
Size GetHandleSize(Handle h);

/* FindFolder */
OSErr FindFolder(SInt16 vRefNum, OSType folderType, Boolean createFolder, SInt16* foundVRefNum, SInt32* foundDirID);
OSErr GenerateUniqueTrashName(Str255 baseName, Str255 uniqueName);

/* Geometry/Math */
SInt16 HiWord(SInt32 x);
SInt16 LoWord(SInt32 x);
double atan2(double y, double x);
double cos(double x);
double sin(double x);
long long __divdi3(long long a, long long b);

/* QuickDraw */
typedef void (*DeskHookProc)(RgnHandle invalidRgn);
void SetDeskHook(DeskHookProc proc);
/* Moved to FontManagerCore.c
void TextSize(short size);
void TextFont(short font);
void TextFace(short face);
*/
OSErr HandleGetInfo(void);

/* Miscellaneous */
OSErr ShowFind(void);
OSErr FindAgain(void);
OSErr ShowAboutFinder(void);
OSErr HandleContentClick(WindowPtr window, EventRecord* event);
OSErr HandleGrowWindow(WindowPtr window, EventRecord* event);
OSErr CloseFinderWindow(WindowPtr window);
void DoUpdate(WindowPtr window);
void DoActivate(WindowPtr window, Boolean becomingActive);
void DoBackgroundTasks(void);
OSErr ShowConfirmDialog(ConstStr255Param message, Boolean* confirmed);
OSErr CloseAllWindows(void);
OSErr CleanUpSelection(WindowPtr window);
OSErr CleanUpBy(WindowPtr window, short sortOrder);
OSErr CleanUpWindow(WindowPtr window, SInt16 cleanupType);
OSErr ScanDirectoryForDesktopEntries(SInt16 vRefNum, SInt32 dirID, SInt16 databaseRefNum);

#endif /* SYS71_STUBS_H */
