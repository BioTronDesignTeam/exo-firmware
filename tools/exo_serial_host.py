#!/usr/bin/env python3
"""Monitor the STM32 direct serial protocol and verify host-to-board ping."""

import argparse
import glob
import os
import select
import struct
import sys
import termios
import time

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


def default_port() -> str:
    matches = glob.glob("/dev/serial/by-id/*STLINK-V3*-if02")
    return matches[0] if matches else "/dev/ttyACM0"


def open_serial(port: str) -> tuple[int, list]:
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    previous = termios.tcgetattr(fd)
    settings = termios.tcgetattr(fd)
    settings[0] = termios.IGNPAR
    settings[1] = 0
    settings[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    settings[3] = 0
    settings[4] = termios.B115200
    settings[5] = termios.B115200
    settings[6][termios.VMIN] = 0
    settings[6][termios.VTIME] = 0
    termios.tcsetattr(fd, termios.TCSANOW, settings)
    return fd, previous


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
    arguments.add_argument("--port", default=default_port(), help="serial device path")
    arguments.add_argument("--duration", type=float, default=0, help="exit after this many seconds (0 = run until Ctrl-C)")
    arguments.add_argument("--no-ping", action="store_true", help="do not send a startup ping")
    arguments.add_argument("--ping-interval", type=float, default=0, help="send another ping at this interval in seconds")
    args = arguments.parse_args()

    fd, previous = open_serial(args.port)
    parser = Parser()
    received = 0
    acknowledgements = 0
    deadline = time.monotonic() + args.duration if args.duration else None
    next_ping = 0.0

    def send_ping() -> None:
        nonlocal next_ping
        nonce = struct.pack("<I", int(time.monotonic() * 1000) & 0xFFFFFFFF)
        os.write(fd, encode_frame(TYPE_PING, nonce))
        print(f"sent ping  {nonce.hex()} on {args.port}")
        next_ping = time.monotonic() + args.ping_interval

    try:
        if not args.no_ping:
            send_ping()
        else:
            print(f"monitoring {args.port}")

        while deadline is None or time.monotonic() < deadline:
            if args.ping_interval and time.monotonic() >= next_ping:
                send_ping()
            ready, _, _ = select.select([fd], [], [], 0.1)
            if not ready:
                continue
            data = os.read(fd, 1024)
            for packet_type, payload in parser.feed(data):
                received += 1
                acknowledgements += packet_type == TYPE_ACK
                print_packet(packet_type, payload)
    except KeyboardInterrupt:
        pass
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, previous)
        os.close(fd)

    print(f"summary: packets={received} acknowledgements={acknowledgements} crc_errors={parser.crc_errors}")
    if parser.crc_errors:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
