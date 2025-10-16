# MacPaint 1.3 Conversion to C for System 7.1

## Overview

This document describes the conversion of MacPaint 1.3 (original source by Bill Atkinson) from Pascal/68k assembly to C for System 7.1 Portable on ARM and x86 architectures.

**Source Statistics:**
- Original: ~9,300 lines of code
  - MacPaint.p: 5,804 lines of Pascal
  - PaintAsm.a: 2,738 lines of 68k assembly
  - MyHeapAsm.a: 159 lines of memory manager stubs
  - MyTools.a: 631 lines of toolbox trap definitions
  - MacPaint.rsrc: Resource file

- Converted: ~2,000 lines of C (with further implementation needed)

## Conversion Strategy

### Phase 1: Foundation (COMPLETED)

1. **Core Application Structure**
   - MacPaint_Core.c: ~600 lines - Core application logic
   - MacPaint_Main.c: ~300 lines - Application entry point and event loop
   - MacPaint.h: ~100 lines - Public API

2. **Assembly Conversion**

   a) **Memory Manager (MyHeapAsm.a)** → Direct System 7.1 Memory Manager calls
   ```
   68k Assembly           C Conversion
   ─────────────────────────────────────
   _MoreMasters trap  →  MoreMasters()
   _FreeMem trap      →  FreeMem()
   _MaxMem trap       →  MaxMem()
   _NewHandle trap    →  NewHandle()
   _DisposHandle trap →  DisposeHandle()
   BSET #LOCK,(A0)    →  HLock()
   BCLR #LOCK,(A0)    →  HUnlock()
   ```

   b) **Toolbox Traps (MyTools.a)** → QuickDraw and Toolbox calls
   ```
   68k Trap Code      Tool              C Conversion
   ─────────────────────────────────────────────────
   $AC50              InitCurs          InitCursor()
   $AC51              SetCursor         SetCursor()
   $AC52              HideCursor        HideCursor()
   $AC53              ShowCursor        ShowCursor()
   $A800-$A8FF        QuickDraw         QDGlobals, CopyBits()
   $A900-$A9FF        Toolbox           Window, Menu, Dialog managers
   ```

   c) **Painting Algorithms (PaintAsm.a)** → C implementations
   ```
   68k Procedure        Description              C Function
   ──────────────────────────────────────────────────────────
   PixelTrue            Bit extraction           MacPaint_PixelTrue()
   SetPixel/ClearPixel  Bit manipulation         MacPaint_SetPixel()
   InvertBuf            XOR all bits             MacPaint_InvertBuf()
   ZeroBuf              Clear buffer             MacPaint_ZeroBuf()
   ExpandPat            Pattern expansion        MacPaint_ExpandPattern()
   DrawBrush            Brush drawing            MacPaint_DrawBrush()
   PackBits/UnpackBits  RLE compression          MacPaint_PackBits()
   CalcEdges            Edge detection           MacPaint_CalcEdges()
   ```

### Phase 2: Tools and Drawing (TODO)

Implement drawing tools:
- Pencil: Direct pixel drawing
- Brush: Pattern-based drawing
- Line: Bresenham's line algorithm (basic version complete)
- Rectangle: Rectangle outline
- Filled Rectangle: Pattern fill
- Oval: Circle algorithm
- Lasso: Freeform selection
- Text: Text insertion

### Phase 3: File I/O and Documents (TODO)

Implement document operations:
- New: Create blank document
- Open: Read MacPaint format files
- Save: Write MacPaint format files
- Save As: Save with new name
- Revert: Reload from disk

### Phase 4: Menus and Events (TODO)

Implement application menus:
- File: New, Open, Close, Save, Save As, Print, Quit
- Edit: Undo, Cut, Copy, Paste, Clear, Invert, Fill
- Aids: Grid, Fat Bits, Pattern Editor, Brush Editor, Help
- Font: Font selection
- Size: Font size
- Style: Text formatting

### Phase 5: Advanced Features (TODO)

- Undo/Redo system
- Selection clipboard
- Pattern and brush editors
- Printing support
- Scrap file integration

## Key Design Decisions

### 1. Binary Compatibility

**Not Attempted**: Full binary compatibility with original MacPaint files
**Rationale**: System 7.1 portable on ARM/x86 has different memory layout and may use different resource formats

**Approach**:
- Support reading classic MacPaint format files
- Write in same format for compatibility
- Resource fork handling through System 7.1 Resource Manager

### 2. Bit Manipulation

**Original**: Direct 68k bit operations (BSET, BCLR, BTST)
**Converted**: Explicit C bit manipulation

```c
// Original 68k: BSET #LOCK,(A0)
// Converted to:
((unsigned char *)ptr)[byteOffset] |= (1 << bitOffset);

// Original 68k: BCLR #PURGE,(A0)
// Converted to:
((unsigned char *)ptr)[byteOffset] &= ~(1 << bitOffset);
```

### 3. QuickDraw Integration

**Original**: Direct QuickDraw ROM traps
**Converted**: System 7.1 QuickDraw API

```c
// Original: _SetCursor trap ($A851)
// Converted: SetCursor(cursorHandle)

// Original: CopyBits ROM
// Converted: CopyBits(&srcBits, &dstBits, &srcRect, &dstRect, mode, maskRgn)
```

### 4. Pascal-to-C Conversions

| Pascal Construct | C Equivalent |
|-----------------|--------------|
| PROGRAM MacPaint | int MacPaintMain(int argc, char **argv) |
| PROCEDURE name | void name() or OSErr name() |
| FUNCTION name | Type name() |
| VAR | static Type var or local variable |
| CONST | #define or static const |
| TYPE record | struct typedef |
| String[255] | char[256] or Str255 |
| Boolean | boolean or int |
| LongInt | LongInt (32-bit) |
| ARRAY[0..N] | arrays with explicit size |

### 5. Memory Management

**Original**: Handles via Memory Manager with HLock/HUnlock
**Converted**: Pointers with explicit management

```c
// Original Pascal:
// VAR h: Handle;
// h := NewHandle(1024);
// HLock(h);
// ptr := Ptr(h^);

// Converted to:
// Ptr ptr = NewPtr(1024);
// No locking needed in System 7.1
```

## Pascal Source Analysis

### Main Procedures (from MacPaint.p)

1. **Document Management**
   - NewDoc: Create new document
   - OpenDoc: Open document from file
   - SaveDoc: Save document to file
   - RevertDoc: Reload from disk

2. **Tool Management**
   - SelectTool: Choose active tool
   - SetLineSize: Set brush/pen width
   - SetPattern: Choose fill pattern

3. **Drawing Operations**
   - PaintLine: Draw line
   - PaintRect: Draw rectangle
   - PaintOval: Draw oval
   - PaintBrush: Brush strokes
   - PaintFill: Flood fill

4. **UI Operations**
   - HandleMouseDown: Mouse click in canvas
   - HandleMouseDrag: Mouse movement with button
   - HandleMouseUp: Mouse button release
   - HandleKeyDown: Keyboard input

5. **Edit Operations**
   - DoCut: Cut selection to scrap
   - DoCopy: Copy selection to scrap
   - DoPaste: Paste from scrap
   - DoClear: Delete selection
   - DoInvert: Invert selection colors

### Type Definitions

Key data structures from MacPaint:

```pascal
QueueEntry = RECORD
  addr: LongInt;      (* bitmap pointer *)
  bump: INTEGER;      (* +2 or -2 *)
  twoH: INTEGER;      (* 2 * horizontal coord *)
  mask: INTEGER;      (* pattern mask *)
END;
```

Converted to C:
```c
struct QueueEntry {
    LongInt addr;       /* bitmap pointer */
    int bump;           /* +2 or -2 */
    int twoH;          /* 2 * horizontal coord */
    int mask;          /* pattern mask */
};
```

## 68k Assembly Analysis

### Painting Algorithm Examples

**PixelTrue**: Check if pixel is set
```68k assembly
;  FUNCTION PixelTrue(h,v: INTEGER; bits: BitMap): BOOLEAN;
MOVE.L  (SP)+,A1       ;get return address
MOVE.L  (SP)+,A0       ;get bits (BitMap pointer)
MOVE.W  (SP)+,D1       ;get v
MOVE.W  (SP)+,D0       ;get h
; Calculate byte offset: v * rowBytes + (h / 8)
; Extract bit: (byte >> (7 - (h % 8))) & 1
```

Converted to C:
```c
boolean MacPaint_PixelTrue(int h, int v, BitMap *bits) {
    int byteOffset = (v * bits->rowBytes) + (h / 8);
    int bitOffset = 7 - (h % 8);
    unsigned char byte = ((unsigned char *)bits->baseAddr)[byteOffset];
    return (byte >> bitOffset) & 1;
}
```

### Drawing Algorithms

Most use variations of:
1. **Line drawing** (Bresenham's algorithm)
2. **Pattern filling** (with different blend modes)
3. **Flood fill** (stack-based region filling)
4. **Edge detection** (for lasso selection)

## Integration with System 7.1

### Build System

Add to Makefile:
```makefile
src/Apps/MacPaint/MacPaint_Core.c
src/Apps/MacPaint/MacPaint_Main.c
```

### Entry Point

MacPaint launched via:
1. Finder double-click
2. System 7.1 Application Manager
3. Calls MacPaintMain(argc, argv)

### Toolbox Dependencies

Required System 7.1 managers:
- Event Manager: Event processing
- Window Manager: Window creation/management
- Menu Manager: Menu handling
- QuickDraw: Drawing operations
- File Manager: File I/O
- Resource Manager: Resource loading

### Resource Format

MacPaint resources (.rsrc file):
- PAT# (ID 0): Pattern table (38 patterns, 8x8 bits each)
- PICT (ID 2400): Intro picture
- PICT (ID 2401): Help picture
- DLOG (various): Dialog definitions
- Fonts: Text font specifications

## Testing Checklist

- [ ] Core initialization completes
- [ ] New document creation works
- [ ] Drawing basic lines/rectangles
- [ ] Pattern selection and drawing
- [ ] File save/open operations
- [ ] Menu navigation
- [ ] Tool selection
- [ ] Full application event loop
- [ ] Resource loading
- [ ] Print dialog/functionality

## Known Issues and Limitations

1. **Undo System**: Currently single-level undo (TODO)
2. **Text Tool**: Partially implemented (TODO)
3. **Selections**: Basic rectangle selection only (TODO)
4. **Advanced Brushes**: All brushes treated as simple patterns (TODO)
5. **Printing**: Basic only, no formatting (TODO)
6. **Performance**: No optimizations yet (TODO)

## Future Improvements

1. **Color Support**: Extend to color painting on color systems
2. **Rotation/Flipping**: Implement bit manipulation for image transforms
3. **Dithering**: Add error diffusion for color approximation
4. **Antialiasing**: Smooth edges on draws
5. **Layers**: Add layer support (not in original, but possible)
6. **Scripting**: AppleScript support via System 7.1 OSA

## File Organization

```
src/Apps/MacPaint/
├── MacPaint_Core.c        (~600 lines) Core logic
├── MacPaint_Main.c        (~300 lines) Entry point
├── MACPAINT_CONVERSION.md This file
└── (Additional modules to come)

include/Apps/
└── MacPaint.h             (~100 lines) Public API
```

## References

- Original MacPaint 1.3 Source (public domain)
- Inside Macintosh volumes 1-5
- QuickDraw documentation
- 68k assembly language reference
- Pascal language reference (Macintosh dialect)

## Contributors

- Bill Atkinson: Original MacPaint design and implementation
- System 7.1 Portable: Adaptation for modern architectures

---

**Status**: Phase 1 (Foundation) Complete, Phase 2+ In Development

**Last Updated**: October 15, 2025
