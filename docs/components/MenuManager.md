# Menu Manager

## Overview
Recreates the System 7 menu bar and pull-down menu experience, from resource loading to live tracking. Handles menu creation, insertion/sorting, drawing, highlighting, tracking loops, and command dispatch.

## Source Layout
- `src/MenuManager/MenuManagerCore.c` – registration APIs (`InitMenus`, `NewMenu`, `InsertMenu`, `AppendMenu`, `DisposeMenu`, `DrawMenuBar`)
- `MenuBarManager.c` – menu bar layout, width calculations, and invalidate logic
- `MenuDisplay.c` / `MenuTrack.c` / `MenuSelection.c` – render dropdowns, track mouse, resolve selections, manage flashing
- `MenuItems.c` – menu item manipulation (enable/disable, check marks, hierarchical menus)
- `MenuTitleTracking.c` – top-level title interaction and click routing
- `menu_savebits.c` – `SaveBits`/`RestoreBits` implementations for flicker-free drawing
- `MenuResources.c` – parsing of 'MENU' resources and population of menu structures
- `menu_dispatch.c` – bridges to Menu Command handlers (e.g. `DoMenuCommand` in `MenuCommands.c`)
- `PopupMenus.c` – popup menu variants used by controls

## Responsibilities
- Maintain the menu list and menu bar data structures populated during `InitMenus`
- Draw the menu bar chrome (lozenge, titles) and each dropdown into off-screen buffers to reduce flicker
- Track mouse movement during a menu session, highlighting items and switching between menus when the pointer crosses titles
- Deliver final selections through the application-provided `MenuSelect`/`MenuChoice` loop and clean up saved bits
- Manage highlighting state, checkmarks, and enabling/disabling of items in response to application state

## Integration Points
- **Window Manager** coordinates activate/deactivate events; menus relinquish highlight when the application loses focus
- **Event Manager** feeds mouse-down events to kick off `MenuSelect` and supplies repeated mouse moves during tracking
- **Control Manager** uses `PopupMenus.c` for pop-up control variants
- **Resource Manager** provides menu templates through 'MENU'/ 'MBAR' resources (tooling currently stubs some calls)

## Testing & Debugging
- Use `make run` and interact with the Finder or SimpleText to exercise menu tracking; verify highlights, scrolling, and keyboard shortcuts (via `MenuKey` once wired)
- Serial logs are tagged with `[MENU]` (enable in `System71StdLib.c`)
- `make check-exports` confirms exported menu traps remain aligned with `docs/symbols_allowlist.txt`
- Edge cases: nested hierarchical menus, disabled items mid-track, SaveBits/RestoreBits correctness when overlapping windows

## Future Work
- Hook in keyboard shortcuts (`MenuKey`) once command tables stabilise
- Implement dynamic menu building from resource forks using the Resource Manager loader
- Add auto-scroll for menus taller than the screen once Scroll Manager infrastructure is ready
