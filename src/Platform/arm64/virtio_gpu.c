/*
 * VirtIO GPU Driver for ARM64
 * Implements proper virtio-gpu device interface
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "virtio_gpu.h"
#include "uart.h"

/* PCI configuration for QEMU virt machine */
#define PCI_ECAM_BASE           0x4010000000UL  /* PCI ECAM base address */
#define PCI_VENDOR_VIRTIO       0x1AF4
#define PCI_DEVICE_VIRTIO_GPU   0x1050

/* VirtIO PCI capability offsets */
#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_BAR0                0x10

static uintptr_t virtio_gpu_base = 0;

/* VirtIO MMIO registers */
#define VIRTIO_MMIO_MAGIC           0x000
#define VIRTIO_MMIO_VERSION         0x004
#define VIRTIO_MMIO_DEVICE_ID       0x008
#define VIRTIO_MMIO_VENDOR_ID       0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020
#define VIRTIO_MMIO_QUEUE_SEL       0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX   0x034
#define VIRTIO_MMIO_QUEUE_NUM       0x038
#define VIRTIO_MMIO_QUEUE_READY     0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY    0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS 0x060
#define VIRTIO_MMIO_INTERRUPT_ACK   0x064
#define VIRTIO_MMIO_STATUS          0x070
#define VIRTIO_MMIO_QUEUE_DESC_LOW  0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH 0x084
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW 0x090
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH 0x094
#define VIRTIO_MMIO_QUEUE_USED_LOW  0x0a0
#define VIRTIO_MMIO_QUEUE_USED_HIGH 0x0a4
#define VIRTIO_MMIO_CONFIG_GENERATION 0x0fc

/* VirtIO MMIO device region (for QEMU virt machine) */
#define VIRTIO_MMIO_BASE_START      0x0a000000
#define VIRTIO_MMIO_SLOT_SIZE       0x00000200

/* VirtIO device IDs */
#define VIRTIO_ID_GPU       16

/* VirtIO status bits */
#define VIRTIO_STATUS_ACKNOWLEDGE   1
#define VIRTIO_STATUS_DRIVER        2
#define VIRTIO_STATUS_DRIVER_OK     4
#define VIRTIO_STATUS_FEATURES_OK   8
#define VIRTIO_STATUS_FAILED        128

/* VirtIO GPU control types */
#define VIRTIO_GPU_CMD_GET_DISPLAY_INFO         0x0100
#define VIRTIO_GPU_CMD_RESOURCE_CREATE_2D       0x0101
#define VIRTIO_GPU_CMD_RESOURCE_UNREF           0x0102
#define VIRTIO_GPU_CMD_SET_SCANOUT              0x0103
#define VIRTIO_GPU_CMD_RESOURCE_FLUSH           0x0104
#define VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D      0x0105
#define VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING  0x0106
#define VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING  0x0107

#define VIRTIO_GPU_RESP_OK_NODATA               0x1100
#define VIRTIO_GPU_RESP_OK_DISPLAY_INFO         0x1101

/* VirtIO GPU formats */
#define VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM        2

/* Framebuffer configuration */
#define FB_WIDTH    320
#define FB_HEIGHT   240

/* VirtIO GPU structures */
struct virtio_gpu_ctrl_hdr {
    uint32_t type;
    uint32_t flags;
    uint64_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
} __attribute__((packed));

struct virtio_gpu_rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} __attribute__((packed));

struct virtio_gpu_resource_create_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t format;
    uint32_t width;
    uint32_t height;
} __attribute__((packed));

struct virtio_gpu_set_scanout {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32_t scanout_id;
    uint32_t resource_id;
} __attribute__((packed));

struct virtio_gpu_transfer_to_host_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint64_t offset;
    uint32_t resource_id;
    uint32_t padding;
} __attribute__((packed));

struct virtio_gpu_resource_flush {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32_t resource_id;
    uint32_t padding;
} __attribute__((packed));

struct virtio_gpu_mem_entry {
    uint64_t addr;
    uint32_t length;
    uint32_t padding;
} __attribute__((packed));

struct virtio_gpu_resource_attach_backing {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t nr_entries;
    struct virtio_gpu_mem_entry entries[1];
} __attribute__((packed));

/* Virtqueue descriptor */
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

/* Virtqueue available ring */
struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[32];
    uint16_t used_event;
} __attribute__((packed));

/* Virtqueue used element */
struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

/* Virtqueue used ring */
struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[32];
    uint16_t avail_event;
} __attribute__((packed));

/* Virtqueue structure */
struct virtqueue {
    struct virtq_desc desc[32];
    struct virtq_avail avail;
    uint8_t padding[4096 - sizeof(struct virtq_desc) * 32 - sizeof(struct virtq_avail)];
    struct virtq_used used;
} __attribute__((aligned(4096)));

/* Driver state - using static allocation with smaller buffers for testing */
static volatile uint32_t *virtio_base = NULL;
static struct virtqueue controlq __attribute__((aligned(4096)));
static uint32_t framebuffer[FB_WIDTH * FB_HEIGHT] __attribute__((aligned(4096)));
static bool initialized = false;
static uint16_t avail_idx = 0;
static uint16_t used_idx = 0;

/* Helper to read MMIO register */
static inline uint32_t virtio_read32(uint32_t offset) {
    return *(volatile uint32_t *)((uintptr_t)virtio_base + offset);
}

/* Helper to write MMIO register */
static inline void virtio_write32(uint32_t offset, uint32_t value) {
    *(volatile uint32_t *)((uintptr_t)virtio_base + offset) = value;
}

/* Helper to print hex digit */
static char hex_digit(unsigned int value) {
    value &= 0xF;
    if (value < 10) return '0' + value;
    return 'A' + (value - 10);
}

/* Helper to print hex value */
static void uart_put_hex(uint32_t value) {
    uart_putc(hex_digit(value >> 28));
    uart_putc(hex_digit(value >> 24));
    uart_putc(hex_digit(value >> 20));
    uart_putc(hex_digit(value >> 16));
    uart_putc(hex_digit(value >> 12));
    uart_putc(hex_digit(value >> 8));
    uart_putc(hex_digit(value >> 4));
    uart_putc(hex_digit(value));
}

/* PCI configuration space access helpers */
static inline uint32_t pci_config_read32(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
    uintptr_t addr = PCI_ECAM_BASE + ((uint64_t)bus << 20) + ((uint64_t)device << 15) +
                     ((uint64_t)function << 12) + offset;
    return *(volatile uint32_t *)addr;
}

static inline uint16_t pci_config_read16(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset) {
    uintptr_t addr = PCI_ECAM_BASE + ((uint64_t)bus << 20) + ((uint64_t)device << 15) +
                     ((uint64_t)function << 12) + offset;
    return *(volatile uint16_t *)addr;
}

static inline void pci_config_write16(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint16_t value) {
    uintptr_t addr = PCI_ECAM_BASE + ((uint64_t)bus << 20) + ((uint64_t)device << 15) +
                     ((uint64_t)function << 12) + offset;
    *(volatile uint16_t *)addr = value;
}

/* Send GPU command and wait for response */
static bool virtio_gpu_send_cmd(void *cmd, size_t cmd_len, void *resp, size_t resp_len) {
    uint16_t desc_idx = avail_idx % 32;

    /* Setup command descriptor */
    controlq.desc[desc_idx].addr = (uint64_t)(uintptr_t)cmd;
    controlq.desc[desc_idx].len = cmd_len;
    controlq.desc[desc_idx].flags = 1; /* NEXT */
    controlq.desc[desc_idx].next = (desc_idx + 1) % 32;

    /* Setup response descriptor */
    controlq.desc[(desc_idx + 1) % 32].addr = (uint64_t)(uintptr_t)resp;
    controlq.desc[(desc_idx + 1) % 32].len = resp_len;
    controlq.desc[(desc_idx + 1) % 32].flags = 2; /* WRITE */
    controlq.desc[(desc_idx + 1) % 32].next = 0;

    /* Add to available ring */
    controlq.avail.ring[avail_idx % 32] = desc_idx;
    __sync_synchronize();
    controlq.avail.idx = ++avail_idx;
    __sync_synchronize();

    /* Notify device */
    virtio_write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0);

    /* Wait for completion */
    while (controlq.used.idx == used_idx) {
        __asm__ volatile("" ::: "memory");
    }
    used_idx++;

    return true;
}

bool virtio_gpu_init(void) {
    uart_puts("[VIRTIO-GPU] ENTRY\n");

    uint32_t magic, version, device_id;
    struct virtio_gpu_ctrl_hdr resp_hdr;

    uart_puts("[VIRTIO-GPU] Initializing virtio-gpu driver\n");

#ifdef USE_VIRTIO_PCI
    /* PCI transport - requires MMU to access ECAM at 0x4010000000 */
    uart_puts("[VIRTIO-GPU] Scanning PCI bus...\n");

    for (uint32_t device = 0; device < 32; device++) {
        uint16_t vendor_id = pci_config_read16(0, device, 0, PCI_VENDOR_ID);

        /* No device present */
        if (vendor_id == 0xFFFF) continue;

        uint16_t device_id = pci_config_read16(0, device, 0, PCI_DEVICE_ID);

        /* Debug output */
        uart_puts("[VIRTIO-GPU] PCI ");
        if (device < 10) {
            uart_putc('0' + device);
        } else {
            uart_putc('0' + (device / 10));
            uart_putc('0' + (device % 10));
        }
        uart_puts(": Vendor=0x");
        uart_put_hex(vendor_id);
        uart_puts(" Device=0x");
        uart_put_hex(device_id);
        uart_puts("\n");

        /* Check for VirtIO GPU */
        if (vendor_id == PCI_VENDOR_VIRTIO && device_id == PCI_DEVICE_VIRTIO_GPU) {
            uart_puts("[VIRTIO-GPU] Found VirtIO GPU at PCI device ");
            if (device < 10) {
                uart_putc('0' + device);
            } else {
                uart_putc('0' + (device / 10));
                uart_putc('0' + (device % 10));
            }
            uart_puts("!\n");

            /* Read BAR0 to get MMIO base address */
            uint32_t bar0 = pci_config_read32(0, device, 0, PCI_BAR0);
            virtio_gpu_base = (uintptr_t)(bar0 & 0xFFFFFFF0); /* Clear flag bits */

            uart_puts("[VIRTIO-GPU] BAR0: 0x");
            uart_put_hex(bar0);
            uart_puts("\n");
            uart_puts("[VIRTIO-GPU] MMIO base: 0x");
            uart_put_hex(virtio_gpu_base);
            uart_puts("\n");

            /* Enable PCI device (Memory Space Enable + Bus Master Enable) */
            uint16_t command = pci_config_read16(0, device, 0, PCI_COMMAND);
            command |= 0x0006; /* Memory Space + Bus Master */
            pci_config_write16(0, device, 0, PCI_COMMAND, command);

            uart_puts("[VIRTIO-GPU] PCI device enabled\n");
            break;
        }
    }

    uart_puts("[VIRTIO-GPU] PCI scan complete\n");
#else
    /* MMIO transport - works without MMU */
    uart_puts("[VIRTIO-GPU] Scanning VirtIO MMIO devices...\n");

    for (int slot = 0; slot < 32; slot++) {
        uintptr_t base = VIRTIO_MMIO_BASE_START + (slot * VIRTIO_MMIO_SLOT_SIZE);
        virtio_base = (volatile uint32_t *)base;

        magic = virtio_read32(VIRTIO_MMIO_MAGIC);

        /* No device at this slot */
        if (magic != 0x74726976) continue;

        /* Read device ID */
        device_id = virtio_read32(VIRTIO_MMIO_DEVICE_ID);

        if (device_id == VIRTIO_ID_GPU) {
            uart_puts("[VIRTIO-GPU] Found GPU at MMIO slot ");
            if (slot < 10) {
                uart_putc('0' + slot);
            } else {
                uart_putc('0' + (slot / 10));
                uart_putc('0' + (slot % 10));
            }
            uart_puts("!\n");
            virtio_gpu_base = base;
            break;
        }
    }

    uart_puts("[VIRTIO-GPU] MMIO scan complete\n");
#endif

    if (virtio_gpu_base == 0) {
        uart_puts("[VIRTIO-GPU] No GPU device found\n");
        return false;
    }

    uart_puts("[VIRTIO-GPU] Using base: 0x");
    uart_put_hex(virtio_gpu_base);
    uart_puts("\n");

    virtio_base = (volatile uint32_t *)virtio_gpu_base;

    uart_puts("[VIRTIO-GPU] Reading magic...\n");
    /* Check VirtIO magic */
    magic = virtio_read32(VIRTIO_MMIO_MAGIC);
    uart_puts("[VIRTIO-GPU] Magic read complete\n");
    uart_puts("[VIRTIO-GPU] Magic: 0x");
    uart_put_hex(magic);
    uart_puts("\n");
    if (magic != 0x74726976) {  /* 'virt' */
        uart_puts("[VIRTIO-GPU] Invalid magic number\n");
        return false;
    }

    version = virtio_read32(VIRTIO_MMIO_VERSION);
    uart_puts("[VIRTIO-GPU] Version: 0x");
    uart_put_hex(version);
    uart_puts("\n");
    if (version != 1 && version != 2) {
        uart_puts("[VIRTIO-GPU] Unsupported version (expected 1 or 2)\n");
        return false;
    }

    device_id = virtio_read32(VIRTIO_MMIO_DEVICE_ID);
    uart_puts("[VIRTIO-GPU] Device ID: 0x");
    uart_put_hex(device_id);
    uart_puts("\n");
    if (device_id != VIRTIO_ID_GPU) {
        uart_puts("[VIRTIO-GPU] Not a GPU device\n");
        return false;
    }

    uart_puts("[VIRTIO-GPU] Found virtio-gpu device\n");

    /* Reset device */
    virtio_write32(VIRTIO_MMIO_STATUS, 0);

    /* Acknowledge device */
    virtio_write32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE);

    /* Set driver status */
    virtio_write32(VIRTIO_MMIO_STATUS,
                   VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

    /* Read and accept features */
    virtio_read32(VIRTIO_MMIO_DEVICE_FEATURES);
    virtio_write32(VIRTIO_MMIO_DRIVER_FEATURES, 0);

    /* Features OK */
    virtio_write32(VIRTIO_MMIO_STATUS,
                   VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER |
                   VIRTIO_STATUS_FEATURES_OK);

    /* Setup control queue */
    virtio_write32(VIRTIO_MMIO_QUEUE_SEL, 0);

    uint32_t max_queue_size = virtio_read32(VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (max_queue_size < 32) {
        uart_puts("[VIRTIO-GPU] Queue too small\n");
        return false;
    }

    virtio_write32(VIRTIO_MMIO_QUEUE_NUM, 32);

    /* Set queue addresses */
    uint64_t desc_addr = (uint64_t)(uintptr_t)&controlq.desc;
    uint64_t avail_addr = (uint64_t)(uintptr_t)&controlq.avail;
    uint64_t used_addr = (uint64_t)(uintptr_t)&controlq.used;

    virtio_write32(VIRTIO_MMIO_QUEUE_DESC_LOW, desc_addr & 0xFFFFFFFF);
    virtio_write32(VIRTIO_MMIO_QUEUE_DESC_HIGH, desc_addr >> 32);
    virtio_write32(VIRTIO_MMIO_QUEUE_AVAIL_LOW, avail_addr & 0xFFFFFFFF);
    virtio_write32(VIRTIO_MMIO_QUEUE_AVAIL_HIGH, avail_addr >> 32);
    virtio_write32(VIRTIO_MMIO_QUEUE_USED_LOW, used_addr & 0xFFFFFFFF);
    virtio_write32(VIRTIO_MMIO_QUEUE_USED_HIGH, used_addr >> 32);

    virtio_write32(VIRTIO_MMIO_QUEUE_READY, 1);

    /* Driver OK */
    virtio_write32(VIRTIO_MMIO_STATUS,
                   VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER |
                   VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);

    uart_puts("[VIRTIO-GPU] Device initialized\n");

    /* Create 2D resource */
    struct virtio_gpu_resource_create_2d create_cmd = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0
        },
        .resource_id = 1,
        .format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM,
        .width = FB_WIDTH,
        .height = FB_HEIGHT
    };

    if (!virtio_gpu_send_cmd(&create_cmd, sizeof(create_cmd), &resp_hdr, sizeof(resp_hdr))) {
        uart_puts("[VIRTIO-GPU] Failed to create resource\n");
        return false;
    }

    uart_puts("[VIRTIO-GPU] Created 2D resource\n");

    /* Attach backing store */
    struct {
        struct virtio_gpu_resource_attach_backing attach;
    } attach_cmd = {
        .attach = {
            .hdr = {
                .type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
                .flags = 0,
                .fence_id = 0,
                .ctx_id = 0,
                .padding = 0
            },
            .resource_id = 1,
            .nr_entries = 1,
            .entries = {{
                .addr = (uint64_t)(uintptr_t)framebuffer,
                .length = FB_WIDTH * FB_HEIGHT * 4,
                .padding = 0
            }}
        }
    };

    if (!virtio_gpu_send_cmd(&attach_cmd, sizeof(attach_cmd), &resp_hdr, sizeof(resp_hdr))) {
        uart_puts("[VIRTIO-GPU] Failed to attach backing\n");
        return false;
    }

    uart_puts("[VIRTIO-GPU] Attached backing store\n");

    /* Set scanout */
    struct virtio_gpu_set_scanout scanout_cmd = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_SET_SCANOUT,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = FB_WIDTH,
            .height = FB_HEIGHT
        },
        .scanout_id = 0,
        .resource_id = 1
    };

    if (!virtio_gpu_send_cmd(&scanout_cmd, sizeof(scanout_cmd), &resp_hdr, sizeof(resp_hdr))) {
        uart_puts("[VIRTIO-GPU] Failed to set scanout\n");
        return false;
    }

    uart_puts("[VIRTIO-GPU] Set scanout complete\n");

    initialized = true;
    return true;
}

void virtio_gpu_flush(void) {
    if (!initialized) return;

    struct virtio_gpu_ctrl_hdr resp_hdr;

    /* Transfer to host */
    struct virtio_gpu_transfer_to_host_2d transfer_cmd = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = FB_WIDTH,
            .height = FB_HEIGHT
        },
        .offset = 0,
        .resource_id = 1,
        .padding = 0
    };

    virtio_gpu_send_cmd(&transfer_cmd, sizeof(transfer_cmd), &resp_hdr, sizeof(resp_hdr));

    /* Flush */
    struct virtio_gpu_resource_flush flush_cmd = {
        .hdr = {
            .type = VIRTIO_GPU_CMD_RESOURCE_FLUSH,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = FB_WIDTH,
            .height = FB_HEIGHT
        },
        .resource_id = 1,
        .padding = 0
    };

    virtio_gpu_send_cmd(&flush_cmd, sizeof(flush_cmd), &resp_hdr, sizeof(resp_hdr));
}

void virtio_gpu_clear(uint32_t color) {
    if (!initialized) return;

    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        framebuffer[i] = color;
    }
}

void virtio_gpu_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!initialized) return;

    for (uint32_t py = y; py < y + h && py < FB_HEIGHT; py++) {
        for (uint32_t px = x; px < x + w && px < FB_WIDTH; px++) {
            framebuffer[py * FB_WIDTH + px] = color;
        }
    }
}

uint32_t* virtio_gpu_get_buffer(void) {
    return framebuffer;
}

uint32_t virtio_gpu_get_width(void) {
    return FB_WIDTH;
}

uint32_t virtio_gpu_get_height(void) {
    return FB_HEIGHT;
}
