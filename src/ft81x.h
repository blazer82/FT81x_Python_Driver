#ifndef FT81X_H
#define FT81X_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "spi_linux.h"
#include "gpio_linux.h"

typedef struct {
    spi_device_t spi;
    gpio_device_t gpio;
    uint16_t cmd_write_address;
    uint16_t width;
    uint16_t height;
} ft81x_t;

/* Returns sizeof(ft81x_t) so Python can allocate the right buffer size. */
size_t ft81x_sizeof(void);

/* Lifecycle */
int  ft81x_init(ft81x_t *dev, const char *spi_device, const char *gpio_chip,
                int cs1_pin, int cs2_pin, int dc_pin);
void ft81x_destroy(ft81x_t *dev);

/* Memory access */
uint8_t  ft81x_read8(ft81x_t *dev, uint32_t address);
uint16_t ft81x_read16(ft81x_t *dev, uint32_t address);
uint32_t ft81x_read32(ft81x_t *dev, uint32_t address);
void     ft81x_write8(ft81x_t *dev, uint32_t address, uint8_t data);
void     ft81x_write16(ft81x_t *dev, uint32_t address, uint16_t data);
void     ft81x_write32(ft81x_t *dev, uint32_t address, uint32_t data);

/* Command buffer */
void ft81x_send_command(ft81x_t *dev, uint32_t cmd);
void ft81x_begin_display_list(ft81x_t *dev);
void ft81x_swap_screen(ft81x_t *dev);
void ft81x_wait_for_command_buffer(ft81x_t *dev);

/* Drawing primitives */
void ft81x_clear(ft81x_t *dev, uint32_t color);
void ft81x_draw_circle(ft81x_t *dev, int16_t x, int16_t y, uint8_t size, uint32_t color);
void ft81x_draw_rect(ft81x_t *dev, int16_t x, int16_t y, uint16_t w, uint16_t h,
                     uint8_t corner_radius, uint32_t color);
void ft81x_draw_line(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                     uint8_t width, uint32_t color);
void ft81x_draw_tri(ft81x_t *dev, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    int16_t x3, int16_t y3, uint32_t color, uint32_t bgcolor);

/* Line strips */
void ft81x_begin_line_strip(ft81x_t *dev, uint8_t width, uint32_t color);
void ft81x_add_vertex(ft81x_t *dev, int16_t x, int16_t y);
void ft81x_end_line_strip(ft81x_t *dev);

/* Text */
void ft81x_draw_letter(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                       uint32_t color, uint8_t letter);
void ft81x_draw_text(ft81x_t *dev, int16_t x, int16_t y, uint8_t font,
                     uint32_t color, uint16_t options, const char *text);

/* Bitmaps */
void ft81x_write_gram(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data);
void ft81x_load_image(ft81x_t *dev, uint32_t offset, uint32_t size, const uint8_t *data);
void ft81x_draw_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h, uint8_t scale, uint8_t rot);
void ft81x_overlay_bitmap(ft81x_t *dev, uint32_t offset, uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, uint8_t scale, uint8_t rot);

/* Widgets */
void ft81x_draw_button(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                       uint8_t font, uint32_t text_color, uint32_t button_color,
                       uint16_t options, const char *text);
void ft81x_draw_clock(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint16_t hours, uint16_t minutes, uint16_t seconds);
void ft81x_draw_gauge(ft81x_t *dev, int16_t x, int16_t y, int16_t radius,
                      uint32_t hands_color, uint32_t bg_color, uint16_t options,
                      uint8_t major, uint8_t minor, uint16_t value, uint16_t range);
void ft81x_draw_gradient(ft81x_t *dev, int16_t x1, int16_t y1, uint32_t color1,
                         int16_t x2, int16_t y2, uint32_t color2);
void ft81x_draw_scrollbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                          uint32_t fg_color, uint32_t bg_color, uint16_t options,
                          uint16_t value, uint16_t size, uint16_t range);
void ft81x_draw_progressbar(ft81x_t *dev, int16_t x, int16_t y, int16_t w, int16_t h,
                            uint32_t fg_color, uint32_t bg_color, uint16_t options,
                            uint16_t value, uint16_t range);
void ft81x_draw_spinner(ft81x_t *dev, int16_t x, int16_t y, uint16_t style,
                        uint16_t scale, uint32_t color);

/* Display control */
void    ft81x_set_rotation(ft81x_t *dev, uint8_t rotation);
uint8_t ft81x_query_display(ft81x_t *dev, uint8_t cmd);

/* Audio */
void ft81x_play_audio(ft81x_t *dev, uint32_t offset, uint32_t size,
                      uint16_t sample_rate, uint8_t format, bool loop);
void ft81x_set_audio_volume(ft81x_t *dev, uint8_t volume);
bool ft81x_is_sound_playing(ft81x_t *dev);
void ft81x_set_sound(ft81x_t *dev, uint8_t effect, uint8_t pitch);
void ft81x_play_sound(ft81x_t *dev);
void ft81x_stop_sound(ft81x_t *dev);

#endif
