/* System Default Icons API */

#pragma once
#include "icon_types.h"

/* Get default icon families */
const IconFamily* IconSys_DefaultFolder(void);
const IconFamily* IconSys_DefaultVolume(void);
const IconFamily* IconSys_DefaultDoc(void);

/* Trash icons */
const IconFamily* IconSys_TrashEmpty(void);
const IconFamily* IconSys_TrashFull(void);

/* Additional system icons */
const IconFamily* IconSys_BatteryEmpty(void);
const IconFamily* IconSys_Battery25(void);
const IconFamily* IconSys_Battery50(void);
const IconFamily* IconSys_BatteryFull(void);
const IconFamily* IconSys_HardDisk(void);