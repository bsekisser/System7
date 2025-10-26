# System 7 Implementation - Next Priorities

**Analysis Date:** October 26, 2025
**PowerPC Interpreter Status:** ✅ **COMPLETE** (418 instructions, 99.9% coverage)

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
- **Dialog Manager**: Modal dialogs with focus rings, keyboard navigation, Cmd-. cancel
- **Control Manager**: Buttons, checkboxes, radio buttons, edit text with focus
- **Keyboard Navigation**: Tab/Shift-Tab traversal, Return/Escape/Space activation

### Apps Currently Running
- **SimpleText**: Opens, displays text, supports typing with caret
- **MacPaint**: Launches, displays canvas

---

## 🔴 CRITICAL PRIORITY (Blocking App Functionality)

### 1. **DrawPicture PICT Opcode Parser**
**Impact:** HIGH - Many Mac apps use PICT resources for graphics
**Current State:** `src/QuickDraw/quickdraw_pictures.c:44-96` - Just frames destination rect, doesn't parse opcodes
**Why Critical:**
- Icons, splash screens, toolbars all use PICT format
- Finder desktop patterns stored as PICT
- Many app UIs broken without this

**Implementation Plan:**
- Parse PICT v1/v2 header (version, bounds rect)
- Implement core opcodes:
  - 0x0001: Clip region
  - 0x0011-0x001F: Font/text setup
  - 0x0020-0x0027: Line/rect/oval drawing
  - 0x0028-0x002B: Same frame/paint/erase
  - 0x0030-0x0037: Frame/fill arc, polygon
  - 0x0090-0x0098: BitsRect/BitsRgn (bitmap drawing)
  - 0x00A1: Short comment (skip)
  - 0x00FF: End of picture
- Scale coordinates based on destination rect vs. picture bounds
- Route drawing calls through existing QuickDraw primitives

**Estimated Effort:** 4-6 hours
**Files to Modify:**
- `src/QuickDraw/quickdraw_pictures.c`

---

### 2. **Resource Manager - File Opening Support**
**Impact:** HIGH - Apps can't load resources from separate files
**Current State:** `src/ResourceMgr/ResourceMgr.c:944` - `OpenResFile` is stub
**Why Critical:**
- Apps need to open their own resource fork
- Desk accessories stored in separate resource files
- Extensions/control panels are resource files

**Implementation Plan:**
- Implement `OpenResFile(filename)`:
  - Call HFS to open file's resource fork
  - Parse resource map header (data offset, map offset, type list, name list)
  - Build in-memory resource map structure
  - Add to resource chain (search order)
- Implement `CloseResFile(refNum)`:
  - Flush any changes (if write support added)
  - Remove from chain
  - Free memory
- Support resource chain search (current file → app file → system file)

**Related APIs to implement:**
- `UseResFile(refNum)` - Set current resource file
- `CurResFile()` - Get current resource file refNum
- `Count1Resources(type)` - Count in current file only
- `Get1IndResource(type, index)` - Get indexed resource from current file

**Estimated Effort:** 3-4 hours
**Files to Modify:**
- `src/ResourceMgr/ResourceMgr.c`
- `include/ResourceMgr.h`

---

### 3. **File Manager - Write Support**
**Impact:** MEDIUM-HIGH - Apps can't save documents
**Current State:** `src/FS/vfs.c:724` - Write operations are stubs
**Why Critical:**
- SimpleText can't save edited files
- MacPaint can't save drawings
- Preferences can't be persisted

**Implementation Plan:**
- Implement `FSWrite(refNum, count, buffer)`:
  - Locate file control block
  - Write data to current position
  - Update file position
  - Update EOF if extended
  - Mark buffer dirty
- Implement `SetEOF(refNum, logEOF)`:
  - Allocate/deallocate extents as needed
  - Update catalog entry
- Implement `FlushFile(refNum)`:
  - Write dirty buffers to disk
  - Update catalog with new sizes/dates
- Support extent allocation for growing files
- Update HFS catalog entries (modify date, data/resource fork sizes)

**Estimated Effort:** 6-8 hours
**Files to Modify:**
- `src/FS/vfs.c`
- `src/FS/hfs_file.c`
- `src/FileMgr/file_manager.c`

---

## 🟠 HIGH PRIORITY (Major Feature Gaps)

### 4. **Font Manager - Resource-Driven Font Loading**
**Impact:** MEDIUM-HIGH - Limited text rendering quality
**Current State:** `docs/components/FontManager/README.md` - Only Chicago 12 available
**Why Important:**
- Geneva, Monaco, Courier not rendering correctly
- No bold/italic/different sizes
- Limits UI appearance

**Implementation Plan:**
- Parse FOND resources (font family descriptors):
  - Family ID, style table, size table
  - NFNT resource IDs for each size/style
- Parse NFNT resources (bitmap font strikes):
  - Font metrics (ascent, descent, width table)
  - Bitmap table (glyph images)
  - Kern table (optional)
- Build font cache keyed by (familyID, size, style)
- Update `RealFont(fontID, size)` to load from resources
- Update `GetFontInfo()` to return actual metrics

**Estimated Effort:** 4-5 hours
**Files to Modify:**
- `src/FontManager/FontResourceLoader.c`
- `src/FontManager/FontManagerCore.c`

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

**Phase 1: Critical App Functionality (12-18 hours)**
1. DrawPicture PICT parser (4-6 hours) ← **START HERE**
2. Resource Manager file opening (3-4 hours)
3. File Manager write support (6-8 hours)

**Phase 2: Major Features (15-19 hours)**
4. Font Manager resource loading (4-5 hours)
5. Sound Manager audio playback (6-8 hours)
6. TextEdit styled text (5-6 hours)

**Phase 3: Polish (7-10 hours)**
7. Cursor Manager (3-4 hours)
8. Control Manager font integration (2-3 hours)
9. List Manager columns (2-3 hours)

---

## Success Metrics

After Phase 1 completion:
- ✅ Apps can display icons and graphics correctly
- ✅ Apps can open their own resource files
- ✅ Apps can save documents

After Phase 2 completion:
- ✅ Multiple fonts render correctly at different sizes
- ✅ System sounds and app audio work
- ✅ Styled text editing works

After Phase 3 completion:
- ✅ Cursors change based on context
- ✅ Controls use proper fonts
- ✅ Multi-column lists supported

---

## Quick Start: Implement DrawPicture Next

**Why:** Highest impact for visual app compatibility. Many Mac apps rely on PICT resources for UI elements.

**Action Items:**
1. Read PICT v1/v2 spec (inside Macintosh: Imaging with QuickDraw)
2. Implement opcode parser loop in `DrawPicture()`
3. Add handlers for top 10-15 most common opcodes
4. Test with MacPaint and SimpleText icons
5. Gradually add more opcodes as needed

**Expected Outcome:** Apps will display icons, splash screens, and toolbar graphics correctly.
