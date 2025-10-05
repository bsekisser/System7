# Serial Logging

The System 7.1 portable kernel routes all `serial_printf` output through a lightweight logging layer in `src/System71StdLib.c`. Logs are grouped by module and filtered by verbosity so you can dial noise up or down without editing string whitelists.

## Levels

Levels follow the familiar severity ordering:
- `kLogLevelError`
- `kLogLevelWarn`
- `kLogLevelInfo`
- `kLogLevelDebug`
- `kLogLevelTrace`

A message is emitted only when its level is at or above both the global threshold and the threshold for its module.

## Modules

`System71StdLib.h` defines `SystemLogModule` identifiers (`kLogModuleWindow`, `kLogModuleControl`, `kLogModuleEvent`, etc.). Existing messages are auto-classified via the leading tag (`[CTRL]`, `[WM]`, etc.) or by keyword heuristics, and land in sensible defaults.

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

Prefer the new helper so module/level are explicit:

```c
serial_logf(kLogModuleControl, kLogLevelDebug,
            "[CTRL] tracking button id=%d state=%d\n", controlID, state);
```

Legacy `serial_printf` calls still work; they are classified automatically based on the existing `[TAG]` prefixes and keywords. Untagged messages default to `kLogLevelDebug` under the `kLogModuleGeneral` module, so they remain silent unless you raise that module’s threshold.

## Custom tags

Strings that begin with a bracketed tag are parsed as `[MODULE[:LEVEL]]`. Example:

```
serial_printf("[DM:TRACE] focus advanced to item %d\n", item);
```

sets the module to `kLogModuleDialog` and the level to `Trace` explicitly.
