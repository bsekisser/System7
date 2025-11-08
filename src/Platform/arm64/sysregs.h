/*
 * ARM64 System Register Inspection Interface
 */

#ifndef ARM64_SYSREGS_H
#define ARM64_SYSREGS_H

/* Display CPU features */
void sysregs_show_cpu_features(void);

/* Display cache information */
void sysregs_show_cache_info(void);

/* Display current system state */
void sysregs_show_current_state(void);

#endif /* ARM64_SYSREGS_H */
