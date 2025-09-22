/* Icon Label Renderer
 * Shared label drawing for all icons
 */

#pragma once
#include <stdbool.h>
#include "icon_types.h"

/* Measure text for label */
void IconLabel_Measure(const char* name, int* outWidth, int* outHeight);

/* Draw label with optional selection */
void IconLabel_Draw(const char* name, int cx, int topY, bool selected);

/* Complete icon+label drawing helper */
IconRect Icon_DrawWithLabel(const IconHandle* h, const char* name, int centerX, int iconTopY, bool selected);