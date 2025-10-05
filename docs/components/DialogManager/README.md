# Dialog Manager

## Overview
Provides modal and modeless dialog services, resource loading, draw/update cycles, event routing, and keyboard navigation that mirror classic System 7 behaviour. Dialog Manager glues together Control Manager controls, QuickDraw ports, and the Event Manager loop.

## Source Layout
- `src/DialogManager/DialogManagerCore.c` / `dialog_manager_core.c` – top-level APIs (`InitDialogs`, `NewDialog`, `ModalDialog`, `IsDialogEvent`, `DialogSelect`)
- `DialogResourceParser.c` / `DialogResources.c` – DITL/DLOG resource parsing and instantiation
- `DialogItems.c` – item table management, item hit dispatch, and `Get/SetDItem`
- `DialogDrawing.c` – standard chrome, item rendering, and region invalidation
- `DialogEvents.c` / `dialog_manager_dispatch.c` – event loop integration, `DialogSelect`
- `DialogHelpers.c` – geometry helpers, item rect calculations, item list traversal utilities
- `DialogKeyboard.c` – Return/Esc/Space/Tab handling, focus tracking, debounce logic (see [KeyboardIntegration](KeyboardIntegration.md))
- `ModalDialogs.c` / `AlertDialogs.c` – canned dialog templates (alert, stop, note) and supervisory helpers
- Smoke harness lives in `AlertSmoke.c`

## Responsibilities
- Construct dialog windows from DLOG/DITL resources and attach cloned controls
- Maintain the dialog item list, including text item state, control handles, and icon references
- Provide modal loop entry points (`ModalDialog`, `StandardAlert`) that block until an item is activated
- Integrate keyboard focus and control activation semantics (default, cancel, Tab order)
- Coordinate dialog invalidation/redraw by deferring to Control Manager for control items and TextEdit for editable fields

## Integration Points
- **Control Manager** supplies the concrete control handles for button/checkbox/radio items; Dialog Manager stores them in the item list and reuses `Draw1Control`, `HiliteControl`, etc.
- **Event Manager** feeds `DialogSelect` from `WaitNextEvent`; modal loops use `GetNextEvent` fallbacks to stay responsive
- **Window Manager** provides window creation, focus, and update notifications; dialogs use window kinds specific to System 7
- **TextEdit** populates edit fields; Dialog Manager installs hooks so keyboard focus can hand off to TE when appropriate

## Testing & Debugging
- Run `make CTRL_SMOKE_TEST=1 run` and open the control smoke window to validate keyboard focus, default/cancel semantics, and item hits
- Use `DialogManager` serial logs (`[DM]` / `[CTRL]`) for tracing; whitelist entries live in `System71StdLib.c`
- Alerts can be exercised through `AlertSmoke.c` or invoking `StandardAlert` from the serial console

## Future Work
- Hook modal dialogs into StandardFile file selection once List Manager and File Manager APIs stabilise
- Expand support for user item procs and custom item types
- Add automated tab-order verification powered by scripted key sequences
