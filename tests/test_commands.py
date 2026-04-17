"""Unit tests for FT81x command encoding (mock SPI)."""

import pytest


class TestCommandEncoding:
    def test_clear_command(self):
        """Verify clear() produces correct display list bytes."""
        pytest.skip("Not implemented yet")

    def test_draw_circle_coordinates(self):
        """Verify 1/16-pixel coordinate shift."""
        pytest.skip("Not implemented yet")

    def test_draw_text_options(self):
        """Verify text option flags are passed correctly."""
        pytest.skip("Not implemented yet")

    def test_command_buffer_wrap(self):
        """Verify circular buffer wraps at 4096."""
        pytest.skip("Not implemented yet")


class TestPythonWrapper:
    def test_context_manager(self):
        """Verify __enter__/__exit__ work correctly."""
        pytest.skip("Not implemented yet")

    def test_color_format(self):
        """Verify 0xRRGGBB integer colors are handled."""
        pytest.skip("Not implemented yet")
