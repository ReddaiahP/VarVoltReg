#include <stdio.h>
#include <string.h>

#include "digit_render.h"
#include "display.h"
#include "../assets/image.h"

// --------------------------------------------------
// Digit image lookup tables
// --------------------------------------------------

static const uint16_t *digit_images[] = {
    _0, _1, _2, _3, _4,
    _5, _6, _7, _8, _9
};

static const uint16_t digit_widths[] = {
    _0_WIDTH, _1_WIDTH, _2_WIDTH, _3_WIDTH, _4_WIDTH,
    _5_WIDTH, _6_WIDTH, _7_WIDTH, _8_WIDTH, _9_WIDTH
};

static const uint16_t digit_heights[] = {
    _0_HEIGHT, _1_HEIGHT, _2_HEIGHT, _3_HEIGHT, _4_HEIGHT,
    _5_HEIGHT, _6_HEIGHT, _7_HEIGHT, _8_HEIGHT, _9_HEIGHT
};

// --------------------------------------------------
// Generic float drawing function
// --------------------------------------------------

static void draw_float_value(int x, int y, float value, int decimals)
{
    char str[16];

    if (value < 0.0f)
        value = 0.0f;

    snprintf(str, sizeof(str), "%.*f", decimals, value);

    int stepX = DIGIT_CELL_WIDTH -
                (DIGIT_CELL_WIDTH * DIGIT_X_OVERLAP_PERCENT) / 100;

    // Clear the entire number area first
    const int MAX_CHARS = 6;

    clear_region(
        x,
        y,
        x + (MAX_CHARS * stepX),
        y + DIGIT_CELL_HEIGHT,
        COLOR_LIGHT_GRAY
    );

    int xpos = x;

    for (int i = 0; str[i] != '\0'; i++) {

        if (str[i] >= '0' && str[i] <= '9') {

            int d = str[i] - '0';

            draw_image(
                xpos,
                y,
                digit_widths[d],
                digit_heights[d],
                digit_images[d]
            );
        }
        else if (str[i] == '.') {

            draw_image(
                xpos,
                y,
                DOT_WIDTH,
                DOT_HEIGHT,
                dot
            );
        }

        xpos += stepX;
    }
}

// --------------------------------------------------
// Public API
// --------------------------------------------------

void outputVoltage(float voltage)
{
    if (voltage > 24.0f)
        voltage = 24.0f;

    draw_float_value(45, 84, voltage, 2);
}

void outputCurrent(float current)
{
    if (current > 5.0f)
        current = 5.0f;

    draw_float_value(209, 84, current, 2);
}

void setVoltage(float voltage)
{
    if (voltage > 24.0f)
        voltage = 24.0f;

    draw_float_value(18, 159, voltage, 2);
}

void setCurrent(float current)
{
    if (current > 5.0f)
        current = 5.0f;

    draw_float_value(18, 204, current, 2);
}

void powerIn(float power)
{
    draw_float_value(212, 166, power, 2);
}

void powerConsumption(float power)
{
    draw_float_value(212, 189, power, 2);
}