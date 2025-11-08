/*
 * ARM64 GIC (Generic Interrupt Controller) Interface
 * GICv2 for Raspberry Pi 3/4/5
 */

#ifndef ARM64_GIC_H
#define ARM64_GIC_H

#include <stdint.h>
#include <stdbool.h>

/* Common ARM interrupt numbers */
#define IRQ_TIMER_PHYS      30  /* Physical timer interrupt (PPI) */
#define IRQ_TIMER_VIRT      27  /* Virtual timer interrupt (PPI) */

/* Initialize GIC */
bool gic_init(void);

/* Enable/disable interrupts */
void gic_enable_interrupt(uint32_t irq);
void gic_disable_interrupt(uint32_t irq);

/* Set interrupt priority (0 = highest, 255 = lowest) */
void gic_set_priority(uint32_t irq, uint8_t priority);

/* Interrupt handling */
uint32_t gic_acknowledge_interrupt(void);
void gic_end_interrupt(uint32_t irq);

/* Configure interrupt type */
void gic_set_config(uint32_t irq, bool edge_triggered);

/* Check initialization status */
bool gic_is_initialized(void);

#endif /* ARM64_GIC_H */
