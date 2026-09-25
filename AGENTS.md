# Exo Firmware Development Guide

## Build system

- This is an STM32H753 C/C++17 firmware project. `CMakeLists.txt` sets
  `cmake/stm32h7.cmake` as the cross-compilation toolchain.
- Add every new firmware source file to `C_SOURCES` or `CXX_SOURCES` in
  `CMakeLists.txt`; headers only need to be under an existing include directory.
- The post-build step creates `build/exo-firmware.elf`, `.hex`, and `.bin`.
  Flash the `.bin` at `0x08000000`.
- The checked-in `build/` cache is normally created in the dev container at
  `/workspace/build`. Do not run a host `cmake --build build` against that cache:
  it reports a source-directory mismatch. Build with the same container mount:

  ```bash
  docker run --rm -v "$PWD":/workspace -w /workspace exo-firmware-dev \
    bash -c 'cmake --build build'
  ```

## Build and upload

- `./upload` is the normal one-command build-and-flash workflow. It configures a
  Debug Ninja build, verifies `build/exo-firmware.bin`, then flashes via
  `STM32_Programmer_CLI -c port=SWD -w ... 0x08000000 -v -rst`.
- The script uses native toolchains when available; otherwise it uses the
  `exo-firmware-dev` image. Its container flash fallback requires USB access.
- For an explicit container flash while troubleshooting, expose the connected
  ST-LINK USB device:

  ```bash
  docker run --rm --device=/dev/bus/usb/003/006 \
    -v "$PWD":/workspace -w /workspace exo-firmware-dev \
    STM32_Programmer_CLI -c port=SWD -w build/exo-firmware.bin 0x08000000 -v -rst
  ```

## Hardware iteration loop

1. Make a narrow change and build in the dev container.
2. Flash through the ST-LINK SWD interface and require programmer verification.
3. Exercise the direct USART3 ST-LINK VCP with the host tool:

   ```bash
   docker run --rm --device=/dev/ttyACM0 \
     -v "$PWD":/workspace -w /workspace exo-firmware-dev \
     python3 tools/exo_serial_host.py --duration 8 --ping-interval 0.73
   ```

   The stable port is `/dev/serial/by-id/*STLINK-V3*-if02`; the host user may
   lack `dialout` permission, so use the container form when necessary.
4. Require telemetry, log frames, ping ACKs, and zero CRC errors before calling
   a serial change verified. Diagnose and repeat after each hardware change.

## Firmware constraints

- Direct host serial uses the NUCLEO ST-LINK VCP on USART3 (PD8/PD9), not UART7.
  The serial protocol owns its TX queue and uses CRC-16/CCITT-FALSE.
- The CPU runs at 480 MHz with I/D cache enabled. Keep AXI SRAM cacheable and
  D2 SRAM non-cacheable in `MPU_Config`; place future DMA buffers in the
  non-cacheable region or perform explicit cache maintenance.
- FreeRTOS uses a 64 KiB heap and CMSIS-RTOS v2. Its wrapper requires
  `configMAX_PRIORITIES == 56`; do not lower it without replacing or adapting
  the wrapper.
- FDCAN timing assumes its PLL-derived 120 MHz kernel clock. Any clock-source
  or divider change requires recalculating and hardware-testing CAN timing.
