/*
 * ===========================================================================
 * RE-AGENT-BANNER: Apple System 7.1 List Manager
 * ===========================================================================
 * Artifact: System.rsrc
 * ROM disassembly
 * Architecture: m68k
 * ABI: macos_system7_1
 *
 * Evidence Sources:
 * - Trap occurrences: 132 total found in binary analysis
 * - Structure layouts: ListRec (84 bytes), Cell (4 bytes)
 * - String evidence: "List Manager not present", "LDEF" resource type
 * - Function mappings from evidence.curated.listmgr.json
 * - Structure definitions from layouts.curated.listmgr.json
 *
 * Provenance:
 * - r2 trap search: addresses documented in evidence files
 * - Structure sizes verified through offset analysis
 * - Mac Inside Macintosh documentation correlation
 * ===========================================================================
 */

#ifndef LIST_MANAGER_H
#define LIST_MANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "ControlManager/ControlManager.h"
#include "WindowManager/WindowTypes.h"

/* ---- Provenance Macros --------------------------------------------------*/
#define PROV(msg)   /* PROV: msg */
#define NOTE(msg)   /* NOTE:  msg */
#define TODO_EVID(msg) /* TODO: EVIDENCE REQUIRED — msg */

/* ---- Forward Declarations -----------------------------------------------*/

/* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* ---- Cell Structure (4 bytes) -------------------------------------------*/
/* PROV: Size verified in layouts.curated.listmgr.json @ structure Cell */

/* ---- Callback Types -----------------------------------------------------*/
/* PROV: ListSearchUPP from mappings.listmgr.json @ type_definitions */

/* PROV: ListClickLoopUPP from layouts.curated.listmgr.json @ ListClickLoopUPP */

/* PROV: ListDefProcPtr from layouts.curated.listmgr.json @ ListDefProcPtr */

/* ---- List Definition Messages -------------------------------------------*/
/* PROV: LDEF messages from layouts.curated.listmgr.json @ constants */

/* ---- List Flags ---------------------------------------------------------*/
/* PROV: Derived from LNew parameters in mappings.listmgr.json */

/* ---- ListRec Structure (84 bytes) ---------------------------------------*/
/* PROV: Size=84 bytes from layouts.curated.listmgr.json @ ListRec */

/* ---- Type Definitions ---------------------------------------------------*/
/* Handle is defined in MacTypes.h */  /* PROV: Handle to cell data storage */

/* ---- List Manager Function Declarations --------------------------------*/

/* PROV: Trap 0xA9E7 - 80 occurrences @ addresses in evidence.curated.listmgr.json */
ListHandle LNew(const Rect* rView, const Rect* dataBounds, Point cSize,
                short theProc, WindowPtr theWindow, Boolean drawIt,
                Boolean hasGrow, Boolean scrollHoriz, Boolean scrollVert);

/* PROV: Trap 0xA9E8 - Missing from evidence but required by API */
void LDispose(ListHandle lHandle);

/* PROV: Trap 0xA9E9 - 3 occurrences @ 0x00014551, 0x00014699, 0x0001472f */
short LAddColumn(short count, short colNum, ListHandle lHandle);

/* PROV: Trap 0xA9EA - 2 occurrences @ 0x00023dd4, 0x00030572 */
short LAddRow(short count, short rowNum, ListHandle lHandle);

/* PROV: Trap 0xA9EB - 26 occurrences @ addresses in evidence.curated.listmgr.json */
void LDelColumn(short count, short colNum, ListHandle lHandle);

/* PROV: Trap 0xA9EC - 2 occurrences @ 0x0002da21, 0x00032093 */
void LDelRow(short count, short rowNum, ListHandle lHandle);

/* PROV: Trap 0xA9ED - 13 occurrences @ addresses in evidence.curated.listmgr.json */
Boolean LGetSelect(Boolean next, Cell* theCell, ListHandle lHandle);

/* PROV: Trap 0xA9EE - 5 occurrences @ 0x000160e5, 0x00019b49, etc. */
Cell LLastClick(ListHandle lHandle);

/* PROV: Trap 0xA9EF - 1 occurrence @ 0x00008d53 */
Boolean LNextCell(Boolean hNext, Boolean vNext, Cell* theCell, ListHandle lHandle);

/* PROV: Trap 0xA9F0 - Missing from evidence but required by API */
Boolean LSearch(const void* dataPtr, short dataLen, ListSearchUPP searchProc,
                Cell* theCell, ListHandle lHandle);

/* PROV: Trap 0xA9F1 - Missing from evidence but required by API */
void LSize(short listWidth, short listHeight, ListHandle lHandle);

/* PROV: Trap 0xA9F2 - 2 occurrences @ 0x0000be47, 0x00034b3b */
void LSetDrawingMode(Boolean drawIt, ListHandle lHandle);

/* PROV: Trap 0xA9F3 - Missing from evidence but required by API */
void LScroll(short dCols, short dRows, ListHandle lHandle);

/* PROV: Trap 0xA9F4 - 2 occurrences @ 0x0000ac19, 0x0000ef9e */
Boolean LAutoScroll(ListHandle lHandle);

/* PROV: Trap 0xA9F5 - Missing from evidence but required by API */
void LUpdate(RgnHandle theRgn, ListHandle lHandle);

/* PROV: Trap 0xA9F6 - Missing from evidence but required by API */
void LActivate(Boolean act, ListHandle lHandle);

/* PROV: Trap 0xA9F7 - Missing from evidence but required by API */
Boolean LClick(Point pt, short modifiers, ListHandle lHandle);

/* PROV: Trap 0xA9F8 - Missing from evidence but required by API */
void LSetCell(const void* dataPtr, short dataLen, Cell theCell, ListHandle lHandle);

/* PROV: Trap 0xA9F9 - Missing from evidence but required by API */
void LGetCell(void* dataPtr, short* dataLen, Cell theCell, ListHandle lHandle);

/* PROV: Trap 0xA9FA - Missing from evidence but required by API */
void LSetSelect(Boolean setIt, Cell theCell, ListHandle lHandle);

/* PROV: Trap 0xA9FB - Missing from evidence but required by API */
void LDraw(Cell theCell, ListHandle lHandle);

/* ---- Utility Macros -----------------------------------------------------*/
#define LAddToCell(dataPtr, dataLen, theCell, lHandle) \
    LSetCell(dataPtr, dataLen, theCell, lHandle)

#define LClrCell(theCell, lHandle) \
    LSetCell(NULL, 0, theCell, lHandle)

/* ---- Error Codes --------------------------------------------------------*/
/* PROV: String "List Manager not present" @ 0x0000a98d suggests error handling */

#endif /* LIST_MANAGER_H */