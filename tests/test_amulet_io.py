from unittest import TestCase

from _test_amulet_io import (
    EndianState,
    test_read_numeric,
    test_read_string,
    test_read_overflow,
    test_write_numeric,
    test_write_string,
)


class AmuletIOTestCase(TestCase):
    def test_read(self) -> None:
        for endian_state in (EndianState.Default, EndianState.Little, EndianState.Big):
            for read_offset in (False, True):
                for read_into in (False, True):
                    with self.subTest(
                        endian_state=endian_state,
                        read_offset=read_offset,
                        read_into=read_into,
                    ):
                        test_read_numeric(endian_state, read_offset, read_into)
                with self.subTest(endian_state=endian_state, read_offset=read_offset):
                    test_read_string(endian_state, read_offset)

    def test_read_errors(self) -> None:
        test_read_overflow()

    def test_write(self) -> None:
        for endian_state in (EndianState.Default, EndianState.Little, EndianState.Big):
            with self.subTest(endian_state=endian_state):
                test_write_numeric(endian_state)
                test_write_string(endian_state)
