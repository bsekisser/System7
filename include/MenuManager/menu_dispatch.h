/*
 * menu_dispatch.h - Menu Manager Dispatch Interface
 *
 * Implementation of Menu Manager dispatch mechanism based on
 *
 * RE-AGENT-BANNER: Extracted from Mac OS System 7.1 Menu Manager dispatch trap
 */

#ifndef __MENU_DISPATCH_H__
#define __MENU_DISPATCH_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "MenuManager/menu_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dispatch parameter structures based on implementation */

/* InsertFontResMenu parameters - selector 0, 4 param words */

/* InsertIntlResMenu parameters - selector 1, 6 param words */

/* GetMenuTitleRect parameters - selector -1, 6 param words */

/* GetMBARRect parameters - selector -2, 4 param words */

/* GetAppMenusRect parameters - selector -3, 4 param words */

/* GetSysMenusRect parameters - selector -4, 4 param words */

/* DrawMBARString parameters - selector -5, 8 param words */

/* IsSystemMenu parameters - selector -6, 3 param words */

/* CalcMenuBar parameters - selector -11, 1 param word */

OSErr MenuDispatch(SInt16 selector, void *params);

/* Stub functions for resource-based menus */
void AddResMenu(MenuHandle theMenu, ResType theType);
void InsertResMenu(MenuHandle theMenu, ResType theType, short afterItem);

/* Convenience wrappers for dispatch calls */
OSErr CallInsertFontResMenu(MenuHandle theMenu, SInt16 afterItem);
OSErr CallInsertIntlResMenu(MenuHandle theMenu, SInt16 afterItem, SInt16 scriptTag);
OSErr CallGetMenuTitleRect(MenuHandle theMenu, Rect *titleRect);
OSErr CallGetMBARRect(Rect *mbarRect);
OSErr CallGetAppMenusRect(Rect *appRect);
OSErr CallGetSysMenusRect(Rect *sysRect);
OSErr CallDrawMBARString(const unsigned char *text, SInt16 script, Rect *bounds, SInt16 just);
Boolean CallIsSystemMenu(SInt16 menuID);
OSErr CallCalcMenuBar(void);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_DISPATCH_H__ */

/*
 * RE-AGENT-TRAILER-JSON: {
 *   "evidence_sources": [
 *     "
 *   ],
 *   "dispatch_selectors": 9,
 *   "parameter_structures": 9,
 *   "wrapper_functions": 9
 * }
 */