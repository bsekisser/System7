# Release Build Configuration
# Optimized build with tests disabled

# Include default settings
include config/default.mk

# Override for release
OPT_LEVEL = 2
DEBUG_SYMBOLS = 0

# Disable all smoke tests
CTRL_SMOKE_TEST = 0
LIST_SMOKE_TEST = 0
ALERT_SMOKE_TEST = 0
