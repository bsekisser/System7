/* Font Manager Internal Functions */
#ifndef FONT_INTERNAL_H
#define FONT_INTERNAL_H

#include "SystemTypes.h"

/* Font drawing internals */
void FM_DrawChicagoCharInternal(UInt8 ch, SInt16 x, SInt16 y, GrafPtr port, UInt32 color);
void DrawChar(SInt16 ch);
void DrawString(ConstStr255Param s);
void DrawText(const void *textBuf, SInt16 firstByte, SInt16 byteCount);

/* Font scaling */
SInt16 FM_FindNearestStandardSize(SInt16 requestedSize);
SInt16 FM_GetScaledCharWidth(SInt16 fontNum, SInt16 fontSize, UInt8 ch);
SInt16 FM_GetScaledStringWidth(SInt16 fontNum, SInt16 fontSize, const UInt8* str, SInt16 len);
void FM_SynthesizeSize(SInt16 fontNum, SInt16 fromSize, SInt16 toSize);
void FM_DrawScaledString(SInt16 fontNum, SInt16 fontSize, const UInt8* str, SInt16 len, SInt16 x, SInt16 y);
void FM_GetScaledMetrics(SInt16 fontNum, SInt16 fontSize, FontInfo* info);
Boolean FM_IsSizeAvailable(SInt16 fontNum, SInt16 fontSize);
SInt16 FM_GetAvailableSizes(SInt16 fontNum, SInt16* sizeList, SInt16 maxSizes);
void FM_FlushScaleCache(void);
SInt16 FM_SelectBestSize(SInt16 fontNum, SInt16 requestedSize);
void FM_DrawTextAtSize(const UInt8* text, SInt16 len, SInt16 x, SInt16 y, SInt16 fontSize);

/* Font style synthesis */
void FM_SynthesizeBold(const UInt8* src, UInt8* dest, SInt16 width, SInt16 height);
SInt16 FM_GetBoldWidth(SInt16 baseWidth);
void FM_SynthesizeItalic(const UInt8* src, UInt8* dest, SInt16 width, SInt16 height);
SInt16 FM_GetItalicWidth(SInt16 baseWidth);
void FM_DrawUnderline(SInt16 x, SInt16 y, SInt16 width, SInt16 thickness);
void FM_SynthesizeShadow(const UInt8* src, UInt8* dest, SInt16 width, SInt16 height);
SInt16 FM_GetShadowWidth(SInt16 baseWidth);
void FM_SynthesizeOutline(const UInt8* src, UInt8* dest, SInt16 width, SInt16 height);
SInt16 FM_GetOutlineWidth(SInt16 baseWidth);
SInt16 FM_GetCondensedWidth(SInt16 baseWidth);
SInt16 FM_GetExtendedWidth(SInt16 baseWidth);
void FM_SynthesizeStyledChar(UInt8 ch, Style style, UInt8* dest, SInt16* width, SInt16* height);
SInt16 FM_GetStyledCharWidth(UInt8 ch, Style style);
void FM_DrawStyledString(const UInt8* str, SInt16 len, SInt16 x, SInt16 y, Style style);
SInt16 FM_MeasureStyledString(const UInt8* str, SInt16 len, Style style);
SInt16 FM_GetStyleExtraWidth(Style style);
SInt16 FM_GetStyleExtraHeight(Style style);

#endif /* FONT_INTERNAL_H */
