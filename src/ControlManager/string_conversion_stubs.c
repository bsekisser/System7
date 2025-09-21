// #include "CompatibilityFix.h" // Removed
#include <string.h>
/**
 * RE-AGENT-BANNER: String Conversion Stubs for Control Manager

 *
 * implementation shows that ROM Control Manager code imports c2pstr and p2cstr functions:
 * Line 21: import c2pstr    ; c2pstr(s) char *s;
 * Line 22: import p2cstr    ; p2cstr(s) char *s;
 *
 * These functions are called by the assembly trap glue to convert between
 * C strings (null-terminated) and Pascal strings (length-prefixed).
 *
 * Copyright (c) 2024 - System 7 Control Manager Reverse Engineering Project
 */

#include "SystemTypes.h"
#include "System71StdLib.h"

#include <stdint.h>


/**
 * Convert C string to Pascal string in place

 *
 * Assembly usage pattern:
 * - Called before trap calls to convert C string parameters to Pascal format
 * - Modifies string buffer in place
 * - Used in newcontrol, setcontroltitle functions
 */
void c2pstr(char *str) {
    if (!str) return;

    size_t len = strlen(str);
    if (len > 255) len = 255;  /* Pascal strings limited to 255 characters */

    /* Shift string contents one byte to the right */
    memmove(str + 1, str, len);

    /* Set length byte at the beginning */
    str[0] = (UInt8)len;

    /* Null terminate after length byte + string (for safety) */
    str[len + 1] = '\0';
}

/**
 * Convert Pascal string to C string in place

 *
 * Assembly usage pattern:
 * - Called after trap calls to convert Pascal string results back to C format
 * - Modifies string buffer in place
 * - Used in newcontrol, setcontroltitle, getcontroltitle functions
 */
void p2cstr(char *str) {
    if (!str) return;

    UInt8 len = (UInt8)str[0];  /* Get length from first byte */

    /* Shift string contents one byte to the left */
    memmove(str, str + 1, len);

    /* Null terminate */
    str[len] = '\0';
}

/**
