/*
 * Platform/Halt.h
 * Architecture-specific CPU halt helper used by cross-platform code.
 *
 * Provides a single function that stops execution in a platform-appropriate
 * way without relying on x86-specific instructions when building for ARM.
 */
#ifndef PLATFORM_HALT_H
#define PLATFORM_HALT_H

#if defined(__i386__) || defined(__x86_64__)
static inline void platform_halt(void) {
    __asm__ volatile("cli; hlt");
}
#elif defined(__arm__) || defined(__aarch64__)
static inline void platform_halt(void) {
    /* Wait for event keeps core in low-power loop on ARM */
    for (;;) {
        __asm__ volatile("wfe");
    }
}
#else
static inline void platform_halt(void) {
    /* Fallback busy loop for unknown architectures */
    for (;;) {
        __asm__ volatile("nop");
    }
}
#endif

#endif /* PLATFORM_HALT_H */
