# Standard Controls QA Checklist

Quick reference for validating push buttons, checkboxes, and radio buttons in System 7.1 Portable. All tests assume the current Control Manager and Dialog Manager integrations.

## Build Status
- ✅ Compiles cleanly with strict `-Wall -Wextra` flags
- ✅ Linked whenever `ControlManager` is enabled; smoke harness gated by `CTRL_SMOKE_TEST`

## Code Quality Highlights
- ✅ No dead locals (e.g., `inset` removal in `DrawButtonFrame()`)
- ✅ `StandardControls.c` documents the procID table and variant flags
- ✅ QuickDraw state saved/restored around every draw path
- ✅ Serial logging intentionally sparse and whitelisted (`[CTRL]` prefix)
- ⚠️ TODO: Swap hard-coded Chicago metrics for `GetFontInfo()` once Geneva/Monaco become real strikes

## Smoke Test Setup
```
make clean
make CTRL_SMOKE_TEST=1 iso
make CTRL_SMOKE_TEST=1 run
```
The harness opens "Control Smoke Test" containing:
- Default **OK** button (refCon=1)
- **Cancel** button (refCon=2)
- Checkbox "Show hidden files" (refCon=3)
- Radio group (group ID 1, refCons=4/5/6): Icons, List, Columns

## Expected Behaviour

### Mouse
- Buttons highlight on press and fire `contrlAction` on in-bounds mouse-up
- Checkbox toggles when clicking glyph or label; logs new state
- Radios enforce exclusivity via `HandleRadioGroup()` and log selection
- Dragging outside the control during a press cancels the click (returns part 0)

### Keyboard (Dialog Manager integration)
- `Tab` / `Shift+Tab` cycle focus with XOR rings, skipping disabled controls
- `Return` activates the default button; `Esc` activates Cancel
- `Space` toggles checkbox/radio or activates a focused button
- Keyboard events produce `[CTRL]` / `[DM]` logs and set `itemHit` for modal loops

### Disabled State
- Set any control to `inactiveHilite`: it draws grey, ignores hits, and is skipped during focus traversal

### Logging Samples
```
[CTRL] Button clicked (refCon=1)
[CTRL] Checkbox value: 1
[CTRL] Radio group state: R1=0 R2=1 R3=0
[CTRL] DM_HandleSpaceKey: Toggled checkbox to 0
```

## Regression Checklist
1. Toggle each control with both mouse and keyboard; verify serial output matches expected refCon/value.
2. Hold mouse on a button, tap Return, confirm only one activation occurs (debounce).
3. Disable a control and ensure focus traversal skips it while mouse still respects inactive state.
4. Resize/Expose the window to confirm focus rings and hilights redraw correctly (XOR removes ghosting).
5. Run `make check-exports` after modifications to ensure exported ToolBox symbols stay stable.

## Future Work
- Hook `GetFontInfo()` to Font Manager once additional strikes land.
- Add mixed-state checkbox coverage in the smoke harness.
- Consider visual regression captures once automated CI is available.
