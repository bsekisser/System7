# ARM64 Kernel Makefile
# Build for Raspberry Pi 3/4/5 and QEMU

# Toolchain
PREFIX = aarch64-none-elf-
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

# Assembler flags
ASFLAGS = -mcpu=cortex-a53

# Linker flags
LDFLAGS = -T link.ld -nostdlib

# Source files
ASM_SOURCES = platform_boot.S
C_SOURCES = hal_boot.c uart.c timer.c gic.c mailbox.c framebuffer.c dtb.c cache.c mmu.c

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

.PHONY: all clean qemu

all: $(IMAGE)

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

# QEMU testing on Apple Silicon Mac
qemu: $(IMAGE)
	qemu-system-aarch64 \
		-M raspi3b \
		-kernel $(IMAGE) \
		-serial stdio \
		-display none

# QEMU with virtual machine (virt)
qemu-virt: $(IMAGE)
	qemu-system-aarch64 \
		-M virt \
		-cpu cortex-a53 \
		-m 1G \
		-kernel $(ELF) \
		-serial stdio \
		-display none
