/*
 * x86_io.c - Low-level x86 I/O port access functions
 *
 * Provides inline assembly wrappers for x86 IN/OUT instructions.
 */

#include <stdint.h>
#include "Platform/PlatformLogging.h"

/*
 * outb - Write byte to I/O port
 *
 * @param port - I/O port address
 * @param value - Byte value to write
 */
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inb - Read byte from I/O port
 *
 * @param port - I/O port address
 * @return Byte value read from port
 */
uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * outw - Write word to I/O port
 *
 * @param port - I/O port address
 * @param value - Word value to write
 */
void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inw - Read word from I/O port
 *
 * @param port - I/O port address
 * @return Word value read from port
 */
uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * outl - Write long to I/O port
 *
 * @param port - I/O port address
 * @param value - Long value to write
 */
void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inl - Read long from I/O port
 *
 * @param port - I/O port address
 * @return Long value read from port
 */
uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
