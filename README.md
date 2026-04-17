# FT81x Python Driver

Python-installable library for driving FT81x (BT815/BT816) GPU from Raspberry Pi 5. A compiled C shared library handles SPI communication and display list encoding, exposed to Python via a `ctypes` wrapper.

## Requirements

- Raspberry Pi 5 (Raspberry Pi OS Bookworm 64-bit)
- Python 3.11+
- `libgpiod-dev` (`sudo apt install libgpiod-dev`)
- FT81x display with ST7701S panel controller (480x480)

## Install

```bash
pip install .
```

## Usage

```python
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
```

## License

MIT

## Reference

Ported from [FT81x Arduino Driver](https://github.com/blazer82/FT81x_Arduino_Driver).
