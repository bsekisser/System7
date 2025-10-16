/*
 * hal_input.c - PowerPC Input HAL scaffolding
 *
 * Simple stubs so that the window manager and event loop can build prior to
 * wiring up real ADB/USB input pipelines on PowerPC hardware.
 */

#include <string.h>
#include "Platform/include/input.h"

void hal_input_poll(void) {
    /* TODO: Poll real PowerPC input hardware once available. */
}

void hal_input_get_mouse(Point* mouse_loc) {
    if (mouse_loc) {
        mouse_loc->h = 0;
        mouse_loc->v = 0;
    }
}

Boolean hal_input_get_keyboard_state(KeyMap key_map) {
    if (key_map) {
        memset(key_map, 0, sizeof(KeyMap));
    }
    return true;
}
