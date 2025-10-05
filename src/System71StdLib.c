/* System 7.1 Standard Library Implementation */

#include "System71StdLib.h"
#include "MacTypes.h"

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
    return *(unsigned char*)s1 - *(unsigned char*)s2;
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
    return *(unsigned char*)s1 - *(unsigned char*)s2;
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
            return (char*)s;
        }
        s++;
    }
    return NULL;
}

char* strstr(const char* haystack, const char* needle) {
    if (!needle || !*needle) return (char*)haystack;

    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) {
            return (char*)haystack;
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

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putchar(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}

void serial_puts(const char* str) {
    /* Direct serial output only - no framebuffer interaction */

    /* Direct serial output only - no framebuffer interaction */
    if (!str) return;
    while (*str) {
        if (*str == '\n') {
            while ((inb(COM1 + 5) & 0x20) == 0);
            outb(COM1, '\r');
        }
        while ((inb(COM1 + 5) & 0x20) == 0);
        outb(COM1, *str);
        str++;
    }
}

int serial_data_ready(void) {
    return (inb(COM1 + 5) & 0x01) != 0;
}

char serial_getchar(void) {
    while (!serial_data_ready());
    return inb(COM1);
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

/*
 * serial_printf - Printf-like function with message filtering
 *
 * WHITELIST FILTERING:
 * To reduce serial output lag and focus on relevant debugging, this function
 * only outputs messages containing specific whitelisted strings. If the format
 * string doesn't contain any whitelisted substring, the message is silently dropped.
 *
 * HOW TO ADD A NEW WHITELIST ENTRY:
 * 1. Find the whitelist check block below (lines 290-317)
 * 2. Add a new line: strstr(fmt, "YourDebugString") == NULL &&
 * 3. Place it before the closing brace that contains "return;"
 * 4. Rebuild the project
 *
 * CURRENT WHITELIST ENTRIES (alphabetical by category):
 *
 * About & Dialogs:
 *   - "About"                 : About This Macintosh window
 *
 * Desktop & Icon Management:
 *   - "Desktop icon"          : Desktop icon operations
 *   - "DrawVolumeIcon"        : Volume icon rendering
 *   - "Hit icon"              : Icon click detection
 *   - "IconAtPoint"           : Icon hit testing
 *   - "Selection"             : Icon selection state
 *   - "Single-click"          : Single-click handling
 *   - "TrackIconDragSync"     : Icon drag operations
 *   - "Trash"                 : Trash folder operations
 *   - "Volume"                : Volume-related operations
 *
 * Event Processing:
 *   - "[DBLCLK"               : Double-click events (marker tag)
 *   - "[MI]"                  : Menu item events (marker tag)
 *   - "[PRE-IF]"              : Pre-if condition (marker tag)
 *   - "GetNextEvent"          : Event queue polling
 *   - "HandleDesktopClick"    : Desktop click handling
 *   - "HandleMouseDown"       : Mouse down event handling
 *   - "PostEvent"             : Event posting
 *   - "WaitNextEvent"         : Event waiting
 *
 * Finder & File System:
 *   - "Finder:"               : General Finder operations
 *   - "FolderWindow_OpenFolder" : Opening folder windows
 *   - "HFS"                   : HFS file system operations
 *   - "InitializeFolderContents" : Folder window initialization
 *   - "read_btree_data"       : B-tree data reading
 *   - "VFS_Enumerate"         : VFS directory enumeration
 *
 * Menu System:
 *   - "DEBUG"                 : Debug messages for crash investigation
 *   - "MenuSelect"            : Menu selection handling
 *
 * Window Management:
 *   - "[NEWWIN]"              : New window creation (marker tag)
 *   - "[WIN_OPEN]"            : Window opening (marker tag)
 *   - "CheckWindowsNeedingUpdate" : Window update checks
 *   - "DoUpdate"              : Window update processing
 *   - "DrawNew"               : New window drawing
 *   - "PaintOne"              : Single window painting
 *   - "ShowWindow"            : Window visibility
 *   - "WindowManager"         : General window manager operations
 *
 * SUPPORTED FORMAT SPECIFIERS:
 *   %d    - signed integer
 *   %x    - unsigned hex (variable width)
 *   %08x  - unsigned hex (8 digits, zero-padded)
 *   %02x  - unsigned hex byte (2 digits, zero-padded)
 *   %s    - null-terminated string
 *   %c    - single character
 *   %%    - literal % character
 *
 * UNSUPPORTED (will print literally):
 *   %p    - pointer (use %08x with (unsigned int) cast instead)
 *   %lx   - long hex (use %08x with (unsigned int) cast instead)
 *   %lu   - long unsigned (not implemented)
 *   %f    - float/double (not implemented)
 */
void serial_printf(const char* fmt, ...) {
    /* Only output critical debug messages to avoid lag */
    if (!fmt) return;

    /* WHITELIST CHECK: Message must contain at least one of these strings */
    if (strstr(fmt, "HandleMouseDown") == NULL &&
        strstr(fmt, "MenuSelect") == NULL &&
        strstr(fmt, "DoMenuCommand") == NULL &&
        strstr(fmt, "PostEvent") == NULL &&
        strstr(fmt, "HandleDesktopClick") == NULL &&
        strstr(fmt, "Desktop icon") == NULL &&
        strstr(fmt, "IconAtPoint") == NULL &&
        strstr(fmt, "Hit icon") == NULL &&
        strstr(fmt, "Single-click") == NULL &&
        strstr(fmt, "Selection") == NULL &&
        strstr(fmt, "Volume") == NULL &&
        strstr(fmt, "Trash") == NULL &&
        strstr(fmt, "DrawVolumeIcon") == NULL &&
        strstr(fmt, "TrackIconDragSync") == NULL &&
        strstr(fmt, "[PRE-IF]") == NULL &&
        strstr(fmt, "[DBLCLK") == NULL &&
        strstr(fmt, "[WIN_OPEN]") == NULL &&
        strstr(fmt, "[NEWWIN]") == NULL &&
        strstr(fmt, "ShowWindow") == NULL &&
        strstr(fmt, "TITLE_INIT") == NULL &&
        strstr(fmt, "TITLE_DRAW") == NULL &&
        strstr(fmt, "[HILITE]") == NULL &&
        strstr(fmt, "[PaintBehind]") == NULL &&
        strstr(fmt, "CheckWindowsNeedingUpdate") == NULL &&
        strstr(fmt, "GetNextEvent") == NULL &&
        strstr(fmt, "WaitNextEvent") == NULL &&
        strstr(fmt, "DoUpdate") == NULL &&
        strstr(fmt, "PaintOne") == NULL &&
        strstr(fmt, "DrawNew") == NULL &&
        strstr(fmt, "WindowManager") == NULL &&
        strstr(fmt, "[MI]") == NULL &&
        strstr(fmt, "InitializeFolderContents") == NULL &&
        strstr(fmt, "FolderWindow_OpenFolder") == NULL &&
        strstr(fmt, "HFS") == NULL &&
        strstr(fmt, "read_btree_data") == NULL &&
        strstr(fmt, "VFS_Enumerate") == NULL &&
        strstr(fmt, "Finder:") == NULL &&
        strstr(fmt, "FW:") == NULL &&
        strstr(fmt, "Icon_DrawWithLabel") == NULL &&
        strstr(fmt, "GhostXOR") == NULL &&
        strstr(fmt, "Sound") == NULL &&
        strstr(fmt, "PCSpkr") == NULL &&
        strstr(fmt, "SysBeep") == NULL &&
        strstr(fmt, "Startup") == NULL &&
        strstr(fmt, "MM:") == NULL &&
        strstr(fmt, "ATA:") == NULL &&
        strstr(fmt, "About") == NULL &&
        strstr(fmt, "CenterPStringInRect") == NULL &&
        strstr(fmt, "CloseWindow") == NULL &&
        strstr(fmt, "CODE PATH") == NULL &&
        strstr(fmt, "TITLE:") == NULL &&
        strstr(fmt, "DrawString") == NULL &&
        strstr(fmt, "FM:") == NULL &&
        strstr(fmt, "DEBUG") == NULL &&
        strstr(fmt, "Apple Menu") == NULL &&
        strstr(fmt, "Shut Down") == NULL &&
        strstr(fmt, "System") == NULL &&
        strstr(fmt, "===") == NULL &&
        strstr(fmt, "safe") == NULL &&
        strstr(fmt, "Version:") == NULL &&
        strstr(fmt, "Build:") == NULL &&
        strstr(fmt, "compatible") == NULL &&
        strstr(fmt, "Macintosh") == NULL &&
        strstr(fmt, "portable") == NULL &&
        strstr(fmt, "Dialog") == NULL &&
        strstr(fmt, "DIALOG") == NULL &&
        strstr(fmt, "Alert") == NULL &&
        strstr(fmt, "ALERT") == NULL &&
        strstr(fmt, "Modal") == NULL &&
        strstr(fmt, "DITL") == NULL &&
        strstr(fmt, "DLOG") == NULL &&
        strstr(fmt, "[LIST]") == NULL &&
        strstr(fmt, "[LIST SMOKE]") == NULL &&
        strstr(fmt, "LNew") == NULL &&
        strstr(fmt, "LDispose") == NULL &&
        strstr(fmt, "LUpdate") == NULL &&
        strstr(fmt, "LClick") == NULL &&
        strstr(fmt, "LScroll") == NULL &&
        strstr(fmt, "LSize") == NULL &&
        strstr(fmt, "LKey") == NULL &&
        strstr(fmt, "DrawCell") == NULL &&
        strstr(fmt, "EraseBackground") == NULL &&
        strstr(fmt, "[CTRL]") == NULL &&
        strstr(fmt, "[CTRL SMOKE]") == NULL &&
        strstr(fmt, "Scrollbar") == NULL &&
        strstr(fmt, "TrackScrollbar") == NULL &&
        strstr(fmt, "Button") == NULL &&
        strstr(fmt, "Checkbox") == NULL &&
        strstr(fmt, "Radio") == NULL &&
        strstr(fmt, "[DM]") == NULL &&
        strstr(fmt, "[WM]") == NULL &&
        strstr(fmt, "[SF]") == NULL) {
        return;  /* Message not whitelisted - silently drop */
    }

    va_list args;
    va_start(args, fmt);
    const char* p = fmt;

    char buffer[256];
    int buf_idx = 0;

    while (*p && buf_idx < 255) {
        if (*p == '%') {
            p++;

            /* Handle %02x format for hex bytes */
            if (*p == '0' && *(p+1) == '2' && *(p+2) == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                const char* hex_digits = "0123456789abcdef";
                buffer[buf_idx++] = hex_digits[(val >> 4) & 0xF];
                if (buf_idx < 255) buffer[buf_idx++] = hex_digits[val & 0xF];
                p += 3;
            }
            /* Handle %d for signed integers */
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
                p++;
            }
            /* Handle %u for unsigned integers */
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
                p++;
            }
            /* Handle %x for hex without leading zeros */
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
                p++;
            }
            /* Handle %08x for hex with leading zeros */
            else if (*p == '0' && *(p+1) == '8' && *(p+2) == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                const char* hex_digits = "0123456789abcdef";
                for (int i = 7; i >= 0 && buf_idx < 255; i--) {
                    buffer[buf_idx++] = hex_digits[(val >> (i * 4)) & 0xF];
                }
                p += 3;
            }
            /* Handle %s for strings */
            else if (*p == 's') {
                const char* s = va_arg(args, const char*);
                while (*s && buf_idx < 255) {
                    buffer[buf_idx++] = *s++;
                }
                p++;
            }
            /* Handle %c for characters */
            else if (*p == 'c') {
                int ch = va_arg(args, int);
                if (buf_idx < 255) buffer[buf_idx++] = (char)ch;
                p++;
            }
            /* Handle %% for literal % */
            else if (*p == '%') {
                if (buf_idx < 255) buffer[buf_idx++] = '%';
                p++;
            }
            /* Unknown format, just print % and continue */
            else {
                if (buf_idx < 255) buffer[buf_idx++] = '%';
            }
        } else {
            if (buf_idx < 255) buffer[buf_idx++] = *p;
            p++;
        }
    }

    buffer[buf_idx] = '\0';
    va_end(args);

    serial_puts(buffer);
}

/* Standard I/O functions */
int sprintf(char* str, const char* format, ...) {
    /* TODO: Implement properly */
    if (str) str[0] = 0;
    return 0;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    /* Minimal implementation - just copy format string */
    if (size == 0) return 0;
    size_t i = 0;
    while (format[i] && i < size - 1) {
        str[i] = format[i];
        i++;
    }
    str[i] = '\0';
    return i;
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
