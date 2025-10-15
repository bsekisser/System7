/*
 * MenuAppIcon.h - Application menu icon drawing helper
 */
#ifndef __MENU_APP_ICON_H__
#define __MENU_APP_ICON_H__

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Draw the application icon at menu bar position (left, top) */
short MenuAppIcon_Draw(struct GrafPort* port, short left, short top, Boolean highlighted);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_APP_ICON_H__ */

