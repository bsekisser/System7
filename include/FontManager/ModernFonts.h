/*
 * ModernFonts.h - Modern Font Format Support
 *
 * Extended Font Manager support for modern font formats including
 * OpenType, WOFF/WOFF2, system fonts, and web fonts.
 * Maintains Mac OS 7.1 API compatibility while adding modern capabilities.
 */

#ifndef MODERN_FONTS_H
#define MODERN_FONTS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "FontTypes.h"
#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Modern Font Initialization */
OSErr InitializeModernFontSupport(void);
void CleanupModernFontSupport(void);

/* OpenType Font Support */
OSErr LoadOpenTypeFont(ConstStr255Param filePath, OpenTypeFont **font);
OSErr UnloadOpenTypeFont(OpenTypeFont *font);
OSErr ParseOpenTypeFont(const void *fontData, unsigned long dataSize, OpenTypeFont **font);
OSErr GetOpenTypeFontInfo(OpenTypeFont *font, char **familyName, char **styleName);
OSErr GetOpenTypeGlyphMetrics(OpenTypeFont *font, unsigned short glyphID,
                              short *advanceWidth, short *leftSideBearing);
OSErr GetOpenTypeKerning(OpenTypeFont *font, unsigned short leftGlyph,
                         unsigned short rightGlyph, short *kerning);

/* WOFF Font Support */
OSErr LoadWOFFFont(ConstStr255Param filePath, WOFFFont **font);
OSErr LoadWOFF2Font(ConstStr255Param filePath, WOFFFont **font);
OSErr UnloadWOFFFont(WOFFFont *font);
OSErr DecompressWOFF(const void *woffData, unsigned long woffSize,
                     void **otfData, unsigned long *otfSize);
OSErr DecompressWOFF2(const void *woff2Data, unsigned long woff2Size,
                      void **otfData, unsigned long *otfSize);
OSErr ValidateWOFFFont(const void *woffData, unsigned long dataSize);

/* System Font Support */
OSErr InitializeSystemFontSupport(void);
OSErr ScanSystemFontDirectories(void);
OSErr LoadSystemFont(ConstStr255Param fontName, SystemFont **font);
OSErr UnloadSystemFont(SystemFont *font);
OSErr GetSystemFontPath(ConstStr255Param fontName, char *filePath, unsigned long maxPath);
OSErr RegisterSystemFontPath(ConstStr255Param directoryPath);
OSErr GetAvailableSystemFonts(char ***fontNames, unsigned long *count);

/* Font Collection Support */
OSErr LoadFontCollection(ConstStr255Param filePath, FontCollection **collection);
OSErr UnloadFontCollection(FontCollection *collection);
OSErr GetFontFromCollection(FontCollection *collection, short index, ModernFont **font);
OSErr GetCollectionFontCount(FontCollection *collection, short *count);
OSErr GetCollectionFontInfo(FontCollection *collection, unsigned long index,
                           char **familyName, char **styleName);

/* Font Directory Management */
OSErr InitializeFontDirectory(void);
OSErr RefreshFontDirectory(void);
OSErr AddFontToDirectory(ConstStr255Param filePath);
OSErr RemoveFontFromDirectory(ConstStr255Param filePath);
OSErr FindFontInDirectory(ConstStr255Param familyName, ConstStr255Param styleName,
                         FontDirectoryEntry **entry);
OSErr GetFontDirectory(FontDirectory **directory);

/* Modern Font Detection and Validation */
unsigned short DetectFontFormat(const void *fontData, unsigned long dataSize);
OSErr ValidateFontFile(ConstStr255Param filePath);
OSErr GetFontFileInfo(ConstStr255Param filePath, unsigned short *format,
                     char **familyName, char **styleName);
Boolean IsModernFontFormat(unsigned short format);

/* Font Format Conversion */
OSErr ConvertToOpenType(const void *sourceData, unsigned long sourceSize,
                       unsigned short sourceFormat, void **otfData, unsigned long *otfSize);
OSErr ExtractFontMetrics(ModernFont *font, FMetricRec *metrics);
OSErr CreateMacFontRecord(ModernFont *font, FontRec **fontRec);

/* Web Font Support */
OSErr DownloadWebFont(ConstStr255Param url, ConstStr255Param cachePath, short *familyID);
OSErr LoadWebFont(ConstStr255Param filePath, WebFontMetadata *metadata, ModernFont **font);
OSErr ParseWebFontCSS(ConstStr255Param cssPath, WebFontMetadata **metadataArray,
                     unsigned long *count);
OSErr ValidateWebFont(ConstStr255Param filePath);

/* Font Rendering Support */
OSErr RenderModernGlyph(ModernFont *font, unsigned short glyphID, unsigned short size,
                       void **bitmap, unsigned long *bitmapSize, short *width, short *height);
OSErr GetModernGlyphOutline(ModernFont *font, unsigned short glyphID, unsigned short size,
                           void **outline, unsigned long *outlineSize);
OSErr ApplyFontHinting(ModernFont *font, Boolean enableHinting);
OSErr SetFontRenderingOptions(ModernFont *font, unsigned short options);

/* Variable Font Support */
OSErr IsVariableFont(ModernFont *font, Boolean *isVariable);
OSErr GetVariableAxes(ModernFont *font, void **axes, unsigned long *axisCount);
OSErr SetVariableInstance(ModernFont *font, const void *coordinates, unsigned long coordCount);
OSErr GetNamedInstances(ModernFont *font, void **instances, unsigned long *instanceCount);

/* Font Subsetting and Optimization */
OSErr CreateFontSubset(ModernFont *font, const unsigned short *glyphIDs,
                      unsigned long glyphCount, ModernFont **subset);
OSErr OptimizeFontForWeb(ModernFont *font, ModernFont **optimized);
OSErr CompressFontToWOFF2(ModernFont *font, void **woff2Data, unsigned long *woff2Size);

/* Platform-Specific Integration */
#ifdef PLATFORM_REMOVED_APPLE
OSErr LoadCoreTextFont(ConstStr255Param fontName, SystemFont **font);
OSErr GetCoreTextFontDescriptor(SystemFont *font, void **descriptor);
#endif

#ifdef PLATFORM_REMOVED_WIN32
OSErr LoadDirectWriteFont(ConstStr255Param fontName, SystemFont **font);
OSErr GetDirectWriteFontFace(SystemFont *font, void **fontFace);
#endif

#ifdef PLATFORM_REMOVED_LINUX
OSErr LoadFontconfigFont(ConstStr255Param fontName, SystemFont **font);
OSErr GetFontconfigPattern(SystemFont *font, void **pattern);
#endif

/* Modern Font Constants */

/* Error codes for modern font support */

/* Font Loading Options */

/* Modern Font Cache */

/* Font Matching Criteria */

/* Advanced Font Operations */
OSErr FindBestFontMatch(FontMatchCriteria *criteria, ModernFont **font);
OSErr GetFontSimilarity(ModernFont *font1, ModernFont *font2, float *similarity);
OSErr CreateFontFallbackChain(ConstStr255Param familyName, ModernFont ***chain,
                             unsigned long *chainLength);
OSErr GetFontCapabilities(ModernFont *font, unsigned long *capabilities);

/* Font Capabilities Flags */

#ifdef __cplusplus
}
#endif

#endif /* MODERN_FONTS_H */
