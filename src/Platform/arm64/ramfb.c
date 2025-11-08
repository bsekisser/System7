/*
 * QEMU ramfb (RAM Framebuffer) Driver
 * Configures QEMU's ramfb device to display our framebuffer
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ramfb.h"
#include "uart.h"

/* QEMU fw_cfg interface */
#define FW_CFG_CTL      0x09020000
#define FW_CFG_DATA     0x09020001
#define FW_CFG_DMA      0x09020010

/* fw_cfg selectors */
#define FW_CFG_SIGNATURE    0x0000
#define FW_CFG_ID           0x0001
#define FW_CFG_FILE_DIR     0x0019

/* ramfb configuration structure */
struct ramfb_cfg {
    uint64_t addr;      /* Framebuffer physical address */
    uint32_t fourcc;    /* Pixel format (XRGB8888 = 0x34325258) */
    uint32_t flags;     /* Flags (0) */
    uint32_t width;     /* Width in pixels */
    uint32_t height;    /* Height in pixels */
    uint32_t stride;    /* Bytes per line */
} __attribute__((packed));

/* Framebuffer: 320x240 pixels */
#define FB_WIDTH  320
#define FB_HEIGHT 240

/* Framebuffer in BSS */
static uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
static bool initialized = false;

/* Helper to write fw_cfg register */
static inline void fw_cfg_write_ctl(uint16_t selector) {
    volatile uint16_t *ctl = (volatile uint16_t *)FW_CFG_CTL;
    *ctl = selector;
}

/* Helper to read fw_cfg data */
static inline uint8_t fw_cfg_read_data(void) {
    volatile uint8_t *data = (volatile uint8_t *)FW_CFG_DATA;
    return *data;
}

/* Helper to write fw_cfg data */
static inline void fw_cfg_write_data(uint8_t value) {
    volatile uint8_t *data = (volatile uint8_t *)FW_CFG_DATA;
    *data = value;
}

/* Find ramfb configuration file in fw_cfg */
static bool fw_cfg_find_file(const char *name, uint32_t *selector, uint32_t *size) {
    uint32_t count;

    /* Read file directory */
    fw_cfg_write_ctl(FW_CFG_FILE_DIR);

    /* Read file count (big-endian) */
    count = ((uint32_t)fw_cfg_read_data() << 24) |
            ((uint32_t)fw_cfg_read_data() << 16) |
            ((uint32_t)fw_cfg_read_data() << 8) |
            (uint32_t)fw_cfg_read_data();

    uart_puts("[RAMFB] fw_cfg file count: ");
    char buf[16];
    buf[0] = '0' + (count / 10);
    buf[1] = '0' + (count % 10);
    buf[2] = '\n';
    buf[3] = 0;
    uart_puts(buf);

    /* Search for file */
    for (uint32_t i = 0; i < count; i++) {
        uint32_t file_size = ((uint32_t)fw_cfg_read_data() << 24) |
                             ((uint32_t)fw_cfg_read_data() << 16) |
                             ((uint32_t)fw_cfg_read_data() << 8) |
                             (uint32_t)fw_cfg_read_data();

        uint16_t file_selector = ((uint16_t)fw_cfg_read_data() << 8) |
                                 (uint16_t)fw_cfg_read_data();

        /* Skip reserved field */
        fw_cfg_read_data();
        fw_cfg_read_data();

        /* Read filename (56 bytes) */
        char filename[57];
        for (int j = 0; j < 56; j++) {
            filename[j] = fw_cfg_read_data();
        }
        filename[56] = 0;

        /* Check if this is ramfb */
        bool match = true;
        for (int j = 0; name[j] != 0 && j < 56; j++) {
            if (filename[j] != name[j]) {
                match = false;
                break;
            }
        }

        if (match && filename[0] == name[0]) {
            *selector = file_selector;
            *size = file_size;
            uart_puts("[RAMFB] Found: ");
            uart_puts(filename);
            uart_puts("\n");
            return true;
        }
    }

    return false;
}

bool ramfb_init(void) {
    uint32_t selector, size;
    struct ramfb_cfg cfg;

    uart_puts("[RAMFB] Initializing ramfb driver\n");

    /* Check fw_cfg signature */
    fw_cfg_write_ctl(FW_CFG_SIGNATURE);
    char sig[5];
    sig[0] = fw_cfg_read_data();
    sig[1] = fw_cfg_read_data();
    sig[2] = fw_cfg_read_data();
    sig[3] = fw_cfg_read_data();
    sig[4] = 0;

    uart_puts("[RAMFB] fw_cfg signature: ");
    uart_puts(sig);
    uart_puts("\n");

    if (sig[0] != 'Q' || sig[1] != 'E' || sig[2] != 'M' || sig[3] != 'U') {
        uart_puts("[RAMFB] fw_cfg not found\n");
        return false;
    }

    /* Find ramfb configuration file */
    if (!fw_cfg_find_file("etc/ramfb", &selector, &size)) {
        uart_puts("[RAMFB] ramfb not available (add -device ramfb to QEMU)\n");
        return false;
    }

    /* Configure ramfb */
    cfg.addr = (uint64_t)(uintptr_t)framebuffer;
    cfg.fourcc = 0x34325258;  /* XRGB8888 */
    cfg.flags = 0;
    cfg.width = FB_WIDTH;
    cfg.height = FB_HEIGHT;
    cfg.stride = FB_WIDTH * 4;

    uart_puts("[RAMFB] Configuring ramfb: 320x240\n");

    /* Write configuration */
    fw_cfg_write_ctl(selector);
    uint8_t *cfg_bytes = (uint8_t *)&cfg;
    for (size_t i = 0; i < sizeof(cfg); i++) {
        fw_cfg_write_data(cfg_bytes[i]);
    }

    initialized = true;
    uart_puts("[RAMFB] Initialization complete\n");

    return true;
}

void ramfb_clear(uint32_t color) {
    if (!initialized) return;

    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        framebuffer[i] = color;
    }
}

void ramfb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!initialized) return;

    for (uint32_t py = y; py < y + h && py < FB_HEIGHT; py++) {
        for (uint32_t px = x; px < x + w && px < FB_WIDTH; px++) {
            framebuffer[py * FB_WIDTH + px] = color;
        }
    }
}

uint32_t* ramfb_get_buffer(void) {
    return framebuffer;
}

uint32_t ramfb_get_width(void) {
    return FB_WIDTH;
}

uint32_t ramfb_get_height(void) {
    return FB_HEIGHT;
}
