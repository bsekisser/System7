# Component Status (2025-10-05)

This snapshot tracks the current readiness of key subsystems after the most
recent integration work. Status levels:

- **Stable** – Feature-complete for current milestones; only bug fixes expected.
- **Active** – Core behavior in place, but expect iterative fixes and polish.
- **Bring-Up** – Under construction; major gaps remain before daily use.

## Core Runtime

- **Memory Manager — Active**
  - System/App zones shared with the 68K interpreter via `MemoryManager_MapToM68K()`.
  - Low-memory globals (`MemTop`, `SysZone`, `ApplZone`) mirror real heap limits.
  - TODO: Purge/compact heuristics and growable zone support.
- **OSUtils Traps — Active**
  - `_TickCount` serviced by a Time Manager task primed at 60 Hz.
  - Integrated lifecycle hooks in Segment Loader smoke tests (`OSUtils_InstallTraps()` / `OSUtils_Shutdown()`).
  - TODO: Extend trap table for Date & Time Manager stubs.
- **Segment Loader — Stable**
  - CODE resource loading, lazy JT stubs, relocation, and smoke checks verified.
  - New trap wiring occurs automatically during `SegmentLoader_TestBoot()`.

## UI Stack

- **QuickDraw — Active**
  - Core primitives ready; Apple menu icon path now draws the 16×16 RGBA asset into the active `GrafPort`.
  - TODO: Region compositing performance pass and patterned menu bar background.
- **Menu Manager — Active**
  - Menu bar titles sized via `StringWidth` for accurate hit regions.
  - Apple menu placement derived from `MenuAppleIcon_Draw()` width instead of constants.
  - TODO: Replace hardcoded menu titles once Resource Manager exports real menu data.
- **Window Manager — Stable**
  - Dragging, activation, and focus plumbing aligned with Keyboard Navigation integration.

## Support Systems

- **Time Manager — Stable**
  - Microsecond scheduler proven in `_TickCount` servicing.
- **Logging — Stable**
  - Module-level filtering covers new OSUtils and Memory Manager logs.
- **Resource Manager — Active**
  - Binary search & caching solid; pending menu resource hookup for dropdown labels.

Please update this file when module status changes or new subsystems come online.
