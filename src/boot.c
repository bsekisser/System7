#include "Platform/include/boot.h"
#include "SystemInternal.h"
#include <stdint.h>

/* Forward declaration of kernel main */
extern void kernel_main(uint32_t magic, uint32_t* mb2_info);

void boot_main(uint32_t magic, uint32_t* mb2_info) {
    hal_boot_init(mb2_info);

    /* Call the kernel main function */
    kernel_main(magic, mb2_info);
}
