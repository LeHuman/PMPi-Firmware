cmake_minimum_required(VERSION 3.14...3.22)

# ---- Pico SDK ----

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# ---- Project ----

project(bootloader C ASM)
set(CMAKE_C_STANDARD 17)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Winline
        -Os
        # -Oz
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

# NOTE: This value should agree with what the linker scripts state
math(EXPR BOOTLOADER_FLASH_SIZE "4096 * 9")
set(MAIN_APP_FLASH_OFFSET ${BOOTLOADER_FLASH_SIZE})
add_compile_definitions(MAIN_APP_FLASH_OFFSET=${MAIN_APP_FLASH_OFFSET} BOOTLOADER_FLASH_SIZE=${BOOTLOADER_FLASH_SIZE})
# set(BOOTLOADER_LINKER_FLAGS "-Wl,--defsym MEME=40096" PARENT_SCOPE)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${BOOTLOADER_LINKER_FLAGS}")

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_memmap.ld)
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_dma hardware_uart hardware_flash hardware_watchdog hardware_pwm)

pico_add_extra_outputs(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

add_custom_target(USAGE_PRINT
    COMMAND ${CMAKE_COMMAND} 
        -DTARGET_NAME=${PROJECT_NAME}
        -DTARGET_PATH=${CMAKE_CURRENT_SOURCE_DIR}
        -DUSAGE_PATH_TO_FILE="${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
        -DUSAGE_FILE_MAX_SIZE="${BOOTLOADER_FLASH_SIZE}"
        -DUSAGE_PREFIX="BOOTLOADER FLASH "
        -P ${CMAKE_SOURCE_DIR}/cmake/usage.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_dependencies(${PROJECT_NAME} USAGE_PRINT)