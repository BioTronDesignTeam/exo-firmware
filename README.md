# Exo Firmware

Firmware for the NUCLEO-H753ZI-based exoskeleton controller. The target is an
STM32H753 (Cortex-M7) and the production build is CMake + Ninja with the STM32
Cube command-line toolchain.

## What you need

- Linux with Docker, or Windows with the native command-line tools listed below.
- A NUCLEO-H753ZI connected through its 2 USB port.
- VS Code with the Dev Containers extension
- STM32CubeIDE only when changing CubeMX hardware configuration, pin mapping, clocks, or generated HAL setup.

The ST-LINK provides both:
- SWD for programming and debugging.
- A virtual COM port for the firmware's direct serial protocol. On Linux it is
  normally `/dev/serial/by-id/*STLINK-V3*-if02` or `/dev/ttyACM0`.

## Repository layout

| Path                       | Purpose                                                                   |
| -------------------------- | ------------------------------------------------------------------------- |
| `Core/`                    | CubeMX-generated startup, HAL configuration, and application entry point. |
| `Tasks/`                   | FreeRTOS tasks and the direct serial protocol.                            |
| `Drivers/Peripherals/`     | Device drivers such as the BNO085 and MSA311 IMUs.                        |
| `exo-firmware.ioc`         | STM32CubeMX configuration for STM32CubeIDE.                               |
| `CMakeLists.txt`           | Authoritative CMake build source list.                                    |
| `upload`                   | Linux/macOS flash and reset; optional build with `--build`.               |
| `upload.cmd` / `upload.ps1` | Windows flash and reset; optional native build with `-Build`.              |
| `tools/exo_serial_host.py` | Decodes telemetry/log packets and checks ping acknowledgements.           |

## Firmware architecture

`main()` creates a high-priority startup task, which runs once the scheduler
starts and creates the tasks below. Priorities live in
`Tasks/Inc/task_priorities.hpp`.

| Task              | Priority      | Role                                                        |
| ----------------- | ------------- | ----------------------------------------------------------- |
| `Supervisor`      | Realtime      | Feeds the hardware watchdog while monitored tasks check in. |
| `MotorController` | High          | Motor state machine at 100 Hz (not started by default).     |
| `UpdateBNO085`    | AboveNormal   | Reads BNO085 rotation vectors over interrupt-driven I2C.    |
| `SerialRx`        | Normal        | Parses host frames fed by the USART3 RX interrupt.          |
| `SerialTx`        | Normal        | Sends queued frames with interrupt-driven TX.               |
| `InitializeDriver`| Normal        | Constructs the drivers, starts CAN, then exits.             |
| `SerialTelemetry` | BelowNormal   | Sends a telemetry frame every 200 ms.                       |
| `UpdateMSA311`    | Low           | Reads and logs the MSA311 every 2 s when its driver exists. |

### Safety

- `Supervisor` refreshes IWDG1 (500 ms) only while every registered task
  checks in within its deadline. On a stall it sends an ODrive estop and lets
  the watchdog reset the MCU. The watchdog is frozen while the debugger halts
  the core.
- `MotorController` arms only on a USER button press with a live ODrive
  heartbeat (300 ms) and host link (2 s since the last valid frame, so keep
  `exo_serial_host.py` pinging). While running it sends an estop and latches a
  fault on link loss, axis errors, leaving closed loop, bus over-current,
  over-speed, or repeated CAN TX failures. Press the button again to clear a
  fault once the links are healthy.
- The current and speed limits in `motor_controller.cpp` match the bench demo.
  Set them for the real joints before wearing the device.
- Also enable the ODrive's own watchdog (`axis0.config.enable_watchdog`,
  `watchdog_timeout`) so it disarms if the STM32 stops sending setpoints.

### CAN

FDCAN1 runs classic CAN at 500 kbit/s from a 120 MHz kernel clock.
`ODRIVES1::startBus()` refuses to start if that clock changes. Each ODrive's
`node_id` must match the value passed to its `ODRIVES1` constructor in
`drivers.cpp`; up to four ODrives are supported. The driver recovers from
bus-off and counts `busOffEvents`, `rxFramesLost`, and per-ODrive `txErrors`.

## Development container setup

### VS Code

1. Install Docker and the VS Code **Dev Containers** extension.
2. Clone this repository and open it in VS Code.
3. Run **Dev Containers: Reopen in Container**. (This will take a while)
4. Confirm the toolchain is available:

   ```bash
   arm-none-eabi-gcc --version
   cmake --version
   STM32_Programmer_CLI --version
   ```

The dev container supplies CMake, Ninja, the ARM cross compiler, and
STM32CubeProgrammer. On Windows, build in the container and flash from the
host using the workflow below. The ST-LINK stays attached to Windows; the
container does not need USB access for building.

### Build in the container, flash from Windows

Install [STM32CubeProgrammer for Windows](https://www.st.com/en/development-tools/stm32cubeprog.html)
and its ST-LINK USB driver. Add its `bin` directory to the Windows PATH.
Windows does not need CMake, Ninja, or an ARM compiler for this workflow.

Open the Windows checkout in VS Code and use
**Dev Containers: Reopen in Container**. Build from the repository root in
the container terminal:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The mounted checkout makes `build/exo-firmware.bin` available on Windows.
Connect the board's ST-LINK USB port, open Windows PowerShell outside the
container, and run from the repository directory:

```powershell
.\upload.cmd
```

This only flashes, verifies, and resets the board. It never builds or reads
the container's CMake cache. Rebuild in the container after each source change
before uploading. Do not attach the ST-LINK to WSL for this host-flash route.

### First-time image build

The container Dockerfile installs STM32CubeCLT 1.22 from
`.devcontainer/stm32cubeclt_1.22-Lin-Deb-x86_64.sh.zip`, tracked with Git LFS.
If it is missing after cloning, run `git lfs pull` before rebuilding the
container.

## Native Windows build and flash

Run from **Windows PowerShell or Command Prompt on the host**, outside the
Linux dev container. The ST-LINK stays attached to Windows; no WSL USB
passthrough, Docker, or dev container is needed for this route.

Install these once:

1. [CMake](https://cmake.org/download/) (3.22 or newer), with `cmake.exe` on PATH.
2. [Ninja](https://ninja-build.org/) with `ninja.exe` on PATH.
3. [STM32CubeCLT for Windows](https://www.st.com/en/development-tools/stm32cubeclt.html).
   It includes the Arm GNU C/C++ compiler, `objcopy`, `size`, and
   STM32CubeProgrammer. Add its `GNU-tools-for-STM32/bin` and
   `STM32CubeProgrammer/bin` directories to PATH. An independently installed
   [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)
   also works for the latter.
4. The **ST-LINK USB driver** (select the ST-LINK drivers option in the
   STM32CubeCLT installer, or install the driver supplied with
   STM32CubeProgrammer). Connect the NUCLEO board's ST-LINK USB port.

Open a new terminal after editing PATH. From the repository directory, run:

```powershell
.\upload.cmd -Build
```

Or run `.\upload.ps1 -Build` from PowerShell if your execution policy allows it.
The CMD wrapper launches PowerShell for this run without changing the system
execution policy. With `-Build`, the script checks each build tool,
builds in `build/windows/`,
checks the generated binary, then programs and verifies it over SWD at
`0x08000000` before resetting. The separate build directory keeps the native
Windows CMake cache away from the Linux dev container cache in `build/`.
If you chose a custom install directory, add its tool directories to PATH;
the script reports any tool it cannot find.

WSL2 remains an option for the Linux workflow, but its ST-LINK USB connection
requires `usbipd-win` attachment before `./upload` can flash.

### Optional WSL2 workflow

Existing WSL2 users can continue the Linux build, flash, and serial workflow.

1. Install WSL2 with an Ubuntu distribution, Docker Desktop with WSL
   integration, VS Code, and the VS Code **WSL** extension.
2. Install `usbipd-win` on Windows. In an elevated PowerShell, find and attach
   the ST-LINK:

   ```powershell
   usbipd list
   usbipd bind --busid <BUSID>
   usbipd attach --wsl --busid <BUSID>
   ```

3. In WSL, clone the repository under your Linux home directory (for example,
   `~/src/exo-firmware`), not `/mnt/c`.
4. Confirm WSL sees the programmer and serial device:

   ```bash
   ls /dev/ttyACM* /dev/serial/by-id/
   ```

5. From the WSL repository directory, run `code .`, then use **Dev
   Containers: Reopen in Container**. Run `./upload --build` and the serial-monitor
   command from that environment.

## STM32CubeIDE and CubeMX

The repository is an STM32CubeIDE project: import it with **File → Import →
Existing Projects into Workspace**, selecting the repository root. Open
`exo-firmware.ioc` to inspect or change the CubeMX configuration.

Use CubeIDE/CubeMX for hardware configuration and code generation; use CMake
and the platform's upload script for normal builds and flashing.

> **Warning:** the clock tree in `exo-firmware.ioc` is out of date. `main.c`
> runs the CPU at 480 MHz with a 120 MHz FDCAN clock, but the `.ioc` still
> describes a 64 MHz tree. Update the clock configuration in CubeMX to match
> `SystemClock_Config()` before generating code, or generation will change the
> system clock and CAN will refuse to start.

Before generating code:

1. Save the working tree and preserve `/* USER CODE BEGIN */` sections.
2. Review generated changes.
3. Add new `.c`/`.cpp` files to `C_SOURCES`/`CXX_SOURCES` in `CMakeLists.txt`.
4. Rebuild and flash with `./upload --build`.

## Build and flash

From the repository root, use `./upload` on Linux/macOS to flash an existing
`build/exo-firmware.bin`, or `./upload --build` to build first. Bash also accepts
`-Build` as an alias for the PowerShell spelling.
On Windows, use `./upload.cmd` to flash the existing container-built binary,
or `./upload.cmd -Build` to build natively first. All upload routes verify the
flash at `0x08000000` and reset the MCU.

To build without flashing on Linux, do not use a host build against an
existing container-created `build/` cache. Run:

```bash
docker run --rm -v "$PWD":/workspace -w /workspace exo-firmware-dev \
  bash -c 'cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build'
```

## Serial monitor

`tools/exo_serial_host.py` reads telemetry and logs from the ST-LINK virtual
COM port and can send pings to check acknowledgements. It uses pySerial on
Linux, macOS, and Windows. The dev container installs Python and pySerial.

For native Windows, install [Python 3](https://www.python.org/downloads/) and
the ST-LINK USB driver, then install the script's dependency in a Windows
terminal from the repository root:

```powershell
py -3 -m pip install -r tools/requirements.txt
py -3 tools/exo_serial_host.py --list-ports
py -3 tools/exo_serial_host.py --duration 8 --ping-interval 0.73
```

The script selects the ST-LINK virtual COM port when exactly one is found. If
auto-detection cannot identify it, use the port shown by `--list-ports` or in
Windows Device Manager under **Ports (COM & LPT)**:

```powershell
py -3 tools/exo_serial_host.py --port COM3 --duration 8 --ping-interval 0.73
```

Substitute your actual COM number. Close other serial monitors before running
the script. On Linux, `python3 -m pip install -r tools/requirements.txt`
provides pySerial for a host Python installation; the dev container already
includes it. The existing `/dev/serial/by-id` selection and `/dev/ttyACM0`
fallback remain available.

## Linux hardware iteration loop

1. Make a focused change and flash it:

   ```bash
   ./upload --build
   ```

2. Check serial output:

   ```bash
   docker run --rm --device=/dev/ttyACM0 \
     -v "$PWD":/workspace -w /workspace exo-firmware-dev \
     python3 tools/exo_serial_host.py --duration 8 --ping-interval 0.73
   ```

3. Verify telemetry, logs, ping ACKs, and `crc_errors=0`. Repeat after every
   hardware-facing change.
