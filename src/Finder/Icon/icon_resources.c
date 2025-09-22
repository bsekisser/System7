/* Icon Resource Loading
 * Handles ICN#, cicn, BNDL/FREF lookups
 */

#include "Finder/Icon/icon_resources.h"
#include <string.h>
#include <stddef.h>

/* Stub: Load icon family by resource ID */
bool IconRes_LoadFamilyByID(int16_t rsrcID, IconFamily* out) {
    /* TODO: Implement ICN# and cicn loading from resources */
    /* For now, return false to fall back to defaults */
    return false;
}

/* Map type/creator to icon resource ID */
bool IconRes_MapTypeCreatorToIcon(uint32_t type, uint32_t creator, int16_t* outRsrcID) {
    /* Simple hardcoded mappings for now */

    /* Application */
    if (type == 'APPL') {
        *outRsrcID = 128;  /* Generic app icon */
        return true;
    }

    /* TeachText document */
    if (type == 'TEXT' && creator == 'ttxt') {
        *outRsrcID = 129;  /* TeachText document */
        return true;
    }

    /* SimpleText document */
    if (type == 'TEXT' && creator == 'stxt') {
        *outRsrcID = 130;
        return true;
    }

    /* Generic text file */
    if (type == 'TEXT') {
        *outRsrcID = 131;
        return true;
    }

    return false;
}

/* Load custom icon from path */
bool IconRes_LoadCustomIconForPath(const char* path, IconFamily* out) {
    /* TODO: Check for Icon\r file in folder */
    /* TODO: Check HFS+ extended attributes */
    return false;
}