#include "ft81x.h"
#include "ft81x_registers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ---- Display List Command Encoding Macros ---- */

#define DL_CLEAR_COLOR(rgb)          (FT81X_DL_CLEAR_COLOR_RGB | ((rgb) & 0xFFFFFF))
#define DL_COLOR(rgb)                (FT81X_DL_COLOR_RGB | ((rgb) & 0xFFFFFF))
#define DL_CLEAR(c, s, t)           (FT81X_DL_CLEAR | ((c) << 2) | ((s) << 1) | (t))
#define DL_POINT_SIZE(s)            (FT81X_DL_POINT_SIZE | ((s) & 0xFFF))
#define DL_LINE_WIDTH(w)            (FT81X_DL_LINE_WIDTH | ((w) & 0xFFF))
#define DL_BEGIN(p)                 (FT81X_DL_BEGIN | (p))
#define DL_END()                    (FT81X_DL_END)
#define DL_VERTEX2F(x, y)          (FT81X_DL_VERTEX2F | (((uint32_t)((x) & 0x7FFF)) << 15) | ((uint32_t)((y) & 0x7FFF)))
#define DL_VERTEX2II(x, y, h, c)   (FT81X_DL_VERTEX2II | (((uint32_t)((x) & 0x1FF)) << 21) | (((uint32_t)((y) & 0x1FF)) << 12) | ((uint32_t)(h) << 7) | (uint32_t)(c))
#define DL_BITMAP_SOURCE(a)         (FT81X_DL_BITMAP_SOURCE | (a))
#define DL_BITMAP_LAYOUT(f, s, h)   (FT81X_DL_BITMAP_LAYOUT | ((uint32_t)(f) << 19) | (((uint32_t)(s) & 0x3FF) << 9) | ((uint32_t)(h) & 0x1FF))
#define DL_BITMAP_SIZE(f, wx, wy, w, h) (FT81X_DL_BITMAP_SIZE | ((uint32_t)((f) & 1) << 20) | ((uint32_t)((wx) & 1) << 19) | ((uint32_t)((wy) & 1) << 18) | (((uint32_t)(w) & 0x1FF) << 9) | ((uint32_t)(h) & 0x1FF))
#define DL_BLEND_FUNC(src, dst)     (FT81X_DL_BLEND_FUNC | ((uint32_t)(src) << 3) | (uint32_t)(dst))

#define PACK16(lo, hi)              ((uint32_t)(uint16_t)(lo) | ((uint32_t)(uint16_t)(hi) << 16))

/* ---- Internal Helpers ---- */

static void cs1_low(ft81x_t *dev) {
    gpio_set(&dev->gpio, dev->gpio.cs1_pin, 0);
}

static void cs1_high(ft81x_t *dev) {
    gpio_set(&dev->gpio, dev->gpio.cs1_pin, 1);
}

static void send_host_command(ft81x_t *dev, uint32_t cmd) {
    uint8_t buf[3] = { (cmd >> 16) & 0xFF, (cmd >> 8) & 0xFF, cmd & 0xFF };
    cs1_low(dev);
    spi_transfer(&dev->spi, buf, NULL, 3);
    cs1_high(dev);
}

static void increase_cmd_write_address(ft81x_t *dev, uint16_t delta) {
    dev->cmd_write_address = (dev->cmd_write_address + delta) & 0xFFF;
}

static void start_cmd(ft81x_t *dev, uint32_t cmd) {
    uint32_t addr = (FT81X_RAM_CMD + dev->cmd_write_address) | 0x800000;
    uint8_t buf[7] = {
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        cmd & 0xFF,
        (cmd >> 8) & 0xFF,
        (cmd >> 16) & 0xFF,
        (cmd >> 24) & 0xFF,
    };
    cs1_low(dev);
    spi_transfer(&dev->spi, buf, NULL, 7);
    increase_cmd_write_address(dev, 4);
}

static void intermediate_cmd(ft81x_t *dev, uint32_t cmd) {
    uint8_t buf[4] = {
        cmd & 0xFF,
        (cmd >> 8) & 0xFF,
        (cmd >> 16) & 0xFF,
        (cmd >> 24) & 0xFF,
    };
    spi_transfer(&dev->spi, buf, NULL, 4);
    increase_cmd_write_address(dev, 4);
}

static void end_cmd(ft81x_t *dev, uint32_t cmd) {
    uint8_t buf[4] = {
        cmd & 0xFF,
        (cmd >> 8) & 0xFF,
        (cmd >> 16) & 0xFF,
        (cmd >> 24) & 0xFF,
    };
    spi_transfer(&dev->spi, buf, NULL, 4);
    cs1_high(dev);
    increase_cmd_write_address(dev, 4);
    ft81x_write16(dev, FT81X_REG_CMD_WRITE, dev->cmd_write_address);
}

static void send_text(ft81x_t *dev, const char *text) {
    uint32_t data = 0xFFFFFFFF;
    for (uint8_t i = 0; (data >> 24) != 0; i += 4) {
        data = 0;
        if (text[i] != 0) {
            data |= (uint8_t)text[i];
            if (text[i + 1] != 0) {
                data |= (uint32_t)(uint8_t)text[i + 1] << 8;
                if (text[i + 2] != 0) {
                    data |= (uint32_t)(uint8_t)text[i + 2] << 16;
                    if (text[i + 3] != 0) {
                        data |= (uint32_t)(uint8_t)text[i + 3] << 24;
                    }
                }
            }
        }
        if ((data >> 24) != 0) {
            intermediate_cmd(dev, data);
        } else {
            end_cmd(dev, data);
        }
    }
    if ((data >> 24) != 0) {
        end_cmd(dev, 0);
    }
}

static uint8_t init_bitmap_handle_for_font(ft81x_t *dev, uint8_t font) {
    if (font > 31) {
        start_cmd(dev, FT81X_CMD_ROMFONT);
        intermediate_cmd(dev, 14);
        end_cmd(dev, font);
        return 14;
    }
    return font;
}

static void send_command_to_display(ft81x_t *dev, uint8_t cmd, uint8_t num_params, const uint8_t *params) {
    gpio_set(&dev->gpio, dev->gpio.dc_pin, 0);
    gpio_set(&dev->gpio, dev->gpio.cs2_pin, 0);
    spi_transfer(&dev->spi, &cmd, NULL, 1);
    if (num_params > 0) {
        gpio_set(&dev->gpio, dev->gpio.dc_pin, 1);
        for (uint8_t i = 0; i < num_params; i++) {
            spi_transfer(&dev->spi, &params[i], NULL, 1);
        }
        gpio_set(&dev->gpio, dev->gpio.dc_pin, 0);
    }
    gpio_set(&dev->gpio, dev->gpio.cs2_pin, 1);
}

#define DISPLAY_CMD(c, ...) do { \
    const uint8_t _p[] = { __VA_ARGS__ }; \
    send_command_to_display(dev, (c), sizeof(_p), _p); \
} while (0)

#define DISPLAY_CMD_NOPARAMS(c) do { \
    send_command_to_display(dev, (c), 0, NULL); \
} while (0)

static void init_display(ft81x_t *dev) {
    /* Sleep mode off */
    DISPLAY_CMD_NOPARAMS(FT81X_ST7701S_CMD_SLPOUT);
    usleep(300000);

    /* Command2, BK0 */
    DISPLAY_CMD(FT81X_ST7701S_CND2BKxSEL, 0x77, 0x01, 0x00, 0x00, FT81X_ST7701S_BKx_CMD);
    DISPLAY_CMD(0xC0, 0x3B, 0x00);           /* LNESET */
    DISPLAY_CMD(0xC1, 0x14, 0x0A);           /* PORCTRL: vbp, vfp */
    DISPLAY_CMD(0xC2, 0x21, 0x08);           /* INVSEL */
    DISPLAY_CMD(0xB0, 0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08,
                     0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18); /* PVGAMCTRL */
    DISPLAY_CMD(0xB1, 0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08,
                     0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18); /* NVGAMCTRL */

    /* Command2, BK1 */
    DISPLAY_CMD(FT81X_ST7701S_CND2BKxSEL, 0x77, 0x01, 0x00, 0x00, FT81X_ST7701S_BK1_CMD);
    DISPLAY_CMD(0xB0, 0x60);                  /* VRHS */
    DISPLAY_CMD(0xB1, 0x30);                  /* VCOM */
    DISPLAY_CMD(0xB2, 0x87);                  /* VGHSS */
    DISPLAY_CMD(0xB3, 0x80);                  /* TESTCMD */
    DISPLAY_CMD(0xB5, 0x49);                  /* VGLS */
    DISPLAY_CMD(0xB7, 0x85);                  /* PWCTLR1 */
    DISPLAY_CMD(0xB8, 0x21);                  /* PWCTLR2 */
    DISPLAY_CMD(0xC1, 0x78);                  /* SPD1 */
    DISPLAY_CMD(0xC2, 0x78);                  /* SPD2 */

    /* Raw register writes */
    DISPLAY_CMD(0xE0, 0x00, 0x1B, 0x02);
    DISPLAY_CMD(0xE1, 0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00,
                     0x00, 0x44, 0x44);
    DISPLAY_CMD(0xE2, 0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00,
                     0xEC, 0xA0, 0x00, 0x00);
    DISPLAY_CMD(0xE3, 0x00, 0x00, 0x11, 0x11);
    DISPLAY_CMD(0xE4, 0x44, 0x44);
    DISPLAY_CMD(0xE5, 0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0,
                     0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0);
    DISPLAY_CMD(0xE6, 0x00, 0x00, 0x11, 0x11);
    DISPLAY_CMD(0xE7, 0x44, 0x44);
    DISPLAY_CMD(0xE8, 0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0,
                     0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0);
    DISPLAY_CMD(0xEB, 0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40);
    DISPLAY_CMD(0xEC, 0x3C, 0x00);
    DISPLAY_CMD(0xED, 0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF,
                     0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA);

    /* Disable Command2 bank selection */
    DISPLAY_CMD(FT81X_ST7701S_CND2BKxSEL, 0x77, 0x01, 0x00, 0x00, 0x00);

    /* Display on */
    DISPLAY_CMD_NOPARAMS(FT81X_ST7701S_CMD_DISPON);

    /* Set pixel format: 24-bit */
    DISPLAY_CMD(FT81X_ST7701S_CMD_COLMOD, 0x70);
}

/* ---- Public API ---- */

size_t ft81x_sizeof(void) {
    return sizeof(ft81x_t);
}

const char *ft81x_get_error(ft81x_t *dev) {
    return dev->error[0] ? dev->error : NULL;
}

/* ---- Lifecycle ---- */

int ft81x_init(ft81x_t *dev, const char *spi_device, const char *gpio_chip,
               int cs1_pin, int cs2_pin, int dc_pin) {
    memset(dev, 0, sizeof(*dev));
    dev->width = FT81X_DISPLAY_WIDTH;
    dev->height = FT81X_DISPLAY_HEIGHT;

    if (spi_open(&dev->spi, spi_device, 24000000) < 0) {
        snprintf(dev->error, sizeof(dev->error), "SPI open failed: %s", spi_device);
        return -1;
    }
    if (gpio_open(&dev->gpio, gpio_chip, cs1_pin, cs2_pin, dc_pin) < 0) {
        snprintf(dev->error, sizeof(dev->error), "GPIO open failed: %s", gpio_chip);
        spi_close(&dev->spi);
        return -1;
    }

    /* Power-up sequence */
    send_host_command(dev, FT81X_CMD_RST_PULSE);
    usleep(300000);
    send_host_command(dev, FT81X_CMD_CLKEXT);
    usleep(300000);
    send_host_command(dev, FT81X_CMD_ACTIVE);
    usleep(100000);

    /* Wait for chip ready */
    int timeout = 500;
    while (ft81x_read8(dev, FT81X_REG_ID) != 0x7C && --timeout > 0)
        usleep(10000);
    if (timeout <= 0) {
        snprintf(dev->error, sizeof(dev->error), "Chip ID timeout (REG_ID != 0x7C)");
        goto fail;
    }

    timeout = 500;
    while (ft81x_read8(dev, FT81X_REG_CPURESET) != 0x00 && --timeout > 0)
        usleep(10000);
    if (timeout <= 0) {
        snprintf(dev->error, sizeof(dev->error), "Coprocessor reset timeout");
        goto fail;
    }

    /* Configure RGB interface timing (480x480) */
    ft81x_write16(dev, FT81X_REG_HCYCLE,  480 + 8 + 8 + 20 + 2);
    ft81x_write16(dev, FT81X_REG_HOFFSET, 8 + 8 + 20);
    ft81x_write16(dev, FT81X_REG_HSYNC0,  8);
    ft81x_write16(dev, FT81X_REG_HSYNC1,  8 + 8);
    ft81x_write16(dev, FT81X_REG_HSIZE,   480);

    ft81x_write16(dev, FT81X_REG_VCYCLE,  480 + 8 + 8 + 2 + 2);
    ft81x_write16(dev, FT81X_REG_VOFFSET, 8 + 8 + 2);
    ft81x_write16(dev, FT81X_REG_VSYNC0,  8);
    ft81x_write16(dev, FT81X_REG_VSYNC1,  8 + 8);
    ft81x_write16(dev, FT81X_REG_VSIZE,   480);

    ft81x_write8(dev, FT81X_REG_SWIZZLE,  2);
    ft81x_write8(dev, FT81X_REG_PCLK_POL, 0);
    ft81x_write8(dev, FT81X_REG_CSPREAD,  1);
    ft81x_write8(dev, FT81X_REG_DITHER,   0);
    ft81x_write8(dev, FT81X_REG_ROTATE,   0);

    /* First display list */
    ft81x_begin_display_list(dev);
    ft81x_clear(dev, 0x000000);
    ft81x_swap_screen(dev);

    /* Enable pixel clock */
    ft81x_write8(dev, FT81X_REG_PCLK, 3);

    /* Reset display via FT81x GPIO3 */
    uint16_t gpio_dir = ft81x_read16(dev, FT81X_REG_GPIOX_DIR);
    ft81x_write16(dev, FT81X_REG_GPIOX_DIR, gpio_dir | 0x8);
    uint16_t gpio_val = ft81x_read16(dev, FT81X_REG_GPIOX);
    ft81x_write16(dev, FT81X_REG_GPIOX, gpio_val | 0x8);
    usleep(300000);

    /* Initialize ST7701S display controller */
    init_display(dev);

    /* Enable backlight */
    ft81x_write8(dev, FT81X_REG_PWM_DUTY, 128);

    return 0;

fail:
    ft81x_destroy(dev);
    return -1;
}

void ft81x_destroy(ft81x_t *dev) {
    spi_close(&dev->spi);
    gpio_close(&dev->gpio);
}

/* ---- Memory Access ---- */

uint8_t ft81x_read8(ft81x_t *dev, uint32_t address) {
    uint8_t tx[5] = { (address >> 16) & 0x3F, (address >> 8) & 0xFF, address & 0xFF, 0x00, 0x00 };
    uint8_t rx[5] = {0};
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, rx, 5);
    cs1_high(dev);
    return rx[4];
}

uint16_t ft81x_read16(ft81x_t *dev, uint32_t address) {
    uint8_t tx[6] = { (address >> 16) & 0x3F, (address >> 8) & 0xFF, address & 0xFF, 0x00, 0x00, 0x00 };
    uint8_t rx[6] = {0};
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, rx, 6);
    cs1_high(dev);
    return (uint16_t)rx[4] | ((uint16_t)rx[5] << 8);
}

uint32_t ft81x_read32(ft81x_t *dev, uint32_t address) {
    uint8_t tx[8] = { (address >> 16) & 0x3F, (address >> 8) & 0xFF, address & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t rx[8] = {0};
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, rx, 8);
    cs1_high(dev);
    return (uint32_t)rx[4] | ((uint32_t)rx[5] << 8) | ((uint32_t)rx[6] << 16) | ((uint32_t)rx[7] << 24);
}

void ft81x_write8(ft81x_t *dev, uint32_t address, uint8_t data) {
    uint8_t tx[4] = { ((address >> 16) & 0x3F) | 0x80, (address >> 8) & 0xFF, address & 0xFF, data };
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, NULL, 4);
    cs1_high(dev);
}

void ft81x_write16(ft81x_t *dev, uint32_t address, uint16_t data) {
    uint8_t tx[5] = { ((address >> 16) & 0x3F) | 0x80, (address >> 8) & 0xFF, address & 0xFF, data & 0xFF, (data >> 8) & 0xFF };
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, NULL, 5);
    cs1_high(dev);
}

void ft81x_write32(ft81x_t *dev, uint32_t address, uint32_t data) {
    uint8_t tx[7] = { ((address >> 16) & 0x3F) | 0x80, (address >> 8) & 0xFF, address & 0xFF,
                       data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF, (data >> 24) & 0xFF };
    cs1_low(dev);
    spi_transfer(&dev->spi, tx, NULL, 7);
    cs1_high(dev);
}

/* ---- Command Buffer ---- */

void ft81x_send_command(ft81x_t *dev, uint32_t cmd) {
    ft81x_write32(dev, FT81X_RAM_CMD + dev->cmd_write_address, cmd);
    increase_cmd_write_address(dev, 4);
    ft81x_write16(dev, FT81X_REG_CMD_WRITE, dev->cmd_write_address);
}

void ft81x_begin_display_list(ft81x_t *dev) {
    ft81x_wait_for_command_buffer(dev);
    start_cmd(dev, FT81X_CMD_DLSTART);
    end_cmd(dev, DL_CLEAR(1, 1, 1));
}

void ft81x_swap_screen(ft81x_t *dev) {
    start_cmd(dev, FT81X_DL_DISPLAY);
    end_cmd(dev, FT81X_CMD_SWAP);
}

void ft81x_wait_for_command_buffer(ft81x_t *dev) {
    while (ft81x_read16(dev, FT81X_REG_CMD_WRITE) != ft81x_read16(dev, FT81X_REG_CMD_READ)) {
        usleep(100);
    }
}

/* ---- Drawing Primitives ---- */

void ft81x_clear(ft81x_t *dev, uint32_t color) {
    start_cmd(dev, DL_CLEAR_COLOR(color));
    end_cmd(dev, DL_CLEAR(1, 1, 1));
}

void ft81x_draw_circle(ft81x_t *dev, int16_t x, int16_t y, uint8_t size, uint32_t color) {
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, DL_POINT_SIZE(size * 16));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_POINTS));
    intermediate_cmd(dev, DL_VERTEX2F(x * 16, y * 16));
    end_cmd(dev, DL_END());
}

void ft81x_draw_rect(ft81x_t *dev, int16_t x, int16_t y, uint16_t w, uint16_t h,
                     uint8_t corner_radius, uint32_t color) {
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, DL_LINE_WIDTH(corner_radius * 16));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_RECTS));
    intermediate_cmd(dev, DL_VERTEX2F(x * 16, y * 16));
    intermediate_cmd(dev, DL_VERTEX2F((x + w) * 16, (y + h) * 16));
    end_cmd(dev, DL_END());
}

void ft81x_draw_line(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                     uint8_t width, uint32_t color) {
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, DL_LINE_WIDTH(width * 16));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_LINES));
    intermediate_cmd(dev, DL_VERTEX2F(x1 * 16, y1 * 16));
    intermediate_cmd(dev, DL_VERTEX2F(x2 * 16, y2 * 16));
    end_cmd(dev, DL_END());
}

void ft81x_draw_tri(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    int16_t x3, int16_t y3, uint32_t color, uint32_t bgcolor) {
    /* Sort points so point 2 (x_2, y_2) has the smallest y (highest on screen) */
    int16_t x_1, y_1, x_2, y_2, x_3, y_3;

    if ((y1 <= y2) && (y1 <= y3)) {
        x_1 = x3; y_1 = y3;
        x_2 = x1; y_2 = y1;
        x_3 = x2; y_3 = y2;
    } else if ((y2 <= y3) && (y2 <= y1)) {
        x_1 = x1; y_1 = y1;
        x_2 = x2; y_2 = y2;
        x_3 = x3; y_3 = y3;
    } else {
        x_1 = x2; y_1 = y2;
        x_2 = x3; y_2 = y3;
        x_3 = x1; y_3 = y1;
    }

    if (x_2 <= x_1) {
        start_cmd(dev, DL_COLOR(color));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_2 * 16, y_2 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_3 * 16, y_3 * 16));
        intermediate_cmd(dev, DL_COLOR(bgcolor));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_3 * 16, y_3 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_1 * 16, y_1 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_2 * 16, y_2 * 16));
    } else if (x_2 >= x_3) {
        start_cmd(dev, DL_COLOR(color));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_1 * 16, y_1 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_2 * 16, y_2 * 16));
        intermediate_cmd(dev, DL_COLOR(bgcolor));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_2 * 16, y_2 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_3 * 16, y_3 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_1 * 16, y_1 * 16));
    } else {
        start_cmd(dev, DL_COLOR(color));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_1 * 16, y_1 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_2 * 16, y_2 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_3 * 16, y_3 * 16));
        intermediate_cmd(dev, DL_COLOR(bgcolor));
        intermediate_cmd(dev, DL_LINE_WIDTH(1 * 16));
        intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_EDGE_STRIP_B));
        intermediate_cmd(dev, DL_VERTEX2F(x_3 * 16, y_3 * 16));
        intermediate_cmd(dev, DL_VERTEX2F(x_1 * 16, y_1 * 16));
    }
    end_cmd(dev, DL_END());
}

/* ---- Line Strips ---- */

void ft81x_begin_line_strip(ft81x_t *dev, uint8_t width, uint32_t color) {
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, DL_LINE_WIDTH(width * 16));
    end_cmd(dev, DL_BEGIN(FT81X_PRIM_LINE_STRIP));
}

void ft81x_add_vertex(ft81x_t *dev, int16_t x, int16_t y) {
    ft81x_send_command(dev, DL_VERTEX2F(x * 16, y * 16));
}

void ft81x_end_line_strip(ft81x_t *dev) {
    ft81x_send_command(dev, DL_END());
}

/* ---- Text ---- */

void ft81x_draw_letter(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                       uint32_t color, uint8_t letter) {
    uint8_t handle = init_bitmap_handle_for_font(dev, font);
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_BITMAPS));
    intermediate_cmd(dev, DL_VERTEX2II(x, y, handle, letter));
    end_cmd(dev, DL_END());
}

void ft81x_draw_text(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                     uint32_t color, uint16_t options, const char *text) {
    uint8_t handle = init_bitmap_handle_for_font(dev, font);
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, FT81X_CMD_LOADIDENTITY);
    intermediate_cmd(dev, FT81X_CMD_SCALE);
    intermediate_cmd(dev, 1 * 65536);
    intermediate_cmd(dev, 1 * 65536);
    intermediate_cmd(dev, FT81X_CMD_SETMATRIX);
    intermediate_cmd(dev, FT81X_CMD_TEXT);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(handle, options));
    send_text(dev, text);
}

/* ---- Bitmaps ---- */

void ft81x_write_gram(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data) {
    uint32_t addr = (FT81X_RAM_G + offset) | 0x800000;
    size_t total = 3 + size;
    uint8_t *buf = (uint8_t *)malloc(total);
    if (!buf) return;
    buf[0] = (addr >> 16) & 0xFF;
    buf[1] = (addr >> 8) & 0xFF;
    buf[2] = addr & 0xFF;
    memcpy(buf + 3, data, size);
    cs1_low(dev);
    spi_transfer(&dev->spi, buf, NULL, total);
    cs1_high(dev);
    free(buf);
}

void ft81x_load_image(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data) {
    ft81x_wait_for_command_buffer(dev);

    /* Set up MEDIAFIFO at end of RAM_G */
    start_cmd(dev, FT81X_CMD_MEDIAFIFO);
    intermediate_cmd(dev, FT81X_RAM_G + 0x100000 - size);
    end_cmd(dev, size);

    ft81x_wait_for_command_buffer(dev);

    /* Write image data to MEDIAFIFO area */
    ft81x_write_gram(dev, 0x100000 - size, size, data);

    /* Update MEDIAFIFO write pointer */
    ft81x_write32(dev, FT81X_REG_MEDIAFIFO_WRITE, size - 1);

    /* Issue LOADIMAGE command: OPT_MEDIAFIFO(16) | OPT_NODL(2) */
    start_cmd(dev, FT81X_CMD_LOADIMAGE);
    intermediate_cmd(dev, FT81X_RAM_G + offset);
    end_cmd(dev, 16 | 2);
}

void ft81x_draw_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h, uint8_t scale, uint8_t rot) {
    start_cmd(dev, DL_COLOR(0xFFFFFF));
    intermediate_cmd(dev, DL_BITMAP_SOURCE(FT81X_RAM_G + offset));
    intermediate_cmd(dev, DL_BITMAP_LAYOUT(FT81X_BITMAP_LAYOUT_RGB565, (uint32_t)w * 2, h));
    intermediate_cmd(dev, DL_BITMAP_SIZE(FT81X_BITMAP_SIZE_NEAREST, 0, 0,
                                          (uint32_t)w * scale, (uint32_t)h * scale));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_BITMAPS));
    intermediate_cmd(dev, FT81X_CMD_LOADIDENTITY);
    intermediate_cmd(dev, FT81X_CMD_SCALE);
    intermediate_cmd(dev, (uint32_t)scale * 65536);
    intermediate_cmd(dev, (uint32_t)scale * 65536);
    intermediate_cmd(dev, FT81X_CMD_TRANSLATE);
    intermediate_cmd(dev, (uint32_t)65536 * (w / 2));
    intermediate_cmd(dev, (uint32_t)65536 * (h / 2));
    intermediate_cmd(dev, FT81X_CMD_ROTATE_CMD);
    intermediate_cmd(dev, (uint32_t)rot * 65536 / 360);
    intermediate_cmd(dev, FT81X_CMD_TRANSLATE);
    intermediate_cmd(dev, (uint32_t)65536 * -(w / 2));
    intermediate_cmd(dev, (uint32_t)65536 * -(h / 2));
    intermediate_cmd(dev, FT81X_CMD_SETMATRIX);
    end_cmd(dev, DL_VERTEX2II(x, y, 0, 0));
}

void ft81x_overlay_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, uint8_t scale, uint8_t rot) {
    start_cmd(dev, DL_COLOR(0xFFFFFF));
    intermediate_cmd(dev, DL_BITMAP_SOURCE(FT81X_RAM_G + offset));
    intermediate_cmd(dev, DL_BITMAP_LAYOUT(FT81X_BITMAP_LAYOUT_RGB565, (uint32_t)w * 2, h));
    intermediate_cmd(dev, DL_BITMAP_SIZE(FT81X_BITMAP_SIZE_NEAREST, 0, 0,
                                          (uint32_t)w * scale, (uint32_t)h * scale));
    intermediate_cmd(dev, DL_BEGIN(FT81X_PRIM_BITMAPS));
    intermediate_cmd(dev, FT81X_CMD_LOADIDENTITY);
    intermediate_cmd(dev, FT81X_CMD_SCALE);
    intermediate_cmd(dev, (uint32_t)scale * 65536);
    intermediate_cmd(dev, (uint32_t)scale * 65536);
    intermediate_cmd(dev, FT81X_CMD_TRANSLATE);
    intermediate_cmd(dev, (uint32_t)65536 * (w / 2));
    intermediate_cmd(dev, (uint32_t)65536 * (h / 2));
    intermediate_cmd(dev, FT81X_CMD_ROTATE_CMD);
    intermediate_cmd(dev, (uint32_t)rot * 65536 / 360);
    intermediate_cmd(dev, FT81X_CMD_TRANSLATE);
    intermediate_cmd(dev, (uint32_t)65536 * -(w / 2));
    intermediate_cmd(dev, (uint32_t)65536 * -(h / 2));
    intermediate_cmd(dev, FT81X_CMD_SETMATRIX);
    intermediate_cmd(dev, DL_BLEND_FUNC(FT81X_BLEND_ONE, FT81X_BLEND_ONE));
    end_cmd(dev, DL_VERTEX2II(x, y, 0, 0));
}

/* ---- Widgets ---- */

void ft81x_draw_button(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                       uint8_t font, uint32_t text_color, uint32_t button_color,
                       uint16_t options, const char *text) {
    uint8_t handle = init_bitmap_handle_for_font(dev, font);
    start_cmd(dev, DL_COLOR(text_color));
    intermediate_cmd(dev, FT81X_CMD_FGCOLOR);
    intermediate_cmd(dev, button_color & 0xFFFFFF);
    intermediate_cmd(dev, FT81X_CMD_BUTTON);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(w, h));
    intermediate_cmd(dev, PACK16(handle, options));
    send_text(dev, text);
}

void ft81x_draw_clock(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint16_t hours, uint16_t minutes, uint16_t seconds) {
    start_cmd(dev, DL_COLOR(hands_color));
    intermediate_cmd(dev, FT81X_CMD_BGCOLOR);
    intermediate_cmd(dev, bg_color & 0xFFFFFF);
    intermediate_cmd(dev, FT81X_CMD_CLOCK);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(radius, options));
    intermediate_cmd(dev, PACK16(hours, minutes));
    end_cmd(dev, (uint32_t)seconds);
}

void ft81x_draw_gauge(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint8_t major, uint8_t minor, uint16_t value, uint16_t range) {
    start_cmd(dev, DL_COLOR(hands_color));
    intermediate_cmd(dev, FT81X_CMD_BGCOLOR);
    intermediate_cmd(dev, bg_color & 0xFFFFFF);
    intermediate_cmd(dev, FT81X_CMD_GAUGE);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(radius, options));
    intermediate_cmd(dev, PACK16(major, minor));
    end_cmd(dev, PACK16(value, range));
}

void ft81x_draw_gradient(ft81x_t *dev, int16_t x1, int16_t y1, uint32_t color1,
                         int16_t x2, int16_t y2, uint32_t color2) {
    start_cmd(dev, FT81X_CMD_GRADIENT);
    intermediate_cmd(dev, PACK16(x1, y1));
    intermediate_cmd(dev, color1 & 0xFFFFFF);
    intermediate_cmd(dev, PACK16(x2, y2));
    end_cmd(dev, color2 & 0xFFFFFF);
}

void ft81x_draw_scrollbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                          uint32_t fg_color, uint32_t bg_color, uint16_t options,
                          uint16_t value, uint16_t size, uint16_t range) {
    start_cmd(dev, DL_COLOR(fg_color));
    intermediate_cmd(dev, FT81X_CMD_BGCOLOR);
    intermediate_cmd(dev, bg_color & 0xFFFFFF);
    intermediate_cmd(dev, FT81X_CMD_SCROLLBAR);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(w, h));
    intermediate_cmd(dev, PACK16(options, value));
    end_cmd(dev, PACK16(size, range));
}

void ft81x_draw_progressbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                            uint32_t fg_color, uint32_t bg_color, uint16_t options,
                            uint16_t value, uint16_t range) {
    start_cmd(dev, DL_COLOR(fg_color));
    intermediate_cmd(dev, FT81X_CMD_BGCOLOR);
    intermediate_cmd(dev, bg_color & 0xFFFFFF);
    intermediate_cmd(dev, FT81X_CMD_PROGRESSBAR);
    intermediate_cmd(dev, PACK16(x, y));
    intermediate_cmd(dev, PACK16(w, h));
    intermediate_cmd(dev, PACK16(options, value));
    end_cmd(dev, (uint32_t)range);
}

void ft81x_draw_spinner(ft81x_t *dev, int16_t x, int16_t y, uint16_t style,
                        uint16_t scale, uint32_t color) {
    start_cmd(dev, DL_COLOR(color));
    intermediate_cmd(dev, FT81X_CMD_SPINNER);
    intermediate_cmd(dev, PACK16(x, y));
    end_cmd(dev, PACK16(style, scale));
}

/* ---- Display Control ---- */

void ft81x_set_rotation(ft81x_t *dev, uint8_t rotation) {
    ft81x_write8(dev, FT81X_REG_ROTATE, rotation & 0x7);
}

uint8_t ft81x_query_display(ft81x_t *dev, uint8_t cmd) {
    gpio_set(&dev->gpio, dev->gpio.dc_pin, 0);
    gpio_set(&dev->gpio, dev->gpio.cs2_pin, 0);
    spi_transfer(&dev->spi, &cmd, NULL, 1);
    uint8_t result = 0;
    spi_transfer(&dev->spi, NULL, &result, 1);
    gpio_set(&dev->gpio, dev->gpio.cs2_pin, 1);
    return result;
}

/* ---- Audio ---- */

void ft81x_play_audio(ft81x_t *dev, uint32_t offset, uint32_t size,
                      uint16_t sample_rate, uint8_t format, bool loop) {
    ft81x_write32(dev, FT81X_REG_PLAYBACK_START, FT81X_RAM_G + offset);
    ft81x_write32(dev, FT81X_REG_PLAYBACK_LENGTH, size);
    ft81x_write16(dev, FT81X_REG_PLAYBACK_FREQ, sample_rate);
    ft81x_write8(dev, FT81X_REG_PLAYBACK_FORMAT, format);
    ft81x_write8(dev, FT81X_REG_PLAYBACK_LOOP, loop ? 1 : 0);
    ft81x_write8(dev, FT81X_REG_PLAYBACK_PLAY, 1);
}

void ft81x_set_audio_volume(ft81x_t *dev, uint8_t volume) {
    ft81x_write8(dev, FT81X_REG_VOL_PB, volume);
    ft81x_write8(dev, FT81X_REG_VOL_SOUND, volume);
}

bool ft81x_is_sound_playing(ft81x_t *dev) {
    return ft81x_read8(dev, FT81X_REG_PLAY) != 0;
}

void ft81x_set_sound(ft81x_t *dev, uint8_t effect, uint8_t pitch) {
    ft81x_write16(dev, FT81X_REG_SOUND, (uint16_t)effect | ((uint16_t)pitch << 8));
}

void ft81x_play_sound(ft81x_t *dev) {
    ft81x_write8(dev, FT81X_REG_PLAY, 1);
}

void ft81x_stop_sound(ft81x_t *dev) {
    ft81x_set_sound(dev, FT81X_SOUND_SILENCE, 0);
    ft81x_play_sound(dev);
}
