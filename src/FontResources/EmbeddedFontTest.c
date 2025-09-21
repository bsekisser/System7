// #include "CompatibilityFix.h" // Removed
#include <stdio.h>
/*
 * Embedded Font System Test
 * Tests the complete integrated font system with embedded fallbacks
 */

#include "SystemTypes.h"

#include "FontResources/SystemFonts.h"
#include "EmbeddedFonts.c" // Include implementation directly for testing


int main(void) {
    printf("=== System 7.1 Embedded Font System Test ===\n");

    /* Test the embedded font system */
    return TestEmbeddedFonts();
}
