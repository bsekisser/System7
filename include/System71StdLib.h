/* System 7.1 Standard Library Functions */

#ifndef SYSTEM71_STDLIB_H
#define SYSTEM71_STDLIB_H

#include <stdint.h>
#include <stddef.h>

/* Logging ------------------------------------------------------------------ */

typedef enum {
    kLogLevelError = 0,
    kLogLevelWarn,
    kLogLevelInfo,
    kLogLevelDebug,
    kLogLevelTrace
} SystemLogLevel;

typedef enum {
    kLogModuleGeneral = 0,
    kLogModuleDesktop,
    kLogModuleEvent,
    kLogModuleFinder,
    kLogModuleFileSystem,
    kLogModuleWindow,
    kLogModuleMenu,
    kLogModuleDialog,
    kLogModuleControl,
    kLogModuleFont,
    kLogModuleSound,
    kLogModuleResource,
    kLogModuleStandardFile,
    kLogModuleListManager,
    kLogModuleSystem,
    kLogModuleTextEdit,
    kLogModulePlatform,
    kLogModuleScrap,
    kLogModuleMemory,
    kLogModuleProcess,
    kLogModuleSegmentLoader,
    kLogModuleCPU,
    kLogModuleCount
} SystemLogModule;

void SysLogSetGlobalLevel(SystemLogLevel level);
SystemLogLevel SysLogGetGlobalLevel(void);
void SysLogSetModuleLevel(SystemLogModule module, SystemLogLevel level);
SystemLogLevel SysLogGetModuleLevel(SystemLogModule module);
const char* SysLogModuleName(SystemLogModule module);
void serial_logf(SystemLogModule module, SystemLogLevel level, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

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
void serial_printf(const char* fmt, ...)
    __attribute__((format(printf, 1, 2)));

/* Compatibility aliases used by some platform ports */
#define Serial_WriteString(...) serial_printf(__VA_ARGS__)
#define Serial_Printf(...)      serial_printf(__VA_ARGS__)
int sprintf(char* str, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
int snprintf(char* str, size_t size, const char* format, ...)
    __attribute__((format(printf, 3, 4)));

/* Pointer-to-unsigned-long helper to avoid %p format pitfalls */
#define P2UL(p) ((unsigned long)(uintptr_t)(p))

#endif /* SYSTEM71_STDLIB_H */
