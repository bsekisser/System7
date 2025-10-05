/*
 * LowMemGlobals.h - Mac OS Low Memory Global Definitions
 *
 * Defines standard Mac OS low memory global addresses (0x0000-0xFFFF)
 * used by System 7.1 for system state and manager communication.
 *
 * These are specific memory locations in the M68K address space that
 * the OS uses for system-wide state and inter-manager communication.
 */

#ifndef LOW_MEM_GLOBALS_H
#define LOW_MEM_GLOBALS_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * System Globals (0x000-0x2FF)
 */
#define LMG_MemTop          0x0108  /* Top of RAM */
#define LMG_BufPtr          0x010C  /* Top of application memory */
#define LMG_StkLowPt        0x0110  /* Lowest stack pointer */
#define LMG_HeapEnd         0x0114  /* End of application heap */
#define LMG_ApplLimit       0x0130  /* Application heap limit */
#define LMG_SysZone         0x02A6  /* System zone pointer */
#define LMG_ApplZone        0x02AA  /* Application zone pointer */
#define LMG_ROMBase         0x02AE  /* ROM base address */
#define LMG_RAMBase         0x02B2  /* RAM base address */
#define LMG_ExpandMem       0x02B6  /* ExpandMem pointer */
#define LMG_DSAlertTab      0x02BA  /* System error alert table */
#define LMG_ExtStsDT        0x02BE  /* External/Status drives */
#define LMG_ABusVars        0x02D8  /* AppleBus variables */
#define LMG_ABusDCE         0x02DC  /* AppleBus DCE */
#define LMG_FinderName      0x02E0  /* Finder name (Str15) */
#define LMG_DoubleTime      0x02F0  /* Double-click interval */
#define LMG_CaretTime       0x02F4  /* Caret blink interval */
#define LMG_ScrDmpEnb       0x02F8  /* Screen dump enable flag */
#define LMG_BufTgFNum       0x02FC  /* Buffer tag file number */
#define LMG_BufTgFFlg       0x0300  /* Buffer tag flags */
#define LMG_BufTgFBkNum     0x0302  /* Buffer tag block number */
#define LMG_BufTgDate       0x0304  /* Buffer tag date */
#define LMG_MinStack        0x031E  /* Minimum stack size */
#define LMG_DefltStack      0x0322  /* Default stack size */
#define LMG_GZRootHnd       0x0328  /* Root handle in heap */
#define LMG_GZRootPtr       0x032C  /* Root pointer in heap */
#define LMG_GZMoveHnd       0x0330  /* Move handle in heap */

/*
 * Process Manager Globals (0x316-0x3FF)
 */
#define LMG_CurrentA5       0x0904  /* Current A5 world */
#define LMG_CurStackBase    0x0908  /* Current stack base */
#define LMG_CurJTOffset     0x0934  /* Current jump table offset */
#define LMG_CurPageOption   0x0936  /* Current page option */
#define LMG_LoaderLMGlobals 0x0A00  /* Loader low-memory globals */
#define LMG_TopMapHndl      0x0A50  /* Top resource map handle */
#define LMG_SysMapHndl      0x0A54  /* System resource map handle */
#define LMG_SysMap          0x0A58  /* System resource map pointer */
#define LMG_TopMapPtr       0x0A5C  /* Top resource map pointer */
#define LMG_TmpResLoad      0x0B9E  /* Temp resource load flag */
#define LMG_IntlSpec        0x0BA0  /* International spec */
#define LMG_ROMMapHndl      0x0B06  /* ROM resource map handle */

/*
 * Scrap Manager Globals (0x960-0x97F)
 */
#define LMG_ScrapSize       0x0960  /* Scrap size */
#define LMG_ScrapHandle     0x0964  /* Scrap handle */
#define LMG_ScrapCount      0x0968  /* Scrap count */
#define LMG_ScrapState      0x096A  /* Scrap state */
#define LMG_ScrapName       0x096C  /* Scrap name */

/*
 * Event Manager Globals (0x140-0x1FF)
 */
#define LMG_Ticks           0x016A  /* Tick count (1/60 second) */
#define LMG_KeyMap          0x0174  /* Keyboard map (16 bytes) */
#define LMG_KeyTime         0x0184  /* Time of last keystroke */
#define LMG_KeyRepTime      0x0186  /* Time of last key repeat */
#define LMG_EventQueue      0x014A  /* Event queue header */
#define LMG_MouseLocation   0x082E  /* Mouse location (Point) */
#define LMG_MouseLocation2  0x0828  /* Alternate mouse location */
#define LMG_MBState         0x0172  /* Mouse button state */
#define LMG_MBTicks         0x016E  /* Mouse button ticks */

/*
 * QuickDraw Globals (0x800-0x8FF)
 */
#define LMG_ThePort         0x0A86  /* Current GrafPort */
#define LMG_DeskPattern     0x0A3C  /* Desktop pattern */
#define LMG_WMgrPort        0x09DE  /* Window Manager port */
#define LMG_Gray            0x09DA  /* Gray pattern */
#define LMG_Arrow           0x093E  /* Arrow cursor */
#define LMG_ScreenBits      0x09A4  /* Screen bitmap */
#define LMG_RandSeed        0x0156  /* Random seed */

/*
 * Window Manager Globals (0x900-0x9FF)
 */
#define LMG_WindowList      0x09D6  /* Window list head */
#define LMG_SaveUpdate      0x09DA  /* SaveUpdate flag */
#define LMG_PaintWhite      0x09DC  /* PaintWhite flag */
#define LMG_WMgrCPort       0x09DE  /* Window Manager color port */
#define LMG_GrayRgn         0x09EE  /* Desktop region */
#define LMG_DragHook        0x09F6  /* Drag hook */
#define LMG_DeskHook        0x0A6C  /* Desk hook */
#define LMG_GhostWindow     0x0A84  /* Ghost window */

/*
 * Menu Manager Globals (0xA1C-0xA3F)
 */
#define LMG_MenuList        0x0A1C  /* Menu list head */
#define LMG_MBarEnable      0x0A20  /* MenuBar enable flags */
#define LMG_MBarHeight      0x0BAA  /* MenuBar height */
#define LMG_MenuFlash       0x0A24  /* Menu flash count */
#define LMG_TheMenu         0x0A26  /* Current menu */

/*
 * Font Manager Globals (0x984-0x9FF)
 */
#define LMG_FMSwapFont      0x0984  /* Font Manager swap font */
#define LMG_FondState       0x0988  /* Font state */
#define LMG_FractEnable     0x0BF4  /* Fractional widths enable */
#define LMG_FScaleDisable   0x0A63  /* Font scaling disable */
#define LMG_ApFontID        0x0984  /* Application font ID */
#define LMG_SysFontFam      0x0BA6  /* System font family */
#define LMG_SysFontSiz      0x0BA8  /* System font size */

/*
 * Initialization
 */

/* Forward declaration */
typedef struct M68KAddressSpace M68KAddressSpace;

/* Initialize low memory globals system with address space */
void LMInit(M68KAddressSpace* as);

/*
 * Utility Functions
 */

/* Read 32-bit long from low memory global */
UInt32 LMGetLong(UInt32 addr);

/* Write 32-bit long to low memory global */
void LMSetLong(UInt32 addr, UInt32 value);

/* Read 16-bit word from low memory global */
UInt16 LMGetWord(UInt32 addr);

/* Write 16-bit word to low memory global */
void LMSetWord(UInt32 addr, UInt16 value);

/* Read 8-bit byte from low memory global */
UInt8 LMGetByte(UInt32 addr);

/* Write 8-bit byte to low memory global */
void LMSetByte(UInt32 addr, UInt8 value);

/* Specific accessors for commonly used globals */
UInt32 LMGetCurrentA5(void);
void LMSetCurrentA5(UInt32 a5);

UInt32 LMGetExpandMem(void);
void LMSetExpandMem(UInt32 expandMem);

UInt32 LMGetTicks(void);
void LMSetTicks(UInt32 ticks);

void* LMGetThePort(void);
void LMSetThePort(void* port);

#ifdef __cplusplus
}
#endif

#endif /* LOW_MEM_GLOBALS_H */
