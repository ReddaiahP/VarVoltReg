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

    /* Choose DISPLAY_ROTATION_0, _90, _180, or _270. */
    display_set_rotation(DISPLAY_ROTATION_0);
    clear_screen(COLOR_MAGENTA);

    display_image_animation_t profile_animation = {
        .x = 0,
        .end_x = 200,
        .step = 1,
        .y = 0U,
        .width = PROFILE_WIDTH,
        .height = PROFILE_HEIGHT,
        .image_data = butterFly,
        .background_color = COLOR_MAGENTA,
    };

    display_image_animation_start(&profile_animation);

    while (1) {
        display_image_animation_step(&profile_animation);
        vTaskDelay(pdMS_TO_TICKS(1));
    
    }
}
