/*
 * test_fontmgr.c - Font Manager Test Program
 * Tests the core Font Manager APIs with Chicago font
 */

#include "FontManager/FontManager.h"
#include "FontManager/FontTypes.h"
#include "QuickDraw/QuickDraw.h"

extern void serial_printf(const char* fmt, ...);

void TestFontManager(void);

void TestFontManager(void) {
    Str255 fontName;
    short familyID;
    FMetricRec metrics;

    serial_printf("FM: === Font Manager Test Suite ===\n");

    /* Initialize Font Manager */
    InitFonts();
    serial_printf("FM: InitFonts() complete\n");

    /* Test 1: Get font names */
    GetFontName(chicagoFont, fontName);
    serial_printf("FM: Font ID 0 name: %.*s\n", fontName[0], &fontName[1]);

    GetFontName(genevaFont, fontName);
    serial_printf("FM: Font ID 3 name: %.*s\n", fontName[0], &fontName[1]);

    /* Test 2: Get font ID from name */
    unsigned char chicagoStr[] = {7, 'C','h','i','c','a','g','o'};
    GetFNum(chicagoStr, &familyID);
    serial_printf("FM: 'Chicago' -> ID %d\n", familyID);

    /* Test 3: Check real font */
    Boolean isReal = RealFont(chicagoFont, 12);
    serial_printf("FM: Chicago 12 is real: %s\n", isReal ? "YES" : "NO");

    isReal = RealFont(chicagoFont, 14);
    serial_printf("FM: Chicago 14 is real: %s\n", isReal ? "YES" : "NO");

    /* Test 4: Set font and get metrics */
    TextFont(chicagoFont);
    TextSize(12);
    TextFace(normal);

    GetFontMetrics(&metrics);
    serial_printf("FM: Chicago 12 metrics:\n");
    serial_printf("FM:   Ascent: %d\n", metrics.ascent);
    serial_printf("FM:   Descent: %d\n", metrics.descent);
    serial_printf("FM:   Leading: %d\n", metrics.leading);
    serial_printf("FM:   WidMax: %d\n", metrics.widMax);

    /* Test 5: Measure text widths */
    unsigned char testStr1[] = {8, 'S','y','s','t','e','m',' ','7'};
    short width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' = %d pixels\n", width);

    unsigned char testStr2[] = {20, 'A','b','o','u','t',' ','T','h','i','s',' ','M','a','c','i','n','t','o','s','h'};
    width = StringWidth(testStr2);
    serial_printf("FM: Width of 'About This Macintosh' = %d pixels\n", width);

    /* Test 6: Character widths */
    serial_printf("FM: Character widths:\n");
    serial_printf("FM:   'A' = %d pixels\n", CharWidth('A'));
    serial_printf("FM:   'W' = %d pixels\n", CharWidth('W'));
    serial_printf("FM:   'i' = %d pixels\n", CharWidth('i'));
    serial_printf("FM:   ' ' = %d pixels\n", CharWidth(' '));

    /* Test 7: Bold face */
    TextFace(bold);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (bold) = %d pixels\n", width);

    /* Test 7b: Italic face */
    TextFace(italic);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (italic) = %d pixels\n", width);

    /* Test 7c: Bold+Italic */
    TextFace(bold | italic);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (bold+italic) = %d pixels\n", width);

    /* Test 7d: Underline */
    TextFace(underline);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (underline) = %d pixels\n", width);

    /* Test 7e: Shadow */
    TextFace(shadow);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (shadow) = %d pixels\n", width);

    /* Test 7f: Outline */
    TextFace(outline);
    width = StringWidth(testStr1);
    serial_printf("FM: Width of 'System 7' (outline) = %d pixels\n", width);

    /* Test 8: FMSwapFont */
    FMInput input = {chicagoFont, 12, normal, 0, 1, 1}; /* FALSE = 0 */
    FMOutPtr output = FMSwapFont(&input);
    if (output) {
        serial_printf("FM: FMSwapFont returned:\n");
        serial_printf("FM:   errNum: %d\n", output->errNum);
        serial_printf("FM:   ascent: %d\n", output->ascent);
        serial_printf("FM:   descent: %d\n", output->descent);
        serial_printf("FM:   widMax: %d\n", output->widMax);
    }

    serial_printf("FM: === Font Manager Tests Complete ===\n");
}