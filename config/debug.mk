# Debug Build Configuration
# Maximum debugging support with all smoke tests enabled

# Include default settings
include config/default.mk

# Override for debugging
OPT_LEVEL = 0
DEBUG_SYMBOLS = 1

# Enable all smoke tests
CTRL_SMOKE_TEST = 1
LIST_SMOKE_TEST = 1
ALERT_SMOKE_TEST = 1

# Enable self-tests
CFLAGS += -DSCRAP_SELFTEST=1 -DDEBUG_DOUBLECLICK=1
