#ifndef QUICKDRAW_DISPLAY_BEZEL_H
#define QUICKDRAW_DISPLAY_BEZEL_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Draw CRT-style bezel edges (superellipse mask) directly to the framebuffer. */
void QD_DrawCRTBezel(void);

#ifdef __cplusplus
}
#endif

#endif /* QUICKDRAW_DISPLAY_BEZEL_H */
