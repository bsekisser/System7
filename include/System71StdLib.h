/* System 7.1 Standard Library Functions */

#ifndef SYSTEM71_STDLIB_H
#define SYSTEM71_STDLIB_H

#include <stdint.h>
#include <stddef.h>

/* Define POSIX types if not available */
#ifndef __useconds_t_defined
#define __useconds_t_defined
typedef uint32_t useconds_t;
#endif

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
void* memchr(const void* s, int c, size_t n);
void bzero(void* s, size_t n);
void explicit_bzero(void* s, size_t n);
int memset_s(void* s, size_t smax, int c, size_t n);
void bcopy(const void* src, void* dest, size_t n);
int bcmp(const void* s1, const void* s2, size_t n);

/* String functions */
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
char* sys71_strerror(int errnum);
void perror(const char* s);
size_t strspn(const char* s, const char* accept);
size_t strcspn(const char* s, const char* reject);
char* strpbrk(const char* s, const char* accept);
char* strtok(char* s, const char* delim);
char* strtok_r(char* s, const char* delim, char** saveptr);
char* strsep(char** stringp, const char* delim);
char* strlcpy(char* dst, const char* src, size_t siz);
char* strlcat(char* dst, const char* src, size_t siz);
char* strcasestr(const char* haystack, const char* needle);
char* index(const char* s, int c);
char* rindex(const char* s, int c);
char* strndup(const char* s, size_t n);
char* strupr(char* s);
char* strlwr(char* s);
char* strrev(char* s);
char* basename(const char* path);
char* dirname(char* path);
void c2pstrcpy(unsigned char* pstr, const char* cstr);
char* p2cstrcpy(char* cstr, const unsigned char* pstr);
void CopyCStringToPascal(const char* src, unsigned char* dst);

/* Conversion functions */
int atoi(const char* str);
long atol(const char* str);

/* Environment and utility functions */
int atexit(void (*func)(void));
char* getenv(const char* name);
int setenv(const char* name, const char* value, int overwrite);
int unsetenv(const char* name);
int getopt(int argc, char* const argv[], const char* optstring);

/* Sleep/delay functions */
unsigned int sleep(unsigned int seconds);
int usleep(useconds_t usec);

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
#if defined(__arm__) || defined(__aarch64__)
void serial_set_pl011_base(uintptr_t base);
#endif

/* Compatibility aliases used by some platform ports */
#define Serial_WriteString(...) serial_printf(__VA_ARGS__)
#define Serial_Printf(...)      serial_printf(__VA_ARGS__)
int sprintf(char* str, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
int snprintf(char* str, size_t size, const char* format, ...)
    __attribute__((format(printf, 3, 4)));

/* Pointer-to-unsigned-long helper to avoid %p format pitfalls */
#define P2UL(p) ((unsigned long)(uintptr_t)(p))

/* System utility functions */
#include "SystemTypes.h"
#ifndef HiWord
SInt16 HiWord(SInt32 x);
#endif
#ifndef LoWord
SInt16 LoWord(SInt32 x);
#endif
void BlockMoveData(const void* srcPtr, void* destPtr, Size byteCount);
void LongMul(SInt32 a, SInt32 b, wide* result);

#endif /* SYSTEM71_STDLIB_H */
