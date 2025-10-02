/*
 * x86_io.h - Low-level x86 I/O port access functions
 *
 * Provides inline assembly wrappers for x86 IN/OUT instructions.
 */

#ifndef X86_IO_H
#define X86_IO_H

#include <stdint.h>

/* Byte I/O */
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

/* Word I/O */
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

/* Long I/O */
void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

#endif /* X86_IO_H */
