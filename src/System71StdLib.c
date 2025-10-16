/* System 7.1 Standard Library Implementation */

#include "System71StdLib.h"
#include "MacTypes.h"
#include "SystemInternal.h"

#include <stdbool.h>

#if defined(__powerpc__) || defined(__powerpc64__)
#include "Platform/PowerPC/OpenFirmware.h"
#endif

/* Global memory error variable */
static OSErr gMemError = noErr;

/* Memory Manager Error Function */
OSErr MemError(void) {
    return gMemError;
}

/* Memory functions */
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/* String functions */
size_t strlen(const char* s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n && (*d++ = *src++)) {
        n--;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) {
        d++;
    }
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == c) {
            return (char*)(uintptr_t)s;
        }
        s++;
    }
    return NULL;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)(uintptr_t)haystack;

    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) {
            return (char*)(uintptr_t)haystack;
        }

        haystack++;
    }
    return NULL;
}

/* Conversion functions */
int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t') {
        str++;
    }

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

long atol(const char* str) {
    long result = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t') {
        str++;
    }

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

/* Math functions */
int abs(int n) {
    return n < 0 ? -n : n;
}

long labs(long n) {
    return n < 0 ? -n : n;
}

/* Serial I/O functions */
#include <stdint.h>
#include <stdarg.h>

#define COM1 0x3F8

#include "Platform/include/io.h"

#define inb(port) hal_inb(port)
#define outb(port, value) hal_outb(port, value)

#if defined(__arm__) || defined(__aarch64__)
#include <stdint.h>
static uintptr_t g_pl011_uart_base = 0x09000000u;
static inline volatile uint32_t* pl011_reg(uint32_t offset) {
    return (volatile uint32_t*)(g_pl011_uart_base + offset);
}
#define PL011_DR   0x00
#define PL011_FR   0x18
#define PL011_IBRD 0x24
#define PL011_FBRD 0x28
#define PL011_LCRH 0x2C
#define PL011_CR   0x30
#define PL011_IMSC 0x38
#define PL011_ICR  0x44
#define PL011_RXFE (1u << 4)
#define PL011_TXFF (1u << 5)
#define PL011_UARTEN (1u << 0)
#define PL011_TXE (1u << 8)
#define PL011_RXE (1u << 9)

void serial_set_pl011_base(uintptr_t base) {
    if (base != 0) {
        g_pl011_uart_base = base;
    }
}
#endif

void serial_init(void) {
#if defined(__arm__) || defined(__aarch64__)
    /* Initialize PL011 UART (115200 baud, 8N1) */
    volatile uint32_t* cr = pl011_reg(PL011_CR);
    volatile uint32_t* icr = pl011_reg(PL011_ICR);
    volatile uint32_t* ibrd = pl011_reg(PL011_IBRD);
    volatile uint32_t* fbrd = pl011_reg(PL011_FBRD);
    volatile uint32_t* lcrh = pl011_reg(PL011_LCRH);
    volatile uint32_t* imsc = pl011_reg(PL011_IMSC);

    *cr = 0;             /* disable UART */
    *icr = 0x7FF;        /* clear interrupts */
    *ibrd = 13;          /* assuming 24 MHz clock */
    *fbrd = 2;           /* fractional divider */
    *lcrh = (1u << 4) | (1u << 5) | (1u << 6); /* FIFO enable, 8-bit */
    *imsc = 0;           /* disable interrupts */
    *cr = PL011_UARTEN | PL011_TXE | PL011_RXE;
    return;
#elif defined(__powerpc__) || defined(__powerpc64__)
    /* TODO: Wire up Open Firmware or UART console. */
    return;
#else
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
#endif
}

void serial_putchar(char c) {
#if defined(__arm__) || defined(__aarch64__)
    /* Basic PL011 transmit */
    volatile uint32_t* fr = pl011_reg(PL011_FR);
    volatile uint32_t* dr = pl011_reg(PL011_DR);
    if (c == '\n') {
        while ((*fr) & PL011_TXFF) { }
        *dr = '\r';
    }
    while ((*fr) & PL011_TXFF) {
        /* wait for space */
    }
    *dr = (uint32_t)c;
    return;
#elif defined(__powerpc__) || defined(__powerpc64__)
    if (!ofw_console_available()) {
        return;
    }
    if (c == '\n') {
        const char cr = '\r';
        ofw_console_write(&cr, 1);
    }
    ofw_console_write(&c, 1);
    return;
#else
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
#endif
}

void serial_puts(const char* str) {
    /* Direct serial output only - no framebuffer interaction */

    /* Direct serial output only - no framebuffer interaction */
    if (!str) return;
#if defined(__arm__) || defined(__aarch64__)
    (void)str;
#elif defined(__powerpc__) || defined(__powerpc64__)
    if (!ofw_console_available()) {
        return;
    }
    while (*str) {
        char ch = *str++;
        if (ch == '\n') {
            const char cr = '\r';
            ofw_console_write(&cr, 1);
        }
        ofw_console_write(&ch, 1);
    }
    return;
#else
    while (*str) {
        if (*str == '\n') {
            while ((inb(COM1 + 5) & 0x20) == 0);
            outb(COM1, '\r');
        }
        while ((inb(COM1 + 5) & 0x20) == 0);
        outb(COM1, *str);
        str++;
    }
#endif
}

int serial_data_ready(void) {
#if defined(__arm__) || defined(__aarch64__)
    volatile uint32_t* fr = pl011_reg(PL011_FR);
    return ((*fr) & PL011_RXFE) == 0;
#elif defined(__powerpc__) || defined(__powerpc64__)
    if (!ofw_console_input_available()) {
        return 0;
    }
    char ch = 0;
    int rc = ofw_console_poll_char(&ch);
    if (rc == 1) {
        return 1;
    }
    return 0;
#else
    return (inb(COM1 + 5) & 0x01) != 0;
#endif
}

char serial_getchar(void) {
#if defined(__arm__) || defined(__aarch64__)
    volatile uint32_t* fr = pl011_reg(PL011_FR);
    volatile uint32_t* dr = pl011_reg(PL011_DR);
    while ((*fr) & PL011_RXFE) {
        /* wait for data */
    }
    return (char)(*dr & 0xFF);
#elif defined(__powerpc__) || defined(__powerpc64__)
    char ch = 0;
    if (ofw_console_read_char(&ch) == 1) {
        return ch;
    }
    return 0;
#else
    while (!serial_data_ready());
    return inb(COM1);
#endif
}

void serial_print_hex(uint32_t value) {
    /* DISABLED - Serial output was somehow appearing in GUI menu bar */
    return;

    const char* hex = "0123456789ABCDEF";
    serial_puts("0x");
    for (int i = 7; i >= 0; i--) {
        serial_putchar(hex[(value >> (i * 4)) & 0xF]);
    }
}

/* -------------------------------------------------------------------------- */
/* Logging infrastructure */


typedef struct {
    const char* tag;
    SystemLogModule module;
    SystemLogLevel level;
} SysLogTag;

static SystemLogLevel g_globalLogLevel = kLogLevelDebug;
static SystemLogLevel g_moduleLevels[kLogModuleCount] = {
    [kLogModuleGeneral] = kLogLevelInfo,
    [kLogModuleDesktop] = kLogLevelDebug,
    [kLogModuleEvent] = kLogLevelDebug,
    [kLogModuleFinder] = kLogLevelDebug,
    [kLogModuleFileSystem] = kLogLevelDebug,
    [kLogModuleWindow] = kLogLevelDebug,
    [kLogModuleMenu] = kLogLevelDebug,
    [kLogModuleDialog] = kLogLevelDebug,
    [kLogModuleControl] = kLogLevelDebug,
    [kLogModuleFont] = kLogLevelDebug,
    [kLogModuleSound] = kLogLevelInfo,
    [kLogModuleResource] = kLogLevelDebug,
    [kLogModuleStandardFile] = kLogLevelDebug,
    [kLogModuleListManager] = kLogLevelDebug,
    [kLogModuleSystem] = kLogLevelInfo,
    [kLogModuleTextEdit] = kLogLevelDebug,
    [kLogModulePlatform] = kLogLevelDebug,
    [kLogModuleScrap] = kLogLevelDebug,
    [kLogModuleMemory] = kLogLevelInfo,
    [kLogModuleProcess] = kLogLevelInfo,
    [kLogModuleSegmentLoader] = kLogLevelDebug,
    [kLogModuleCPU] = kLogLevelInfo
};

static const SysLogTag kLogTagTable[] = {
    { "CTRL", kLogModuleControl, kLogLevelDebug },
    { "CTRL SMOKE", kLogModuleControl, kLogLevelTrace },
    { "DM", kLogModuleDialog, kLogLevelDebug },
    { "WM", kLogModuleWindow, kLogLevelDebug },
    { "SF", kLogModuleStandardFile, kLogLevelDebug },
    { "LIST", kLogModuleListManager, kLogLevelDebug },
    { "LIST SMOKE", kLogModuleListManager, kLogLevelTrace },
    { "MI", kLogModuleMenu, kLogLevelTrace },
    { "PRE-IF", kLogModuleEvent, kLogLevelTrace },
    { "DBLCLK", kLogModuleEvent, kLogLevelTrace },
    { "WIN_OPEN", kLogModuleWindow, kLogLevelDebug },
    { "NEWWIN", kLogModuleWindow, kLogLevelDebug },
    { "HILITE", kLogModuleWindow, kLogLevelTrace },
    { "PaintBehind", kLogModuleWindow, kLogLevelTrace },
    { "MEM", kLogModuleMemory, kLogLevelInfo },
    { "FM", kLogModuleFont, kLogLevelDebug },
    { "M68K", kLogModuleCPU, kLogLevelDebug }
};


static const size_t kLogTagCount = sizeof(kLogTagTable) / sizeof(kLogTagTable[0]);

static SystemLogLevel SysLogLevelFromString(const char* levelStr) {
    if (!levelStr) {
        return kLogLevelDebug;
    }

    if (strcmp(levelStr, "ERROR") == 0) return kLogLevelError;
    if (strcmp(levelStr, "WARN") == 0 || strcmp(levelStr, "WARNING") == 0) return kLogLevelWarn;
    if (strcmp(levelStr, "INFO") == 0) return kLogLevelInfo;
    if (strcmp(levelStr, "DEBUG") == 0) return kLogLevelDebug;
    if (strcmp(levelStr, "TRACE") == 0) return kLogLevelTrace;
    return kLogLevelDebug;
}

static void SysLogNormalizeString(char* str) {
    if (!str) return;
    for (char* p = str; *p; ++p) {
        if (*p >= 'a' && *p <= 'z') {
            *p = (char)(*p - 'a' + 'A');
        }
    }
}

static bool SysLogParseBracketTag(const char* fmt, SystemLogModule* module, SystemLogLevel* level) {
    if (!fmt) return false;

    while (*fmt == ' ' || *fmt == '\t') {
        ++fmt;
    }

    if (*fmt != '[') {
        return false;
    }

    const char* closing = strchr(fmt, ']');
    if (!closing || closing == fmt + 1) {
        return false;
    }

    size_t tagLen = (size_t)(closing - fmt - 1);
    if (tagLen >= 32) {
        tagLen = 31;
    }

    char tagBuffer[32];
    memcpy(tagBuffer, fmt + 1, tagLen);
    tagBuffer[tagLen] = '\0';

    char* levelSep = strchr(tagBuffer, ':');
    if (levelSep) {
        *levelSep = '\0';
        ++levelSep;
        SysLogNormalizeString(levelSep);
        *level = SysLogLevelFromString(levelSep);
    }

    SysLogNormalizeString(tagBuffer);

    for (size_t i = 0; i < kLogTagCount; ++i) {
        if (strcmp(tagBuffer, kLogTagTable[i].tag) == 0) {
            *module = kLogTagTable[i].module;
            if (!levelSep) {
                *level = kLogTagTable[i].level;
            }
            return true;
        }
    }

    return false;
}

/* Deprecated: serial_printf() classifier - only parses bracket tags, defaults to General */
static void SysLogClassifyMessage(const char* fmt, SystemLogModule* outModule, SystemLogLevel* outLevel) {
    SystemLogModule module = kLogModuleGeneral;
    SystemLogLevel level = kLogLevelDebug;

    if (!fmt) {
        *outModule = module;
        *outLevel = level;
        return;
    }

    /* Try to parse bracket tag like [WM], [DM], etc. */
    SysLogParseBracketTag(fmt, &module, &level);

    /* If no bracket tag found, use defaults (General/Debug) */
    *outModule = module;
    *outLevel = level;
}

static bool SysLogShouldEmit(SystemLogModule module, SystemLogLevel level) {
    if (module < 0 || module >= kLogModuleCount) {
        module = kLogModuleGeneral;
    }

    if (level > g_globalLogLevel) {
        return false;
    }

    if (level > g_moduleLevels[module]) {
        return false;
    }

    return true;
}

static void SysLogFormatAndSend(const char* fmt, va_list args) {
    const char* p = fmt;
    char buffer[256];
    int buf_idx = 0;

    while (p && *p && buf_idx < 255) {
        if (*p == '%') {
            ++p;

            if (*p == '0' && *(p + 1) == '2' && *(p + 2) == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                const char* hex_digits = "0123456789abcdef";
                buffer[buf_idx++] = hex_digits[(val >> 4) & 0xF];
                if (buf_idx < 255) buffer[buf_idx++] = hex_digits[val & 0xF];
                p += 3;
            }
            else if (*p == '0' && *(p + 1) == '8' && *(p + 2) == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                const char* hex_digits = "0123456789abcdef";
                for (int i = 7; i >= 0 && buf_idx < 255; --i) {
                    buffer[buf_idx++] = hex_digits[(val >> (i * 4)) & 0xF];
                }
                p += 3;
            }
            else if (*p == 'd') {
                int val = va_arg(args, int);
                char num_buf[12];
                int i = 0;

                if (val < 0) {
                    if (buf_idx < 255) buffer[buf_idx++] = '-';
                    val = -val;
                }

                if (val == 0) {
                    if (buf_idx < 255) buffer[buf_idx++] = '0';
                } else {
                    while (val > 0 && i < 11) {
                        num_buf[i++] = '0' + (val % 10);
                        val /= 10;
                    }
                    while (i > 0 && buf_idx < 255) {
                        buffer[buf_idx++] = num_buf[--i];
                    }
                }
                ++p;
            }
            else if (*p == 'u') {
                unsigned int val = va_arg(args, unsigned int);
                char num_buf[12];
                int i = 0;

                if (val == 0) {
                    if (buf_idx < 255) buffer[buf_idx++] = '0';
                } else {
                    while (val > 0 && i < 11) {
                        num_buf[i++] = '0' + (val % 10);
                        val /= 10;
                    }
                    while (i > 0 && buf_idx < 255) {
                        buffer[buf_idx++] = num_buf[--i];
                    }
                }
                ++p;
            }
            else if (*p == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                char hex_buf[9];
                int i = 0;
                const char* hex_digits = "0123456789abcdef";

                if (val == 0) {
                    if (buf_idx < 255) buffer[buf_idx++] = '0';
                } else {
                    while (val > 0 && i < 8) {
                        hex_buf[i++] = hex_digits[val & 0xF];
                        val >>= 4;
                    }
                    while (i > 0 && buf_idx < 255) {
                        buffer[buf_idx++] = hex_buf[--i];
                    }
                }
                ++p;
            }
            else if (*p == 's') {
                const char* s = va_arg(args, const char*);
                while (s && *s && buf_idx < 255) {
                    buffer[buf_idx++] = *s++;
                }
                ++p;
            }
            else if (*p == 'c') {
                int ch = va_arg(args, int);
                if (buf_idx < 255) buffer[buf_idx++] = (char)ch;
                ++p;
            }
            else if (*p == '%') {
                if (buf_idx < 255) buffer[buf_idx++] = '%';
                ++p;
            }
            else {
                if (buf_idx < 255) buffer[buf_idx++] = '%';
            }
        } else {
            if (buf_idx < 255) buffer[buf_idx++] = *p;
            ++p;
        }
    }

    buffer[buf_idx] = '\0';
    serial_puts(buffer);
}

void SysLogSetGlobalLevel(SystemLogLevel level) {
    g_globalLogLevel = level;
}

SystemLogLevel SysLogGetGlobalLevel(void) {
    return g_globalLogLevel;
}

void SysLogSetModuleLevel(SystemLogModule module, SystemLogLevel level) {
    if (module < 0 || module >= kLogModuleCount) {
        return;
    }
    g_moduleLevels[module] = level;
}

SystemLogLevel SysLogGetModuleLevel(SystemLogModule module) {
    if (module < 0 || module >= kLogModuleCount) {
        return g_moduleLevels[kLogModuleGeneral];
    }
    return g_moduleLevels[module];
}

const char* SysLogModuleName(SystemLogModule module) {
    switch (module) {
        case kLogModuleGeneral: return "general";
        case kLogModuleDesktop: return "desktop";
        case kLogModuleEvent: return "event";
        case kLogModuleFinder: return "finder";
        case kLogModuleFileSystem: return "filesystem";
        case kLogModuleWindow: return "window";
        case kLogModuleMenu: return "menu";
        case kLogModuleDialog: return "dialog";
        case kLogModuleControl: return "control";
        case kLogModuleFont: return "font";
        case kLogModuleSound: return "sound";
        case kLogModuleResource: return "resource";
        case kLogModuleStandardFile: return "standardfile";
        case kLogModuleListManager: return "list";
        case kLogModuleSystem: return "system";
        case kLogModuleTextEdit: return "textedit";
        case kLogModulePlatform: return "platform";
        case kLogModuleScrap: return "scrap";
        case kLogModuleMemory: return "memory";
        case kLogModuleProcess: return "process";
        default: return "general";
    }
}

static void SysLogEmit(SystemLogModule module, SystemLogLevel level, const char* fmt, va_list args) {
    if (!SysLogShouldEmit(module, level)) {
        return;
    }

    va_list argsCopy;
    va_copy(argsCopy, args);
    SysLogFormatAndSend(fmt, argsCopy);
    va_end(argsCopy);
}

void serial_logf(SystemLogModule module, SystemLogLevel level, const char* fmt, ...) {
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);
    SysLogEmit(module, level, fmt, args);
    va_end(args);
}

void serial_printf(const char* fmt, ...) {
    if (!fmt) return;

    SystemLogModule module;
    SystemLogLevel level;
    SysLogClassifyMessage(fmt, &module, &level);

    va_list args;
    va_start(args, fmt);
    SysLogEmit(module, level, fmt, args);
    va_end(args);
}

/* Internal vsnprintf implementation */
static int vsnprintf(char* str, size_t size, const char* format, va_list args) {
    if (!str || size == 0 || !format) return 0;

    size_t written = 0;
    const char* p = format;

    while (*p && written < size - 1) {
        if (*p == '%') {
            p++;

            /* Handle format flags and width (simplified) */
            int width = 0;
            int leftAlign = 0;
            int zeroPad = 0;

            /* Check for flags */
            if (*p == '-') {
                leftAlign = 1;
                p++;
            }
            if (*p == '0') {
                zeroPad = 1;
                p++;
            }

            /* Parse width */
            while (*p >= '0' && *p <= '9') {
                width = width * 10 + (*p - '0');
                p++;
            }

            /* Handle format specifiers */
            if (*p == 's') {
                /* Handle %s - string argument */
                const char* s = va_arg(args, const char*);
                if (s) {
                    while (*s && written < size - 1) {
                        str[written++] = *s++;
                    }
                }
                p++;
            } else if (*p == 'd' || *p == 'i') {
                /* Handle %d/%i - signed integer */
                int val = va_arg(args, int);
                char numBuf[32];
                int numLen = 0;
                int isNeg = 0;

                if (val < 0) {
                    isNeg = 1;
                    val = -val;
                }

                if (val == 0) {
                    numBuf[numLen++] = '0';
                } else {
                    while (val > 0 && numLen < 31) {
                        numBuf[numLen++] = '0' + (val % 10);
                        val /= 10;
                    }
                }

                if (isNeg && written < size - 1) {
                    str[written++] = '-';
                }

                /* Reverse the digits */
                for (int i = numLen - 1; i >= 0 && written < size - 1; i--) {
                    str[written++] = numBuf[i];
                }
                p++;
            } else if (*p == 'u') {
                /* Handle %u - unsigned integer */
                unsigned int val = va_arg(args, unsigned int);
                char numBuf[32];
                int numLen = 0;

                if (val == 0) {
                    numBuf[numLen++] = '0';
                } else {
                    while (val > 0 && numLen < 31) {
                        numBuf[numLen++] = '0' + (val % 10);
                        val /= 10;
                    }
                }

                for (int i = numLen - 1; i >= 0 && written < size - 1; i--) {
                    str[written++] = numBuf[i];
                }
                p++;
            } else if (*p == 'x' || *p == 'X') {
                /* Handle %x/%X - hexadecimal */
                unsigned int val = va_arg(args, unsigned int);
                char numBuf[32];
                int numLen = 0;
                char hexBase = (*p == 'X') ? 'A' : 'a';

                if (val == 0) {
                    numBuf[numLen++] = '0';
                } else {
                    while (val > 0 && numLen < 31) {
                        int digit = val % 16;
                        numBuf[numLen++] = (digit < 10) ? ('0' + digit) : (hexBase + digit - 10);
                        val /= 16;
                    }
                }

                for (int i = numLen - 1; i >= 0 && written < size - 1; i--) {
                    str[written++] = numBuf[i];
                }
                p++;
            } else if (*p == 'c') {
                /* Handle %c - character */
                char c = (char)va_arg(args, int);
                if (written < size - 1) {
                    str[written++] = c;
                }
                p++;
            } else if (*p == 'p') {
                /* Handle %p - pointer */
                void* ptr = va_arg(args, void*);
                unsigned long val = (unsigned long)ptr;

                /* Add 0x prefix */
                if (written < size - 1) str[written++] = '0';
                if (written < size - 1) str[written++] = 'x';

                /* Convert to hex */
                char numBuf[32];
                int numLen = 0;
                if (val == 0) {
                    numBuf[numLen++] = '0';
                } else {
                    while (val > 0 && numLen < 31) {
                        int digit = val % 16;
                        numBuf[numLen++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                        val /= 16;
                    }
                }

                for (int i = numLen - 1; i >= 0 && written < size - 1; i--) {
                    str[written++] = numBuf[i];
                }
                p++;
            } else if (*p == '%') {
                /* Handle %% - literal % */
                if (written < size - 1) {
                    str[written++] = '%';
                }
                p++;
            } else {
                /* Unknown format - just copy it */
                if (written < size - 1) str[written++] = '%';
                if (*p && written < size - 1) str[written++] = *p++;
            }
        } else {
            str[written++] = *p++;
        }
    }

    str[written] = '\0';
    return written;
}

/* Standard I/O functions */
int sprintf(char* str, const char* format, ...) {
    if (!str || !format) return 0;

    va_list args;
    va_start(args, format);

    /* Use a large buffer size for sprintf (no bounds checking) */
    /* In practice, sprintf is unbounded - caller must ensure buffer is large enough */
    int result = vsnprintf(str, 4096, format, args);

    va_end(args);
    return result;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    if (size == 0) return 0;

    va_list args;
    va_start(args, format);

    int result = vsnprintf(str, size, format, args);

    va_end(args);
    return result;
}

/* Assert implementation */
void __assert_fail(const char* expr, const char* file, int line, const char* func) {
    serial_printf("Assertion failed: %s at %s:%d in %s\n", expr, file, line, func);
    /* In production, could halt or reset system */
}
/* Math library functions for bare metal */
/* Newton-Raphson square root for QuickDraw distance calculations */
double sqrt(double x) {
    if (x < 0.0) return 0.0;
    if (x == 0.0) return 0.0;

    /* Newton-Raphson method for square root */
    double guess = x / 2.0;
    double epsilon = 0.00001;

    for (int i = 0; i < 20; i++) {
        double next = (guess + x / guess) / 2.0;
        if (next - guess < epsilon && guess - next < epsilon) {
            return next;
        }
        guess = next;
    }

    return guess;
}
