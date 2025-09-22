/* Icon Resource Loading
 * Handles ICN#, cicn, BNDL/FREF lookups
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "icon_types.h"

/* Returns true if found, fills IconFamily (prefers cicn; falls back to ICN#) */
bool IconRes_LoadFamilyByID(int16_t rsrcID, IconFamily* out);

/* BNDL/FREF lookup:
 * 1) Find app with matching creator (search owners in desktop / running apps)
 * 2) Read its BNDL resource to map type/creator → icon ID
 */
bool IconRes_MapTypeCreatorToIcon(uint32_t type, uint32_t creator, int16_t* outRsrcID);

/* For custom icons: folder/file contains flag + "Icon\r" file holding an icon family */
bool IconRes_LoadCustomIconForPath(const char* path, IconFamily* out);