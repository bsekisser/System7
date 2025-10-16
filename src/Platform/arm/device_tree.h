#ifndef ARM_DEVICE_TREE_H
#define ARM_DEVICE_TREE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t base;
    uint64_t size;
} device_tree_reg_t;

void device_tree_init(void *dtb_ptr);
const char *device_tree_get_model(void);
uint32_t device_tree_get_memory_size(void);
const char *device_tree_get_property_string(const char *node, const char *prop);
uint32_t device_tree_get_property_u32(const char *node, const char *prop, uint32_t default_val);
void device_tree_dump(void);
size_t device_tree_find_compatible(const char *compatible,
                                   device_tree_reg_t *regs,
                                   size_t max_entries);

#endif /* ARM_DEVICE_TREE_H */
