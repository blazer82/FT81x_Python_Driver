# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

FT81x GPU driver for Raspberry Pi 5 — a Python/C library controlling BT815/BT816 GPUs with ST7701S 480x480 displays over SPI. Ported from an Arduino driver.

## Build & Test

Requires system packages: `libgpiod-dev`, `cmake`, `build-essential`, `python3-pip`.

```bash
pip install .              # build (compiles C via scikit-build-core + CMake)
pip install -e ".[dev]"    # editable install with pytest
pytest tests/              # run tests (unit tests mock SPI; integration tests skip without /dev/spidev0.0)
```

Build system: scikit-build-core orchestrates CMake, which compiles C sources into `libft81x.so` linked against libgpiod.

## Architecture

Three layers:

1. **Platform abstraction (C)** — `src/spi_linux.c` and `src/gpio_linux.c` wrap Linux `/dev/spidevX.Y` (ioctl) and libgpiod v2 respectively. These replace Arduino's SPI/GPIO APIs.

2. **FT81x core (C)** — `src/ft81x.c` is the main driver ported from Arduino. Handles initialization (power-up sequence, chip ID 0x7C verification, ST7701S panel init, display timing for 480x480), SPI address protocol (24-bit address with read/write flag 0x800000), 4096-byte circular command buffer at RAM_CMD, and all drawing/widget/audio operations. Register constants live in `src/ft81x_registers.h`.

3. **Python wrapper** — `ft81x/__init__.py` loads `libft81x.so` via ctypes and exposes an `FT81x` class with context manager support. `ft81x/constants.py` mirrors C-side register addresses, options, bitmap formats, and sound effects.

## Key Implementation Details

- **SPI address protocol**: reads include a dummy byte; writes set bit 23 (0x800000 flag)
- **Coordinate system**: 1/16-pixel precision (values left-shifted by 4 in C)
- **Command buffer**: atomic multi-command writes use start_cmd/intermediate_cmd/end_cmd pattern within a single CS assertion
- **ST7701S init**: hardcoded 480x480 panel initialization sequence with GPIO3 reset pulse
