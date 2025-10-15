#ifndef QUICKDRAW_DISPLAY_BEZEL_H
#define QUICKDRAW_DISPLAY_BEZEL_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kQDBezelAuto = 0,   /* Choose based on machine profile */
    kQDBezelRounded,    /* Always draw CRT-style corners */
    kQDBezelFlat        /* Never draw bezel (flat panel) */
} QDBezelMode;

void QD_SetBezelMode(QDBezelMode mode);
QDBezelMode QD_GetBezelMode(void);

/* Draw CRT-style bezel edges (superellipse mask) directly to the framebuffer. */
void QD_DrawCRTBezel(void);

#ifdef __cplusplus
}
#endif

#endif /* QUICKDRAW_DISPLAY_BEZEL_H */
