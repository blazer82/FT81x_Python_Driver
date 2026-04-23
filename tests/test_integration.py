"""Integration tests — require hardware (FT81x + Raspberry Pi 5)."""

import ctypes
import pathlib
import pytest

import ft81x as _ft81x_mod
from ft81x import FT81x
from ft81x.constants import REG_ID

pytestmark = pytest.mark.skipif(
    not pathlib.Path("/dev/spidev0.0").exists()
    or not isinstance(_ft81x_mod._lib, ctypes.CDLL),
    reason="Hardware tests require Raspberry Pi 5 with FT81x connected and compiled C library (pip install .)",
)


class TestHardwareInit:
    def test_chip_id(self):
        """Verify REG_ID reads 0x7C."""
        with FT81x() as dev:
            chip_id = dev.read8(REG_ID)
            assert chip_id == 0x7C

    def test_display_init(self):
        """Verify display initializes without error."""
        with FT81x() as dev:
            pass


class TestDrawing:
    def test_clear_screen(self):
        """Clear screen to a solid color."""
        with FT81x() as dev:
            dev.begin_display_list()
            dev.clear(0x0000FF)
            dev.swap()

    def test_draw_text(self):
        """Draw text on screen."""
        with FT81x() as dev:
            dev.begin_display_list()
            dev.clear(0x000000)
            dev.draw_text(240, 240, font=28, color=0xFFFFFF,
                          options=FT81x.OPT_CENTER, text="Test OK")
            dev.swap()
