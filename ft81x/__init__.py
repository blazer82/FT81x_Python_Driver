"""FT81x GPU driver for Raspberry Pi 5."""

import ctypes
from pathlib import Path

from ft81x.constants import (
    OPT_3D, OPT_FLAT, OPT_CENTERX, OPT_CENTERY, OPT_CENTER, OPT_RIGHTX,
    OPT_NOBACK, OPT_NOTICKS, OPT_NOHM, OPT_NOPOINTER, OPT_NOSECS, OPT_NOHANDS,
    ROTATE_LANDSCAPE, ROTATE_LANDSCAPE_INVERTED,
    ROTATE_PORTRAIT, ROTATE_PORTRAIT_INVERTED,
    ROTATE_LANDSCAPE_MIRRORED, ROTATE_LANDSCAPE_INVERTED_MIRRORED,
    ROTATE_PORTRAIT_MIRRORED, ROTATE_PORTRAIT_INVERTED_MIRRORED,
    BITMAP_LAYOUT_ARGB1555, BITMAP_LAYOUT_ARGB4, BITMAP_LAYOUT_RGB565,
    BITMAP_SIZE_NEAREST, BITMAP_SIZE_BILINEAR,
    AUDIO_FORMAT_LINEAR, AUDIO_FORMAT_ULAW, AUDIO_FORMAT_ADPCM,
)

_lib_path = Path(__file__).parent / "libft81x.so"
_lib = ctypes.CDLL(str(_lib_path))

_lib.ft81x_sizeof.argtypes = []
_lib.ft81x_sizeof.restype = ctypes.c_size_t

_lib.ft81x_init.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p,
                             ctypes.c_int, ctypes.c_int, ctypes.c_int]
_lib.ft81x_init.restype = ctypes.c_int

_lib.ft81x_destroy.argtypes = [ctypes.c_void_p]
_lib.ft81x_destroy.restype = None

_lib.ft81x_get_error.argtypes = [ctypes.c_void_p]
_lib.ft81x_get_error.restype = ctypes.c_char_p

_lib.ft81x_read8.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.ft81x_read8.restype = ctypes.c_uint8
_lib.ft81x_read16.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.ft81x_read16.restype = ctypes.c_uint16
_lib.ft81x_read32.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.ft81x_read32.restype = ctypes.c_uint32
_lib.ft81x_write8.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint8]
_lib.ft81x_write8.restype = None
_lib.ft81x_write16.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16]
_lib.ft81x_write16.restype = None
_lib.ft81x_write32.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32]
_lib.ft81x_write32.restype = None

_lib.ft81x_send_command.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.ft81x_send_command.restype = None
_lib.ft81x_begin_display_list.argtypes = [ctypes.c_void_p]
_lib.ft81x_begin_display_list.restype = None
_lib.ft81x_swap_screen.argtypes = [ctypes.c_void_p]
_lib.ft81x_swap_screen.restype = None
_lib.ft81x_wait_for_command_buffer.argtypes = [ctypes.c_void_p]
_lib.ft81x_wait_for_command_buffer.restype = None

_lib.ft81x_clear.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
_lib.ft81x_clear.restype = None
_lib.ft81x_draw_circle.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                    ctypes.c_uint8, ctypes.c_uint32]
_lib.ft81x_draw_circle.restype = None
_lib.ft81x_draw_rect.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                  ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint8,
                                  ctypes.c_uint32]
_lib.ft81x_draw_rect.restype = None
_lib.ft81x_draw_line.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                  ctypes.c_int16, ctypes.c_int16, ctypes.c_uint8,
                                  ctypes.c_uint32]
_lib.ft81x_draw_line.restype = None
_lib.ft81x_draw_tri.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                 ctypes.c_int16, ctypes.c_int16, ctypes.c_int16,
                                 ctypes.c_int16, ctypes.c_uint32, ctypes.c_uint32]
_lib.ft81x_draw_tri.restype = None

_lib.ft81x_begin_line_strip.argtypes = [ctypes.c_void_p, ctypes.c_uint8, ctypes.c_uint32]
_lib.ft81x_begin_line_strip.restype = None
_lib.ft81x_add_vertex.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16]
_lib.ft81x_add_vertex.restype = None
_lib.ft81x_end_line_strip.argtypes = [ctypes.c_void_p]
_lib.ft81x_end_line_strip.restype = None

_lib.ft81x_draw_letter.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                    ctypes.c_uint8, ctypes.c_uint32, ctypes.c_uint8]
_lib.ft81x_draw_letter.restype = None
_lib.ft81x_draw_text.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                  ctypes.c_uint8, ctypes.c_uint32, ctypes.c_uint16,
                                  ctypes.c_char_p]
_lib.ft81x_draw_text.restype = None

_lib.ft81x_write_gram.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
                                   ctypes.c_void_p]
_lib.ft81x_write_gram.restype = None
_lib.ft81x_load_image.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
                                   ctypes.c_void_p]
_lib.ft81x_load_image.restype = None
_lib.ft81x_draw_bitmap.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16,
                                    ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint16,
                                    ctypes.c_uint8, ctypes.c_uint8]
_lib.ft81x_draw_bitmap.restype = None
_lib.ft81x_overlay_bitmap.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16,
                                       ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint16,
                                       ctypes.c_uint8, ctypes.c_uint8]
_lib.ft81x_overlay_bitmap.restype = None

_lib.ft81x_draw_button.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                    ctypes.c_int16, ctypes.c_int16, ctypes.c_uint8,
                                    ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint16,
                                    ctypes.c_char_p]
_lib.ft81x_draw_button.restype = None
_lib.ft81x_draw_clock.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                   ctypes.c_int16, ctypes.c_uint32, ctypes.c_uint32,
                                   ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint16,
                                   ctypes.c_uint16]
_lib.ft81x_draw_clock.restype = None
_lib.ft81x_draw_gauge.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                   ctypes.c_int16, ctypes.c_uint32, ctypes.c_uint32,
                                   ctypes.c_uint16, ctypes.c_uint8, ctypes.c_uint8,
                                   ctypes.c_uint16, ctypes.c_uint16]
_lib.ft81x_draw_gauge.restype = None
_lib.ft81x_draw_gradient.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                      ctypes.c_uint32, ctypes.c_int16, ctypes.c_int16,
                                      ctypes.c_uint32]
_lib.ft81x_draw_gradient.restype = None
_lib.ft81x_draw_scrollbar.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                       ctypes.c_int16, ctypes.c_int16, ctypes.c_uint32,
                                       ctypes.c_uint32, ctypes.c_uint16, ctypes.c_uint16,
                                       ctypes.c_uint16, ctypes.c_uint16]
_lib.ft81x_draw_scrollbar.restype = None
_lib.ft81x_draw_progressbar.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                         ctypes.c_int16, ctypes.c_int16, ctypes.c_uint32,
                                         ctypes.c_uint32, ctypes.c_uint16, ctypes.c_uint16,
                                         ctypes.c_uint16]
_lib.ft81x_draw_progressbar.restype = None
_lib.ft81x_draw_spinner.argtypes = [ctypes.c_void_p, ctypes.c_int16, ctypes.c_int16,
                                     ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint32]
_lib.ft81x_draw_spinner.restype = None

_lib.ft81x_set_rotation.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
_lib.ft81x_set_rotation.restype = None
_lib.ft81x_query_display.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
_lib.ft81x_query_display.restype = ctypes.c_uint8

_lib.ft81x_play_audio.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
                                   ctypes.c_uint16, ctypes.c_uint8, ctypes.c_bool]
_lib.ft81x_play_audio.restype = None
_lib.ft81x_set_audio_volume.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
_lib.ft81x_set_audio_volume.restype = None
_lib.ft81x_is_sound_playing.argtypes = [ctypes.c_void_p]
_lib.ft81x_is_sound_playing.restype = ctypes.c_bool
_lib.ft81x_set_sound.argtypes = [ctypes.c_void_p, ctypes.c_uint8, ctypes.c_uint8]
_lib.ft81x_set_sound.restype = None
_lib.ft81x_play_sound.argtypes = [ctypes.c_void_p]
_lib.ft81x_play_sound.restype = None
_lib.ft81x_stop_sound.argtypes = [ctypes.c_void_p]
_lib.ft81x_stop_sound.restype = None


class FT81x:
    """FT81x display driver."""

    OPT_3D = OPT_3D
    OPT_FLAT = OPT_FLAT
    OPT_CENTERX = OPT_CENTERX
    OPT_CENTERY = OPT_CENTERY
    OPT_CENTER = OPT_CENTER
    OPT_RIGHTX = OPT_RIGHTX
    OPT_NOBACK = OPT_NOBACK
    OPT_NOTICKS = OPT_NOTICKS
    OPT_NOHM = OPT_NOHM
    OPT_NOPOINTER = OPT_NOPOINTER
    OPT_NOSECS = OPT_NOSECS
    OPT_NOHANDS = OPT_NOHANDS

    ROTATE_LANDSCAPE = ROTATE_LANDSCAPE
    ROTATE_LANDSCAPE_INVERTED = ROTATE_LANDSCAPE_INVERTED
    ROTATE_PORTRAIT = ROTATE_PORTRAIT
    ROTATE_PORTRAIT_INVERTED = ROTATE_PORTRAIT_INVERTED
    ROTATE_LANDSCAPE_MIRRORED = ROTATE_LANDSCAPE_MIRRORED
    ROTATE_LANDSCAPE_INVERTED_MIRRORED = ROTATE_LANDSCAPE_INVERTED_MIRRORED
    ROTATE_PORTRAIT_MIRRORED = ROTATE_PORTRAIT_MIRRORED
    ROTATE_PORTRAIT_INVERTED_MIRRORED = ROTATE_PORTRAIT_INVERTED_MIRRORED

    BITMAP_LAYOUT_ARGB1555 = BITMAP_LAYOUT_ARGB1555
    BITMAP_LAYOUT_ARGB4 = BITMAP_LAYOUT_ARGB4
    BITMAP_LAYOUT_RGB565 = BITMAP_LAYOUT_RGB565
    BITMAP_SIZE_NEAREST = BITMAP_SIZE_NEAREST
    BITMAP_SIZE_BILINEAR = BITMAP_SIZE_BILINEAR

    AUDIO_FORMAT_LINEAR = AUDIO_FORMAT_LINEAR
    AUDIO_FORMAT_ULAW = AUDIO_FORMAT_ULAW
    AUDIO_FORMAT_ADPCM = AUDIO_FORMAT_ADPCM

    def __init__(self, spi_device="/dev/spidev0.0", gpio_chip="/dev/gpiochip4",
                 cs1_pin=8, cs2_pin=7, dc_pin=25):
        self._dev = ctypes.create_string_buffer(_lib.ft81x_sizeof())
        rc = _lib.ft81x_init(
            self._dev, spi_device.encode(), gpio_chip.encode(),
            cs1_pin, cs2_pin, dc_pin,
        )
        if rc != 0:
            err = _lib.ft81x_get_error(self._dev)
            msg = err.decode() if err else "Failed to initialize FT81x"
            raise RuntimeError(msg)

    def close(self):
        _lib.ft81x_destroy(self._dev)

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    # ---- Memory Access ----

    def read8(self, address):
        return _lib.ft81x_read8(self._dev, address)

    def read16(self, address):
        return _lib.ft81x_read16(self._dev, address)

    def read32(self, address):
        return _lib.ft81x_read32(self._dev, address)

    def write8(self, address, data):
        _lib.ft81x_write8(self._dev, address, data)

    def write16(self, address, data):
        _lib.ft81x_write16(self._dev, address, data)

    def write32(self, address, data):
        _lib.ft81x_write32(self._dev, address, data)

    # ---- Command Buffer ----

    def send_command(self, cmd):
        _lib.ft81x_send_command(self._dev, cmd)

    def begin_display_list(self):
        _lib.ft81x_begin_display_list(self._dev)

    def swap(self):
        _lib.ft81x_swap_screen(self._dev)

    def wait_for_command_buffer(self):
        _lib.ft81x_wait_for_command_buffer(self._dev)

    # ---- Drawing Primitives ----

    def clear(self, color):
        _lib.ft81x_clear(self._dev, color)

    def draw_circle(self, x, y, size, color):
        _lib.ft81x_draw_circle(self._dev, x, y, size, color)

    def draw_rect(self, x, y, w, h, corner_radius=0, color=0xFFFFFF):
        _lib.ft81x_draw_rect(self._dev, x, y, w, h, corner_radius, color)

    def draw_line(self, x1, y1, x2, y2, width=1, color=0xFFFFFF):
        _lib.ft81x_draw_line(self._dev, x1, y1, x2, y2, width, color)

    def draw_tri(self, x1, y1, x2, y2, x3, y3, color, bgcolor=0x000000):
        _lib.ft81x_draw_tri(self._dev, x1, y1, x2, y2, x3, y3, color, bgcolor)

    # ---- Line Strips ----

    def begin_line_strip(self, width=1, color=0xFFFFFF):
        _lib.ft81x_begin_line_strip(self._dev, width, color)

    def add_vertex(self, x, y):
        _lib.ft81x_add_vertex(self._dev, x, y)

    def end_line_strip(self):
        _lib.ft81x_end_line_strip(self._dev)

    # ---- Text ----

    def draw_letter(self, x, y, font, color, letter):
        _lib.ft81x_draw_letter(self._dev, x, y, font, color, letter)

    def draw_text(self, x, y, font, color, options=0, text=""):
        _lib.ft81x_draw_text(self._dev, x, y, font, color, options, text.encode())

    # ---- Bitmaps ----

    def write_gram(self, offset, data):
        _lib.ft81x_write_gram(self._dev, offset, len(data), data)

    def load_image(self, offset, data):
        _lib.ft81x_load_image(self._dev, offset, len(data), data)

    def draw_bitmap(self, offset, x, y, w, h, scale=1, rot=0):
        _lib.ft81x_draw_bitmap(self._dev, offset, x, y, w, h, scale, rot)

    def overlay_bitmap(self, offset, x, y, w, h, scale=1, rot=0):
        _lib.ft81x_overlay_bitmap(self._dev, offset, x, y, w, h, scale, rot)

    # ---- Widgets ----

    def draw_button(self, x, y, w, h, font, text_color, button_color, options=0, text=""):
        _lib.ft81x_draw_button(self._dev, x, y, w, h, font, text_color, button_color,
                               options, text.encode())

    def draw_clock(self, x, y, radius, hands_color, bg_color, options=0,
                   hours=0, minutes=0, seconds=0):
        _lib.ft81x_draw_clock(self._dev, x, y, radius, hands_color, bg_color,
                              options, hours, minutes, seconds)

    def draw_gauge(self, x, y, radius, hands_color, bg_color, options=0,
                   major=0, minor=0, value=0, range_=100):
        _lib.ft81x_draw_gauge(self._dev, x, y, radius, hands_color, bg_color,
                              options, major, minor, value, range_)

    def draw_gradient(self, x1, y1, color1, x2, y2, color2):
        _lib.ft81x_draw_gradient(self._dev, x1, y1, color1, x2, y2, color2)

    def draw_scrollbar(self, x, y, w, h, fg_color, bg_color, options=0,
                       value=0, size=0, range_=100):
        _lib.ft81x_draw_scrollbar(self._dev, x, y, w, h, fg_color, bg_color,
                                  options, value, size, range_)

    def draw_progressbar(self, x, y, w, h, fg_color, bg_color, options=0,
                         value=0, range_=100):
        _lib.ft81x_draw_progressbar(self._dev, x, y, w, h, fg_color, bg_color,
                                    options, value, range_)

    def draw_spinner(self, x, y, style=0, scale=1, color=0xFFFFFF):
        _lib.ft81x_draw_spinner(self._dev, x, y, style, scale, color)

    # ---- Display Control ----

    def set_rotation(self, rotation):
        _lib.ft81x_set_rotation(self._dev, rotation)

    def query_display(self, cmd):
        return _lib.ft81x_query_display(self._dev, cmd)

    # ---- Audio ----

    def play_audio(self, offset, size, sample_rate, format_=AUDIO_FORMAT_LINEAR, loop=False):
        _lib.ft81x_play_audio(self._dev, offset, size, sample_rate, format_, loop)

    def set_audio_volume(self, volume):
        _lib.ft81x_set_audio_volume(self._dev, volume)

    def is_sound_playing(self):
        return _lib.ft81x_is_sound_playing(self._dev)

    def set_sound(self, effect, pitch):
        _lib.ft81x_set_sound(self._dev, effect, pitch)

    def play_sound(self):
        _lib.ft81x_play_sound(self._dev)

    def stop_sound(self):
        _lib.ft81x_stop_sound(self._dev)
