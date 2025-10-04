/*
 * FontStyleSynthesis.h - Font Style Synthesis API
 *
 * Style generation functions for System 7.1 Font Manager
 */

#ifndef FONT_STYLE_SYNTHESIS_H
#define FONT_STYLE_SYNTHESIS_H

#include "SystemTypes.h"
#include "FontTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Individual Style Synthesis */
void FM_SynthesizeBold(short x, short y, char ch, uint32_t color);
void FM_SynthesizeItalic(short x, short y, char ch, uint32_t color);
void FM_SynthesizeShadow(short x, short y, char ch, uint32_t foreColor, uint32_t shadowColor);
void FM_SynthesizeOutline(short x, short y, char ch, uint32_t outlineColor, uint32_t fillColor);

/* Underline Drawing */
void FM_DrawUnderline(short x, short y, short width, uint32_t color);

/* Width Calculations for Styles */
short FM_GetBoldWidth(short normalWidth);
short FM_GetItalicWidth(short normalWidth, short height);
short FM_GetShadowWidth(short normalWidth);
short FM_GetOutlineWidth(short normalWidth);
short FM_GetCondensedWidth(short normalWidth);
short FM_GetExtendedWidth(short normalWidth);

/* Combined Style Support */
void FM_SynthesizeStyledChar(short x, short y, char ch, Style face, uint32_t color);
short FM_GetStyledCharWidth(char ch, Style face);

/* String Operations with Styles */
void FM_DrawStyledString(ConstStr255Param s, Style face);
short FM_MeasureStyledString(ConstStr255Param s, Style face);

/* Style Metrics */
short FM_GetStyleExtraWidth(Style face, short baseSize);
short FM_GetStyleExtraHeight(Style face);

#ifdef __cplusplus
}
#endif

#endif /* FONT_STYLE_SYNTHESIS_H */