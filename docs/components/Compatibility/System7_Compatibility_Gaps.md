# System 7.1 API Compatibility Gaps

This checklist captures the most significant differences between the current toolbox reimplementation and the classic System 7.1 APIs. Each bullet calls out the incomplete behaviour, the file/line where the gap is documented in code, and the work expected to regain parity.

## QuickDraw & Graphics Pipeline
- `src/QuickDraw/Bitmaps.c:134` – `CopyBits` still needs full mask handling, colour depth conversion, and transfer-mode coverage to match the System 7 trap.
- `src/QuickDraw/quickdraw_drawing.c:41` / `:68` – `LineTo`, `DrawString`, and `DrawText` are placeholders that only advance the pen; they should rasterize using the active pen/pattern and integrate with the Font Manager for glyph metrics.
- ~~`src/QuickDraw/PatternManager.c:177`–`257` – All patterned fill variants (`FillRect`, `FillOval`, `FillRgn`, etc.) are stubs that set patterns but never rasterize geometry.~~ **VERIFIED** (2025-10-06): Patterned fills are fully implemented in QuickDrawCore.c via DrawPrimitive(); PatternManager.c is not in build
- `src/QuickDraw/quickdraw_pictures.c:44`–`96` – `DrawPicture` frames the destination rect instead of executing PICT opcodes; full QuickDraw opcode parsing, scaling, and region copying remain to be implemented.
- ~~`src/QuickDraw/quickdraw_pictures.c:94`–`115` – `SetClip`/`GetClip` do not copy regions, breaking callers that expect independent clip regions.~~ **VERIFIED** (2025-10-06): SetClip/GetClip in QuickDrawCore.c properly use CopyRgn() for independent region copies
- `src/QuickDraw/CursorManager.c:151`–`177` – Cursor show/hide/obscure/spin still defer to TODOs; Mac OS required hardware cursor toggles and watch-cursor animation tied to `SpinCursor`.

## Window, Dialog, and Control Managers
- `src/WindowManager/WindowEvents.c:557`–`560` – Grow and drag tracking branches return immediately; Window Manager must honour `inDrag`/`inGrow` parts with live XOR outlines and constraint callbacks like the classic implementation.
- `src/WindowManager/WindowEvents.c:574`–`599` – Platform mouse state, update ports, and empty-region checks are stubbed, so `TrackGoAway`, `TrackBox`, and update propagation diverge from System 7 behaviour.
- `src/EventManager/SystemEvents.c:331` & `:390` – Update regions are never merged or reduced after validation, causing duplicate `updateEvt`s; the classic manager subtracts validated areas from pending invalidations.
- ~~`src/DialogManager/DialogDrawing.c:428` – Edit-text items ignore focus rings; System 7 drew a focus frame and moved the caret when the control is active.~~ **FIXED** (2025-10-06): Edit-text focus rings and caret blinking implemented in DialogEditText.c
- ~~`src/DialogManager/dialog_manager_private.c:104` – `GetNextUserCancelEvent` is a stub; modal dialogs should scan the event queue for cancel gestures (Command-.) as the Classic API allowed.~~ **FIXED** (2025-10-06): IsUserCancelEvent/GetNextUserCancelEvent implemented, modal dialogs support Cmd-. and Escape
- `src/ControlManager/StandardControls.c:84` – Control metrics are hard-coded to Chicago 12; real `GetFontInfo` must come from the Font Manager so controls respect the active font.
- `docs/components/ControlManager/QA.md:14` (via code) – Mixed-state checkbox paths remain unvalidated; native System 7 controls supported tri-state checkboxes.

## Event & Input Handling
- ~~`src/EventManager/event_manager.c:251` – Posted events always report `modifiers = 0`; modifier bits (shift, option, command) need to be sampled from the PS/2 layer so Command shortcuts and shift-clicking behave correctly.~~ **FIXED** (2025-10-06): PostEvent now calls GetPS2Modifiers() to populate modifier fields from hardware
- ~~`src/EventManager/event_manager.c:285` – `WaitNextEvent` ignores the caller-supplied `mouseRgn`; classic Mac OS clipped null events and mouse moved events to that region.~~ **FIXED** (2025-10-06): WaitNextEvent now monitors mouseRgn and generates null events when mouse exits region
- ~~`src/EventManager/EventDispatcher.c:413`–`416` – Command-key menu shortcuts are unimplemented; menu command routing should call `MenuKey`/`MenuChoice` analogues when `cmdKey` is set.~~ **FIXED** (2025-10-06): Event Dispatcher now calls MenuKey() for all command-key events and routes through DoMenuCommand()
- ~~`src/EventManager/MouseEvents.c:454` & `src/EventManager/EventManagerCore.c:655` – `StillDown` references stubs in `control_stubs.c`; until the real implementation arrives, hit testing during tracking is unreliable.~~ **VERIFIED** (2025-10-06): StillDown and Button are properly implemented in MouseEvents.c

## Text Input & Editing
- `src/TextEdit/textedit_core.c:586`–`604` – Core TextEdit routines (`TECalcLines`, font setup, drawing, caret updates) remain TODOs; the current implementation cannot handle wrapping, styled runs, or caret management like System 7’s TextEdit.
- `src/TextEdit/TextEditScroll.c:91` & `:180` – Horizontal scroll limits are uncomputed, so TE windows cannot properly constrain scroll bars.
- `src/TextEdit/TextEditClipboard.c:164`–`267` – Styled scrap handling is stubbed; classic TE mirrored styled text into the clipboard flavours.
- `src/textedit_stubs.c:13` – Numerous TextEdit traps still drop to simple stubs, preventing compatibility with applications calling less common TE routines.

## Resource & File Systems
- `src/ResourceManager.c:1095`–`1518` – Purge flags, map enumeration (`Count1Resources`, `Get1IndResource`, `CountTypes`, etc.), unique ID generation, and resource file attribute setters all return placeholders; full resource map traversal is required for ROM compatibility.
- `src/ResourceMgr/ResourceMgr.c:783`–`1013` – Alternate Resource Manager implementation keeps key APIs (`GetNamedResource`, `OpenResFile`, type enumeration`) as stubs, so clients cannot open separate resource forks yet.
- `src/FileManagerStubs.c:340`–`366` – Block and fork I/O paths log “stub” and return `ioErr`; Finder-level file access must flow through HFS and block device drivers to match System 7 semantics.
- `src/FS/vfs.c:545` – Write operations are stubbed, so the virtual file system remains read-only unlike classic Mac OS.
- `src/ScrapManager/ScrapManager.c:382`–`400` – Clipboard persistence (`LoadScrap`, `UnloadScrap`) is stubbed; System 7 wrote scrap data to the desktop file.

## Memory & Process Infrastructure
- `src/MemoryMgr/memory_manager_core.c:445`–`451` – `SetHandleSize` fakes success without reallocating; handle-based memory semantics (moveable/relocatable blocks, zone compaction) must be honoured for legacy callers.
- ~~`src/System71StdLib.c:576`–`583` – `sprintf`/`snprintf` are placeholder implementations; Toolbox routines expecting formatted output (e.g., `NumToString`) will misbehave.~~ **FIXED** (2025-10-06): Implemented vsnprintf() with format specifiers (%s, %d, %u, %x, %c, %p); sprintf() and snprintf() now fully functional
- `src/ProcessMgr/ProcessManager.c:324` – Comment notes `WaitNextEvent` still comes from stubs; multi-process scheduling remains experimental.

## Fonts & Typography
- `docs/components/FontManager/README.md` & `src/FontManager/FontManagerCore.c:56` – Only the Chicago 12 strike is available; Geneva/Monaco map to Chicago metrics, and true resource-driven strike loading is pending, unlike System 7’s font ecosystem.
- `src/FontManager/FontResourceLoader.c:36` – Memory allocation helpers are still stubbed, blocking runtime NFNT/FOND ingestion.

## Peripheral Toolbox Managers
- `src/ListManager/ListManager.c:428`–`438` – Column APIs (`LAddColumn`, `LDelColumn`) return stub responses; System 7 supported dynamic column manipulation.
- `src/AppleEventManager/AppleEventManagerCore.c:541` & `:641` – Apple Event parameter coercion and event source attribution are TODOs; cross-process AppleEvents depend on these.
- `src/SoundManager/SoundManagerBareMetal.c:150`–`205` – Core Sound Manager channels and playback APIs return `unimpErr`; only `SysBeep` exists, whereas System 7 provided channel-based audio playback.
- `src/QuickDraw/PatternManager.c:286` – Desktop pattern installation is unimplemented, leaving the Finder without classic patterned backgrounds.
- `src/MenuManager/menu_geometry.c:157`–`166` – Menu bar string drawing defers to a stub; real menu titles should be rendered through QuickDraw so font changes and width calculations are correct.
- `src/QuickDraw/quickdraw_pictures.c:123`–`141` – Region allocation/free rely on `NewHandle` without proper zone management; classic QuickDraw used Region Manager semantics.

## Next Steps
The items above should be prioritised for implementation or alignment work. Restoring these behaviours will unblock compatibility with classic System 7 applications that rely on the documented Toolbox contract.
