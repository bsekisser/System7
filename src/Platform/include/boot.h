#ifndef HAL_BOOT_H
#define HAL_BOOT_H

#include <stdint.h>

typedef struct {
    void *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t depth;
    uint8_t red_offset;
    uint8_t red_size;
    uint8_t green_offset;
    uint8_t green_size;
    uint8_t blue_offset;
    uint8_t blue_size;
} hal_framebuffer_info_t;

void hal_boot_init(void *boot_arg);
int hal_get_framebuffer_info(hal_framebuffer_info_t *info);
uint32_t hal_get_memory_size(void);
int hal_platform_init(void);
void hal_platform_shutdown(void);
int hal_framebuffer_present(void);

#endif /* HAL_BOOT_H */
