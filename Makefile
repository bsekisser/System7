# System 7.1 Portable Makefile
# Builds complete System 7.1 kernel

# Directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
ISO_DIR = iso

# Output files
KERNEL = kernel.elf
ISO = system71.iso

# Tools
CC = gcc
AS = as
LD = ld
GRUB = grub-mkrescue

# Feature flags
ENABLE_RESOURCES ?= 1
ENABLE_FILEMGR_EXTRA ?= 1
ENABLE_PROCESS_COOP ?= 1
MODERN_INPUT_ONLY ?= 1

# Flags
# [WM-050] SYS71_PROVIDE_FINDER_TOOLBOX=1 means: DO NOT provide Toolbox stubs; real implementations win.
#          When defined, stubs in sys71_stubs.c are excluded via #ifndef guards.
#          This ensures single source of truth per symbol (no duplicate definitions).
# [WM-052] Warnings are errors - no papering over issues
CFLAGS = -DSYS71_PROVIDE_FINDER_TOOLBOX=1 -DTM_SMOKE_TEST
ifeq ($(ENABLE_RESOURCES),1)
CFLAGS += -DENABLE_RESOURCES=1
endif
ifeq ($(ENABLE_FILEMGR_EXTRA),1)
CFLAGS += -DENABLE_FILEMGR_EXTRA=1
endif
ifeq ($(ENABLE_PROCESS_COOP),1)
CFLAGS += -DENABLE_PROCESS_COOP=1
endif
ifeq ($(MODERN_INPUT_ONLY),1)
CFLAGS += -DMODERN_INPUT_ONLY=1
endif
CFLAGS += -ffreestanding -fno-builtin -fno-stack-protector -nostdlib \
         -Wall -Wextra -Wmissing-prototypes -Wmissing-declarations -Wshadow -Wcast-qual \
         -Wpointer-arith -Wstrict-prototypes -Wno-unused-parameter \
         -g -O1 -fno-inline -fno-optimize-sibling-calls -I./include -std=c99 -m32 \
         -Wuninitialized -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
         -Wno-multichar -Wno-pointer-sign -Wno-sign-compare
ASFLAGS = --32
LDFLAGS = -melf_i386 -nostdlib

# Resource files
RSRC_JSON = patterns.json
RSRC_BIN = Patterns.rsrc

# Source files
C_SOURCES = src/main.c \
            src/SystemInit.c \
            src/sys71_stubs.c \
            src/System71StdLib.c \
            src/System/SystemTheme.c \
            src/ToolboxCompat.c \
            src/Finder/finder_main.c \
            src/Finder/desktop_manager.c \
            src/Finder/folder_window.c \
            src/Finder/alias_manager.c \
            src/Finder/trash_folder.c \
            src/Finder/AboutThisMac.c \
            src/QuickDraw/QuickDrawCore.c \
            src/QuickDraw/QuickDrawPlatform.c \
            src/QuickDraw/Coordinates.c \
            src/QuickDraw/Regions.c \
            src/QuickDraw/quickdraw_shapes.c \
            src/Platform/WindowPlatform.c \
            src/Platform/x86_io.c \
            src/Platform/ATA_Driver.c \
            src/SoundManager/SoundManagerBareMetal.c \
            src/SoundManager/SoundHardwarePC.c \
            src/MenuManager/MenuManagerCore.c \
            src/MenuManager/MenuSelection.c \
            src/MenuManager/MenuDisplay.c \
            src/MenuManager/menu_savebits.c \
            src/MenuManager/MenuTitleTracking.c \
            src/MenuManager/MenuTrack.c \
            src/MenuManager/MenuItems.c \
            src/MenuManager/platform_stubs.c \
            src/MenuCommands.c \
            src/Finder/Icon/icon_system.c \
            src/Finder/Icon/icon_resources.c \
            src/Finder/Icon/icon_resolver.c \
            src/Finder/Icon/icon_draw.c \
            src/Finder/Icon/icon_label.c \
            src/trash_icons.c \
            src/chicago_font_data.c \
            src/FontManager/FontManagerCore.c \
            src/FontManager/FontResourceLoader.c \
            src/FontManager/FontStyleSynthesis.c \
            src/FontManager/FontScaling.c \
            src/test_fontmgr.c \
            src/PS2Controller.c \
            src/PatternMgr/pattern_manager.c \
            src/PatternMgr/pattern_resources.c \
            src/PatternMgr/pram_prefs.c \
            src/Resources/pattern_data.c \
            src/ControlPanels/cdev_desktop.c \
            src/simple_resource_manager.c \
            src/ControlManager/ControlManagerCore.c \
            src/ControlManager/ControlTracking.c \
            src/control_stubs.c \
            src/patterns_rsrc.c \
            src/FS/hfs_diskio.c \
            src/FS/hfs_volume.c \
            src/FS/hfs_btree.c \
            src/FS/hfs_catalog.c \
            src/FS/hfs_file.c \
            src/FS/vfs.c \
            src/FS/trash.c \
            src/FS/vfs_ops.c \
            src/MemoryMgr/MemoryManager.c \
            src/MemoryMgr/memory_manager_core.c \
            src/MemoryMgr/heap_compaction.c \
            src/MemoryMgr/blockmove_optimization.c \
            src/Resources/Icons/hd_icon.c \
            src/color_icons.c \
            src/DeskManager/DeskManagerCore.c \
            src/DeskManager/DeskManagerStubs.c \
            src/DialogManager/DialogManagerCore.c \
            src/DialogManager/DialogManagerStubs.c \
            src/DialogManager/ModalDialogs.c \
            src/DialogManager/AlertDialogs.c \
            src/DialogManager/AlertSmoke.c \
            src/DialogManager/DialogEvents.c \
            src/DialogManager/DialogItems.c \
            src/DialogManager/DialogResourceParser.c \
            src/DialogManager/DialogDrawing.c \
            src/DialogManager/DialogHelpers.c \
            src/StandardFile/StandardFile.c \
            src/StandardFile/StandardFileHAL_Shims.c \
            src/FileManager.c \
            src/FileManagerStubs.c \
            src/EventManager/event_manager.c \
            src/EventManager/EventGlobals.c \
            src/EventManager/ModernInput.c \
            src/EventManager/EventDispatcher.c \
            src/EventManager/MouseEvents.c \
            src/EventManager/KeyboardEvents.c \
            src/EventManager/SystemEvents.c \
            src/ProcessMgr/ProcessManager.c \
            src/TextEdit/TextEdit.c \
            src/TextEdit/TextEditDraw.c \
            src/TextEdit/TextEditInput.c \
            src/TextEdit/TextEditScroll.c \
            src/TextEdit/TextEditClipboard.c \
            src/TextEdit/TextBreak.c \
            src/TextEdit/TextEditTest.c \
            src/WindowManager/WindowDisplay.c \
            src/WindowManager/WindowEvents.c \
            src/WindowManager/WindowManagerCore.c \
            src/WindowManager/WindowManagerHelpers.c \
            src/WindowManager/WindowDragging.c \
            src/WindowManager/WindowResizing.c \
            src/WindowManager/WindowLayering.c \
            src/WindowManager/WindowParts.c \
            src/TimeManager/PlatformTime.c \
            src/TimeManager/TimeBase.c \
            src/TimeManager/MicrosecondTimer.c \
            src/TimeManager/TimeManager.c \
            src/TimeManager/TimeManagerCore.c \
            src/TimeManager/TimerInterrupts.c \
            src/TimeManager/TimerTasks.c \
            src/Apps/SimpleText/SimpleText.c \
            src/Apps/SimpleText/STDocument.c \
            src/Apps/SimpleText/STView.c \
            src/Apps/SimpleText/STMenus.c \
            src/Apps/SimpleText/STFileIO.c \
            src/Apps/SimpleText/STClipboard.c

# Add ResourceMgr sources if enabled
ifeq ($(ENABLE_RESOURCES),1)
C_SOURCES += src/ResourceMgr/ResourceMgr.c
endif

# Add FileMgr extra sources if enabled
ifeq ($(ENABLE_FILEMGR_EXTRA),1)
C_SOURCES += src/FileMgr/btree_services.c \
             src/FileMgr/extent_manager.c \
             src/FileMgr/tfs_dispatch.c \
             src/FileMgr/volume_manager.c
endif

# Add Gestalt Manager if enabled
ENABLE_GESTALT ?= 1
ifeq ($(ENABLE_GESTALT),1)
C_SOURCES += src/Gestalt/Gestalt.c \
             src/Gestalt/GestaltBuiltins.c
CFLAGS += -DENABLE_GESTALT=1
endif

# Conditionally add Process Manager cooperative scheduling
ifeq ($(ENABLE_PROCESS_COOP),1)
C_SOURCES += src/ProcessMgr/CooperativeScheduler.c \
             src/ProcessMgr/EventIntegration.c
endif

# Add ScrapManager if enabled
ENABLE_SCRAP ?= 1
ifeq ($(ENABLE_SCRAP),1)
C_SOURCES += src/ScrapManager/ScrapManager.c
CFLAGS += -DENABLE_SCRAP=1
# Enable self-test for debugging
CFLAGS += -DSCRAP_SELFTEST=1 -DDEBUG_DOUBLECLICK=1
endif

# Add ListManager if enabled
ENABLE_LIST ?= 1
LIST_SMOKE_TEST ?= 0
ifeq ($(ENABLE_LIST),1)
C_SOURCES += src/ListManager/ListManager.c \
             src/ListManager/list_manager.c \
             src/ListManager/ListSmoke.c
CFLAGS += -DENABLE_LIST=1
ifeq ($(LIST_SMOKE_TEST),1)
CFLAGS += -DLIST_SMOKE_TEST=1
endif
endif

ASM_SOURCES = src/multiboot2.S

# Object files
C_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(C_SOURCES)))
ASM_OBJECTS = $(patsubst %.S,$(OBJ_DIR)/%.o,$(notdir $(ASM_SOURCES)))
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Create directories
$(shell mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(ISO_DIR)/boot/grub)

# Default target
all: $(RSRC_BIN) $(KERNEL)

# Build resource file
$(RSRC_BIN): $(RSRC_JSON) gen_rsrc.py
	@echo "GEN $(RSRC_BIN)"
	@python3 gen_rsrc.py $(RSRC_JSON) /home/k/Documents/patterns_authentic_color.json $(RSRC_BIN)

# Convert resource file to C source
src/patterns_rsrc.c: $(RSRC_BIN)
	@echo "XXDC $<"
	@echo '/* Auto-generated from Patterns.rsrc */' > $@
	@echo 'const unsigned char patterns_rsrc_data[] = {' >> $@
	@xxd -i < $< >> $@
	@echo '};' >> $@
	@echo 'const unsigned int patterns_rsrc_size = sizeof(patterns_rsrc_data);' >> $@

# Link kernel
$(KERNEL): $(OBJECTS)
	@echo "LD $(KERNEL)"
	@$(LD) $(LDFLAGS) -T linker_mb2.ld -o $(KERNEL) $(OBJECTS)

# Compile source files
$(OBJ_DIR)/%.o: src/%.S
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

# Pattern rules for each subdirectory
$(OBJ_DIR)/%.o: src/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/System/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/patterns_rsrc.o: src/patterns_rsrc.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

src/patterns_rsrc.c: $(RSRC_BIN)

$(OBJ_DIR)/%.o: src/MemoryMgr/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ResourceMgr/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/FileMgr/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Gestalt/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/QuickDraw/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/WindowManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/MenuManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ControlManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/EventManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/DialogManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/StandardFile/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/TextEdit/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ListManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ScrapManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ProcessMgr/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/TimeManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/SoundManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/PrintManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/HelpManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ComponentManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/EditionManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/NotificationManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/PackageManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/NetworkExtension/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ColorManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/CommunicationToolbox/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/FontManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/FontResources/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/GestaltManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/SpeechManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/BootLoader/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/SegmentLoader/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/DeviceManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Finder/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Finder/Icon/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/FS/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/DeskManager/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ControlPanels/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Keyboard/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Datetime/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Calculator/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Chooser/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/StartupScreen/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Platform/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/PatternMgr/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Resources/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Resources/Icons/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Apps/SimpleText/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/Finder/Icon/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/ControlPanels/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Special targets for stubs
build/obj/universal_stubs.o: src/universal_stubs.c
	@echo "CC src/universal_stubs.c"
	@$(CC) $(CFLAGS) -c src/universal_stubs.c -o build/obj/universal_stubs.o

build/obj/ultimate_stubs.o: src/ultimate_stubs.c
	@echo "CC src/ultimate_stubs.c"
	@$(CC) $(CFLAGS) -c src/ultimate_stubs.c -o build/obj/ultimate_stubs.o

# ISO target
iso: $(ISO)

$(ISO): $(KERNEL)
	@echo "Creating bootable ISO..."
	@cp $(KERNEL) $(ISO_DIR)/boot/
	@echo 'menuentry "System 7.1 Portable" {' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/$(KERNEL)' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@$(GRUB) -d /usr/lib/grub/i386-pc -o $(ISO) $(ISO_DIR)

# Run with QEMU (PC speaker with PulseAudio backend)
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -drive file=test_disk.img,format=raw,if=ide -m 1024 -vga std -serial file:/tmp/serial.log \
		-audiodev pa,id=snd0,server=/run/user/$(shell id -u)/pulse/native -machine pcspk-audiodev=snd0

# Debug with QEMU
debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -drive file=test_disk.img,format=raw,if=ide -m 1024 -vga std -s -S

# Clean
clean:
	rm -rf $(BUILD_DIR) obj isodir iso system71.iso kernel.elf

# Show compilation info
info:
	@echo "C Sources: $(words $(C_SOURCES)) files"
	@echo "Source Files: $(words $(ASM_SOURCES)) files"
	@echo "Total Objects: $(words $(OBJECTS)) files"

# Check exported symbols against allowlist
check-exports: kernel.elf
	@bash tools/check_exports.sh

.PHONY: all clean iso run debug info check-exports