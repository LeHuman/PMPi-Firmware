cmake_minimum_required(VERSION 3.14...3.22)

include(../cmake/usage.cmake)

# ---- Pico SDK ----

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# ---- Project ----

set(PARENT_PROJECT_NAME ${PROJECT_NAME})
# set(PARENT_PROJECT_FILE_BIN ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
# set(PARENT_PROJECT_FILE_ASM ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.S)

project(bootloader C ASM)
set(CMAKE_C_STANDARD 17)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Winline
        -Os
        )

# ---- Add source files ----

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c")
# file(GLOB_RECURSE pios CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.pio")

# ---- Create executable ----

add_executable(${PROJECT_NAME} ${sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})
# FOREACH(PIO_FILE ${pios})
#     pico_generate_pio_header(${PROJECT_NAME} ${PIO_FILE})
# ENDFOREACH()

target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_dma hardware_uart hardware_flash hardware_watchdog hardware_pwm)

# ---- Setup Linker ----

# NOTE: These values should agree with what the linker scripts state
math(EXPR FLASH_XIP_BASE "0x10000000" OUTPUT_FORMAT DECIMAL)
math(EXPR FLASH_TOTAL_LENGTH "2048 * 1024")

math(EXPR FLASH_BOOTLOADER_ORIGIN "${FLASH_XIP_BASE} + 0")
math(EXPR FLASH_BOOTLOADER_LENGTH "4096 * 9")
 
math(EXPR FLASH_HEADER_ORIGIN "${FLASH_BOOTLOADER_ORIGIN} + ${FLASH_BOOTLOADER_LENGTH}")
math(EXPR FLASH_HEADER_LENGTH "4096")

math(EXPR FLASH_MAIN_ORIGIN "${FLASH_HEADER_ORIGIN} + ${FLASH_HEADER_LENGTH}")
math(EXPR FLASH_MAIN_LENGTH "${FLASH_TOTAL_LENGTH} - ${FLASH_BOOTLOADER_LENGTH} - ${FLASH_HEADER_LENGTH}")

add_compile_definitions(FLASH_MAIN_ORIGIN=${FLASH_MAIN_ORIGIN} FLASH_HEADER_ORIGIN=${FLASH_HEADER_ORIGIN} FLASH_BOOTLOADER_ORIGIN=${FLASH_BOOTLOADER_ORIGIN})
add_compile_definitions(FLASH_BOOTLOADER_LENGTH=${FLASH_BOOTLOADER_LENGTH} FLASH_HEADER_LENGTH=${FLASH_HEADER_LENGTH} FLASH_MAIN_LENGTH=${FLASH_MAIN_LENGTH})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/com_memmap.in.ld ${CMAKE_BINARY_DIR}/com_memmap.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boot_memmap.in.ld ${CMAKE_BINARY_DIR}/boot_memmap.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_memmap.in.ld ${CMAKE_BINARY_DIR}/app_memmap.ld)

set(HEADER_FILE_BIN ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_HDR.bin)
set(HEADER_FILE_ASM ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_HDR.S)
set(PROJECT_FILE_BIN ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(PROJECT_FILE_ASM ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.S)

find_package (Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(OUTPUT ${PROJECT_FILE_ASM}
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_FILE_ASM}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf ${PROJECT_FILE_BIN}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PROJECT_FILE_BIN} ${PROJECT_FILE_ASM} "boot3" "ax"
    COMMENT "Generating Bootloader ASM file"
    VERBATIM
)
add_custom_target(BootloaderAssembly ALL DEPENDS ${PROJECT_FILE_ASM})

# ---- Misc. Configuration ----

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# pico_add_extra_outputs(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# Select linker script for both bootloader and application
pico_set_linker_script(${PROJECT_NAME} "${CMAKE_BINARY_DIR}/boot_memmap.ld")
# pico_set_linker_script(${PROJECT_NAME} "${CMAKE_BINARY_DIR}/com_memmap.ld")

set(__BOOTLOADER_NAME ${PROJECT_NAME} PARENT_SCOPE)
set(__BOOTLOADER_FILE_ASM ${PROJECT_FILE_ASM} PARENT_SCOPE)
get_filename_component(__LINKER_DIR "${CMAKE_BINARY_DIR}/com_memmap.ld" ABSOLUTE)
get_filename_component(__APP_LINKER_DIR "${CMAKE_BINARY_DIR}/app_memmap.ld" ABSOLUTE)
set(__LINKER_DIR "${__LINKER_DIR}" PARENT_SCOPE)
set(__APP_LINKER_DIR "${__APP_LINKER_DIR}" PARENT_SCOPE)

set(__HEADER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/header.py" PARENT_SCOPE)
set(__ASM_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/asm.py" PARENT_SCOPE)

set(__FLASH_MAIN_ORIGIN ${FLASH_MAIN_ORIGIN} PARENT_SCOPE)
set(__FLASH_MAIN_LENGTH ${FLASH_MAIN_LENGTH} PARENT_SCOPE)

# TODO: generate standalone/stripped ihex file

function(bootloader_attach proj_name)
    pico_set_linker_script(${proj_name} "${__LINKER_DIR}")
    add_dependencies(${proj_name} BootloaderAssembly ${__BOOTLOADER_NAME})
    target_sources(${proj_name} PRIVATE ${__BOOTLOADER_FILE_ASM})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.hex"
        COMMAND ${Python3_EXECUTABLE} "${__HEADER_SCRIPT}" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.hex" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.bin" ${__FLASH_MAIN_ORIGIN} ${__FLASH_MAIN_LENGTH}
        # COMMAND ${Python3_EXECUTABLE} "${__ASM_SCRIPT}" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.bin" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.S" "flash_header" "an"
        COMMAND ${CMAKE_OBJCOPY} --output-target=elf32-littlearm --update-section .flash_header="${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_Header.bin" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_OUT.elf"
        COMMAND ${CMAKE_OBJCOPY} -O ihex "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_OUT.elf" "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}_OUT.hex"
    )

    message(STATUS "Attached ${__BOOTLOADER_NAME} to ${PROJECT_NAME}")
endfunction(bootloader_attach)

# Print out flash usage for fun
# usage_print(${PROJECT_NAME} ${PROJECT_FILE_BIN} "${FLASH_BOOTLOADER_LENGTH}" "BOOTLOADER FLASH")
# usage_print(BOOTLOADER_ASM ${PARENT_PROJECT_HDR} "${FLASH_HEADER_LENGTH}" "FLASH HEADER")
# usage_print(${PARENT_PROJECT_NAME} ${PARENT_PROJECT_FILE_BIN} "${FLASH_MAIN_LENGTH}" "MAIN FLASH")

# arm-none-eabi-objcopy.exe -j .text -j .binary_info -j .rodata --debugging .\build\PMPi.elf .\build\PMPi_min.elf;arm-none-eabi-objcopy.exe -O ihex .\build\PMPi_min.elf .\build\PMPi.hex;arm-none-eabi-objcopy.exe -O binary .\build\PMPi_min.elf .\build\PMPi_min.bin;arm-none-eabi-objdump.exe -h .\build\PMPi_min.bin > .\build\PMPi_min.map;arm-none-eabi-readelf.exe .\build\PMPi_min.elf -lS
