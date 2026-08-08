#include "display.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stddef.h>
#include <stdbool.h>

#define PIN_NUM_MOSI 23
#define PIN_NUM_MISO -1
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   5
#define PIN_NUM_DC   2
#define PIN_NUM_RST  4
#define PIN_NUM_LED  15

#define DISPLAY_WIDTH  240U
#define DISPLAY_HEIGHT 320U
#define DISPLAY_SPI_CLOCK_HZ (10U * 1000U * 1000U)
#define SPI_TRANSFER_CHUNK_SIZE 4096U
#define ILI9341_MADCTL 0x36U
#define ILI9341_PIXFMT 0x3AU
#define ILI9341_MADCTL_BGR 0x08U
#define ILI9341_MADCTL_MV  0x20U
#define ILI9341_MADCTL_MX  0x40U
#define ILI9341_MADCTL_MY  0x80U

spi_device_handle_t spi;
static uint16_t display_width = DISPLAY_WIDTH;
static uint16_t display_height = DISPLAY_HEIGHT;
static bool spi_ready;

static const char *TAG = "display";

void display_gpio_init(void) {
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_LED, 1);

    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void display_spi_init(void) {
    if (spi_ready) {
        return;
    }

    spi_bus_config_t spi_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    spi_device_interface_config_t spi_device_config = {
        .clock_speed_hz = DISPLAY_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
        .flags = 0
    };

    esp_err_t result = spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(result));
        return;
    }

    result = spi_bus_add_device(SPI2_HOST, &spi_device_config, &spi);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(result));
        spi_bus_free(SPI2_HOST);
        spi = NULL;
        return;
    }

    spi_ready = true;
}

void send_cmd(uint8_t cmd) {
    if (!spi_ready) {
        return;
    }

    gpio_set_level(PIN_NUM_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_transmit(spi, &t);
}

void send_data(const uint8_t *data, int len) {
    if (!spi_ready || data == NULL || len <= 0) {
        return;
    }

    gpio_set_level(PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_transmit(spi, &t);
}

static void send_pixels(const uint8_t *data, size_t byte_count) {
    if (!spi_ready || data == NULL) {
        return;
    }

    gpio_set_level(PIN_NUM_DC, 1);

    while (byte_count > 0U) {
        size_t chunk = byte_count > SPI_TRANSFER_CHUNK_SIZE
                           ? SPI_TRANSFER_CHUNK_SIZE
                           : byte_count;
        spi_transaction_t t = {
            .length = chunk * 8U,
            .tx_buffer = data,
        };
        spi_device_transmit(spi, &t);
        data += chunk;
        byte_count -= chunk;
    }
}

void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t caset[] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    uint8_t raset[] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};

    send_cmd(0x2A); send_data(caset, sizeof(caset));
    send_cmd(0x2B); send_data(raset, sizeof(raset));
    send_cmd(0x2C);
}

void fill_color(uint16_t color) {
    clear_screen(color);
}

void display_set_rotation(display_rotation_t rotation) {
    static const uint8_t madctl_values[] = {
        ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
        ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR,
        ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR,
        ILI9341_MADCTL_MX | ILI9341_MADCTL_MY |
            ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR,
    };

    if (rotation > DISPLAY_ROTATION_270) {
        return;
    }

    send_cmd(ILI9341_MADCTL);
    send_data(&madctl_values[rotation], 1);

    /* Some ILI9341-compatible modules need these settings re-latched. */
    send_cmd(ILI9341_PIXFMT);
    const uint8_t rgb565 = 0x55U;
    send_data(&rgb565, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    if ((rotation & 1U) != 0U) {
        display_width = DISPLAY_HEIGHT;
        display_height = DISPLAY_WIDTH;
    } else {
        display_width = DISPLAY_WIDTH;
        display_height = DISPLAY_HEIGHT;
    }
}

uint16_t display_get_width(void) {
    return display_width;
}

uint16_t display_get_height(void) {
    return display_height;
}

void clear_screen(uint16_t color) {
    set_window(0, 0, display_width - 1U, display_height - 1U);

    static uint8_t line_buf[DISPLAY_HEIGHT * 2U];
    for (uint16_t i = 0; i < display_width; i++) {
        line_buf[i * 2] = color >> 8;
        line_buf[i * 2 + 1] = color & 0xFF;
    }

    for (uint16_t y = 0; y < display_height; y++) {
        send_pixels(line_buf, (size_t)display_width * 2U);
    }
}

void clear_region(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    if (x1 < x0 || y1 < y0 || x0 >= display_width || y0 >= display_height) {
        return;
    }

    if (x1 >= display_width) {
        x1 = display_width - 1U;
    }
    if (y1 >= display_height) {
        y1 = display_height - 1U;
    }

    set_window(x0, y0, x1, y1);

    uint16_t width = x1 - x0 + 1U;
    uint16_t height = y1 - y0 + 1U;
    static uint8_t buf[DISPLAY_HEIGHT * 2U];
    for (uint16_t i = 0; i < width; i++) {
        buf[i * 2] = color >> 8;
        buf[i * 2 + 1] = color & 0xFF;
    }

    for (uint16_t row = 0; row < height; row++) {
        send_pixels(buf, (size_t)width * 2U);
    }
}

/* 3. New: Draw image from provided pixel buffer */
void draw_image(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, const uint16_t *image_data) {
    /*
     * Clip the destination rectangle before programming the controller window.
     * Image data is row-major, so each visible row must start at its original
     * width; sending one contiguous clipped block would scramble the rows.
     */
    if (image_data == NULL || w == 0U || h == 0U ||
        x0 >= display_width || y0 >= display_height) {
        return;
    }

    uint16_t visible_w = w;
    uint16_t visible_h = h;

    if ((uint32_t)x0 + w > display_width) {
        visible_w = display_width - x0;
    }
    if ((uint32_t)y0 + h > display_height) {
        visible_h = display_height - y0;
    }

    set_window(x0, y0, x0 + visible_w - 1U, y0 + visible_h - 1U);

    /* A non-horizontally-clipped image is contiguous in memory. */
    if (visible_w == w) {
        send_pixels((const uint8_t *)image_data,
                    (size_t)visible_w * visible_h * sizeof(*image_data));
        return;
    }

    for (uint16_t row = 0; row < visible_h; row++) {
        const uint8_t *data_ptr = (const uint8_t *)(image_data + ((size_t)row * w));
        send_pixels(data_ptr, (size_t)visible_w * sizeof(*image_data));
    }
}

void display_image_animation_start(display_image_animation_t *animation) {
    if (animation == NULL || animation->image_data == NULL ||
        animation->width == 0U || animation->height == 0U || animation->x < 0) {
        return;
    }

    draw_image((uint16_t)animation->x, animation->y,
               animation->width, animation->height, animation->image_data);
}

void display_image_animation_step(display_image_animation_t *animation) {
    if (animation == NULL || animation->image_data == NULL ||
        animation->width == 0U || animation->height == 0U ||
        animation->step == 0U || animation->x < 0 || animation->end_x < 0) {
        return;
    }

    if (animation->x == animation->end_x) {
        return;
    }

    int32_t previous_x = animation->x;
    int32_t next_x = previous_x;

    if (animation->end_x > previous_x) {
        next_x += animation->step;
        if (next_x > animation->end_x) {
            next_x = animation->end_x;
        }
    } else {
        next_x -= animation->step;
        if (next_x < animation->end_x) {
            next_x = animation->end_x;
        }
    }

    draw_image((uint16_t)next_x, animation->y,
               animation->width, animation->height, animation->image_data);

    if (next_x > previous_x) {
        clear_region((uint16_t)previous_x, animation->y,
                     (uint16_t)(next_x - 1),
                     animation->y + animation->height - 1U,
                     animation->background_color);
    } else {
        clear_region((uint16_t)(next_x + animation->width), animation->y,
                     (uint16_t)(previous_x + animation->width - 1),
                     animation->y + animation->height - 1U,
                     animation->background_color);
    }

    animation->x = (int16_t)next_x;
}

void ili9341_init(void) {
    send_cmd(0xEF); uint8_t ef[] = {0x03, 0x80, 0x02}; send_data(ef, 3);
    send_cmd(0xCF); uint8_t cf[] = {0x00, 0xC1, 0x30}; send_data(cf, 3);
    send_cmd(0xED); uint8_t ed[] = {0x64, 0x03, 0x12, 0x81}; send_data(ed, 4);
    send_cmd(0xE8); uint8_t e8[] = {0x85, 0x00, 0x78}; send_data(e8, 3);
    send_cmd(0xCB); uint8_t cb[] = {0x39, 0x2C, 0x00, 0x34, 0x02}; send_data(cb, 5);
    send_cmd(0xF7); uint8_t f7[] = {0x20}; send_data(f7, 1);
    send_cmd(0xEA); uint8_t ea[] = {0x00, 0x00}; send_data(ea, 2);
    send_cmd(0xC0); uint8_t c0[] = {0x23}; send_data(c0, 1);
    send_cmd(0xC1); uint8_t c1[] = {0x10}; send_data(c1, 1);
    send_cmd(0xC5); uint8_t c5[] = {0x3e, 0x28}; send_data(c5, 2);
    send_cmd(0xC7); uint8_t c7[] = {0x86}; send_data(c7, 1);
    send_cmd(ILI9341_MADCTL); uint8_t m[] = {ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR}; send_data(m, 1);
    send_cmd(ILI9341_PIXFMT); uint8_t pix[] = {0x55}; send_data(pix, 1);
    send_cmd(0xB1); uint8_t b1[] = {0x00, 0x18}; send_data(b1, 2);
    send_cmd(0xB6); uint8_t b6[] = {0x08, 0x82, 0x27}; send_data(b6, 3);
    send_cmd(0xF2); uint8_t f2[] = {0x00}; send_data(f2, 1);
    send_cmd(0x26); uint8_t gamma[] = {0x01}; send_data(gamma, 1);
    send_cmd(0xE0); uint8_t e0[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}; send_data(e0, 15);
    send_cmd(0xE1); uint8_t e1[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}; send_data(e1, 15);
    send_cmd(0x11); vTaskDelay(pdMS_TO_TICKS(120));
    send_cmd(0x29); vTaskDelay(pdMS_TO_TICKS(120));
}
