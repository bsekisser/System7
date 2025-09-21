/*
 * TrueTypeFonts.h - TrueType Font Support APIs
 *
 * Support for TrueType outline fonts including parsing, scaling,
 * rasterization, and hinting.
 */

#ifndef TRUETYPE_FONTS_H
#define TRUETYPE_FONTS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "FontTypes.h"
#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TrueType Constants */

/* TrueType Table Tags */

/* TrueType Structures */

/* TrueType Font Loading */
OSErr LoadTrueTypeFont(short fontID, TTFont **font);
OSErr LoadTrueTypeFontFromResource(Handle sfntResource, TTFont **font);
OSErr LoadTrueTypeFontFromFile(ConstStr255Param filePath, TTFont **font);
OSErr UnloadTrueTypeFont(TTFont *font);

/* SFNT Resource Parsing */
OSErr ParseSFNTResource(Handle resource, TTFont **font);
OSErr ValidateSFNTResource(Handle resource);
OSErr GetSFNTTableDirectory(TTFont *font, unsigned long **tags, short *count);
OSErr GetSFNTTable(TTFont *font, unsigned long tag, void **table, long *size);

/* TrueType Table Access */
OSErr GetHeadTable(TTFont *font, TTHeader **head);
OSErr GetHorizontalHeaderTable(TTFont *font, TTHorizontalHeader **hhea);
OSErr GetMaxProfileTable(TTFont *font, TTMaxProfile **maxp);
OSErr GetCharacterMapTable(TTFont *font, void **cmap, long *size);
OSErr GetGlyphDataTable(TTFont *font, void **glyf, long *size);
OSErr GetLocationTable(TTFont *font, void **loca, long *size);
OSErr GetHorizontalMetricsTable(TTFont *font, void **hmtx, long *size);

/* Character Mapping */
OSErr MapCharacterToGlyph(TTFont *font, unsigned short character, unsigned short *glyphIndex);
OSErr MapGlyphToCharacter(TTFont *font, unsigned short glyphIndex, unsigned short *character);
OSErr GetCharacterEncoding(TTFont *font, short *platformID, short *encodingID);
OSErr GetSupportedCharacters(TTFont *font, unsigned short **characters, short *count);

/* Glyph Access */
OSErr GetGlyph(TTFont *font, unsigned short glyphIndex, TTGlyph **glyph);
OSErr GetGlyphMetrics(TTFont *font, unsigned short glyphIndex,
                      short *advanceWidth, short *leftSideBearing);
OSErr GetGlyphBounds(TTFont *font, unsigned short glyphIndex, Rect *bounds);
OSErr UnloadGlyph(TTGlyph *glyph);

/* Glyph Outline Processing */
OSErr GetGlyphOutline(TTFont *font, unsigned short glyphIndex,
                      Point **points, unsigned char **flags, unsigned short **contours,
                      short *numPoints, short *numContours);
OSErr ScaleGlyphOutline(Point *points, short numPoints, Fixed scaleX, Fixed scaleY);
OSErr TransformGlyphOutline(Point *points, short numPoints, Fixed matrix[4]);

/* Font Scaling and Rasterization */
OSErr ScaleTrueTypeFont(TTFont *font, short pointSize, short dpi, TTFont **scaledFont);
OSErr RasterizeGlyph(TTFont *font, unsigned short glyphIndex, short pointSize, short dpi,
                     void **bitmap, short *width, short *height, short *rowBytes);
OSErr RasterizeCharacter(TTFont *font, unsigned short character, short pointSize, short dpi,
                        void **bitmap, short *width, short *height, short *rowBytes);

/* Font Hinting */
OSErr ExecuteGlyphInstructions(TTFont *font, TTGlyph *glyph, short pointSize, short dpi);
OSErr GetInstructionTables(TTFont *font, void **cvt, void **fpgm, void **prep,
                          long *cvtSize, long *fpgmSize, long *prepSize);
OSErr InitializeTrueTypeInterpreter(TTFont *font);
OSErr ExecuteFontProgram(TTFont *font);
OSErr ExecuteControlValueProgram(TTFont *font);

/* Font Metrics */
OSErr GetTrueTypeFontMetrics(TTFont *font, FMetricRec *metrics);
OSErr GetTrueTypeFontBounds(TTFont *font, Rect *bounds);
OSErr GetStringMetrics(TTFont *font, const unsigned short *text, short length,
                       short pointSize, short *width, short *height, Rect *bounds);

/* Font Information */
OSErr GetTrueTypeFontName(TTFont *font, short nameID, Str255 name);
OSErr GetTrueTypeFontFamily(TTFont *font, Str255 familyName);
OSErr GetTrueTypeFontStyle(TTFont *font, Str255 styleName);
OSErr GetTrueTypeFontVersion(TTFont *font, Fixed *version);
OSErr GetTrueTypeFontUnitsPerEm(TTFont *font, unsigned short *unitsPerEm);

/* Composite Glyph Support */
OSErr IsCompositeGlyph(TTFont *font, unsigned short glyphIndex, Boolean *isComposite);
OSErr GetCompositeComponents(TTFont *font, unsigned short glyphIndex,
                            unsigned short **components, short *count);
OSErr DecomposeGlyph(TTFont *font, unsigned short glyphIndex, TTGlyph **decomposedGlyph);

/* Font Validation */
OSErr ValidateTrueTypeFont(TTFont *font);
OSErr CheckTrueTypeFontIntegrity(TTFont *font, Boolean *isValid, char **errorMessage);
OSErr ValidateTrueTypeTable(void *table, unsigned long tag, long size);
OSErr CalculateTableChecksum(void *table, long size, unsigned long *checksum);

/* Font Conversion */
OSErr ConvertTrueTypeToBitmap(TTFont *font, short pointSize, short dpi,
                             BitmapFontData **bitmapFont);
OSErr ExtractBitmapFromTrueType(TTFont *font, short pointSize, short dpi,
                               short firstChar, short lastChar, BitmapFontData **bitmapFont);

/* Advanced Features */
OSErr GetKerningData(TTFont *font, KernPair **pairs, short *count);
OSErr GetGlyphClass(TTFont *font, unsigned short glyphIndex, short *glyphClass);
OSErr GetLanguageSupport(TTFont *font, short **languageCodes, short *count);

/* Memory Management */
OSErr AllocateTrueTypeFont(TTFont **font);
OSErr DisposeTrueTypeFont(TTFont *font);
OSErr CompactTrueTypeFont(TTFont *font);

/* Platform Integration */
OSErr LoadSystemTrueTypeFont(ConstStr255Param fontName, TTFont **font);
OSErr GetSystemTrueTypeFonts(Str255 **fontNames, short *count);
OSErr RegisterTrueTypeFontFile(ConstStr255Param filePath, short *fontID);

#ifdef __cplusplus
}
#endif

#endif /* TRUETYPE_FONTS_H */
