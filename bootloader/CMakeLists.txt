cmake_minimum_required(VERSION 3.14...3.22)

include(../cmake/usage.cmake)

# ---- Pico SDK ----

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# ---- Project ----

set(PARENT_PROJECT_NAME ${PROJECT_NAME})
set(PARENT_PROJECT_BIN ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)

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
# foreach(source IN LISTS ${sources})
#     get_source_file_property(curr_props ${source} COMPILE_OPTIONS)
#     set_source_files_properties(${source} PROPERTIES COMPILE_OPTIONS "${curr_props} -Winline")
# endforeach()

# ---- Setup Linker ----

# NOTE: These values should agree with what the linker scripts state
math(EXPR FLASH_XIP_BASE "0x10000000" OUTPUT_FORMAT DECIMAL)
math(EXPR FLASH_TOTAL_LENGTH "2048 * 1024")

math(EXPR FLASH_BOOTLOADER_ORIGIN "${FLASH_XIP_BASE} + 0")
math(EXPR FLASH_BOOTLOADER_LENGTH "4096 * 10")

math(EXPR FLASH_HEADER_ORIGIN "${FLASH_BOOTLOADER_ORIGIN} + ${FLASH_BOOTLOADER_LENGTH}")
math(EXPR FLASH_HEADER_LENGTH "4096")

math(EXPR FLASH_MAIN_ORIGIN "${FLASH_HEADER_ORIGIN} + ${FLASH_HEADER_LENGTH} + 4096")
math(EXPR FLASH_MAIN_LENGTH "${FLASH_TOTAL_LENGTH} - ${FLASH_BOOTLOADER_LENGTH} - ${FLASH_HEADER_LENGTH}")

# math(EXPR FLASH_BOOTLOADER_ORIGIN "${FLASH_XIP_BASE} + 0")
# math(EXPR FLASH_BOOTLOADER_LENGTH "4096 * 10")

# math(EXPR FLASH_HEADER_ORIGIN "${FLASH_BOOTLOADER_ORIGIN} + ${FLASH_BOOTLOADER_LENGTH}")
# math(EXPR FLASH_HEADER_LENGTH "4096")

# math(EXPR FLASH_MAIN_ORIGIN "${FLASH_HEADER_ORIGIN} + ${FLASH_HEADER_LENGTH} + 4096")
# math(EXPR FLASH_MAIN_LENGTH "${FLASH_TOTAL_LENGTH} - ${FLASH_BOOTLOADER_LENGTH} - ${FLASH_HEADER_LENGTH}")

add_compile_definitions(FLASH_MAIN_ORIGIN=${FLASH_MAIN_ORIGIN} FLASH_BOOTLOADER_ORIGIN=${FLASH_BOOTLOADER_ORIGIN})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/com_memmap.in.ld ${CMAKE_BINARY_DIR}/com_memmap.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boot_memmap.in.ld ${CMAKE_BINARY_DIR}/boot_memmap.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_memmap.in.ld ${CMAKE_BINARY_DIR}/app_memmap.ld)

set(HEADER_BIN ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_HDR.bin)
set(HEADER_ASM ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_HDR.S)
set(PROJECT_BIN ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(PROJECT_ASM ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.S)

find_package (Python3 REQUIRED COMPONENTS Interpreter)

# add_custom_target(HEADER_BIN DEPENDS ${PARENT_PROJECT_NAME})
# add_custom_command(OUTPUT HEADER_BIN COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/header.py ${PARENT_PROJECT_BIN} ${PARENT_PROJECT_HDR} -a ${FLASH_MAIN_ORIGIN} DEPENDS ${PARENT_PROJECT_NAME})

# add_custom_target(HEADER_ASM DEPENDS HEADER_BIN)
# add_custom_command(OUTPUT HEADER_ASM COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PARENT_PROJECT_HDR} ${PARENT_PROJECT_HDR_ASM} "flash_header" "an" DEPENDS ${PARENT_PROJECT_NAME})

# add_custom_target(PROJECT_ASM DEPENDS ${PROJECT_NAME})
# add_custom_command(OUTPUT PROJECT_ASM COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PROJECT_BIN} ${PROJECT_ASM} "boot3" "ax" DEPENDS ${PROJECT_NAME})
# add_library(bootloader_library INTERFACE)
# add_dependencies(bootloader_library PROJECT_ASM)
# target_link_libraries(bootloader_library INTERFACE PROJECT_ASM)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/header.py -a ${FLASH_MAIN_ORIGIN} ${PARENT_PROJECT_BIN} ${HEADER_BIN}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${HEADER_BIN} ${HEADER_ASM} "flash_header" "an"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PROJECT_BIN} ${PROJECT_ASM} "boot3" "ax"
    # COMMAND ${CMAKE_READELF} -e ${PARENT_PROJECT_NAME}.elf
    COMMAND ${CMAKE_OBJCOPY} bootloader/bootloader.elf -R RAM -R data -R ram_vector_table -R bss --output-target=elf32-littlearm bootloader/bootloader_flash.bin # --input-target=binary --binary-architecture=arm 
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py bootloader/bootloader_flash.bin ${PROJECT_ASM} "boot3" "ax"
    COMMAND ${CMAKE_OBJCOPY} --output-target=elf32-littlearm --update-section .boot3=${PROJECT_ASM} --update-section .flash_header=${HEADER_ASM} ${PARENT_PROJECT_NAME}.elf ${PARENT_PROJECT_NAME}_COM.elf # --input-target=binary --binary-architecture=arm 
    # COMMAND ${CMAKE_OBJCOPY} --output-target=elf32-littlearm --update-section .boot3="bootloader/bootloader_flash.bin" ${PARENT_PROJECT_NAME}_COM2.elf ${PARENT_PROJECT_NAME}_COM.elf # --input-target=binary --binary-architecture=arm 
    # COMMAND ${CMAKE_OBJCOPY} --output-target=elf32-littlearm --update-section .boot3=${PROJECT_ASM} ${PARENT_PROJECT_NAME}_COM2.elf ${PARENT_PROJECT_NAME}_COM.elf # --input-target=binary --binary-architecture=arm 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(run_post_build DEPENDS ${PROJECT_NAME} HEADER_ASM PROJECT_ASM ${PARENT_PROJECT_NAME})

# "C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\gcc-arm-none-eabi\bin\arm-none-eabi-objcopy.exe" --input-target=binary --output-target=elf32-littlearm --binary-architecture=arm --update-section .flash_header=F:\GitHub\PMPi\PMPi-Firmware\build\bootloader\bootloader_HDR.S F:\GitHub\PMPi\PMPi-Firmware\build\PMPi.elf F:\GitHub\PMPi\PMPi-Firmware\build\PMPi_COM.elf

# add_custom_target(BOOTLOADER_FINAL
#     COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PROJECT_BIN} ${PROJECT_ASM} "boot3" "ax"
#     COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/header.py ${PARENT_PROJECT_BIN} ${PARENT_PROJECT_HDR} -a ${FLASH_MAIN_ORIGIN}
#     COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/asm.py ${PARENT_PROJECT_HDR} ${PARENT_PROJECT_HDR_ASM} "flash_header" "an"
# )
# add_library(BOOTLOADER_DUMMY INTERFACE)
# add_dependencies(BOOTLOADER_DUMMY BOOTLOADER_FINAL)
# # not strictly (or indeed actually) a link library, but this avoids dependency cycle
# target_link_libraries(BOOTLOADER_DUMMY INTERFACE ${PROJECT_NAME})
# # add_dependencies(${PROJECT_NAME} BOOTLOADER_FINAL)

# ---- Misc. Configuration ----

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# Select linker script for both bootloader and application
pico_set_linker_script(${PARENT_PROJECT_NAME} "${CMAKE_BINARY_DIR}/com_memmap.ld")
pico_set_linker_script(${PROJECT_NAME} "${CMAKE_BINARY_DIR}/boot_memmap.ld")

# Print out flash usage for fun
# usage_print(${PROJECT_NAME} ${PROJECT_BIN} "${FLASH_BOOTLOADER_LENGTH}" "BOOTLOADER FLASH")
# usage_print(BOOTLOADER_ASM ${PARENT_PROJECT_HDR} "${FLASH_HEADER_LENGTH}" "FLASH HEADER")
# usage_print(${PARENT_PROJECT_NAME} ${PARENT_PROJECT_BIN} "${FLASH_MAIN_LENGTH}" "MAIN FLASH")
