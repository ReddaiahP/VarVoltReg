#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "image.h"

// RGB565 color definitions
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

void app_main(void)
{
    printf("Initializing display...\n");

    display_gpio_init();
    display_spi_init();
    ili9341_init();
    clear_screen(COLOR_YELLOW);   // black background
    while (1)
    {
        
        draw_image(0,0, PROFILE_WIDTH, PROFILE_HEIGHT, profilePic);
    }
}