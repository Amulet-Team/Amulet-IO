from __future__ import annotations

import enum

__all__: list[str] = [
    "EndianState",
    "test_read_array",
    "test_read_endianness",
    "test_read_numeric",
    "test_read_overflow",
    "test_read_string",
    "test_reserve",
    "test_write_endianness",
    "test_write_numeric",
    "test_write_string",
]

class EndianState(enum.Enum):
    Big = 1
    Default = 0
    Little = 2

def test_read_array() -> None: ...
def test_read_endianness() -> None: ...
def test_read_numeric(
    endian_data: EndianState, read_offset: bool, read_into: bool
) -> None: ...
def test_read_overflow() -> None: ...
def test_read_string(endian_data: EndianState, read_offset: bool) -> None: ...
def test_reserve() -> None: ...
def test_write_endianness() -> None: ...
def test_write_numeric(endian_data: EndianState) -> None: ...
def test_write_string(endian_data: EndianState) -> None: ...
