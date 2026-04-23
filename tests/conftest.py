"""Shared fixtures for FT81x tests."""

import ctypes
import pytest
from unittest.mock import MagicMock, patch

# Bootstrap: ensure ft81x can be imported even without libft81x.so.
_bootstrap_lib = MagicMock()
_bootstrap_lib.ft81x_sizeof.return_value = 256
_bootstrap_lib.ft81x_init.return_value = 0
_bootstrap_lib.ft81x_get_error.return_value = None

try:
    import ft81x
except OSError:
    with patch.object(ctypes, 'CDLL', return_value=_bootstrap_lib):
        import ft81x


@pytest.fixture
def mock_lib():
    """Patch ft81x._lib with a MagicMock for unit testing without hardware."""
    lib = MagicMock()
    lib.ft81x_sizeof.return_value = 256
    lib.ft81x_init.return_value = 0
    lib.ft81x_get_error.return_value = None
    with patch('ft81x._lib', lib):
        yield lib


@pytest.fixture
def ft81x_dev(mock_lib):
    """Return an FT81x instance backed by the mock library."""
    from ft81x import FT81x
    return FT81x()
