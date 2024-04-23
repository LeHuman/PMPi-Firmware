cmake_minimum_required(VERSION 3.14...3.22)

include(../cmake/usage.cmake)

# ---- Pico SDK ----

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# ---- Project ----

set(PARENT_PROJECT_NAME ${PROJECT_NAME})
set(PARENT_PROJECT_FILE_BIN ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
set(PARENT_PROJECT_FILE_ASM ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.S)

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

math(EXPR FLASH_MAIN_ORIGIN "${FLASH_HEADER_ORIGIN} + ${FLASH_HEADER_LENGTH} + 4096")
math(EXPR FLASH_MAIN_LENGTH "${FLASH_TOTAL_LENGTH} - ${FLASH_BOOTLOADER_LENGTH} - ${FLASH_HEADER_LENGTH}")

add_compile_definitions(FLASH_MAIN_ORIGIN=${FLASH_MAIN_ORIGIN} FLASH_BOOTLOADER_ORIGIN=${FLASH_BOOTLOADER_ORIGIN})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/com_memmap.in.ld ${CMAKE_BINARY_DIR}/com_memmap.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boot_memmap.in.ld ${CMAKE_BINARY_DIR}/boot_memmap.ld)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_memmap.in.ld ${CMAKE_BINARY_DIR}/app_memmap.ld)

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
pico_set_linker_script(${PARENT_PROJECT_NAME} "${CMAKE_BINARY_DIR}/com_memmap.ld")
pico_set_linker_script(${PROJECT_NAME} "${CMAKE_BINARY_DIR}/boot_memmap.ld")

add_dependencies(${PARENT_PROJECT_NAME} BootloaderAssembly ${PROJECT_NAME})
target_sources(${PARENT_PROJECT_NAME} PRIVATE ${PROJECT_FILE_ASM})

# Print out flash usage for fun
# usage_print(${PROJECT_NAME} ${PROJECT_FILE_BIN} "${FLASH_BOOTLOADER_LENGTH}" "BOOTLOADER FLASH")
# usage_print(BOOTLOADER_ASM ${PARENT_PROJECT_HDR} "${FLASH_HEADER_LENGTH}" "FLASH HEADER")
# usage_print(${PARENT_PROJECT_NAME} ${PARENT_PROJECT_FILE_BIN} "${FLASH_MAIN_LENGTH}" "MAIN FLASH")