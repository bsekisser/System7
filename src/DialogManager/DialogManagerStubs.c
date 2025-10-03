/* DialogManager Stub Functions */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/DialogItems.h"
#include "DialogManager/DialogManagerStateExt.h"

/* DialogItemInternal is now defined in DialogManagerStateExt.h */

/* Get dialog item pointer stub */
DialogItemInternal* GetDialogItemPtr(DialogPtr theDialog, SInt16 itemNo) {
    /* Stub implementation */
    return NULL;
}

/* SysBeep - now provided by SoundManagerBareMetal.c */

/* DrawDialog & UpdateDialog - provided by DialogManagerCore.c */

/* ShowWindow/HideWindow/DrawWindow - provided by WindowDisplay.c */

/* GetNewControl - provided by ControlManagerCore.c */

/* Control functions - provided by sys71_stubs.c, ControlTracking.c and ControlManagerCore.c */

/* Dialog text stubs - now provided by DialogItems.c */

/* Dialog Manager initialization - now provided by real implementations:
 * - InitModalDialogs() in ModalDialogs.c
 * - InitDialogItems() in DialogItems.c
 * - InitDialogResources() in DialogResources.c
 * - InitDialogEvents() in DialogEvents.c
 */

/* Dialog template management - now provided by DialogResources.c */

/* Modal dialog functions - now provided by ModalDialogs.c */

/* Dialog item functions - now provided by DialogItems.c */

/* Modal dialog stub - until ModalDialogs.c is fixed */
void EndModalDialog(void) {
    serial_printf("EndModalDialog stub\n");
}

/* Dialog item stubs - until DialogItems.c is fixed */
SInt16 CountDITL(DialogPtr theDialog) {
    return 0;
}

void DrawDialogItem(DialogPtr theDialog, SInt16 itemNo) {
    serial_printf("DrawDialogItem stub: item=%d\n", itemNo);
}

/* Dialog resource stubs - until DialogResources.c is fixed */
void DisposeDialogTemplate(Handle theTemplate) {
    serial_printf("DisposeDialogTemplate stub\n");
}

void DisposeDialogItemList(Handle itemList) {
    serial_printf("DisposeDialogItemList stub\n");
}

OSErr LoadDialogTemplate(SInt16 dialogID, Handle* template) {
    serial_printf("LoadDialogTemplate stub: id=%d\n", dialogID);
    return -192; /* resNotFound */
}

OSErr LoadDialogItemList(SInt16 itemListID, Handle* itemList) {
    serial_printf("LoadDialogItemList stub: id=%d\n", itemListID);
    return -192; /* resNotFound */
}

/* Dialog init stubs */
void InitModalDialogs(void) {
    serial_printf("InitModalDialogs stub\n");
}

void InitDialogItems(void) {
    serial_printf("InitDialogItems stub\n");
}

void InitDialogResources(void) {
    serial_printf("InitDialogResources stub\n");
}

void InitDialogEvents(void) {
    serial_printf("InitDialogEvents stub\n");
}

/* Resource stubs - provided by simple_resource_manager.c and sys71_stubs.c */