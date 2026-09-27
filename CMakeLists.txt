cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32h7.cmake)

project(exo-firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(
    USE_HAL_DRIVER
    STM32H753xx
    USE_PWR_LDO_SUPPLY
    $<$<CONFIG:Debug>:DEBUG>
)

set(INCLUDE_DIRS
    Core/Inc
    Tasks/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    Drivers/BSP/STM32H7xx_Nucleo
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/CMSIS/Include
    Drivers/CMSIS/RTOS2/Include
    Drivers/Peripherals/Communication/CAN_Simple
    Drivers/Peripherals/IMU/BNO085
    Drivers/Peripherals/IMU/MPU6050
    Drivers/Peripherals/IMU/MSA311
    Drivers/Peripherals/Instances
    Drivers/Peripherals/Motors/OdriveS1
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

set(C_SOURCES
    Core/Src/main.c
    Core/Src/freertos.c
    Core/Src/stm32h7xx_hal_msp.c
    Core/Src/stm32h7xx_hal_timebase_tim.c
    Core/Src/stm32h7xx_it.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32h7xx.c

    Drivers/BSP/STM32H7xx_Nucleo/stm32h7xx_nucleo.c

    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c

    Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    Middlewares/Third_Party/FreeRTOS/Source/list.c
    Middlewares/Third_Party/FreeRTOS/Source/queue.c
    Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    Middlewares/Third_Party/FreeRTOS/Source/timers.c
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
)

set(CXX_SOURCES
    Tasks/Src/general_thread.cpp
    Tasks/Src/imu.cpp
    Tasks/Src/motor_controller.cpp
    Tasks/Src/serial_protocol.cpp
    Tasks/Src/uart.cpp

    Drivers/Peripherals/IMU/BNO085/bno085.cpp
    Drivers/Peripherals/IMU/MPU6050/mpu6050.cpp
    Drivers/Peripherals/IMU/MSA311/msa311.cpp
    Drivers/Peripherals/Instances/drivers.cpp
    Drivers/Peripherals/Motors/OdriveS1/odriveS1.cpp
    Drivers/Peripherals/override.cpp
)

set(ASM_SOURCES
    Core/Startup/startup_stm32h753zitx.s
)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32H753ZITX_FLASH.ld)

add_executable(${PROJECT_NAME}
    ${C_SOURCES}
    ${CXX_SOURCES}
    ${ASM_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_link_options(${PROJECT_NAME} PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--print-memory-usage
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE}    $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Generating hex/bin and printing size"
)

add_custom_target(flash
    COMMAND STM32_Programmer_CLI -c port=SWD -w ${PROJECT_NAME}.bin 0x08000000 -v -rst
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flashing via ST-Link SWD"
)
