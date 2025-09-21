/* Menu Manager Stubs - Minimal implementation for initial boot */
#include "../../include/SystemTypes.h"

/* Menu creation and disposal */
MenuHandle NewMenu(short menuID, const unsigned char* menuTitle) { return NULL; }
MenuHandle GetMenu(short resourceID) { return NULL; }
void DisposeMenu(MenuHandle theMenu) {}
void DeleteMenu(short menuID) {}
void InsertMenu(MenuHandle theMenu, short beforeID) {}

/* Menu bar */
void DrawMenuBar(void) {}
void ClearMenuBar(void) {}
Handle GetMenuBar(void) { return NULL; }
void SetMenuBar(Handle menuBar) {}
Handle GetNewMBar(short menuBarID) { return NULL; }

/* Menu items */
void AppendMenu(MenuHandle menu, const unsigned char* data) {}
void InsertMenuItem(MenuHandle menu, const unsigned char* itemString, short afterItem) {}
void DeleteMenuItem(MenuHandle menu, short item) {}
void SetMenuItemText(MenuHandle menu, short item, const unsigned char* itemString) {}
void GetMenuItemText(MenuHandle menu, short item, unsigned char* itemString) {}
void EnableMenuItem(MenuHandle menu, short item) {}
void DisableMenuItem(MenuHandle menu, short item) {}
void CheckMenuItem(MenuHandle menu, short item, Boolean checked) {}
void SetItemMark(MenuHandle menu, short item, short markChar) {}
void GetItemMark(MenuHandle menu, short item, short* markChar) {}
void SetItemIcon(MenuHandle menu, short item, unsigned char icon) {}
void GetItemIcon(MenuHandle menu, short item, unsigned char* icon) {}
void SetItemStyle(MenuHandle menu, short item, short style) {}
void GetItemStyle(MenuHandle menu, short item, unsigned char* style) {}

/* Menu selection */
long MenuSelect(Point startPt) { return 0; }
long MenuKey(short ch) { return 0; }
void HiliteMenu(short menuID) {}
long MenuChoice(void) { return 0; }

/* Menu utilities */
short CountMenuItems(MenuHandle menu) { return 0; }
MenuHandle GetMenuHandle(short menuID) { return NULL; }
void SetMenuFlash(short count) {}
void FlashMenuBar(short menuID) {}
Boolean IsMenuItemEnabled(MenuHandle menu, short item) { return false; }
void CalcMenuSize(MenuHandle theMenu) {}

/* Standard menu commands */
void AddResMenu(MenuHandle theMenu, ResType theType) {}
void InsertResMenu(MenuHandle theMenu, ResType theType, short afterItem) {}

/* Popup menus */
long PopUpMenuSelect(MenuHandle menu, short top, short left, short popUpItem) { return 0; }