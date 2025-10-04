# Window Title Drawing Code Paths

## Investigation Summary

There are two separate code paths that could potentially draw window titles, but only one is actually being used at runtime.

## Code Paths Identified

### Path A: WindowParts.c (UNUSED)
- **File**: `src/WindowManager/WindowParts.c`
- **Function**: `WM_DrawWindowTitle()` (line 289)
- **Called by**: `WM_DrawWindowTitleBar()` → `WM_DrawStandardWindowFrame()` → `WM_StandardWindowDefProc()`
- **Status**: ❌ **NOT EXECUTING** - This code path is compiled but never called at runtime
- **Implementation**: Stub - only logs debug message, does not actually render text

### Path B: WindowDisplay.c (ACTIVE)
- **File**: `src/WindowManager/WindowDisplay.c`
- **Function**: `DrawWindowFrame()` (line 711)
- **Called by**: `PaintOne()`, `DrawNew()`, `DrawWindow()`, etc.
- **Status**: ✅ **ACTIVELY EXECUTING** - This is the actual rendering path
- **Implementation**: Complete rendering implementation including:
  - Title bar background with stripes
  - System 7 lozenge background for active windows
  - Bold text rendering via `DrawString()` from `ChicagoRealFont.c`

## Rendering Stack (Active Path)

```
DrawWindow() / PaintOne() / DrawNew()
    └─> DrawWindowFrame()  [WindowDisplay.c:352]
        └─> DrawString()  [ChicagoRealFont.c:86]
            └─> DrawChar()  [ChicagoRealFont.c:63]
                └─> DrawRealChicagoChar()  [ChicagoRealFont.c:26]
```

## Font Rendering

Window titles are rendered using the **Chicago bitmap font** via:
- `src/ChicagoRealFont.c` - Real Chicago font from System 7.1 NFNT resources
- `include/chicago_font.h` - Font bitmap data and character metrics
- Character spacing: `bit_width + 2` pixels (corrected from original advance values)

## Debugging Note

When adding debug logging via `serial_printf()`, messages must be whitelisted in:
- **File**: `src/System71StdLib.c`
- **Function**: `serial_printf()` (line 349)
- **Whitelist location**: Lines 354-402

Required whitelist entries for title debugging:
```c
strstr(fmt, "CODE PATH") == NULL &&
strstr(fmt, "TITLE:") == NULL &&
strstr(fmt, "DrawString") == NULL
```

## Conclusion

Window titles are working correctly through Path B (WindowDisplay.c). Path A (WindowParts.c) appears to be unused legacy code that could potentially be removed or is intended for a different window definition procedure that isn't currently being invoked.
