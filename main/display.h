#ifndef DISPLAY_H
#define DISPLAY_H

#include "driver/spi_master.h"
#include <stdint.h>

#define COLOR_LIGHT_GRAY 0xDEDB
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_CYAN 0x07FF

typedef enum {
    DISPLAY_ROTATION_0 = 0,
    DISPLAY_ROTATION_90,
    DISPLAY_ROTATION_180,
    DISPLAY_ROTATION_270,
} display_rotation_t;

typedef struct {
    int16_t x;
    int16_t end_x;
    uint16_t step;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    const uint16_t *image_data;
    uint16_t background_color;
} display_image_animation_t;

void display_gpio_init(void);
void display_spi_init(void);
void ili9341_init(void);
void fill_color(uint16_t color);
void display_set_rotation(display_rotation_t rotation);
uint16_t display_get_width(void);
uint16_t display_get_height(void);

// New functions
void clear_screen(uint16_t color);
void clear_region(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
/* Draws the on-screen portion of a row-major RGB565 image; off-screen pixels are clipped. */
void draw_image(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, const uint16_t *image_data);
void display_image_animation_start(display_image_animation_t *animation);
void display_image_animation_step(display_image_animation_t *animation);

#endif
