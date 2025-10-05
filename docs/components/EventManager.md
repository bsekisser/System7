# Event Manager

## Overview
Implements the cooperative event loop used by System 7 applications. Manages the unified event queue, dispatching keyboard/mouse/system events, honouring sleep times, and providing the `WaitNextEvent`/`GetNextEvent` API surface.

## Source Layout
- `src/EventManager/event_manager.c` – top-level APIs exposed to Toolbox clients (`InitEvents`, `GetNextEvent`, `WaitNextEvent`, `PostEvent`)
- `EventManagerCore.c` – queue data structures, scheduling, sleep calculation, idle time handling
- `EventGlobals.c` – global state, application event mask, mouse button state, double-click timing
- `EventDispatcher.c` – routes events to Window Manager, Dialog Manager, and Process Manager as required
- `MouseEvents.c` / `KeyboardEvents.c` – low-level input decoding, click tracking, keyboard repeat, modifier bookkeeping
- `ModernInput.c` – PS/2 device glue and translation into classic event records
- `SystemEvents.c` – high-level events (activate/deactivate, suspend/resume) and cooperative multitasking hooks

## Responsibilities
- Maintain an ordered queue of classic `EventRecord` entries with timestamping and priority stamping
- Honour `eventMask` filters and convert raw device input into toolbox events
- Provide `WaitNextEvent` sleep semantics using Time Manager tick scheduling
- Track mouse button transitions, double-click thresholds, and cursor location (`globalMouse`)
- Coordinate application activation/deactivation and propagate activate events to windows

## Integration Points
- **PS/2 Controller** feeds scancodes and button changes via `ModernInput.c`
- **Window Manager** receives activate/deactivate, update, and mouse events through `EventDispatcher`
- **Dialog Manager** hooks into `DialogSelect` using `WaitNextEvent` output when in modal loops
- **Time Manager** supplies ticks for sleep intervals and double-click windows

## Testing & Debugging
- `make run` with serial logging enabled will surface `[EVT]` traces (toggle via `System71StdLib.c` whitelist)
- Use the serial shell commands to post synthetic events or inspect queue depth when diagnosing starvation
- Keyboard repeat, modifier handling, and mouse tracking were validated using the control smoke app and Finder window interactions

## Future Work
- Integrate Process Manager idle coalescing so background cooperative tasks get predictable slices
- Surface instrumentation counters (queue length, average sleep) via the serial console
- Add optional logging categories for high-frequency mouse move events without flooding the console
