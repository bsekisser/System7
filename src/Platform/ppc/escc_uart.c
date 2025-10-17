/*
 * escc_uart.c - ESCC (Zilog Z8530) serial driver for QEMU PowerPC mac99
 *
 * The mac99 machine uses an ESCC controller (Enhanced Serial Communications
 * Controller). This is a simplified driver that just writes to the data register.
 *
 * Note: The ESCC on QEMU mac99 appears to be directly connected to the serial
 * output, so we just write characters directly without complex initialization.
 */

#include <stdint.h>
#include <stdbool.h>

#include "Platform/PowerPC/escc_uart.h"

/* ESCC on QEMU mac99: Channel offsets from 0x80013000 */
#define ESCC_BASE_ADDRESS       ((uintptr_t)0x80013000u)
#define ESCC_CH_A_CONTROL       ((volatile uint8_t*)(ESCC_BASE_ADDRESS + 0x04u))
#define ESCC_CH_A_DATA          ((volatile uint8_t*)(ESCC_BASE_ADDRESS + 0x05u))

/* ESCC register numbers (write) */
#define ESCC_WR0                 0u
#define ESCC_WR1                 1u
#define ESCC_WR3                 3u
#define ESCC_WR4                 4u
#define ESCC_WR5                 5u
#define ESCC_WR9                 9u
#define ESCC_WR11               11u
#define ESCC_WR12               12u
#define ESCC_WR13               13u
#define ESCC_WR14               14u

/* ESCC read register 0 status bits */
#define ESCC_RR0_TX_BUFFER_EMPTY (1u << 2)

/* Helper to ensure register accesses are ordered */
#define escc_barrier() __asm__ volatile("eieio" ::: "memory")

static volatile bool g_escc_initialized = false;

static inline void escc_write_index(uint8_t reg) {
    *ESCC_CH_A_CONTROL = reg;
    escc_barrier();
}

static inline void escc_write_reg(uint8_t reg, uint8_t value) {
    escc_write_index(reg);
    *ESCC_CH_A_CONTROL = value;
    escc_barrier();
}

static inline uint8_t escc_read_reg(uint8_t reg) {
    escc_write_index(reg);
    return *ESCC_CH_A_CONTROL;
}

static inline uint8_t escc_read_status(void) {
    /* Status lives in RR0 when index set to 0 */
    escc_write_index(ESCC_WR0);
    return *ESCC_CH_A_CONTROL;
}

static inline void escc_wait_tx_ready(void) {
    int spin = 0;
    const int kMaxSpins = 100000;
    while (spin < kMaxSpins) {
        if (escc_read_status() & ESCC_RR0_TX_BUFFER_EMPTY) {
            return;
        }
        ++spin;
    }
}

/*
 * Initialize ESCC - minimal setup
 */
void escc_init(void) {
    if (g_escc_initialized) {
        return;
    }

    /* Reset both channels to a known state. This temporarily disrupts OF console. */
    escc_write_reg(ESCC_WR9, 0xC0);   /* Reset A/B */

    /* Baud rate generator: point clock to BRG, 16x, 8N1 @ 115200. */
    escc_write_reg(ESCC_WR14, 0x03);  /* Enable BRG, source from crystal */
    escc_write_reg(ESCC_WR11, 0x50);  /* Transmit clock = BRG, receive clock = BRG */
    escc_write_reg(ESCC_WR12, 0x00);  /* BRG time constant low byte (default 0 => 115200 with 14.7456MHz) */
    escc_write_reg(ESCC_WR13, 0x00);  /* BRG time constant high byte */
    escc_write_reg(ESCC_WR4, 0x44);   /* x16 clock, 1 stop bit, no parity, 8-bit */

    /* Disable interrupts while we bring link up. */
    escc_write_reg(ESCC_WR1, 0x00);

    /* Receiver: enable, 8-bit, no auto modes. */
    escc_write_reg(ESCC_WR3, 0xC1);

    /* Transmitter: 8-bit, assert DTR/RTS, enable TX. */
    escc_write_reg(ESCC_WR5, 0xEA);

    /* Ensure index points back at RR0 for status reads. */
    escc_write_index(ESCC_WR0);

    g_escc_initialized = true;
}

/*
 * Send a single character
 */
void escc_putchar(char c) {
    if (!g_escc_initialized) {
        escc_init();
    }

    escc_wait_tx_ready();

    if (c == '\n') {
        escc_wait_tx_ready();
        *ESCC_CH_A_DATA = (uint8_t)'\r';
        escc_barrier();
    }

    *ESCC_CH_A_DATA = (uint8_t)c;
    escc_barrier();
}

/*
 * Send a string
 */
void escc_puts(const char *str) {
    if (!str) {
        return;
    }

    while (*str) {
        escc_putchar(*str++);
    }
}

/*
 * Check if character is available (non-blocking)
 */
bool escc_rx_ready(void) {
    if (!g_escc_initialized) {
        escc_init();
    }
    /* Simplified - just return false for now */
    return false;
}

/*
 * Receive a single character (blocking)
 */
char escc_getchar(void) {
    if (!g_escc_initialized) {
        escc_init();
    }
    /* Simplified - return null */
    return '\0';
}
