# Display Library API Guide

This project drives a 240 x 320 RGB565 ILI9341 display over SPI. Include the library in your application with:

```c
#include "display.h"
```

## Initialize the display

Call the setup functions once, in this order, before using any drawing API:

```c
display_gpio_init();
display_spi_init();
ili9341_init();
```

| Function | Description |
| --- | --- |
| `display_gpio_init()` | Configures the display's DC, reset, and backlight GPIO pins, then resets the display. |
| `display_spi_init()` | Creates the SPI bus and registers the display as an SPI device. |
| `ili9341_init()` | Sends the ILI9341 controller initialization sequence and selects RGB565 pixels. |

## Colors

All colors use 16-bit RGB565 format.

```c
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_MAGENTA 0xF81F
```

## Rotation

```c
display_set_rotation(DISPLAY_ROTATION_0);
```

`display_set_rotation()` accepts one of these values:

| Value | Orientation |
| --- | --- |
| `DISPLAY_ROTATION_0` | Portrait |
| `DISPLAY_ROTATION_90` | Landscape |
| `DISPLAY_ROTATION_180` | Portrait, upside down |
| `DISPLAY_ROTATION_270` | Landscape, upside down |

Use these optional functions when application code needs the current logical dimensions:

```c
uint16_t width = display_get_width();
uint16_t height = display_get_height();
```

They return 240 x 320 in portrait and 320 x 240 in landscape.

## Clear functions

```c
clear_screen(COLOR_BLACK);
fill_color(COLOR_BLUE);  // Same result as clear_screen(COLOR_BLUE)
```

| Function | Parameters | Description |
| --- | --- | --- |
| `clear_screen(color)` | `color`: RGB565 color | Fills the entire visible screen. |
| `fill_color(color)` | `color`: RGB565 color | Alias for `clear_screen()`. |
| `clear_region(x0, y0, x1, y1, color)` | Start coordinate, end coordinate, RGB565 color | Fills a rectangle. `x1` and `y1` are inclusive. Out-of-screen portions are clipped. |

Example:

```c
clear_region(10, 20, 109, 69, COLOR_RED);
```

This clears a 100 x 50 rectangle.

## Draw an image

```c
draw_image(20, 30, PROFILE_WIDTH, PROFILE_HEIGHT, profilePic);
```

Parameters:

| Parameter | Description |
| --- | --- |
| `x0`, `y0` | Top-left destination coordinate. |
| `w`, `h` | Original image width and height in pixels. |
| `image_data` | Pointer to row-major RGB565 pixel data, for example `profilePic`. |

`draw_image()` clips the image safely. For example, a 200 x 200 image at `x = 50` on a 240-pixel-wide portrait screen displays its visible 190-pixel-wide portion without corrupting the image.

## One-way horizontal image animation

The animation API moves an opaque image without clearing the whole screen each frame. It draws the new position first, then erases only the uncovered strip from the old position. Set `background_color` to the same color used by `clear_screen()`.

```c
display_image_animation_t profile_animation = {
    .x = 0,                       // Start x coordinate
    .end_x = 100,                 // Final x coordinate
    .step = 2,                    // Pixels moved per animation step
    .y = 30U,                     // Fixed y coordinate
    .width = PROFILE_WIDTH,
    .height = PROFILE_HEIGHT,
    .image_data = profilePic,
    .background_color = COLOR_BLACK,
};

clear_screen(COLOR_BLACK);
display_image_animation_start(&profile_animation);

while (1) {
    display_image_animation_step(&profile_animation);
    vTaskDelay(pdMS_TO_TICKS(30));
}
```

Animation structure fields:

| Field | Description |
| --- | --- |
| `x` | Starting position and current x position. The API updates this after every step. |
| `end_x` | Final x coordinate. The image stops here. |
| `step` | Positive number of pixels to move each update. |
| `y` | Fixed vertical position. |
| `width`, `height` | Original dimensions of the image. |
| `image_data` | RGB565 image pointer. |
| `background_color` | Color used to erase the trail. It must match the background. |

To move right, set `x` lower than `end_x`:

```c
.x = 0,
.end_x = 100,
```

To move left, set `x` higher than `end_x`:

```c
.x = 100,
.end_x = 0,
```

| Function | Description |
| --- | --- |
| `display_image_animation_start(&animation)` | Draws the first image frame at `animation.x`. |
| `display_image_animation_step(&animation)` | Moves one step toward `animation.end_x`. It does nothing once the endpoint is reached. |

The delay in the loop controls animation speed. A smaller delay produces more frequent updates; a larger `step` moves farther per update.

## Complete basic example

```c
void app_main(void)
{
    display_gpio_init();
    display_spi_init();
    ili9341_init();

    display_set_rotation(DISPLAY_ROTATION_0);
    clear_screen(COLOR_BLACK);
    draw_image(0, 30, PROFILE_WIDTH, PROFILE_HEIGHT, profilePic);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```
