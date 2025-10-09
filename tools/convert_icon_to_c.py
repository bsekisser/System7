#!/usr/bin/env python3
"""
Convert Mac Picasso logo PNG to C bitmap array for StartupScreen
"""
from PIL import Image
import sys

def convert_to_c_array(input_file, output_file, target_width=128, target_height=128):
    # Load and resize image
    img = Image.open(input_file)
    img = img.resize((target_width, target_height), Image.Resampling.LANCZOS)
    img = img.convert('RGBA')  # Ensure RGBA mode

    pixels = img.load()

    with open(output_file, 'w') as f:
        f.write("/*\n")
        f.write(" * Happy Mac Icon - Mac Picasso Logo\n")
        f.write(f" * Size: {target_width}x{target_height} pixels\n")
        f.write(" * Format: RGBA (32-bit per pixel)\n")
        f.write(" * Auto-generated from Mac_picasso_logo.png\n")
        f.write(" */\n\n")

        f.write("#include \"SystemTypes.h\"\n\n")

        f.write(f"#define HAPPY_MAC_WIDTH {target_width}\n")
        f.write(f"#define HAPPY_MAC_HEIGHT {target_height}\n\n")

        # Write pixel data as RGBA bytes
        f.write("static const UInt8 gHappyMacIconData[] = {\n")

        for y in range(target_height):
            f.write("    ")
            for x in range(target_width):
                r, g, b, a = pixels[x, y]
                f.write(f"0x{r:02X},0x{g:02X},0x{b:02X},0x{a:02X}, ")
                if (x + 1) % 4 == 0:
                    f.write("\n    ")
            f.write(f" /* row {y} */\n")

        f.write("};\n")

    print(f"Converted {input_file} -> {output_file}")
    print(f"Icon size: {target_width}x{target_height}")
    print(f"Data size: {target_width * target_height * 4} bytes")

if __name__ == "__main__":
    convert_to_c_array("resources/Mac_picasso_logo.png",
                       "src/Resources/happy_mac_icon.c",
                       128, 128)
