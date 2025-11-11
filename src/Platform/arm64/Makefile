# ARM64 Kernel Makefile
# Build for Raspberry Pi 3/4/5 and QEMU

# Toolchain - auto-detect available compiler
PREFIX ?= aarch64-none-elf-
ifeq ($(shell which $(PREFIX)gcc 2>/dev/null),)
	PREFIX = aarch64-elf-
endif

CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

# Directories
SRC_DIR = .
BUILD_DIR = build
STDLIB_DIR = ../../

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles
CFLAGS += -mcpu=cortex-a53 -mtune=cortex-a53
CFLAGS += -I$(SRC_DIR) -I$(STDLIB_DIR)

# Add QEMU define if building for QEMU
ifdef QEMU
CFLAGS += -DQEMU_BUILD
# Note: Using MMIO transport (virtio-gpu-device) instead of PCI
# PCI transport would require MMU for ECAM access at high addresses
endif

# Add test exception handler flag if requested
ifdef TEST_EXCEPTIONS
CFLAGS += -DTEST_EXCEPTION_HANDLER
endif

# Assembler flags
ASFLAGS = -mcpu=cortex-a53

# Linker flags
LDFLAGS = -T link.ld -nostdlib

# Source files
ASM_SOURCES = platform_boot.S exceptions.S

# QEMU build uses different UART addresses
ifdef QEMU
C_SOURCES = hal_boot.c uart_qemu.c timer.c gic.c dtb.c cache.c mmu.c string.c printf.c exception_handlers.c virtio_gpu.c
else
C_SOURCES = hal_boot.c uart.c timer.c gic.c mailbox.c framebuffer.c dtb.c cache.c mmu.c string.c printf.c exception_handlers.c
endif

# Add minimal kernel stub for testing
C_SOURCES += kernel_stub.c

# Object files
ASM_OBJECTS = $(ASM_SOURCES:%.S=$(BUILD_DIR)/%.o)
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Targets
TARGET = kernel8
IMAGE = $(BUILD_DIR)/$(TARGET).img
ELF = $(BUILD_DIR)/$(TARGET).elf

.PHONY: all clean qemu qemu-build qemu-virt

all: $(IMAGE)

# Build for QEMU
qemu-build:
	$(MAKE) clean
	$(MAKE) QEMU=1

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(IMAGE): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

# QEMU testing - build and run on virt machine
qemu-virt: qemu-build
	@echo ""
	@echo "Starting QEMU virt machine..."
	@echo "Press Ctrl-A then X to exit QEMU"
	@echo ""
	qemu-system-aarch64 \
		-M virt \
		-cpu cortex-a53 \
		-m 1G \
		-kernel $(ELF) \
		-serial stdio \
		-display none

# QEMU with graphics display
qemu-graphics: qemu-build
	@echo ""
	@echo "Starting QEMU with graphics (virtio-gpu-pci)..."
	@echo "Press Ctrl-A then X to exit QEMU"
	@echo ""
	qemu-system-aarch64 \
		-M virt \
		-cpu cortex-a53 \
		-m 1G \
		-kernel $(ELF) \
		-device virtio-gpu-pci \
		-serial stdio \
		-display cocoa

# Alias for qemu-graphics
qemu: qemu-graphics
