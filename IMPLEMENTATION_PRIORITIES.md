# System 7 Implementation - Next Priorities

**Last Updated:** November 24, 2025
**Previous Analysis Date:** October 26, 2025
**PowerPC Interpreter Status:** ✅ **COMPLETE** (418 instructions, 99.9% coverage)

**⚠️ MAJOR UPDATE:** Three previously critical priorities are now marked COMPLETE. See "Important Discovery" below for details.

---

## Current State Summary

### What's Working ✅
- **PowerPC Interpreter**: 418 instructions fully implemented (220 base + 11 601 + 13 supervisor + 174 AltiVec)
- **68K Interpreter**: Complete instruction set
- **Segment Loader**: CODE resource loading with lazy JT stubs and relocation
- **Memory Manager**: Handle/pointer allocation with proper master pointer management
- **Event Manager**: Mouse/keyboard events with modifier support
- **Window Manager**: Complete with drag, resize, activation, focus
- **QuickDraw Core**: Drawing primitives, text rendering (Chicago 12), CopyBits with depth conversion
- **QuickDraw Pictures**: PICT format parser with 25+ opcodes (v1 and v2 supported)
- **Dialog Manager**: Modal dialogs with focus rings, keyboard navigation, Cmd-. cancel
- **Control Manager**: Buttons, checkboxes, radio buttons, edit text with focus
- **Keyboard Navigation**: Tab/Shift-Tab traversal, Return/Escape/Space activation
- **Resource Manager**: File opening, closing, and resource chain management
- **File Manager**: Read and write operations with file extension support (with limitations)

### Apps Currently Running
- **SimpleText**: Opens, displays text, supports typing with caret
- **MacPaint**: Launches, displays canvas

### Important Discovery (November 24, 2025)
The three items listed as "CRITICAL PRIORITY" below are **already substantially implemented**. The codebase is further along than the priorities document indicated:

1. **DrawPicture PICT Opcode Parser** - Fully implemented with 25+ opcodes
2. **Resource Manager File Opening** - Fully implemented with proper chain management
3. **File Manager Write Support** - Substantially implemented with one known limitation (pre-allocated extents)

This discovery was made through systematic code investigation of the three priority items. The focus has shifted from implementing these features to completing/testing them.

---

## 🔴 CRITICAL PRIORITY (Blocking App Functionality)

### 1. **DrawPicture PICT Opcode Parser** ✅ ALREADY IMPLEMENTED
**Impact:** HIGH - Many Mac apps use PICT resources for graphics
**Current State:** `src/QuickDraw/quickdraw_pictures.c:261-584` - **COMPREHENSIVE IMPLEMENTATION**
**Status:** COMPLETE with 25+ opcodes supported

**Already Implemented:**
- PICT v1/v2 header parsing (version, bounds rect, frame rate)
- Core opcodes implemented:
  - 0x00: NOP
  - 0x01: Clip region
  - 0x03-0x0D: Font/text setup, pen settings, origin
  - 0x30-0x34: Frame/paint/erase/invert rect
  - 0x40-0x53: Round rects and arcs
  - 0x70-0x71: Polygons
  - 0x90, 0x98: Bitmap and packed bitmap drawing
  - 0xA0-0xA1: Comments (parsed and skipped)
  - 0xFF: End of picture
- Coordinate scaling based on destination rect vs. picture bounds
- QuickDraw primitive routing for all drawing operations

**What's Left:**
- Testing with actual PICT files from Mac apps
- Verifying all opcode handlers work correctly in real scenarios
- Potential edge case handling for malformed PICT data

**Files Modified:**
- `src/QuickDraw/quickdraw_pictures.c` (DrawPicture function)

---

### 2. **Resource Manager - File Opening Support** ✅ ALREADY IMPLEMENTED
**Impact:** HIGH - Apps need to load resources from separate files
**Current State:** `src/ResourceMgr/ResourceMgr.c:501-512, 1060-1109, 1167-1180` - **FULLY IMPLEMENTED**
**Status:** COMPLETE

**Already Implemented:**
- `OpenResFile(filename)` - Opens resource fork, reads header, parses resource map
- `CloseResFile(refNum)` - Closes resource file and updates current context
- `UseResFile(refNum)` - Sets current resource file for searches
- `CurResFile()` - Returns current resource file reference
- Resource chain support with proper search order
- Full resource map parsing (data offset, map offset, type list, name list)

**What's Left:**
- Testing with actual application resource files
- Verifying resource chain search order works correctly
- Testing with multiple open resource files simultaneously
- Potential optimization of resource lookup performance

**Files Modified:**
- `src/ResourceMgr/ResourceMgr.c` (Resource file management)

---

### 3. **File Manager - Write Support** ✅ PARTIALLY IMPLEMENTED
**Impact:** MEDIUM-HIGH - Apps can save documents
**Current State:** `src/FileManager.c:208-225, 419-458, 1079-1120` - **SUBSTANTIALLY IMPLEMENTED**
**Status:** WORKING with limitations

**Already Implemented:**
- `FSWrite(refNum, count, buffer)` - Delegates to `PBWriteSync()` with proper parameter marshaling
- `PBWriteSync()` - Calls `IO_WriteFork()` with position handling and count updates
- `FSSetEOF(refNum, logEOF)` - Calls `Ext_Extend()` and `Ext_Truncate()` for file extension
- `IO_WriteFork()` - Low-level write implementation with:
  - Extent/allocation block mapping
  - Permission checking
  - Physical block writes to disk
  - File position tracking and updates

**Known Limitation:**
- Writes cannot extend files beyond pre-allocated extents
- File extension code at `src/FileManagerStubs.c:1270-1274` prevents automatic growth
- Comment states: "Don't extend files - just write up to physical length"

**What's Left:**
- Enable automatic extent allocation when writing beyond current EOF
- Implement proper allocation block management for file growth
- Test end-to-end save functionality with real applications
- Verify catalog entry updates with modified dates/sizes

**Files Modified:**
- `src/FileManager.c` (FSWrite, FSSetEOF, PBWriteSync)
- `src/FileManagerStubs.c` (IO_WriteFork)

---

## 🟠 HIGH PRIORITY (Major Feature Gaps)

### 4. **Font Manager - Resource-Driven Font Loading** ✅ ALREADY IMPLEMENTED
**Impact:** MEDIUM-HIGH - Multi-font text rendering
**Current State:** `src/FontManager/FontManagerCore.c:334-472` - **FULLY IMPLEMENTED**
**Status:** COMPLETE with comprehensive font resource loading

**Already Implemented:**
- `RealFont(fontID, size)` - Checks cache and loads from resources
- `FM_LoadFontStrike()` - Full pipeline for loading individual font sizes
- FOND resource parsing (font family descriptors):
  - Family ID, style table, size table
  - NFNT resource ID matching for sizes and styles
- NFNT resource loading (bitmap font strikes):
  - Font metrics (ascent, descent, leading, widMax)
  - Offset/Width table (OWT) parsing for character metrics
  - Bitmap table extraction with row word calculation
  - Character range support (firstChar, lastChar)
- Font cache management with LRU eviction
- Multi-size and multi-style support through resource lookup
- Proper error handling with cleanup on failure

**Supported Features:**
- Font family selection by FOND ID
- Size matching with best-fit algorithm
- Style application (normal, bold, italic, etc.)
- Automatic caching of loaded strikes
- Memory-efficient bitmap font handling

**What's Left:**
- Testing with various font resources from System 7
- Verifying style synthesis for unavailable combinations
- Testing edge cases with incomplete font families
- Performance optimization of cache eviction

**Files Modified:**
- `src/FontManager/FontManagerCore.c` (RealFont, FM_LoadFontStrike, caching)
- `src/FontManager/FontResourceLoader.c` (FOND/NFNT parsing)

---

### 5. **Sound Manager - Basic Audio Playback**
**Impact:** MEDIUM - No audio feedback
**Current State:** `src/SoundManager/SoundManagerBareMetal.c:170-178` - Returns `unimpErr`
**Why Important:**
- System beeps/alerts silent
- No application sound effects
- Missing user feedback

**Implementation Plan:**
- Implement PC Speaker backend (simple tones):
  - `SndPlay(sndHdl, async)` - Play tone from 'snd ' resource
  - Parse 'snd ' format 1 (square wave) and format 2 (sampled sound)
  - Program PC speaker (port 0x61 and 0x43 timer)
  - For sampled sounds, downsample to simple tones
- For better quality, implement SB16 backend:
  - Initialize Sound Blaster 16 (detect, set IRQ/DMA)
  - `SndPlay` - Write to DMA buffer and trigger playback
  - Support 8-bit mono at 11025 Hz (good enough for System 7)
- Implement async playback with completion callbacks

**Estimated Effort:** 6-8 hours (PC Speaker: 3-4 hours, SB16: 6-8 hours)
**Files to Modify:**
- `src/SoundManager/SoundManagerBareMetal.c`
- `src/SoundManager/SoundBackend_SB16.c` (if SB16 chosen)

---

### 6. **TextEdit - Styled Text Support**
**Impact:** MEDIUM - Text editing limited
**Current State:** `src/TextEdit/TextEditClipboard.c:311` - Style scrap stubbed
**Why Important:**
- Can't copy/paste styled text between apps
- SimpleText loses formatting
- Word processors need this

**Implementation Plan:**
- Parse 'styl' resource format:
  - Style run table (offset, font, size, face, color)
  - Line height table
- Implement `TESetStyle(mode, style, redraw, hTE)`:
  - Apply style to selection
  - Rebuild style run table
  - Recalculate line breaks
- Implement `TEGetStyle(offset, style, lineHeight, hTE)`:
  - Lookup style at character offset
- Update clipboard operations:
  - `TEToScrap()` - Export 'TEXT' and 'styl' flavors
  - `TEFromScrap()` - Import both flavors
- Update drawing to render multiple styles in one line

**Estimated Effort:** 5-6 hours
**Files to Modify:**
- `src/TextEdit/TextEditClipboard.c`
- `src/TextEdit/TextEditCore.c`
- `src/TextEdit/textedit_core.c`

---

## 🟡 MEDIUM PRIORITY (Polish & Compatibility)

### 7. **Cursor Manager - Hardware Cursor Support**
**Impact:** LOW-MEDIUM - Visual polish
**Current State:** `src/QuickDraw/CursorManager.c:151-177` - Stubs
**Why Useful:**
- Watch cursor for long operations
- I-beam cursor for text
- Hand cursor for clickable items

**Implementation Plan:**
- Implement `SetCursor(cursor)`:
  - Parse CURS resource (16×16 bitmap + mask + hotspot)
  - Update VGA hardware cursor or software cursor
- Implement `InitCursor()` - Reset to arrow
- Implement `ObscureCursor()` - Hide until mouse moves
- Implement `ShowCursor()` / `HideCursor()` - Show/hide with counter

**Estimated Effort:** 3-4 hours
**Files to Modify:**
- `src/QuickDraw/CursorManager.c`
- `src/Platform/x86/vga_cursor.c` (new)

---

### 8. **Control Manager - Font Manager Integration**
**Impact:** LOW-MEDIUM - Better text rendering in controls
**Current State:** `src/ControlManager/StandardControls.c:84` - Hard-coded Chicago 12
**Why Useful:**
- Respect system font setting
- Allow apps to customize control fonts
- Better high-DPI support

**Implementation Plan:**
- Update control drawing to call `GetFontInfo()` for metrics
- Remove hard-coded constants (12, 9, 15)
- Use actual character widths from font
- Update all control types:
  - Buttons (push, default, cancel)
  - Checkboxes and radio buttons
  - Static text
  - Edit text

**Estimated Effort:** 2-3 hours
**Files to Modify:**
- `src/ControlManager/StandardControls.c`
- `src/ControlManager/ControlDrawing.c`

---

### 9. **List Manager - Column Support**
**Impact:** LOW - Feature gap
**Current State:** `src/ListManager/ListManager.c:428-438` - Column APIs stubbed
**Why Useful:**
- Multi-column lists (file listings)
- Spreadsheet-like UIs
- Data tables

**Implementation Plan:**
- Implement `LAddColumn(count, colNum, lHandle)`:
  - Extend column width array
  - Recalculate cell bounds
  - Shift existing data
- Implement `LDelColumn(count, colNum, lHandle)`:
  - Remove column(s)
  - Compact cell data
  - Recalculate bounds

**Estimated Effort:** 2-3 hours
**Files to Modify:**
- `src/ListManager/ListManager.c`

---

## 🟢 LOW PRIORITY (Nice-to-Have)

### 10. **Desktop Pattern Support**
**Current State:** `src/QuickDraw/PatternManager.c:286` - Stub
**Why Useful:** Visual polish, classic Mac look

### 11. **Apple Event Coercion**
**Current State:** `src/AppleEventManager/AppleEventManagerCore.c:541` - Stub
**Why Useful:** Inter-app communication (advanced feature)

### 12. **TextEdit Horizontal Scroll**
**Current State:** `src/TextEdit/TextEditScroll.c:91` - Uncomputed limits
**Why Useful:** Wide text documents

---

## Recommended Implementation Order

**⚠️ MAJOR UPDATE (November 24, 2025):**
The first three "critical" items are already substantially implemented! See details above. Focus has shifted to:

**Phase 1: Complete/Test Existing Functionality (8-12 hours)**
1. File Manager - Enable file extension in writes (2-3 hours)
2. Test DrawPicture with real PICT files (2-3 hours)
3. Test ResourceManager with application files (2-2 hours)
4. Functional testing of save/load workflows (2-4 hours)

**Phase 2: Major Features (15-19 hours)**
5. Font Manager resource loading (4-5 hours)
6. Sound Manager audio playback (6-8 hours)
7. TextEdit styled text (5-6 hours)

**Phase 3: Polish (7-10 hours)**
8. Cursor Manager (3-4 hours)
9. Control Manager font integration (2-3 hours)
10. List Manager columns (2-3 hours)

---

## Success Metrics

After Phase 1 completion:
- ✅ File save/load fully functional for all file sizes
- ✅ PICT graphics rendering verified in real applications
- ✅ Multi-file resource loading confirmed working
- ✅ End-to-end workflows tested (SimpleText save/load, etc.)

After Phase 2 completion:
- ✅ Multiple fonts render correctly at different sizes
- ✅ System sounds and app audio work
- ✅ Styled text editing works

After Phase 3 completion:
- ✅ Cursors change based on context
- ✅ Controls use proper fonts
- ✅ Multi-column lists supported

---

## Quick Start: Testing Phase 1 Features

**Why:** The critical features are already implemented. Focus now on verifying they work correctly.

**Immediate Action Items:**

1. **File Manager Write Support Testing:**
   - Enable automatic file extension in `src/FileManagerStubs.c:IO_WriteFork()`
   - Test creating new files with FSWrite
   - Test extending existing files
   - Verify SimpleText can save edited documents

2. **DrawPicture Testing:**
   - Load PICT resources from MacPaint and other apps
   - Verify all 25+ implemented opcodes work correctly
   - Test edge cases (empty pictures, malformed data)

3. **ResourceManager Testing:**
   - Open application-specific resource files
   - Verify resource chain search works
   - Test UseResFile() and CurResFile() behavior

4. **Integration Testing:**
   - Save and reload documents in SimpleText
   - Verify graphics display correctly in MacPaint
   - Test multi-app resource loading scenarios

**Expected Outcome:** Confirm the three critical features are production-ready.
