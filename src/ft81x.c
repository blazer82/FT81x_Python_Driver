#include "ft81x.h"
#include "ft81x_registers.h"

#include <string.h>
#include <unistd.h>

/* ---- Helpers ---- */

size_t ft81x_sizeof(void) {
    return sizeof(ft81x_t);
}

/* ---- Lifecycle ---- */

int ft81x_init(ft81x_t *dev, const char *spi_device, const char *gpio_chip,
               int cs1_pin, int cs2_pin, int dc_pin) {
    /* TODO: spi_open, gpio_open, power-up host commands, verify REG_ID == 0x7C,
       configure display timing, init ST7701S, enable PCLK, set PWM_DUTY */
    return -1;
}

void ft81x_destroy(ft81x_t *dev) {
    /* TODO: spi_close, gpio_close */
}

/* ---- Memory Access ---- */

uint8_t ft81x_read8(ft81x_t *dev, uint32_t address) {
    /* TODO: 3-byte address | 0x000000, 1 dummy byte, read 1 byte */
    return 0;
}

uint16_t ft81x_read16(ft81x_t *dev, uint32_t address) {
    /* TODO: read 2 bytes LSB first */
    return 0;
}

uint32_t ft81x_read32(ft81x_t *dev, uint32_t address) {
    /* TODO: read 4 bytes LSB first */
    return 0;
}

void ft81x_write8(ft81x_t *dev, uint32_t address, uint8_t data) {
    /* TODO: 3-byte address | 0x800000, write 1 byte */
}

void ft81x_write16(ft81x_t *dev, uint32_t address, uint16_t data) {
    /* TODO: write 2 bytes LSB first */
}

void ft81x_write32(ft81x_t *dev, uint32_t address, uint32_t data) {
    /* TODO: write 4 bytes LSB first */
}

/* ---- Command Buffer ---- */

void ft81x_send_command(ft81x_t *dev, uint32_t cmd) {
    /* TODO: write cmd to RAM_CMD + cmd_write_address, advance pointer, wrap at 4096 */
}

void ft81x_begin_display_list(ft81x_t *dev) {
    /* TODO: wait for cmd buffer, send CMD_DLSTART */
}

void ft81x_swap_screen(ft81x_t *dev) {
    /* TODO: send DL_DISPLAY, CMD_SWAP */
}

void ft81x_wait_for_command_buffer(ft81x_t *dev) {
    /* TODO: poll REG_CMD_WRITE vs REG_CMD_READ until equal */
}

/* ---- Drawing Primitives ---- */

void ft81x_clear(ft81x_t *dev, uint32_t color) {
    /* TODO: CLEAR_COLOR_RGB + CLEAR */
}

void ft81x_draw_circle(ft81x_t *dev, int16_t x, int16_t y, uint8_t size, uint32_t color) {
    /* TODO: POINT_SIZE, COLOR_RGB, BEGIN(POINTS), VERTEX2F, END */
}

void ft81x_draw_rect(ft81x_t *dev, int16_t x, int16_t y, uint16_t w, uint16_t h,
                     uint8_t corner_radius, uint32_t color) {
    /* TODO: LINE_WIDTH(corner_radius), COLOR_RGB, BEGIN(RECTS), VERTEX2F x2, END */
}

void ft81x_draw_line(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                     uint8_t width, uint32_t color) {
    /* TODO: LINE_WIDTH, COLOR_RGB, BEGIN(LINES), VERTEX2F x2, END */
}

void ft81x_draw_tri(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    int16_t x3, int16_t y3, uint32_t color, uint32_t bgcolor) {
    /* TODO: STENCIL-based triangle fill + color overlay */
}

/* ---- Line Strips ---- */

void ft81x_begin_line_strip(ft81x_t *dev, uint8_t width, uint32_t color) {
    /* TODO: LINE_WIDTH, COLOR_RGB, BEGIN(LINE_STRIP) */
}

void ft81x_add_vertex(ft81x_t *dev, int16_t x, int16_t y) {
    /* TODO: VERTEX2F with 1/16 pixel shift */
}

void ft81x_end_line_strip(ft81x_t *dev) {
    /* TODO: END */
}

/* ---- Text ---- */

void ft81x_draw_letter(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                       uint32_t color, uint8_t letter) {
    /* TODO: COLOR_RGB, VERTEX2II with font handle and cell */
}

void ft81x_draw_text(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                     uint32_t color, uint16_t options, const char *text) {
    /* TODO: COLOR_RGB, CMD_TEXT, string data padded to 4-byte boundary */
}

/* ---- Bitmaps ---- */

void ft81x_write_gram(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data) {
    /* TODO: direct write to RAM_G + offset */
}

void ft81x_load_image(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data) {
    /* TODO: CMD_MEDIAFIFO + CMD_LOADIMAGE for HW-accelerated JPEG/PNG decode */
}

void ft81x_draw_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h, uint8_t scale, uint8_t rot) {
    /* TODO: BITMAP_SOURCE, BITMAP_LAYOUT, BITMAP_SIZE, transform, VERTEX2F */
}

void ft81x_overlay_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, uint8_t scale, uint8_t rot) {
    /* TODO: same as draw_bitmap but with BLEND_FUNC for alpha blending */
}

/* ---- Widgets ---- */

void ft81x_draw_button(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                       uint8_t font, uint32_t text_color, uint32_t button_color,
                       uint16_t options, const char *text) {
    /* TODO: CMD_FGCOLOR, CMD_BGCOLOR, COLOR_RGB, CMD_BUTTON */
}

void ft81x_draw_clock(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint16_t hours, uint16_t minutes, uint16_t seconds) {
    /* TODO: CMD_FGCOLOR, CMD_BGCOLOR, COLOR_RGB, CMD_CLOCK */
}

void ft81x_draw_gauge(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint8_t major, uint8_t minor, uint16_t value, uint16_t range) {
    /* TODO: CMD_FGCOLOR, CMD_BGCOLOR, COLOR_RGB, CMD_GAUGE */
}

void ft81x_draw_gradient(ft81x_t *dev, int16_t x1, int16_t y1, uint32_t color1,
                         int16_t x2, int16_t y2, uint32_t color2) {
    /* TODO: CMD_GRADIENT */
}

void ft81x_draw_scrollbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                          uint32_t fg_color, uint32_t bg_color, uint16_t options,
                          uint16_t value, uint16_t size, uint16_t range) {
    /* TODO: CMD_FGCOLOR, CMD_BGCOLOR, CMD_SCROLLBAR */
}

void ft81x_draw_progressbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                            uint32_t fg_color, uint32_t bg_color, uint16_t options,
                            uint16_t value, uint16_t range) {
    /* TODO: CMD_FGCOLOR, CMD_BGCOLOR, CMD_PROGRESSBAR */
}

void ft81x_draw_spinner(ft81x_t *dev, int16_t x, int16_t y, uint16_t style,
                        uint16_t scale, uint32_t color) {
    /* TODO: COLOR_RGB, CMD_SPINNER */
}

/* ---- Display Control ---- */

void ft81x_set_rotation(ft81x_t *dev, uint8_t rotation) {
    /* TODO: write REG_ROTATE */
}

uint8_t ft81x_query_display(ft81x_t *dev, uint8_t cmd) {
    /* TODO: send query via CS2/DC to ST7701S, read response */
    return 0;
}

/* ---- Audio ---- */

void ft81x_play_audio(ft81x_t *dev, uint32_t offset, uint32_t size,
                      uint16_t sample_rate, uint8_t format, bool loop) {
    /* TODO: write PLAYBACK registers, set PLAY */
}

void ft81x_set_audio_volume(ft81x_t *dev, uint8_t volume) {
    /* TODO: write REG_VOL_PB */
}

bool ft81x_is_sound_playing(ft81x_t *dev) {
    /* TODO: read REG_PLAY */
    return false;
}

void ft81x_set_sound(ft81x_t *dev, uint8_t effect, uint8_t pitch) {
    /* TODO: write REG_SOUND */
}

void ft81x_play_sound(ft81x_t *dev) {
    /* TODO: write REG_PLAY = 1 */
}

void ft81x_stop_sound(ft81x_t *dev) {
    /* TODO: write REG_SOUND = SILENCE, REG_PLAY = 1 */
}
