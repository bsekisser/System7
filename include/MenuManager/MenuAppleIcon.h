#ifndef MENU_MANAGER_MENU_APPLE_ICON_H
#define MENU_MANAGER_MENU_APPLE_ICON_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MENU_APPLE_ICON_WIDTH 30
#define MENU_APPLE_ICON_HEIGHT 18

struct GrafPort;

short MenuAppleIcon_Draw(struct GrafPort* port, short left, short top, Boolean highlighted);

#ifdef __cplusplus
}
#endif

#endif /* MENU_MANAGER_MENU_APPLE_ICON_H */
