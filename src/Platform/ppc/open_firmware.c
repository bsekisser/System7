/*
 * open_firmware.c - Minimal Open Firmware client helpers for PowerPC bring-up.
 *
 * Provides just enough of the IEEE 1275 client interface to obtain the
 * standard output handle and emit diagnostic text during early boot.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "Platform/PowerPC/OpenFirmware.h"

typedef uint32_t ofw_cell_t;
typedef ofw_cell_t (*ofw_entry_t)(void *args);

struct ofw_args {
    const char *service;
    uint32_t nargs;
    uint32_t nrets;
    ofw_cell_t cells[12];
};

#define OFW_MAX_REG_CELLS     128

static ofw_entry_t g_ofw_entry = NULL;
static uint32_t g_stdout_ihandle = 0;
static bool g_stdout_available = false;
static uint32_t g_stdin_ihandle = 0;
static bool g_stdin_available = false;
static char g_peek_char = 0;
static bool g_has_peek_char = false;
static ofw_memory_range_t g_memory_ranges[OFW_MAX_MEMORY_RANGES];
static size_t g_memory_range_count = 0;
static char g_stdout_path[128] = {0};

static const char kServiceFindDevice[] = "finddevice";
static const char kServiceGetProp[] = "getprop";
static const char kServiceOpen[] = "open";
static const char kServiceCallMethod[] = "call-method";
static const char kChosenPath[] = "/chosen";
static const char kPropStdout[] = "stdout";
static const char kPropStdoutPath[] = "stdout-path";
static const char kPropStdin[] = "stdin";
static const char kPropStdinPath[] = "stdin-path";
static const char kMethodWrite[] = "write";
static const char kMethodRead[] = "read";
static const char kMemoryPath[] = "/memory";
static const char kPropReg[] = "reg";
static const char kPropFrameBuffer[] = "framebuffer";
static const char kPropLineBytes[] = "linebytes";
static const char kPropAddress[] = "address";
static const char kPropWidth[] = "width";
static const char kPropHeight[] = "height";
static const char kPropDepth[] = "depth";

static int ofw_call(const char *service, uint32_t nargs, uint32_t nrets, ofw_cell_t *cells) {
    if (!g_ofw_entry) {
        return -1;
    }

    struct ofw_args args;
    args.service = service;
    args.nargs = nargs;
    args.nrets = nrets;
    memset(args.cells, 0, sizeof(args.cells));

    uint32_t total = nargs + nrets;
    if (total > (sizeof(args.cells) / sizeof(args.cells[0]))) {
        return -1;
    }

    for (uint32_t i = 0; i < nargs; ++i) {
        args.cells[i] = cells[i];
    }

    ofw_cell_t rc = g_ofw_entry(&args);
    if (rc != 0) {
        return -1;
    }

    for (uint32_t i = 0; i < nrets; ++i) {
        cells[nargs + i] = args.cells[nargs + i];
    }

    return 0;
}

static int ofw_finddevice(const char *path, uint32_t *out_handle) {
    ofw_cell_t cells[2];
    cells[0] = (ofw_cell_t)(uintptr_t)path;
    cells[1] = 0;
    if (ofw_call(kServiceFindDevice, 1, 1, cells) != 0) {
        return -1;
    }
    uint32_t handle = cells[1];
    if (handle == 0 || handle == (uint32_t)-1) {
        return -1;
    }
    if (out_handle) {
        *out_handle = handle;
    }
    return 0;
}

static int ofw_getprop(uint32_t phandle, const char *name, void *buf, uint32_t buflen, int32_t *out_len) {
    ofw_cell_t cells[5];
    cells[0] = phandle;
    cells[1] = (ofw_cell_t)(uintptr_t)name;
    cells[2] = (ofw_cell_t)(uintptr_t)buf;
    cells[3] = buflen;
    cells[4] = 0;

    if (ofw_call(kServiceGetProp, 4, 1, cells) != 0) {
        return -1;
    }

    if (out_len) {
        *out_len = (int32_t)cells[4];
    }
    return 0;
}

static int ofw_open(const char *path, uint32_t *out_ihandle) {
    ofw_cell_t cells[2];
    cells[0] = (ofw_cell_t)(uintptr_t)path;
    cells[1] = 0;

    if (ofw_call(kServiceOpen, 1, 1, cells) != 0) {
        return -1;
    }

    uint32_t handle = cells[1];
    if (handle == 0 || handle == (uint32_t)-1) {
        return -1;
    }

    if (out_ihandle) {
        *out_ihandle = handle;
    }
    return 0;
}

static uint32_t ofw_get_handle_property(uint32_t chosen, const char *prop_name) {
    uint32_t handle = 0;
    int32_t prop_len = 0;
    if (ofw_getprop(chosen, prop_name, &handle, sizeof(handle), &prop_len) == 0) {
        if (prop_len == (int32_t)sizeof(handle) && handle != 0 && handle != (uint32_t)-1) {
            return handle;
        }
    }
    return 0;
}

static uint32_t ofw_open_from_property(uint32_t chosen, const char *prop_name) {
    char path[128] = {0};
    int32_t prop_len = 0;
    if (ofw_getprop(chosen, prop_name, path, sizeof(path) - 1, &prop_len) != 0) {
        return 0;
    }

    if (prop_len <= 0) {
        return 0;
    }

    if (prop_len >= (int32_t)sizeof(path)) {
        prop_len = (int32_t)sizeof(path) - 1;
    }
    path[prop_len] = '\0';

    uint32_t handle = 0;
    if (ofw_open(path, &handle) != 0) {
        return 0;
    }

    return handle;
}

static int ofw_get_string_property(uint32_t phandle, const char *name, char *buf, size_t buf_len) {
    if (!buf || buf_len == 0) {
        return -1;
    }

    int32_t prop_len = 0;
    if (ofw_getprop(phandle, name, buf, buf_len - 1, &prop_len) != 0) {
        buf[0] = '\0';
        return -1;
    }

    if (prop_len <= 0) {
        buf[0] = '\0';
        return -1;
    }

    if ((size_t)prop_len >= buf_len) {
        prop_len = (int32_t)buf_len - 1;
    }

    buf[prop_len] = '\0';
    return 0;
}

static void ofw_locate_io_handles(void) {
    g_stdout_available = false;
    g_stdout_ihandle = 0;
    g_stdin_available = false;
    g_stdin_ihandle = 0;
    g_has_peek_char = false;
    memset(g_stdout_path, 0, sizeof(g_stdout_path));

    uint32_t chosen = 0;
    if (ofw_finddevice(kChosenPath, &chosen) != 0) {
        return;
    }

    uint32_t stdout_handle = ofw_get_handle_property(chosen, kPropStdout);
    if (stdout_handle == 0) {
        stdout_handle = ofw_open_from_property(chosen, kPropStdoutPath);
    }
    if (stdout_handle != 0) {
        g_stdout_ihandle = stdout_handle;
        g_stdout_available = true;
        if (ofw_get_string_property(chosen, kPropStdoutPath, g_stdout_path, sizeof(g_stdout_path)) != 0) {
            g_stdout_path[0] = '\0';
        }
    }

    uint32_t stdin_handle = ofw_get_handle_property(chosen, kPropStdin);
    if (stdin_handle == 0) {
        stdin_handle = ofw_open_from_property(chosen, kPropStdinPath);
    }
    if (stdin_handle != 0) {
        g_stdin_ihandle = stdin_handle;
        g_stdin_available = true;
    }
}

static void ofw_cache_memory_ranges(void) {
    g_memory_range_count = 0;
    memset(g_memory_ranges, 0, sizeof(g_memory_ranges));

    uint32_t mem_phandle = 0;
    if (ofw_finddevice(kMemoryPath, &mem_phandle) != 0) {
        return;
    }

    ofw_cell_t cells[OFW_MAX_REG_CELLS] = {0};
    int32_t prop_len = 0;
    if (ofw_getprop(mem_phandle, kPropReg, cells, sizeof(cells), &prop_len) != 0) {
        return;
    }

    if (prop_len <= 0) {
        return;
    }

    size_t cell_count = (size_t)prop_len / sizeof(ofw_cell_t);
    if (cell_count == 0) {
        return;
    }

    bool use64 = (prop_len % 16) == 0;
    size_t idx = 0;
    size_t range_idx = 0;

    while (idx + (use64 ? 3 : 1) < cell_count && range_idx < OFW_MAX_MEMORY_RANGES) {
        uint64_t base = 0;
        uint64_t size = 0;

        if (use64) {
            base = ((uint64_t)cells[idx] << 32) | cells[idx + 1];
            size = ((uint64_t)cells[idx + 2] << 32) | cells[idx + 3];
            idx += 4;
        } else {
            base = cells[idx];
            size = cells[idx + 1];
            idx += 2;
        }

        if (size == 0) {
            continue;
        }

        g_memory_ranges[range_idx].base = base;
        g_memory_ranges[range_idx].size = size;
        range_idx++;
    }

    g_memory_range_count = range_idx;
}

void ofw_client_init(void *entry) {
    g_ofw_entry = (ofw_entry_t)entry;
    g_stdout_available = false;
    g_stdout_ihandle = 0;
    g_stdin_available = false;
    g_stdin_ihandle = 0;
    g_has_peek_char = false;
    memset(g_stdout_path, 0, sizeof(g_stdout_path));
    g_memory_range_count = 0;

    if (!g_ofw_entry) {
        return;
    }

    ofw_locate_io_handles();
    ofw_cache_memory_ranges();
}

int ofw_console_available(void) {
    return (g_ofw_entry != NULL) && g_stdout_available;
}

int ofw_console_write(const char *buffer, uint32_t length) {
    if (!ofw_console_available() || !buffer || length == 0) {
        return -1;
    }

    ofw_cell_t cells[5];
    cells[0] = (ofw_cell_t)(uintptr_t)kMethodWrite;
    cells[1] = g_stdout_ihandle;
    cells[2] = (ofw_cell_t)(uintptr_t)buffer;
    cells[3] = length;
    cells[4] = 0;

    if (ofw_call(kServiceCallMethod, 4, 1, cells) != 0) {
        return -1;
    }

    return (int)cells[4];
}

int ofw_console_input_available(void) {
    return (g_ofw_entry != NULL) && g_stdin_available;
}

int ofw_console_poll_char(char *out_char) {
    if (!ofw_console_input_available()) {
        return -1;
    }

    if (g_has_peek_char) {
        if (out_char) {
            *out_char = g_peek_char;
        }
        return 1;
    }

    char ch = 0;
    ofw_cell_t cells[5];
    cells[0] = (ofw_cell_t)(uintptr_t)kMethodRead;
    cells[1] = g_stdin_ihandle;
    cells[2] = (ofw_cell_t)(uintptr_t)&ch;
    cells[3] = 1;
    cells[4] = 0;

    if (ofw_call(kServiceCallMethod, 4, 1, cells) != 0) {
        return 0;
    }

    int32_t read_len = (int32_t)cells[4];
    if (read_len <= 0) {
        return 0;
    }

    g_peek_char = ch;
    g_has_peek_char = true;
    if (out_char) {
        *out_char = ch;
    }
    return 1;
}

int ofw_console_read_char(char *out_char) {
    if (!out_char) {
        return -1;
    }

    while (true) {
        int rc = ofw_console_poll_char(out_char);
        if (rc == 1) {
            g_has_peek_char = false;
            return 1;
        } else if (rc < 0) {
            return -1;
        }
    }
}

int ofw_get_memory_range(uint64_t *out_base, uint64_t *out_size) {
    if (g_memory_range_count == 0) {
        return -1;
    }
    if (out_base) {
        *out_base = g_memory_ranges[0].base;
    }
    if (out_size) {
        *out_size = g_memory_ranges[0].size;
    }
    return 0;
}

size_t ofw_memory_range_count(void) {
    return g_memory_range_count;
}

size_t ofw_get_memory_ranges(ofw_memory_range_t *out_ranges, size_t max_ranges) {
    if (!out_ranges || max_ranges == 0) {
        return 0;
    }

    size_t copy_count = g_memory_range_count;
    if (copy_count > max_ranges) {
        copy_count = max_ranges;
    }

    for (size_t i = 0; i < copy_count; ++i) {
        out_ranges[i] = g_memory_ranges[i];
    }
    return copy_count;
}

int ofw_get_framebuffer_info(ofw_framebuffer_info_t *out) {
    if (!out || !g_ofw_entry) {
        return -1;
    }

    uint32_t chosen = 0;
    if (ofw_finddevice(kChosenPath, &chosen) != 0) {
        return -1;
    }

    char path[128] = {0};
    if (g_stdout_path[0] != '\0') {
        strncpy(path, g_stdout_path, sizeof(path) - 1);
    } else if (ofw_get_string_property(chosen, "linux,stdout-path", path, sizeof(path)) != 0) {
        if (ofw_get_string_property(chosen, kPropStdoutPath, path, sizeof(path)) != 0) {
            path[0] = '\0';
        }
    }

    if (path[0] == '\0') {
        return -1;
    }

    /* Skip purely serial consoles */
    if (strstr(path, "tty") || strstr(path, "serial")) {
        return -1;
    }

    uint32_t display = 0;
    if (ofw_finddevice(path, &display) != 0) {
        return -1;
    }

    uint64_t base = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint32_t stride = 0;
    ofw_cell_t cells[4] = {0};
    int32_t len = 0;

    if (ofw_getprop(display, kPropAddress, cells, sizeof(cells), &len) == 0 && len >= 4) {
        if (len >= 8) {
            base = ((uint64_t)cells[0] << 32) | cells[1];
        } else {
            base = cells[0];
        }
    } else if (ofw_getprop(display, kPropFrameBuffer, cells, sizeof(cells), &len) == 0 && len >= 4) {
        if (len >= 8) {
            base = ((uint64_t)cells[0] << 32) | cells[1];
        } else {
            base = cells[0];
        }
    }

    if (ofw_getprop(display, kPropWidth, cells, sizeof(ofw_cell_t), &len) == 0 && len >= 4) {
        width = cells[0];
    }
    if (ofw_getprop(display, kPropHeight, cells, sizeof(ofw_cell_t), &len) == 0 && len >= 4) {
        height = cells[0];
    }
    if (ofw_getprop(display, kPropDepth, cells, sizeof(ofw_cell_t), &len) == 0 && len >= 4) {
        depth = cells[0];
    }
    if (ofw_getprop(display, kPropLineBytes, cells, sizeof(ofw_cell_t), &len) == 0 && len >= 4) {
        stride = cells[0];
    }

    if (stride == 0 && width != 0 && depth != 0) {
        stride = width * ((depth + 7) / 8);
    }

    if (base == 0 || width == 0 || height == 0) {
        return -1;
    }

    out->base = base;
    out->width = width;
    out->height = height;
    out->depth = depth;
    out->stride = stride;
    return 0;
}
