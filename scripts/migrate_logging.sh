#!/bin/bash
# Migrate serial_printf to module-specific logging
# Usage: migrate_logging.sh <subsystem> <file1> [file2...]

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 <subsystem> <file1> [file2...]"
    echo "Subsystems: textedit, filesystem, platform, scrap, dialog, event, font, window, sound, memory, process, resource, finder"
    exit 1
fi

SUBSYSTEM=$1
shift

case "$SUBSYSTEM" in
    textedit)
        HEADER="TextEdit/TELogging.h"
        PREFIX="TE"
        ;;
    filesystem)
        HEADER="FS/FSLogging.h"
        PREFIX="FS"
        ;;
    platform)
        HEADER="Platform/PlatformLogging.h"
        PREFIX="PLATFORM"
        ;;
    scrap)
        HEADER="ScrapManager/ScrapLogging.h"
        PREFIX="SCRAP"
        ;;
    dialog)
        HEADER="DialogManager/DialogLogging.h"
        PREFIX="DIALOG"
        ;;
    event)
        HEADER="EventManager/EventLogging.h"
        PREFIX="EVT"
        ;;
    font)
        HEADER="FontManager/FontLogging.h"
        PREFIX="FONT"
        ;;
    window)
        HEADER="WindowManager/WMLogging.h"
        PREFIX="WM"
        ;;
    sound)
        HEADER="SoundManager/SoundLogging.h"
        PREFIX="SND"
        ;;
    memory)
        HEADER="MemoryMgr/MemoryLogging.h"
        PREFIX="MEMORY"
        ;;
    process)
        HEADER="ProcessMgr/ProcessLogging.h"
        PREFIX="PROCESS"
        ;;
    resource)
        HEADER="ResourceManager/ResourceLogging.h"
        PREFIX="RESOURCE"
        ;;
    finder)
        HEADER="Finder/FinderLogging.h"
        PREFIX="FINDER"
        ;;
    *)
        echo "Unknown subsystem: $SUBSYSTEM"
        exit 1
        ;;
esac

echo "Migrating $SUBSYSTEM subsystem to new logging..."
echo "Using header: $HEADER"
echo ""

for FILE in "$@"; do
    if [ ! -f "$FILE" ]; then
        echo "Skipping $FILE (not found)"
        continue
    fi

    COUNT=$(grep -c "serial_printf" "$FILE" || echo "0")
    if [ "$COUNT" = "0" ]; then
        echo "Skipping $FILE (no serial_printf calls)"
        continue
    fi

    echo "Migrating $FILE ($COUNT serial_printf calls)..."

    # Add include if not already present
    if ! grep -q "#include \"$HEADER\"" "$FILE"; then
        # Find the last #include line
        LAST_INCLUDE=$(grep -n "^#include" "$FILE" | tail -1 | cut -d: -f1)
        if [ -n "$LAST_INCLUDE" ]; then
            sed -i "${LAST_INCLUDE}a\\#include \"$HEADER\"" "$FILE"
            echo "  Added #include \"$HEADER\""
        fi
    fi

    # For now, just convert simple serial_printf to _LOG_DEBUG
    # More sophisticated migration would parse the format string for levels
    # This is a safe default that preserves all output at DEBUG level
    sed -i "s/serial_printf(\"\\[TE\\] /TE_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[FS\\] /FS_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[PLATFORM\\] /PLATFORM_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[SCRAP\\] /SCRAP_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[DM\\] /DIALOG_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[EVENT\\] /EVT_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[FONT\\] /FONT_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[WM\\] /WM_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[SOUND\\] /SND_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[MEM\\] /MEMORY_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[PROC\\] /PROCESS_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[RES\\] /RESOURCE_LOG_DEBUG(\"/g" "$FILE"
    sed -i "s/serial_printf(\"\\[FINDER\\] /FINDER_LOG_DEBUG(\"/g" "$FILE"

    # Generic serial_printf -> ${PREFIX}_LOG_DEBUG for this subsystem
    sed -i "s/serial_printf(/${PREFIX}_LOG_DEBUG(/g" "$FILE"

    echo "  Migrated $COUNT calls"
done

echo ""
echo "Migration complete!"
echo "NOTE: All calls migrated to DEBUG level. Review and adjust levels as appropriate."
