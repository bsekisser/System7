/* Visual test of Chicago font rendering */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "include/chicago_font.h"

extern const uint8_t chicago_bitmap[2100];

void print_char_bitmap(char ch) {
    if (ch < 32 || ch > 126) return;

    ChicagoCharInfo info = chicago_ascii[ch - 32];

    printf("\nCharacter '%c' (0x%02X):\n", ch, ch);
    printf("  bit_start: %d, bit_width: %d, advance: %d\n",
           info.bit_start, info.bit_width, info.advance);

    // Print the bitmap for this character
    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int row_start_byte = row * CHICAGO_ROW_BYTES;

        printf("  ");
        for (int col = 0; col < info.bit_width; col++) {
            int bit_position = info.bit_start + col;
            int byte_offset = row_start_byte + (bit_position / 8);
            int bit_index = 7 - (bit_position % 8);

            if (byte_offset < 2100) {
                if (chicago_bitmap[byte_offset] & (1 << bit_index)) {
                    printf("#");
                } else {
                    printf(".");
                }
            } else {
                printf("?");
            }
        }
        printf("\n");
    }
}

int main() {
    // Test menu characters
    const char* menu_text = "File";

    printf("Testing menu text: %s\n", menu_text);

    for (int i = 0; menu_text[i]; i++) {
        print_char_bitmap(menu_text[i]);
    }

    return 0;
}