/*
 * stdlib.h - Minimal standard library for bare-metal ARM64 kernel
 */

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory allocation (minimal - just declarations) */
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

/* Conversion functions */
int atoi(const char *nptr);
long atol(const char *nptr);
long strtol(const char *nptr, char **endptr, int base);

/* Utility functions */
void abort(void);
void exit(int status);

/* Integer absolute value functions */
int abs(int x);
long labs(long x);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H_ */
