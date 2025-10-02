/* System 7.1 Standard Library Functions */

#ifndef SYSTEM71_STDLIB_H
#define SYSTEM71_STDLIB_H

#include <stdint.h>
#include <stddef.h>

/* Memory functions */
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

/* String functions */
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);

/* Conversion functions */
int atoi(const char* str);
long atol(const char* str);

/* Math functions */
int abs(int n);
long labs(long n);

/* Serial output functions */
void serial_init(void);
void serial_putchar(char c);
void serial_puts(const char* str);
int serial_data_ready(void);
char serial_getchar(void);
void serial_print_hex(uint32_t value);
void serial_printf(const char* fmt, ...);

#endif /* SYSTEM71_STDLIB_H */