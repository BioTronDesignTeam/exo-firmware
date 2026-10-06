"""Hardware-free checks for serial port selection and the monitor loop."""

import contextlib
import io
import sys
import unittest
from types import SimpleNamespace
from unittest import mock

from tools import exo_serial_host as host


def port(device, description, manufacturer="STMicroelectronics", vid=0x0483):
    return SimpleNamespace(
        device=device, description=description, manufacturer=manufacturer,
        product=None, interface=None, vid=vid,
    )


class SerialHostTests(unittest.TestCase):
    def test_auto_detect_windows_stlink_port(self):
        ports = [port("COM4", "STLink Virtual COM Port"), port("COM8", "USB Serial Device", "Other", 0x1234)]
        with mock.patch.object(sys, "platform", "win32"), mock.patch.object(host.list_ports, "comports", return_value=ports):
            self.assertEqual(host.default_port(), "COM4")

    def test_multiple_windows_ports_require_selection(self):
        ports = [port("COM4", "STLink Virtual COM Port"), port("COM7", "STLink Virtual COM Port")]
        with mock.patch.object(sys, "platform", "win32"), mock.patch.object(host.list_ports, "comports", return_value=ports):
            with self.assertRaisesRegex(ValueError, "--port"):
                host.default_port()

    def test_linux_stable_port_is_preserved(self):
        stable = "/dev/serial/by-id/usb-STLINK-V3-if02"
        with mock.patch.object(sys, "platform", "linux"), mock.patch.object(host.glob, "glob", return_value=[stable]):
            self.assertEqual(host.default_port(), stable)

    def test_monitor_sends_ping_and_decodes_ack(self):
        connection = mock.MagicMock()
        connection.__enter__.return_value = connection
        connection.write.side_effect = lambda frame: len(frame)
        connection.read.side_effect = [host.encode_frame(host.TYPE_ACK, b"ok"), KeyboardInterrupt()]
        output = io.StringIO()
        with mock.patch.object(sys, "argv", ["exo_serial_host.py", "--port", "COM7"]), \
             mock.patch.object(host.serial, "Serial", return_value=connection) as serial_open, \
             contextlib.redirect_stdout(output):
            self.assertEqual(host.main(), 0)
        serial_open.assert_called_once_with("COM7", baudrate=115200, timeout=0.1, write_timeout=1)
        self.assertEqual(connection.write.call_count, 1)
        self.assertIn("ack       6f6b", output.getvalue())
        self.assertIn("acknowledgements=1 crc_errors=0", output.getvalue())
        connection.__exit__.assert_called_once()


if __name__ == "__main__":
    unittest.main()
