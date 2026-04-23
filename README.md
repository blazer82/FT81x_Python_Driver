# FT81x Python Driver

Python driver for FT81x-series GPUs (BT815/BT816) on Raspberry Pi 5. A compiled C shared library handles SPI communication and display-list encoding, exposed to Python via `ctypes`.

## Requirements

- Raspberry Pi 5 running Raspberry Pi OS Bookworm (64-bit)
- Python 3.11+
- FT81x display with ST7701S panel controller (480 x 480)

### Hardware wiring (default pins)

| Signal        | RPi 5 pin      | FT81x pin |
| ------------- | -------------- | --------- |
| SPI MOSI      | GPIO 10 (SPI0) | MOSI      |
| SPI MISO      | GPIO 9 (SPI0)  | MISO      |
| SPI SCLK      | GPIO 11 (SPI0) | SCLK      |
| CS1 (FT81x)   | GPIO 8         | CS        |
| CS2 (ST7701S) | GPIO 7         | CS        |
| DC            | GPIO 25        | DC        |

### Raspberry Pi configuration

Add the following to `/boot/firmware/config.txt` and reboot:

```ini
dtparam=spi=on
dtoverlay=spi0-0cs
```

`spi0-0cs` prevents the kernel SPI driver from claiming GPIO 7 and 8 as hardware chip-selects, freeing them for the driver to manage via libgpiod.

Add your user to the required groups:

```bash
sudo usermod -aG spi,gpio $USER
```

Log out and back in for the group change to take effect.

## Installation

```bash
sudo apt update
sudo apt install libgpiod-dev cmake build-essential python3-pip
```

Then install the driver in a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install .
```

For development (includes pytest):

```bash
pip install -e ".[dev]"
```

**Note:** Run scripts from outside the repository directory (or ensure the local `ft81x/` source folder does not shadow the installed package). The compiled C library is only present in the installed copy under `site-packages/`.

## Testing

```bash
pytest tests/
```

- **`tests/test_commands.py`** — unit tests for command encoding (no hardware required, but the C extension must be built).
- **`tests/test_integration.py`** — hardware tests that run only when `/dev/spidev0.0` is present. These are automatically skipped on machines without SPI.

## Usage

### Basic example

```python
from ft81x import FT81x

with FT81x() as display:
    display.begin_display_list()
    display.clear(0x000000)
    display.draw_circle(240, 240, 50, color=0xFFFFFF)
    display.draw_text(
        240, 400, font=28, color=0xFFFFFF,
        options=FT81x.OPT_CENTER, text="Hello!",
    )
    display.swap()
```

### Custom pin configuration

```python
display = FT81x(
    spi_device="/dev/spidev0.0",
    gpio_chip="/dev/gpiochip4",
    cs1_pin=8,
    cs2_pin=7,
    dc_pin=25,
)
```

All parameters shown are the defaults.

### Drawing primitives

```python
display.clear(0x1A1A2E)                               # fill background
display.draw_circle(100, 100, 30, color=0xFF0000)      # circle
display.draw_rect(50, 50, 200, 100, corner_radius=8,
                  color=0x00FF00)                       # rounded rectangle
display.draw_line(0, 0, 480, 480, width=3,
                  color=0x0000FF)                       # line
display.draw_tri(240, 50, 140, 200, 340, 200,
                 color=0xFFFF00, bgcolor=0x000000)      # triangle
```

### Line strips

```python
display.begin_line_strip(width=2, color=0xFF00FF)
display.add_vertex(10, 10)
display.add_vertex(100, 200)
display.add_vertex(200, 50)
display.end_line_strip()
```

### Text

```python
display.draw_letter(100, 100, font=31, color=0xFFFFFF, letter=ord('A'))
display.draw_text(240, 240, font=28, color=0xFFFFFF,
                  options=FT81x.OPT_CENTER, text="Centered text")
```

Built-in fonts range from 16 to 34.

### Widgets

```python
display.draw_button(100, 100, 160, 50, font=28,
                    text_color=0xFFFFFF, button_color=0x333333,
                    text="Press me")
display.draw_clock(240, 240, radius=100,
                   hands_color=0xFFFFFF, bg_color=0x000033,
                   hours=10, minutes=30, seconds=0)
display.draw_gauge(240, 240, radius=80,
                   hands_color=0xFF0000, bg_color=0x222222,
                   major=5, minor=4, value=75, range_=100)
display.draw_gradient(0, 0, 0xFF0000, 0, 480, 0x0000FF)
display.draw_progressbar(50, 400, 380, 20,
                         fg_color=0x00FF00, bg_color=0x333333,
                         value=65, range_=100)
display.draw_spinner(240, 240, style=0, scale=1, color=0xFFFFFF)
```

### Bitmaps

```python
with open("image.raw", "rb") as f:
    data = f.read()

display.write_gram(0, data)
display.draw_bitmap(offset=0, x=100, y=100, w=64, h=64, scale=1, rot=0)
display.overlay_bitmap(offset=0, x=200, y=200, w=64, h=64)
```

`load_image` can be used for JPEG/PNG data that the FT81x decodes on-chip:

```python
with open("photo.jpg", "rb") as f:
    display.load_image(0, f.read())
```

### Display rotation

```python
display.set_rotation(FT81x.ROTATE_PORTRAIT)
```

Available rotations: `ROTATE_LANDSCAPE`, `ROTATE_LANDSCAPE_INVERTED`, `ROTATE_PORTRAIT`, `ROTATE_PORTRAIT_INVERTED`, and their `_MIRRORED` variants.

### Audio

```python
with open("sample.raw", "rb") as f:
    data = f.read()

display.write_gram(0, data)
display.set_audio_volume(128)
display.play_audio(offset=0, size=len(data), sample_rate=8000,
                   format_=FT81x.AUDIO_FORMAT_LINEAR, loop=False)
```

Built-in sound effects:

```python
from ft81x.constants import SOUND_CLICK

display.set_sound(SOUND_CLICK, 60)
display.play_sound()
```

### Low-level access

```python
from ft81x.constants import REG_ID

chip_id = display.read8(REG_ID)   # should return 0x7C
display.write32(address, value)
display.send_command(cmd)
```

## License

MIT

## Reference

Ported from [FT81x Arduino Driver](https://github.com/blazer82/FT81x_Arduino_Driver).
