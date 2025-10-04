/*
 * DialogResourceParser.h - DLOG/DITL Resource Parser Interface
 */

#ifndef DIALOG_RESOURCE_PARSER_H
#define DIALOG_RESOURCE_PARSER_H

#include "SystemTypes.h"
#include "DialogManagerInternal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parse DITL resource into DialogItemEx array */
OSErr ParseDITL(Handle ditlHandle, DialogItemEx** items, SInt16* itemCount);

/* Free parsed DITL items */
void FreeParsedDITL(DialogItemEx* items, SInt16 itemCount);

/* Create a simple default DITL for testing */
Handle CreateDefaultDITL(void);

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_RESOURCE_PARSER_H */
