# Serial Logging

The System 7.1 portable kernel uses a hierarchical module-based logging framework in `src/System71StdLib.c`. All subsystems use module-specific logging macros (e.g., `WM_LOG_DEBUG()`, `CTRL_LOG_DEBUG()`) that route through `serial_logf()`. Logs are grouped by module and filtered by verbosity level.

## Levels

Levels follow the familiar severity ordering:
- `kLogLevelError`
- `kLogLevelWarn`
- `kLogLevelInfo`
- `kLogLevelDebug`
- `kLogLevelTrace`

A message is emitted only when its level is at or above both the global threshold and the threshold for its module.

## Modules

`System71StdLib.h` defines `SystemLogModule` identifiers (`kLogModuleWindow`, `kLogModuleControl`, `kLogModuleEvent`, etc.). Each subsystem has a dedicated logging header (e.g., `WindowManager/WMLogging.h`, `ControlManager/CtrlLogging.h`) that defines module-specific macros like:

```c
// From WindowManager/WMLogging.h
#define WM_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelDebug, "[WM] " fmt, ##__VA_ARGS__)
#define WM_LOG_WARN(fmt, ...)  serial_logf(kLogModuleWindow, kLogLevelWarn,  "[WM] " fmt, ##__VA_ARGS__)
```

Legacy `serial_printf()` calls are auto-classified via bracket tag parsing (`[CTRL]`, `[WM]`, etc.) and default to `kLogModuleGeneral`/`kLogLevelDebug` if no tag is found.

## Runtime control

```c
#include "System71StdLib.h"

// Drop everything to WARN globally
typedef enum { ... } SystemLogLevel; // see header
SysLogSetGlobalLevel(kLogLevelWarn);

// Re-enable verbose Window Manager traces
SysLogSetModuleLevel(kLogModuleWindow, kLogLevelDebug);
```

`SysLogGetGlobalLevel`, `SysLogGetModuleLevel`, and `SysLogModuleName` are available for status dumps.

## Emitting logs

**Always use the module-specific macros** defined in each subsystem's logging header:

```c
#include "ControlManager/CtrlLogging.h"

CTRL_LOG_DEBUG("tracking button id=%d state=%d\n", controlID, state);
CTRL_LOG_WARN("invalid control handle: %p\n", (void*)controlHandle);
```

For direct API access when implementing new subsystems:

```c
serial_logf(kLogModuleControl, kLogLevelDebug,
            "[CTRL] tracking button id=%d state=%d\n", controlID, state);
```

Legacy `serial_printf()` calls are still supported for backward compatibility. They are classified via bracket tag parsing (`[WM]`, `[CTRL]`, etc.). Untagged messages default to `kLogLevelDebug` under `kLogModuleGeneral`, so they remain silent unless you raise that module's threshold.

## Custom tags

Strings that begin with a bracketed tag are parsed as `[MODULE[:LEVEL]]`. Example:

```
serial_printf("[DM:TRACE] focus advanced to item %d\n", item);
```

sets the module to `kLogModuleDialog` and the level to `Trace` explicitly.
