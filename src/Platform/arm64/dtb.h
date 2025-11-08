/*
 * ARM64 Device Tree Blob Parser Interface
 * For reading hardware configuration from DTB
 */

#ifndef ARM64_DTB_H
#define ARM64_DTB_H

#include <stdint.h>
#include <stdbool.h>

/* Initialize DTB parser with pointer to DTB */
bool dtb_init(void *dtb);

/* Get DTB information */
uint32_t dtb_get_version(void);
uint32_t dtb_get_size(void);

/* Get property from node */
const void *dtb_get_property(const char *node_path, const char *property, uint32_t *len);
bool dtb_get_property_u32(const char *node_path, const char *property, uint32_t *value);

/* Get common information */
bool dtb_get_memory(uint64_t *base, uint64_t *size);
const char *dtb_get_model(void);

/* Check initialization status */
bool dtb_is_initialized(void);

#endif /* ARM64_DTB_H */
