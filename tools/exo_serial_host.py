#!/usr/bin/env python3
"""Print everything the board sends over the ST-LINK virtual COM port."""

import argparse
import codecs
import glob
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:
    raise SystemExit("pySerial is required. Install it with: python -m pip install -r tools/requirements.txt") from exc


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
        return "/dev/ttyACM0"
    raise ValueError("No ST-LINK serial port found. Use --list-ports, then specify --port COMx (or its device path).")


def main() -> int:
    arguments = argparse.ArgumentParser(description=__doc__)
    arguments.add_argument("--port", help="serial port, for example COM3 or /dev/ttyACM0 (auto-detected by default)")
    arguments.add_argument("--baud", type=int, default=115200, help="baud rate (default 115200)")
    arguments.add_argument("--list-ports", action="store_true", help="list available serial ports and exit")
    arguments.add_argument("--duration", type=float, default=0, help="exit after this many seconds (0 = run until Ctrl-C)")
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

    deadline = time.monotonic() + args.duration if args.duration else None
    # Incremental decoder so a multi-byte character split across reads still decodes;
    # anything that isn't valid UTF-8 shows up as the replacement character.
    decoder = codecs.getincrementaldecoder("utf-8")(errors="replace")

    try:
        with serial.Serial(port, baudrate=args.baud, timeout=0.1) as connection:
            print(f"--- reading {port} at {args.baud} baud (Ctrl-C to stop) ---", file=sys.stderr)
            while deadline is None or time.monotonic() < deadline:
                data = connection.read(1024)
                if data:
                    sys.stdout.write(decoder.decode(data.replace(b"\r", b"")))
                    sys.stdout.flush()
    except KeyboardInterrupt:
        pass
    except serial.SerialException as exc:
        print(f"Serial error on {port}: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
