#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "digit_render.h"
#include "../assets/image.h"


void app_main(void)
{
    printf("Initializing display...\\n");

    display_gpio_init();
    display_spi_init();
    ili9341_init();

    display_set_rotation(DISPLAY_ROTATION_90);
    // clear_screen(COLOR_MAGENTA);

    draw_image(0, 0, BG_WIDTH, BG_HEIGHT, bg);

    outputVoltage(12.34f);
    outputCurrent(1.23f);

    setVoltage(5.00f);
    setCurrent(2.50f);

    powerIn(30.45f);
    powerConsumption(15.23f);

    while (1)
    {
        for (float v = 0.0f; v <= 24.0f; v += 0.1f)
        {
            float i = (v / 24.0f) * 5.0f;   // Current varies 0–5 A

            outputVoltage(v);
            outputCurrent(i);

            setVoltage(v);
            setCurrent(i);

            float power = v * i;
            powerIn(power);
            powerConsumption(power);

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}