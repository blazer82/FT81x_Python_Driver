"""Minimal FT81x example: clear screen, draw text, draw circle."""

from ft81x import FT81x

with FT81x(
    spi_device="/dev/spidev0.0",
    gpio_chip="/dev/gpiochip4",
    cs1_pin=8,
    cs2_pin=7,
    dc_pin=25,
) as display:
    display.begin_display_list()
    display.clear(0x000000)
    display.draw_circle(240, 240, 50, color=0xFFFFFF)
    display.draw_text(
        240, 400, font=28, color=0xFFFFFF,
        options=FT81x.OPT_CENTER, text="Hallo!",
    )
    display.swap()
