/* Quick test to verify Chicago font data */
#include <stdio.h>
#include <stdint.h>
#include "include/chicago_font.h"

extern const uint8_t chicago_bitmap[2100];

int main() {
    printf("Chicago font test:\n");
    printf("  Height: %d, Ascent: %d, Descent: %d\n",
           CHICAGO_HEIGHT, CHICAGO_ASCENT, CHICAGO_DESCENT);
    printf("  Row bytes: %d\n", CHICAGO_ROW_BYTES);

    // Test a few characters
    printf("\nCharacter info:\n");
    printf("  'A' (0x41): bit_start=%d, bit_width=%d, advance=%d\n",
           chicago_ascii['A' - 32].bit_start,
           chicago_ascii['A' - 32].bit_width,
           chicago_ascii['A' - 32].advance);

    printf("  'a' (0x61): bit_start=%d, bit_width=%d, advance=%d\n",
           chicago_ascii['a' - 32].bit_start,
           chicago_ascii['a' - 32].bit_width,
           chicago_ascii['a' - 32].advance);

    // Check bitmap data
    printf("\nFirst 16 bytes of bitmap: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", chicago_bitmap[i]);
    }
    printf("\n");

    return 0;
}