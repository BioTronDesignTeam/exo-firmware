# Exo Firmware

Firmware for the NUCLEO-H753ZI-based exoskeleton controller. The target is an
STM32H753 (Cortex-M7) and the production build is CMake + Ninja with the STM32
Cube command-line toolchain.

## What you need

- Linux, or Windows with WSL2.
- Docker with permission to use it.
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
| `upload`                   | Build, verify, flash, and reset the target.                               |
| `tools/exo_serial_host.py` | Decodes telemetry/log packets and checks ping acknowledgements.           |

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
STM32CubeProgrammer. It runs privileged so the ST-LINK is accessible from
inside the container.

### First-time image build

The container Dockerfile installs STM32CubeCLT 1.22 from
`.devcontainer/stm32cubeclt_1.22-Lin-Deb-x86_64.sh.zip`, tracked with Git LFS.
If it is missing after cloning, run `git lfs pull` before rebuilding the
container.

### Windows with WSL2

Use WSL2 for all build, flash, and serial commands; the native Windows shell is
not supported for the hardware loop.

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
   Containers: Reopen in Container**. Run `./upload` and the serial-monitor
   command from that environment.

## STM32CubeIDE and CubeMX

The repository is an STM32CubeIDE project: import it with **File → Import →
Existing Projects into Workspace**, selecting the repository root. Open
`exo-firmware.ioc` to inspect or change the CubeMX configuration.

Use CubeIDE/CubeMX for hardware configuration and code generation; use CMake
and `./upload` for normal builds and flashing.

Before generating code:

1. Save the working tree and preserve `/* USER CODE BEGIN */` sections.
2. Review generated changes.
3. Add new `.c`/`.cpp` files to `C_SOURCES`/`CXX_SOURCES` in `CMakeLists.txt`.
4. Rebuild with `./upload`.

## Build and flash

From the repository root:

```bash
./upload
```

It builds, verifies, flashes `build/exo-firmware.bin` at `0x08000000`, and
resets the MCU.

To build without flashing, do not use a host build against an existing
container-created `build/` cache. Run:

```bash
docker run --rm -v "$PWD":/workspace -w /workspace exo-firmware-dev \
  bash -c 'cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build'
```

## Normal hardware iteration loop

1. Make a focused change and flash it:

   ```bash
   ./upload
   ```

2. Check serial output:

   ```bash
   docker run --rm --device=/dev/ttyACM0 \
     -v "$PWD":/workspace -w /workspace exo-firmware-dev \
     python3 tools/exo_serial_host.py --duration 8 --ping-interval 0.73
   ```

3. Verify telemetry, logs, ping ACKs, and `crc_errors=0`. Repeat after every
   hardware-facing change.
