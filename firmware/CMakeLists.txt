cmake_minimum_required(VERSION 3.16)

# --- 1. TOOLCHAIN ---
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(PREFIX "riscv64-elf")
set(CMAKE_C_COMPILER   ${PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${PREFIX}-gcc)
set(CMAKE_OBJCOPY ${PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${PREFIX}-objdump)
set(CMAKE_SIZE    ${PREFIX}-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(spotwelder_code C CXX ASM)

# --- 2. RUTAS ---
set(CH32FUN_ROOT    "${CMAKE_SOURCE_DIR}/ch32fun")
set(CH32FUN_BACKEND "${CH32FUN_ROOT}/ch32fun")
set(EXTRALIBS_PATH  "${CH32FUN_ROOT}/extralibs")
# Aquí está la libgcc de 32 bits que viene con ch32fun
set(MISC_PATH       "${CH32FUN_ROOT}/misc")

# --- 3. FLAGS ---
set(ARCH_FLAGS -march=rv32ec -mabi=ilp32e)

add_compile_definitions(CH32V003=1)

set(COMMON_FLAGS
    ${ARCH_FLAGS}
    -g -Os -flto
    -ffunction-sections
    -fdata-sections
    -fmessage-length=0
    -msmall-data-limit=8
    -fno-tree-loop-distribute-patterns
    -static-libgcc
    -Wall
)

# --- 4. LINKER SCRIPT ---
set(LINKER_TEMPLATE "${CH32FUN_BACKEND}/ch32fun.ld")
set(GENERATED_LD    "${CMAKE_BINARY_DIR}/generated_ch32v003.ld")

add_custom_command(
    OUTPUT ${GENERATED_LD}
    COMMAND ${CMAKE_C_COMPILER} -E -P -x c
            -DTARGET_MCU=CH32V003
            -DMCU_PACKAGE=1
            -DTARGET_MCU_LD=0
            -DTARGET_MCU_MEMORY_SPLIT=0
            ${LINKER_TEMPLATE} > ${GENERATED_LD}
    DEPENDS ${LINKER_TEMPLATE}
    COMMENT "Generando linker script..."
)

add_custom_target(linker_script DEPENDS ${GENERATED_LD})

# --- 5. EXECUTABLE ---
add_executable(firmware
    src/main.cpp
    src/st7789.c
    "${CH32FUN_BACKEND}/ch32fun.c"
)

add_dependencies(firmware linker_script)

# --- 6. COMPILE OPTIONS ---
target_compile_options(firmware PRIVATE
    ${COMMON_FLAGS}
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-exceptions
        -fno-rtti
        -fno-threadsafe-statics
        -std=gnu++17>
    $<$<COMPILE_LANGUAGE:C>:-std=gnu99>
)

target_include_directories(firmware PRIVATE
    ${CH32FUN_BACKEND}
    ${EXTRALIBS_PATH}
    /usr/riscv64-elf/include
    src
) # --- 7. LINK OPTIONS ---
# Clave: -L apunta al misc/ del repo donde vive la libgcc de 32 bits
target_link_options(firmware PRIVATE
    ${ARCH_FLAGS}
    -T${GENERATED_LD}
    -nostdlib
    -static-libgcc
    -L${MISC_PATH}          # libgcc rv32 de ch32fun, NO la del sistema
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
)

target_link_libraries(firmware PRIVATE gcc)

# --- 9. TARGET FLASH (reemplaza tu hack) ---
add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -S $<TARGET_FILE:firmware>
            > ${CMAKE_BINARY_DIR}/firmware.lst
    COMMAND ${CMAKE_OBJCOPY} -R .storage -O binary
            $<TARGET_FILE:firmware>
            ${CMAKE_BINARY_DIR}/firmware.bin
    COMMAND ${CMAKE_OBJCOPY} -j .storage -O binary
            $<TARGET_FILE:firmware>
            ${CMAKE_BINARY_DIR}/firmware_ext.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:firmware>
            ${CMAKE_BINARY_DIR}/firmware.hex

    # HACKY FIX: mueve el elf al root, pone el wrapper en su lugar
    COMMAND ${CMAKE_COMMAND} -E rename
            $<TARGET_FILE:firmware>
            "${CMAKE_SOURCE_DIR}/firmware.elf"
    COMMAND echo "#!/bin/bash" > $<TARGET_FILE:firmware>
    COMMAND echo "${CMAKE_SOURCE_DIR}/flash_and_monitor.sh ${CMAKE_SOURCE_DIR}/firmware.elf" >> $<TARGET_FILE:firmware>
    COMMAND chmod +x $<TARGET_FILE:firmware>

    COMMENT "Generando binarios y wrapper de flash..."
)
