/*
 * PS2Controller.c - PS/2 Keyboard and Mouse Driver
 *
 * Provides input support for QEMU's emulated PS/2 devices
 * Integrates with the Mac OS System 7.1 Event Manager
 */

#include "SystemTypes.h"
#include "EventManager/EventTypes.h"
#include <stdint.h>

/* Event modifier key constants */
#define shiftKey    0x0200
#define alphaLock   0x0400
#define optionKey   0x0800
#define controlKey  0x1000

/* PS/2 Controller ports */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

/* PS/2 Controller status bits */
#define PS2_STATUS_OUTPUT_FULL  0x01
#define PS2_STATUS_INPUT_FULL   0x02
#define PS2_STATUS_SYSTEM_FLAG  0x04
#define PS2_STATUS_COMMAND      0x08
#define PS2_STATUS_TIMEOUT_ERR  0x40
#define PS2_STATUS_PARITY_ERR   0x80

/* PS/2 Controller commands */
#define PS2_CMD_READ_CONFIG     0x20
#define PS2_CMD_WRITE_CONFIG    0x60
#define PS2_CMD_DISABLE_PORT2   0xA7
#define PS2_CMD_ENABLE_PORT2    0xA8
#define PS2_CMD_TEST_PORT2      0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1      0xAB
#define PS2_CMD_DISABLE_PORT1   0xAD
#define PS2_CMD_ENABLE_PORT1    0xAE
#define PS2_CMD_WRITE_PORT2     0xD4

/* PS/2 Device commands */
#define PS2_DEV_RESET           0xFF
#define PS2_DEV_ENABLE_SCAN     0xF4
#define PS2_DEV_DISABLE_SCAN    0xF5
#define PS2_DEV_SET_DEFAULTS    0xF6
#define PS2_DEV_IDENTIFY        0xF2

/* Mouse commands */
#define PS2_MOUSE_SET_SAMPLE    0xF3
#define PS2_MOUSE_ENABLE_DATA   0xF4
#define PS2_MOUSE_SET_DEFAULTS  0xF6
#define PS2_MOUSE_SET_SCALING   0xE6
#define PS2_MOUSE_SET_RESOLUTION 0xE8

/* Keyboard scan code sets */
#define SCAN_CODE_SET_1         1
#define SCAN_CODE_SET_2         2

/* External functions */
extern void serial_printf(const char* fmt, ...);
extern Boolean GetNextEvent(short eventMask, EventRecord* theEvent);
extern SInt16 PostEvent(SInt16 eventNum, SInt32 eventMsg);
extern UInt32 TickCount(void);

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Global state */
static Boolean g_ps2Initialized = false;
static Boolean g_mouseEnabled = false;
static Boolean g_keyboardEnabled = false;

/* Global mouse position - shared with Event Manager */
Point g_mousePos = {400, 300};

/* Mouse state */
static struct {
    int16_t x;
    int16_t y;
    uint8_t buttons;
    uint8_t packet[3];
    uint8_t packet_index;
} g_mouseState = {400, 300, 0, {0, 0, 0}, 0};

/* Keyboard state */
static struct {
    Boolean shift_pressed;
    Boolean ctrl_pressed;
    Boolean alt_pressed;
    Boolean caps_lock;
    uint8_t last_scancode;
} g_keyboardState = {false, false, false, false, 0};

/* Wait for PS/2 controller ready for input */
static Boolean ps2_wait_input(void) {
    int timeout = 10000;
    while (timeout--) {
        if ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) == 0) {
            return true;
        }
    }
    return false;
}

/* Wait for PS/2 controller output available */
static Boolean ps2_wait_output(void) {
    int timeout = 10000;
    while (timeout--) {
        if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            return true;
        }
    }
    return false;
}

/* Send command to PS/2 controller */
static void ps2_send_command(uint8_t cmd) {
    ps2_wait_input();
    outb(PS2_COMMAND_PORT, cmd);
}

/* Send data to PS/2 device */
static void ps2_send_data(uint8_t data) {
    ps2_wait_input();
    outb(PS2_DATA_PORT, data);
}

/* Read data from PS/2 device */
static uint8_t ps2_read_data(void) {
    ps2_wait_output();
    return inb(PS2_DATA_PORT);
}

/* Send command to mouse (via second PS/2 port) */
static Boolean ps2_mouse_command(uint8_t cmd) {
    /* Tell controller we're sending to mouse */
    ps2_send_command(PS2_CMD_WRITE_PORT2);
    ps2_send_data(cmd);

    /* Wait for ACK */
    if (ps2_wait_output()) {
        uint8_t response = ps2_read_data();
        return (response == 0xFA); /* ACK */
    }
    return false;
}

/* Scan code to ASCII translation (simplified) */
static char scancode_to_ascii(uint8_t scancode, Boolean shift) {
    /* Basic US keyboard layout - scan code set 1 */
    static const char normal_map[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    static const char shift_map[128] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    if (scancode >= 128) return 0;

    if (shift || g_keyboardState.caps_lock) {
        char ch = shift_map[scancode];
        if (g_keyboardState.caps_lock && ch >= 'A' && ch <= 'Z' && !shift) {
            ch = normal_map[scancode]; /* Caps lock without shift = lowercase */
        }
        return ch;
    }

    return normal_map[scancode];
}

/* Process keyboard scancode */
static void process_keyboard_scancode(uint8_t scancode) {
    Boolean key_released = (scancode & 0x80) != 0;
    scancode &= 0x7F;

    /* Update modifier keys */
    switch (scancode) {
        case 0x2A: /* Left Shift */
        case 0x36: /* Right Shift */
            g_keyboardState.shift_pressed = !key_released;
            return;

        case 0x1D: /* Control */
            g_keyboardState.ctrl_pressed = !key_released;
            return;

        case 0x38: /* Alt */
            g_keyboardState.alt_pressed = !key_released;
            return;

        case 0x3A: /* Caps Lock */
            if (!key_released) {
                g_keyboardState.caps_lock = !g_keyboardState.caps_lock;
            }
            return;
    }

    /* Only process key presses, not releases (for now) */
    if (key_released) return;

    /* Convert to ASCII */
    char ascii = scancode_to_ascii(scancode, g_keyboardState.shift_pressed);
    if (ascii == 0) return;

    /* Create keyboard event message */
    /* Message format: high byte = virtual key code, low byte = ASCII */
    UInt32 message = (scancode << 8) | ascii;

    /* Modifiers are encoded in the message for key events */
    UInt16 modifiers = 0;
    if (g_keyboardState.shift_pressed) modifiers |= shiftKey;
    if (g_keyboardState.ctrl_pressed) modifiers |= controlKey;
    if (g_keyboardState.alt_pressed) modifiers |= optionKey;
    if (g_keyboardState.caps_lock) modifiers |= alphaLock;

    /* Add modifiers to message (high word) */
    message |= ((UInt32)modifiers << 16);

    PostEvent(keyDown, message);

    serial_printf("Key pressed: '%c' (scancode 0x%02x)\n", ascii, scancode);
}

/* Process mouse packet */
static void process_mouse_packet(void) {
    uint8_t status = g_mouseState.packet[0];
    int16_t dx = (int8_t)g_mouseState.packet[1];
    int16_t dy = (int8_t)g_mouseState.packet[2];

    /* Check for overflow */
    if (status & 0x40 || status & 0x80) {
        g_mouseState.packet_index = 0;
        return;
    }

    /* Update mouse position */
    g_mouseState.x += dx;
    g_mouseState.y -= dy; /* Y is inverted in PS/2 */

    /* Clamp to screen bounds (assume 800x600 for now) */
    if (g_mouseState.x < 0) g_mouseState.x = 0;
    if (g_mouseState.x > 799) g_mouseState.x = 799;
    if (g_mouseState.y < 0) g_mouseState.y = 0;
    if (g_mouseState.y > 599) g_mouseState.y = 599;

    /* Check button state changes */
    uint8_t new_buttons = status & 0x07;
    if (new_buttons != g_mouseState.buttons) {
        /* Left button */
        if ((new_buttons & 0x01) != (g_mouseState.buttons & 0x01)) {
            /* Message contains mouse position in low word */
            UInt32 message = ((UInt32)g_mouseState.y << 16) | g_mouseState.x;

            PostEvent((new_buttons & 0x01) ? mouseDown : mouseUp, message);

            serial_printf("Mouse %s at (%d, %d)\n",
                         (new_buttons & 0x01) ? "down" : "up",
                         g_mouseState.x, g_mouseState.y);
        }

        g_mouseState.buttons = new_buttons;
    }

    /* Also update global mouse position for tracking */
    g_mousePos.h = g_mouseState.x;
    g_mousePos.v = g_mouseState.y;

    /* Reset packet index */
    g_mouseState.packet_index = 0;
}

/* Initialize PS/2 keyboard */
static Boolean init_keyboard(void) {
    serial_printf("Initializing PS/2 keyboard...\n");

    /* Reset keyboard */
    ps2_send_data(PS2_DEV_RESET);
    if (ps2_wait_output()) {
        uint8_t response = ps2_read_data();
        if (response != 0xFA) {
            serial_printf("Keyboard reset failed: 0x%02x\n", response);
            return false;
        }
    }

    /* Wait for self-test result */
    if (ps2_wait_output()) {
        uint8_t result = ps2_read_data();
        if (result != 0xAA) {
            serial_printf("Keyboard self-test failed: 0x%02x\n", result);
            return false;
        }
    }

    /* Enable scanning */
    ps2_send_data(PS2_DEV_ENABLE_SCAN);
    if (ps2_wait_output()) {
        uint8_t response = ps2_read_data();
        if (response != 0xFA) {
            serial_printf("Failed to enable keyboard scanning: 0x%02x\n", response);
            return false;
        }
    }

    g_keyboardEnabled = true;
    serial_printf("PS/2 keyboard initialized\n");
    return true;
}

/* Initialize PS/2 mouse */
static Boolean init_mouse(void) {
    serial_printf("Initializing PS/2 mouse...\n");

    /* Enable second PS/2 port for mouse */
    ps2_send_command(PS2_CMD_ENABLE_PORT2);

    /* Reset mouse */
    if (!ps2_mouse_command(PS2_DEV_RESET)) {
        serial_printf("Mouse reset failed\n");
        return false;
    }

    /* Wait for self-test result */
    if (ps2_wait_output()) {
        uint8_t result = ps2_read_data();
        if (result != 0xAA) {
            serial_printf("Mouse self-test failed: 0x%02x\n", result);
            return false;
        }
    }

    /* Read and discard mouse ID */
    if (ps2_wait_output()) {
        ps2_read_data();
    }

    /* Set defaults */
    if (!ps2_mouse_command(PS2_MOUSE_SET_DEFAULTS)) {
        serial_printf("Failed to set mouse defaults\n");
        return false;
    }

    /* Enable data reporting */
    if (!ps2_mouse_command(PS2_MOUSE_ENABLE_DATA)) {
        serial_printf("Failed to enable mouse data reporting\n");
        return false;
    }

    g_mouseEnabled = true;
    serial_printf("PS/2 mouse initialized\n");
    return true;
}

/* Initialize PS/2 controller */
Boolean InitPS2Controller(void) {
    if (g_ps2Initialized) return true;

    serial_printf("Initializing PS/2 controller...\n");

    /* Disable both PS/2 ports during initialization */
    ps2_send_command(PS2_CMD_DISABLE_PORT1);
    ps2_send_command(PS2_CMD_DISABLE_PORT2);

    /* Flush output buffer */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
        inb(PS2_DATA_PORT);
    }

    /* Get controller configuration */
    ps2_send_command(PS2_CMD_READ_CONFIG);
    uint8_t config = ps2_read_data();

    /* Enable interrupts for both ports, disable translation */
    config |= 0x03;  /* Enable interrupts */
    config &= ~0x40; /* Disable translation */

    /* Write configuration back */
    ps2_send_command(PS2_CMD_WRITE_CONFIG);
    ps2_send_data(config);

    /* Perform controller self-test */
    ps2_send_command(PS2_CMD_TEST_CONTROLLER);
    if (ps2_wait_output()) {
        uint8_t result = ps2_read_data();
        if (result != 0x55) {
            serial_printf("PS/2 controller self-test failed: 0x%02x\n", result);
            return false;
        }
    }

    /* Test first PS/2 port */
    ps2_send_command(PS2_CMD_TEST_PORT1);
    if (ps2_wait_output()) {
        uint8_t result = ps2_read_data();
        if (result != 0x00) {
            serial_printf("PS/2 port 1 test failed: 0x%02x\n", result);
            return false;
        }
    }

    /* Test second PS/2 port */
    ps2_send_command(PS2_CMD_TEST_PORT2);
    if (ps2_wait_output()) {
        uint8_t result = ps2_read_data();
        if (result != 0x00) {
            serial_printf("PS/2 port 2 test failed: 0x%02x\n", result);
            /* Mouse is optional, continue anyway */
        }
    }

    /* Enable first PS/2 port */
    ps2_send_command(PS2_CMD_ENABLE_PORT1);

    /* Initialize keyboard */
    if (!init_keyboard()) {
        serial_printf("Warning: Keyboard initialization failed\n");
    }

    /* Initialize mouse */
    if (!init_mouse()) {
        serial_printf("Warning: Mouse initialization failed\n");
    }

    g_ps2Initialized = true;
    serial_printf("PS/2 controller initialized\n");
    return true;
}

/* Poll for PS/2 input (call this regularly) */
void PollPS2Input(void) {
    if (!g_ps2Initialized) return;

    /* Check for available data */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
        uint8_t data = inb(PS2_DATA_PORT);

        /* Check if this is mouse data (bit 5 of status port) */
        if (inb(PS2_STATUS_PORT) & 0x20) {
            /* Mouse data */
            if (g_mouseEnabled) {
                g_mouseState.packet[g_mouseState.packet_index++] = data;
                if (g_mouseState.packet_index >= 3) {
                    process_mouse_packet();
                }
            }
        } else {
            /* Keyboard data */
            if (g_keyboardEnabled) {
                process_keyboard_scancode(data);
            }
        }
    }
}

/* Get current mouse position */
void GetMouse(Point* mouseLoc) {
    if (mouseLoc) {
        mouseLoc->h = g_mouseState.x;
        mouseLoc->v = g_mouseState.y;
    }
}

/* Check if mouse button is pressed */
Boolean Button(void) {
    return (g_mouseState.buttons & 0x01) != 0;
}