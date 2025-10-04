# Font Manager Implementation

## Overview

System 7.1-compatible Font Manager implementation providing bitmap font support with Chicago font integration.

## Current Implementation Status

### Phase 1: Core Architecture ✓

**Completed:**
- FontTypes.h: Type definitions compatible with System 7.1
- FontManager.h: Public API declarations
- FontManagerCore.c: Core implementation with Chicago 12 support

**Features:**
- InitFonts(): Initializes Font Manager with Chicago, Geneva, Monaco families
- GetFontName()/GetFNum(): Font family name/ID lookups
- RealFont(): Reports non-synthesized fonts (Chicago 12 only currently)
- TextFont()/TextFace()/TextSize()/TextMode(): GrafPort font state management
- GetFontMetrics(): Returns font metrics (ascent, descent, leading, widMax)
- CharWidth()/StringWidth()/TextWidth(): Text measurement functions
- FMSwapFont(): Font selection with style parameters

### Phase 2: Resource Loading ✓

**Completed:**
- FontResources.h: FOND/NFNT resource structures
- FontResourceLoader.c: Resource parsing implementation

**Features:**
- FM_LoadFONDResource(): Parse font family resources
- FM_LoadNFNTResource(): Parse bitmap font resources
- FM_ParseOWTTable(): Extract offset/width tables
- FM_BuildWidthTable(): Generate width lookup tables
- FM_ExtractBitmap(): Extract font bitmaps
- FM_FindBestMatch(): Find optimal font for size/style
- Resource validation functions
- Debug dump functions

### Integration Points

**With QuickDraw:**
- Updates g_currentPort font fields (txFont, txFace, txSize, txMode)
- Integrates with existing ChicagoRealFont.c for rendering
- Provides FM_DrawRun() for text drawing

**With Window Manager:**
- Window titles use Font Manager metrics
- DrawString() delegates to Chicago font implementation

### Font Data

**Chicago Font:**
- Source: chicago_font.h (actual System 7.1 NFNT data)
- Size: 12 point only (bitmap)
- Metrics:
  - Ascent: 10 pixels (CHICAGO_ASCENT)
  - Descent: 3 pixels (CHICAGO_DESCENT)
  - Leading: 3 pixels (CHICAGO_LEADING)
  - Character range: 32-126 (ASCII printable)
- Character spacing: bit_width + 2 pixels (corrected from original)
- Space character: additional +3 pixels

### Style Synthesis

**Planned (not yet implemented):**
- Bold: +1 pixel horizontal emboldening
- Italic: 1:4 shear (~14 degrees)
- Underline: 1px at descent/2 from baseline
- Shadow: 1px offset right and down
- Outline: 1px stroke around glyph
- Condense: -10% horizontal spacing
- Extend: +10% horizontal spacing

### Caching Strategy

**Current:**
- Single static strike: Chicago 12
- No dynamic allocation

**Future:**
- LRU cache of FontStrike structures
- Dynamic strike synthesis on demand
- Memory limit: 256KB per strike

### Testing

Test program (test_fontmgr.c) validates:
1. Font family enumeration
2. Name/ID mapping
3. Metrics retrieval
4. Width measurements
5. Style modifications (bold)
6. FMSwapFont operation

### Known Limitations

1. Only Chicago 12 is fully implemented
2. No runtime FOND/NFNT resource loading (structures ready)
3. No TrueType support
4. Geneva and Monaco are registered but use Chicago metrics
5. Style synthesis uses simplified algorithms (not full bitmap transforms)

### Phase 3: Style Synthesis ✓

**Completed:**
- FontStyleSynthesis.h/c: Complete style synthesis implementation

**Features:**
- FM_SynthesizeBold(): Horizontal emboldening (+1 pixel)
- FM_SynthesizeItalic(): 1:4 shear ratio (~14 degrees)
- FM_SynthesizeUnderline(): Line at descent/2 from baseline
- FM_SynthesizeShadow(): 1px offset right and down
- FM_SynthesizeOutline(): 1px stroke around glyph
- FM_GetCondensedWidth(): -10% horizontal spacing
- FM_GetExtendedWidth(): +10% horizontal spacing
- Combined style support for multiple effects
- Width calculations for all styles

### Next Steps

**Phase 4: Multiple Sizes**
- 9, 10, 12, 14, 18, 24 point strikes
- Nearest-neighbor scaling for missing sizes
- Strike cache management

**Phase 5: Performance**
- Width table caching
- Optimized glyph blitting
- Memory management improvements

### API Compatibility

The implementation follows Mac OS System 7.1 Font Manager APIs:
- All Pascal strings (length byte first)
- Fixed-point (16.16) arithmetic where needed
- MacRoman encoding (no Unicode)
- Compatible error codes

### Debug Support

Serial output with "FM:" prefix (whitelisted in System71StdLib.c)
Enable with FM_DEBUG=1 in FontManagerCore.c