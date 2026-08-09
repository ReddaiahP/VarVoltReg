from PIL import Image
import os
import re

# =====================================================
# Configuration
# =====================================================
SUPPORTED_EXT = ('.png', '.jpg', '.jpeg', '.bmp')
VALUES_PER_LINE = 12

# Most SPI TFT controllers (ILI9341, ST7789, etc.) expect RGB565 pixel
# bytes MSB-first (big-endian) on the wire. ESP32/STM32 are little-endian,
# so a uint16_t is stored in memory low-byte-first. If we don't swap the
# bytes here, whatever reads this array as raw bytes (DMA/SPI) will send
# them in the wrong order and colors will come out corrupted/shifted.
# Set to False only if your display driver already does the swap itself
# (e.g. some LCD panel APIs / esp_lcd byte-swap in software or hardware).
BIG_ENDIAN_OUTPUT = True

# =====================================================
# RGB888 to RGB565
# =====================================================
def rgb888_to_rgb565(r, g, b):
    val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    if BIG_ENDIAN_OUTPUT:
        # swap bytes so the little-endian compiler stores them
        # in big-endian order in memory
        val = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF)
    return val

# =====================================================
# Convert filename to valid C variable
# =====================================================
def sanitize_name(filename):
    name = os.path.splitext(filename)[0]
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name)
    # C identifiers can't start with a digit
    if name and name[0].isdigit():
        name = '_' + name
    return name

# =====================================================
# Convert image to RGB565 array
# =====================================================
def convert_image(image_path):
    img = Image.open(image_path).convert('RGB')
    width, height = img.size

    pixels = []

    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            pixels.append(rgb888_to_rgb565(r, g, b))

    return width, height, pixels

# =====================================================
# Generate image.h
# =====================================================
def generate_header(images):
    with open('image.h', 'w') as f:

        f.write('#ifndef IMAGE_H\n')
        f.write('#define IMAGE_H\n\n\n')

        f.write('#include <stdint.h>\n\n\n')

        for img in images:

            upper = img['name'].upper()

            f.write(f'#define {upper}_HEIGHT {img["height"]}\n')
            f.write(f'#define {upper}_WIDTH {img["width"]}\n\n')

            f.write(f'extern const uint16_t {img["name"]}[];\n\n\n')

        f.write('#endif\n')

# =====================================================
# Generate image.c
# =====================================================
def generate_source(images):
    with open('image.c', 'w') as f:

        f.write('#include "image.h"\n\n\n\n')

        for img in images:

            f.write(f'const uint16_t {img["name"]}[]  = {{\n')

            pixels = img['pixels']

            for i in range(0, len(pixels), VALUES_PER_LINE):

                chunk = pixels[i:i + VALUES_PER_LINE]

                line = ', '.join(f'0x{p:04x}' for p in chunk)

                f.write(f'  {line}, \n')

            f.write('};\n\n')

# =====================================================
# Main
# =====================================================
def main():

    images = []

    for filename in sorted(os.listdir('.')):

        if filename.lower().endswith(SUPPORTED_EXT):

            name = sanitize_name(filename)

            width, height, pixels = convert_image(filename)

            images.append({
                'name': name,
                'width': width,
                'height': height,
                'pixels': pixels
            })

            print(f'Converted: {filename} -> {name} ({width}x{height})')

    if not images:
        print('No images found in current directory.')
        return

    generate_header(images)
    generate_source(images)

    print('\nGenerated image.h and image.c successfully.')

if __name__ == '__main__':
    main()