/*
 * SDHCI SD/eMMC Card Driver for Raspberry Pi
 * Implements block device interface for storage access
 *
 * SDHCI = SD Host Controller Interface (standard specification)
 * This driver supports:
 * - Pi 3: SDHCI over Broadcom DMA
 * - Pi 4: SDHCI over PCIe (similar interface)
 * - Pi 5: SDHCI over PCIe (faster)
 *
 * References:
 *   - JEDEC SD Specification
 *   - SD Host Controller Standard Specification
 *   - Raspberry Pi Firmware source
 */

#include <stdint.h>
#include <stddef.h>
#include "System71StdLib.h"
#include "mmio.h"

/* SDHCI register base addresses
 * These vary by Raspberry Pi model
 */
#define SDHCI_BASE_PI34         0x3F300000
#define SDHCI_BASE_PI5          0xFE330000

static uint32_t sdhci_base = 0;
static int sdhci_initialized = 0;

/* SDHCI register offsets */
#define SDHCI_DMA_ADDRESS       0x00
#define SDHCI_BLOCK_SIZE        0x04
#define SDHCI_BLOCK_COUNT       0x06
#define SDHCI_COMMAND           0x0C
#define SDHCI_ARGUMENT          0x08
#define SDHCI_RESPONSE          0x10
#define SDHCI_BUFFER_DATA_PORT  0x20
#define SDHCI_PRESENT_STATE     0x24
#define SDHCI_HOST_CONTROL      0x28
#define SDHCI_POWER_CONTROL     0x29
#define SDHCI_BLOCK_GAP_CONTROL 0x2A
#define SDHCI_WAKE_UP_CONTROL   0x2B
#define SDHCI_CLOCK_CONTROL     0x2C
#define SDHCI_TIMEOUT_CONTROL   0x2E
#define SDHCI_SOFTWARE_RESET    0x2F
#define SDHCI_INT_STATUS        0x30
#define SDHCI_INT_ENABLE        0x34
#define SDHCI_INT_SIGNAL_ENABLE 0x38
#define SDHCI_CAPABILITIES      0x40
#define SDHCI_CAPABILITIES_1    0x44

/*
 * Initialize SDHCI controller
 */
int sdhci_init(void) {
    Serial_WriteString("[SDHCI] Initializing SD card controller\n");

    /* Detect Pi model and set base address */
    /* TODO: Read device tree to determine correct base
     * For now, try Pi 3/4 first (most common)
     */
    sdhci_base = SDHCI_BASE_PI34;

    /* Verify we can read registers */
    uint32_t caps = mmio_read32(sdhci_base + SDHCI_CAPABILITIES);
    if (caps == 0) {
        Serial_WriteString("[SDHCI] Error: Cannot read SDHCI capabilities\n");
        return -1;
    }

    Serial_Printf("[SDHCI] Base address: 0x%x, Capabilities: 0x%x\n", sdhci_base, caps);

    /* Reset controller */
    uint8_t reset = mmio_read8(sdhci_base + SDHCI_SOFTWARE_RESET);
    reset |= 0x01;  /* Reset all */
    mmio_write8(sdhci_base + SDHCI_SOFTWARE_RESET, reset);

    /* Wait for reset to complete */
    uint32_t timeout = 10000;
    while (timeout--) {
        reset = mmio_read8(sdhci_base + SDHCI_SOFTWARE_RESET);
        if ((reset & 0x01) == 0) {
            break;
        }
    }

    if (timeout == 0) {
        Serial_WriteString("[SDHCI] Error: Reset timeout\n");
        return -1;
    }

    Serial_WriteString("[SDHCI] Controller reset complete\n");

    /* TODO: Initialize card:
     * 1. Enable clock
     * 2. Send CMD0 (reset)
     * 3. Send CMD1 (init, for eMMC) or ACMD41 (init for SD)
     * 4. Send CMD2 (get CID)
     * 5. Send CMD3 (get RCA)
     * 6. Send CMD7 (select card)
     * 7. Set block size and other parameters
     */

    sdhci_initialized = 1;
    return 0;
}

/*
 * Read blocks from SD card
 * addr: Starting LBA (Logical Block Address)
 * count: Number of 512-byte blocks to read
 * buffer: Destination buffer
 */
int sdhci_read_blocks(uint32_t addr, uint32_t count, void *buffer) {
    if (!sdhci_initialized || !buffer) {
        return -1;
    }

    Serial_Printf("[SDHCI] Reading %u blocks from LBA 0x%x\n", count, addr);

    /* TODO: Implement block read:
     * 1. Set DMA address to buffer
     * 2. Set block size to 512
     * 3. Set block count to count
     * 4. Send CMD18 (read multiple blocks) with address
     * 5. Wait for interrupt or busy
     * 6. Check for errors
     */

    /* Stub: return success */
    return count;
}

/*
 * Write blocks to SD card
 * addr: Starting LBA
 * count: Number of 512-byte blocks to write
 * buffer: Source buffer
 */
int sdhci_write_blocks(uint32_t addr, uint32_t count, const void *buffer) {
    if (!sdhci_initialized || !buffer) {
        return -1;
    }

    Serial_Printf("[SDHCI] Writing %u blocks to LBA 0x%x\n", count, addr);

    /* TODO: Implement block write:
     * 1. Set DMA address to buffer
     * 2. Set block size to 512
     * 3. Set block count to count
     * 4. Send CMD25 (write multiple blocks) with address
     * 5. Wait for interrupt or busy
     * 6. Check for errors
     */

    /* Stub: return success */
    return count;
}

/*
 * Get SD card information
 */
int sdhci_get_card_info(void *info) {
    if (!sdhci_initialized) {
        return -1;
    }

    /* TODO: Return card info structure */
    return 0;
}

/*
 * Shutdown SDHCI controller
 */
void sdhci_shutdown(void) {
    Serial_WriteString("[SDHCI] Shutting down SD card controller\n");

    if (sdhci_base) {
        /* Disable interrupts */
        mmio_write32(sdhci_base + SDHCI_INT_ENABLE, 0);
        mmio_write32(sdhci_base + SDHCI_INT_SIGNAL_ENABLE, 0);
    }

    sdhci_initialized = 0;
}

/*
 * Check if card is detected
 */
int sdhci_card_present(void) {
    if (!sdhci_initialized) {
        return 0;
    }

    uint32_t state = mmio_read32(sdhci_base + SDHCI_PRESENT_STATE);
    /* Bit 16: Card inserted */
    return (state & (1 << 16)) != 0;
}
