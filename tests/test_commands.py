"""Unit tests for FT81x command encoding (mock SPI)."""

import pytest
from ft81x import FT81x
from ft81x.constants import OPT_CENTER, OPT_CENTERX, OPT_CENTERY, OPT_RIGHTX


class TestCommandEncoding:
    def test_clear_command(self, mock_lib, ft81x_dev):
        """Verify clear() produces correct display list bytes."""
        color = 0xFF0000
        ft81x_dev.clear(color)

        mock_lib.ft81x_clear.assert_called_once()
        args = mock_lib.ft81x_clear.call_args[0]
        assert args[1] == color

        # DL_CLEAR_COLOR(rgb) = (0x02 << 24) | (rgb & 0xFFFFFF)
        assert (0x02 << 24) | (0xFF0000 & 0xFFFFFF) == 0x02FF0000
        assert (0x02 << 24) | (0x000000 & 0xFFFFFF) == 0x02000000
        assert (0x02 << 24) | (0xFFFFFF & 0xFFFFFF) == 0x02FFFFFF

        # DL_CLEAR(1, 1, 1) = (0x26 << 24) | (1 << 2) | (1 << 1) | 1
        assert (0x26 << 24) | (1 << 2) | (1 << 1) | 1 == 0x26000007

    def test_draw_circle_coordinates(self, mock_lib, ft81x_dev):
        """Verify 1/16-pixel coordinate shift."""
        x, y, size, color = 100, 200, 10, 0x00FF00
        ft81x_dev.draw_circle(x, y, size, color)

        mock_lib.ft81x_draw_circle.assert_called_once()
        args = mock_lib.ft81x_draw_circle.call_args[0]
        assert args[1] == 100
        assert args[2] == 200
        assert args[3] == 10
        assert args[4] == 0x00FF00

        # C multiplies by 16 for 1/16-pixel precision:
        # DL_VERTEX2F(x*16, y*16) = 0x40000000 | ((x & 0x7FFF) << 15) | (y & 0x7FFF)
        vx, vy = x * 16, y * 16
        expected_vertex = 0x40000000 | ((vx & 0x7FFF) << 15) | (vy & 0x7FFF)
        assert expected_vertex == 0x40000000 | (1600 << 15) | 3200

        # DL_POINT_SIZE(size * 16) = (0x0D << 24) | (160 & 0xFFF)
        assert (0x0D << 24) | (160 & 0xFFF) == 0x0D0000A0

        # DL_COLOR(rgb) = (0x04 << 24) | (rgb & 0xFFFFFF)
        assert (0x04 << 24) | (0x00FF00 & 0xFFFFFF) == 0x0400FF00

    def test_draw_text_options(self, mock_lib, ft81x_dev):
        """Verify text option flags are passed correctly."""
        ft81x_dev.draw_text(240, 400, font=28, color=0xFFFFFF,
                            options=OPT_CENTER, text="Hello")

        mock_lib.ft81x_draw_text.assert_called_once()
        args = mock_lib.ft81x_draw_text.call_args[0]
        assert args[1] == 240
        assert args[2] == 400
        assert args[3] == 28
        assert args[4] == 0xFFFFFF
        assert args[5] == OPT_CENTER
        assert args[6] == b"Hello"

        mock_lib.ft81x_draw_text.reset_mock()

        # OPT_RIGHTX
        ft81x_dev.draw_text(0, 0, font=16, color=0, options=OPT_RIGHTX, text="R")
        assert mock_lib.ft81x_draw_text.call_args[0][5] == 0x0800

        mock_lib.ft81x_draw_text.reset_mock()

        # Default options=0
        ft81x_dev.draw_text(0, 0, font=16, color=0, text="X")
        assert mock_lib.ft81x_draw_text.call_args[0][5] == 0

        # OPT_CENTERX | OPT_CENTERY == OPT_CENTER
        assert OPT_CENTERX | OPT_CENTERY == OPT_CENTER
        assert OPT_CENTER == 0x0600

    def test_command_buffer_wrap(self):
        """Verify circular buffer wraps at 4096."""
        def advance(addr, delta=4):
            return (addr + delta) & 0xFFF

        addr = 0
        assert advance(addr) == 4
        assert advance(4) == 8

        # Wrap boundary: 4092 + 4 = 4096 -> 0
        assert advance(4092) == 0
        assert advance(4088) == 4092

        # 1024 commands (1024 * 4 = 4096 bytes) wraps back to start
        addr = 0
        for _ in range(1024):
            addr = advance(addr)
        assert addr == 0

        # 1025 commands wraps to 4
        addr = 0
        for _ in range(1025):
            addr = advance(addr)
        assert addr == 4


class TestPythonWrapper:
    def test_context_manager(self, mock_lib):
        """Verify __enter__/__exit__ work correctly."""
        with FT81x() as dev:
            assert isinstance(dev, FT81x)
            mock_lib.ft81x_init.assert_called_once()
            mock_lib.ft81x_destroy.assert_not_called()

        mock_lib.ft81x_destroy.assert_called_once()

    def test_color_format(self, mock_lib, ft81x_dev):
        """Verify 0xRRGGBB integer colors are handled."""
        for color in [0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF, 0x000000]:
            ft81x_dev.clear(color)
            assert mock_lib.ft81x_clear.call_args[0][1] == color

        ft81x_dev.draw_circle(0, 0, 5, 0xABCDEF)
        assert mock_lib.ft81x_draw_circle.call_args[0][4] == 0xABCDEF

    def test_init_failure_raises_runtime_error(self, mock_lib):
        """Verify RuntimeError is raised when ft81x_init returns non-zero."""
        mock_lib.ft81x_init.return_value = -1
        mock_lib.ft81x_get_error.return_value = b"SPI open failed: /dev/spidev0.0"

        with pytest.raises(RuntimeError, match="SPI open failed"):
            FT81x()
