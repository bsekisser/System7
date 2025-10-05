# Segment Loader Test Configuration
# Build with: make CONFIG=segloader_test

# Enable segment loader test harness
CFLAGS += -DSEGLOADER_TEST_BOOT=1

# Debug symbols for easier testing
DEBUG_SYMBOLS = 1
OPT_LEVEL = 0

# Include standard test configuration
include config/debug.mk
