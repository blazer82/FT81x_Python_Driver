"""Integration tests — require hardware (FT81x + Raspberry Pi 5)."""

import pathlib
import pytest

pytestmark = pytest.mark.skipif(
    not pathlib.Path("/dev/spidev0.0").exists(),
    reason="No SPI device — hardware tests require Raspberry Pi 5 with FT81x connected",
)


class TestHardwareInit:
    def test_chip_id(self):
        """Verify REG_ID reads 0x7C."""
        pytest.skip("Not implemented yet")

    def test_display_init(self):
        """Verify display initializes without error."""
        pytest.skip("Not implemented yet")


class TestDrawing:
    def test_clear_screen(self):
        """Clear screen to a solid color."""
        pytest.skip("Not implemented yet")

    def test_draw_text(self):
        """Draw text on screen."""
        pytest.skip("Not implemented yet")
