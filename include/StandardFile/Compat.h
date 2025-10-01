/*
 * StandardFile/Compat.h - Compatibility typedefs for Standard File Package
 *
 * These typedefs are specific to the Standard File Package and do not
 * conflict with system-wide definitions in SystemTypes.h.
 */

#ifndef STANDARDFILE_COMPAT_H
#define STANDARDFILE_COMPAT_H

#include "SystemTypes.h"

/* Standard File Package callback types - not in SystemTypes.h */
typedef void (*DlgHookProcPtr)(short item, DialogPtr dialog);
typedef short (*DlgHookYDProcPtr)(short item, DialogPtr dialog, void* yourDataPtr);
typedef Boolean (*ModalFilterYDProcPtr)(DialogPtr dialog, EventRecord* event, short* itemHit, void* yourDataPtr);
typedef void (*ActivateYDProcPtr)(DialogPtr dialog, short itemNo, Boolean activating, void* yourDataPtr);

/* Note: FileFilterYDProcPtr is already defined in SystemTypes.h */

/* Const pointer type for type lists */
typedef const OSType* ConstSFTypeListPtr;

#endif /* STANDARDFILE_COMPAT_H */
