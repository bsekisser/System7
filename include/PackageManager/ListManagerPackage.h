/*
 * ListManagerPackage.h
 * System 7.1 Portable List Manager Package Implementation
 *
 * Implements Mac OS List Manager Package (Pack 0) for scrollable list display and management.
 * Essential for applications with list controls, file browsers, and selection interfaces.
 */

#ifndef __LIST_MANAGER_PACKAGE_H__
#define __LIST_MANAGER_PACKAGE_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "PackageTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* List Manager selector constants */
#define lSelAddColumn       0x0004
#define lSelAddRow          0x0008
#define lSelAddToCell       0x000C
#define lSelAutoScroll      0x0010
#define lSelCellSize        0x0014
#define lSelClick           0x0018
#define lSelClrCell         0x001C
#define lSelDelColumn       0x0020
#define lSelDelRow          0x0024
#define lSelDispose         0x0028
#define lSelDoDraw          0x002C
#define lSelDraw            0x0030
#define lSelFind            0x0034
#define lSelGetCell         0x0038
#define lSelGetSelect       0x003C
#define lSelLastClick       0x0040
#define lSelNew             0x0044
#define lSelNextCell        0x0048
#define lSelRect            0x004C
#define lSelScroll          0x0050
#define lSelSearch          0x0054
#define lSelSetCell         0x0058
#define lSelSetSelect       0x005C
#define lSelSize            0x0060
#define lSelUpdate          0x0064
#define lSelActivate        0x0267  /* Special selector */

/* List behavior flags */
#define lDoVAutoscroll      2
#define lDoHAutoscroll      1
#define lOnlyOne            -128
#define lExtendDrag         64
#define lNoDisjoint         32
#define lNoExtend           16
#define lNoRect             8
#define lUseSense           4
#define lNoNilHilite        2

/* List definition messages */
#define lInitMsg            0
#define lDrawMsg            1
#define lHiliteMsg          2
#define lCloseMsg           3

/* Cell definition */

/* Data structures */

/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Search procedure type */

/* List record structure */

/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Cell data structure */

/* List drawing context */

/* List Manager Package API Functions */

/* Package initialization and management */
SInt32 InitListManagerPackage(void);
void CleanupListManagerPackage(void);
SInt32 ListManagerDispatch(SInt16 selector, void *params);

/* List creation and destruction */
ListHandle LNew(const Rect *rView, const Rect *dataBounds, Point cSize,
                SInt16 theProc, WindowPtr theWindow, Boolean drawIt,
                Boolean hasGrow, Boolean scrollHoriz, Boolean scrollVert);

ListHandle lnew(Rect *rView, Rect *dataBounds, Point *cSize, SInt16 theProc,
                WindowPtr theWindow, Boolean drawIt, Boolean hasGrow,
                Boolean scrollHoriz, Boolean scrollVert);

void LDispose(ListHandle lHandle);

/* List structure modification */
SInt16 LAddColumn(SInt16 count, SInt16 colNum, ListHandle lHandle);
SInt16 LAddRow(SInt16 count, SInt16 rowNum, ListHandle lHandle);
void LDelColumn(SInt16 count, SInt16 colNum, ListHandle lHandle);
void LDelRow(SInt16 count, SInt16 rowNum, ListHandle lHandle);

/* Cell data management */
void LAddToCell(const void *dataPtr, SInt16 dataLen, Cell theCell, ListHandle lHandle);
void LClrCell(Cell theCell, ListHandle lHandle);
void LGetCell(void *dataPtr, SInt16 *dataLen, Cell theCell, ListHandle lHandle);
void LSetCell(const void *dataPtr, SInt16 dataLen, Cell theCell, ListHandle lHandle);
void LFind(SInt16 *offset, SInt16 *len, Cell theCell, ListHandle lHandle);

/* Cell selection and navigation */
Boolean LGetSelect(Boolean next, Cell *theCell, ListHandle lHandle);
void LSetSelect(Boolean setIt, Cell theCell, ListHandle lHandle);
Cell LLastClick(ListHandle lHandle);
Boolean LNextCell(Boolean hNext, Boolean vNext, Cell *theCell, ListHandle lHandle);
Boolean LSearch(const void *dataPtr, SInt16 dataLen, SearchProcPtr searchProc,
                Cell *theCell, ListHandle lHandle);

/* List display and interaction */
void LDoDraw(Boolean drawIt, ListHandle lHandle);
void LDraw(Cell theCell, ListHandle lHandle);
void LUpdate(RgnHandle theRgn, ListHandle lHandle);
void LActivate(Boolean act, ListHandle lHandle);
Boolean LClick(Point pt, SInt16 modifiers, ListHandle lHandle);

/* List geometry and scrolling */
void LCellSize(Point cSize, ListHandle lHandle);
void LSize(SInt16 listWidth, SInt16 listHeight, ListHandle lHandle);
void LRect(Rect *cellRect, Cell theCell, ListHandle lHandle);  /* Classic Mac naming (conflicts with typedef) */
void LScroll(SInt16 dCols, SInt16 dRows, ListHandle lHandle);
void LAutoScroll(ListHandle lHandle);

/* C-style interface functions */
void ldraw(Cell *theCell, ListHandle lHandle);
Boolean lclick(Point *pt, SInt16 modifiers, ListHandle lHandle);
void lcellsize(Point *cSize, ListHandle lHandle);

/* Extended List Manager functions */

/* List data source interface */

/* List configuration */

/* Enhanced list creation */
ListHandle LCreateWithConfig(const Rect *rView, const ListConfiguration *config,
                            WindowPtr theWindow);

/* List configuration functions */
void LSetConfiguration(ListHandle lHandle, const ListConfiguration *config);
void LGetConfiguration(ListHandle lHandle, ListConfiguration *config);
void LSetDataSource(ListHandle lHandle, const ListDataSource *dataSource);
ListDataSource *LGetDataSource(ListHandle lHandle);

/* Selection management */
SInt16 LCountSelectedItems(ListHandle lHandle);
void LGetSelectedItems(ListHandle lHandle, Cell *cells, SInt16 maxCells, SInt16 *actualCount);
void LSelectAll(ListHandle lHandle);
void LSelectNone(ListHandle lHandle);
void LSelectRange(ListHandle lHandle, Cell startCell, Cell endCell, Boolean extend);
void LToggleSelection(ListHandle lHandle, Cell theCell);
Boolean LIsItemSelected(ListHandle lHandle, Cell theCell);

/* List sorting */
void LSortItems(ListHandle lHandle, SearchProcPtr compareProc);
void LSortItemsWithData(ListHandle lHandle, void *userData,
                       SInt16 (*compareProc)(Cell cell1, Cell cell2, void *userData));

/* Keyboard navigation */
void LHandleKeyDown(ListHandle lHandle, char keyCode, SInt16 modifiers);
void LSetKeyboardNavigation(ListHandle lHandle, Boolean enabled);
Boolean LGetKeyboardNavigation(ListHandle lHandle);

/* List metrics and geometry */
SInt16 LGetVisibleCells(ListHandle lHandle, Cell *topLeft, Cell *bottomRight);
Boolean LIsCellVisible(ListHandle lHandle, Cell theCell);
void LScrollToCell(ListHandle lHandle, Cell theCell, Boolean centerInView);
void LGetCellBounds(ListHandle lHandle, Cell theCell, Rect *bounds);
Cell LPointToCell(ListHandle lHandle, Point pt);
Boolean LIsPointInList(ListHandle lHandle, Point pt);

/* List content management */
void LRefreshList(ListHandle lHandle);
void LInvalidateCell(ListHandle lHandle, Cell theCell);
void LInvalidateRange(ListHandle lHandle, Cell startCell, Cell endCell);
void LBeginUpdate(ListHandle lHandle);
void LEndUpdate(ListHandle lHandle);

/* List state management */
void LSaveState(ListHandle lHandle, void **stateData, SInt32 *stateSize);
void LRestoreState(ListHandle lHandle, const void *stateData, SInt32 stateSize);
void LResetToDefaults(ListHandle lHandle);

/* List validation and debugging */
Boolean LValidateList(ListHandle lHandle);
void LDumpListInfo(ListHandle lHandle);
SInt32 LGetMemoryUsage(ListHandle lHandle);

/* Platform integration */

void LSetPlatformCallbacks(const ListPlatformCallbacks *callbacks);
void LEnablePlatformDrawing(Boolean enabled);

/* Thread safety */
void LLockList(ListHandle lHandle);
void LUnlockList(ListHandle lHandle);
void LSetThreadSafe(Boolean threadSafe);

/* Memory management */
void LCompactMemory(ListHandle lHandle);
void LSetMemoryGrowthIncrement(ListHandle lHandle, SInt32 increment);
SInt32 LGetMemoryGrowthIncrement(ListHandle lHandle);

/* Performance optimization */
void LSetLazyDrawing(ListHandle lHandle, Boolean enabled);
Boolean LGetLazyDrawing(ListHandle lHandle);
void LSetUpdateMode(ListHandle lHandle, SInt16 mode);
SInt16 LGetUpdateMode(ListHandle lHandle);

/* List Manager utilities */
void LCopyCell(ListHandle srcList, Cell srcCell, ListHandle dstList, Cell dstCell);
void LMoveCell(ListHandle srcList, Cell srcCell, ListHandle dstList, Cell dstCell);
void LExchangeCells(ListHandle lHandle, Cell cell1, Cell cell2);
void LFillCells(ListHandle lHandle, Cell startCell, Cell endCell, const void *data, SInt16 dataLen);

/* Constants for configuration */
#define kListSelectionStyleHighlight    0
#define kListSelectionStyleInvert       1
#define kListSelectionStyleFrame        2

#define kListUpdateModeImmediate        0
#define kListUpdateModeDeferred         1
#define kListUpdateModeLazy             2

#define kListKeyNavEnabled              1
#define kListKeyNavDisabled             0

#ifdef __cplusplus
}
#endif

#endif /* __LIST_MANAGER_PACKAGE_H__ */
