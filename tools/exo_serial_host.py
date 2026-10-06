#!/usr/bin/env python3
"""Monitor the STM32 direct serial protocol and verify host-to-board ping."""

import argparse
import glob
import struct
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:
    raise SystemExit("pySerial is required. Install it with: python -m pip install -r tools/requirements.txt") from exc

MAGIC = b"\xAA\x55"
TYPE_TELEMETRY = 0x01
TYPE_LOG = 0x04
TYPE_PING = 0x05
TYPE_ACK = 0x06
MAX_PAYLOAD = 256


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def encode_frame(packet_type: int, payload: bytes = b"") -> bytes:
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload exceeds protocol limit")
    body = bytes((packet_type,)) + struct.pack("<H", len(payload)) + payload
    return MAGIC + body + struct.pack("<H", crc16_ccitt_false(body))


class Parser:
    def __init__(self) -> None:
        self.buffer = bytearray()
        self.crc_errors = 0

    def feed(self, data: bytes):
        self.buffer.extend(data)
        packets = []
        while True:
            start = self.buffer.find(MAGIC)
            if start < 0:
                del self.buffer[:-1]
                return packets
            if start:
                del self.buffer[:start]
            if len(self.buffer) < 7:
                return packets

            packet_type = self.buffer[2]
            length = struct.unpack_from("<H", self.buffer, 3)[0]
            if length > MAX_PAYLOAD:
                del self.buffer[0]
                continue
            frame_length = 7 + length
            if len(self.buffer) < frame_length:
                return packets

            body = bytes(self.buffer[2 : 5 + length])
            received_crc = struct.unpack_from("<H", self.buffer, 5 + length)[0]
            del self.buffer[:frame_length]
            if crc16_ccitt_false(body) != received_crc:
                self.crc_errors += 1
                continue
            packets.append((packet_type, body[3:]))


def is_stlink_port(info) -> bool:
    description = " ".join(
        str(value or "") for value in (info.description, info.manufacturer, info.product, info.interface)
    )
    normalized = "".join(char for char in description.casefold() if char.isalnum())
    return "stlink" in normalized or (info.vid == 0x0483 and "virtualcom" in normalized)


def default_port() -> str:
    if sys.platform.startswith("linux"):
        matches = sorted(glob.glob("/dev/serial/by-id/*STLINK-V3*-if02"))
        if len(matches) == 1:
            return matches[0]

    matches = sorted(info.device for info in list_ports.comports() if is_stlink_port(info))
    if len(matches) == 1:
        return matches[0]
    if len(matches) > 1:
        raise ValueError(f"Multiple ST-LINK serial ports found: {', '.join(matches)}. Choose one with --port.")
    if sys.platform.startswith("linux"):
        return "/dev/ttyACM0"  # Preserve the existing Linux fallback.
    raise ValueError("No ST-LINK serial port found. Use --list-ports, then specify --port COMx (or its device path).")


def print_packet(packet_type: int, payload: bytes) -> None:
    if packet_type == TYPE_TELEMETRY and len(payload) == 38:
        timestamp, accel_x, accel_y, accel_z, quat_i, quat_j, quat_k, quat_real, accuracy, status, valid = (
            struct.unpack("<IffffffffBB", payload)
        )
        bno_text = (
            f" bno_q=({quat_i:+.4f},{quat_j:+.4f},{quat_k:+.4f},{quat_real:+.4f})"
            f" status={status} acc={accuracy:.3f}"
            if valid
            else " bno=unavailable"
        )
        print(
            f"telemetry t={timestamp:>10} ms accel=({accel_x:+.3f}, {accel_y:+.3f}, {accel_z:+.3f}) g"
            f"{bno_text}"
        )
    elif packet_type == TYPE_TELEMETRY and len(payload) == 16:
        timestamp, x, y, z = struct.unpack("<Ifff", payload)
        print(f"telemetry t={timestamp:>10} ms accel=({x:+.3f}, {y:+.3f}, {z:+.3f}) g")
    elif packet_type == TYPE_LOG:
        print(f"log       {payload.decode('utf-8', errors='replace')}", end="")
    elif packet_type == TYPE_ACK:
        print(f"ack       {payload.hex()}")
    elif packet_type == TYPE_PING:
        print(f"ping      {payload.hex()}")
    else:
        print(f"type=0x{packet_type:02X} payload={payload.hex()}")


def main() -> int:
    arguments = argparse.ArgumentParser(description=__doc__)
    arguments.add_argument("--port", help="serial port, for example COM3 or /dev/ttyACM0 (auto-detected by default)")
    arguments.add_argument("--list-ports", action="store_true", help="list available serial ports and exit")
    arguments.add_argument("--duration", type=float, default=0, help="exit after this many seconds (0 = run until Ctrl-C)")
    arguments.add_argument("--no-ping", action="store_true", help="do not send a startup ping")
    arguments.add_argument("--ping-interval", type=float, default=0, help="send another ping at this interval in seconds")
    args = arguments.parse_args()

    if args.list_ports:
        ports = sorted(list_ports.comports())
        for info in ports:
            print(f"{info.device}: {info.description}")
        if not ports:
            print("No serial ports found.")
        return 0

    try:
        port = args.port or default_port()
    except ValueError as exc:
        arguments.error(str(exc))

    parser = Parser()
    received = 0
    acknowledgements = 0
    deadline = time.monotonic() + args.duration if args.duration else None
    next_ping = 0.0

    def send_ping() -> None:
        nonlocal next_ping
        nonce = struct.pack("<I", int(time.monotonic() * 1000) & 0xFFFFFFFF)
        frame = encode_frame(TYPE_PING, nonce)
        if connection.write(frame) != len(frame):
            raise serial.SerialException("incomplete ping write")
        print(f"sent ping  {nonce.hex()} on {port}")
        next_ping = time.monotonic() + args.ping_interval

    try:
        with serial.Serial(port, baudrate=115200, timeout=0.1, write_timeout=1) as connection:
            if not args.no_ping:
                send_ping()
            else:
                print(f"monitoring {port}")

            while deadline is None or time.monotonic() < deadline:
                if args.ping_interval and time.monotonic() >= next_ping:
                    send_ping()
                data = connection.read(1024)
                if not data:
                    continue
                for packet_type, payload in parser.feed(data):
                    received += 1
                    acknowledgements += packet_type == TYPE_ACK
                    print_packet(packet_type, payload)
    except KeyboardInterrupt:
        pass
    except serial.SerialException as exc:
        print(f"Serial error on {port}: {exc}", file=sys.stderr)
        return 2

    print(f"summary: packets={received} acknowledgements={acknowledgements} crc_errors={parser.crc_errors}")
    if parser.crc_errors:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
