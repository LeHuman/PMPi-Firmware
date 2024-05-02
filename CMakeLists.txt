cmake_minimum_required(VERSION 3.14...3.22)

# ---- Pico SDK ----

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# ---- Project ----

project(PMPi C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(PICO_BOARD pico_w)

# Initialize the SDK
pico_sdk_init()

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

# set(USE_CCACHE OFF)
include(get_env.cmake)
include(cmake/CPM.cmake)
include(cmake/tools.cmake)

if (NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
CPMAddPackage(
    NAME freertos_kernel
    GIT_TAG V11.1.0
    GITHUB_REPOSITORY FreeRTOS/FreeRTOS-Kernel
    DOWNLOAD_ONLY YES
)
set(ENV{FREERTOS_KERNEL_PATH} ${freertos_kernel_SOURCE_DIR})
endif()
include(FreeRTOS_Kernel_import.cmake)

CPMAddPackage(
    NAME mongoose
    GIT_TAG 7.13
    GITHUB_REPOSITORY cesanta/mongoose
    DOWNLOAD_ONLY YES
)
file(GLOB mongoose_sources
    # ${mongoose_SOURCE_DIR}/*.h
    ${mongoose_SOURCE_DIR}/*.c
)
# add_library(mongoose INTERFACE ${mongoose_sources})
# target_compile_definitions(mongoose INTERFACE
#     NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
#     LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)

#     MG_ARCH=MG_ARCH_FREERTOS
#     MG_ENABLE_LWIP=1
#     MG_ENABLE_PACKED_FS=1
# )
# target_include_directories(mongoose INTERFACE ${mongoose_SOURCE_DIR})

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.S")
file(GLOB_RECURSE pios CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.pio")

# ---- Create executable ----
add_executable(${PROJECT_NAME} ${headers} ${sources} ${mongoose_sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})

# being a cross-platform target, we enforce standards conformance on MSVC
# target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create C header file with the name <pio program>.pio.h
FOREACH(PIO_FILE ${pios})
    pico_generate_pio_header(${PROJECT_NAME} ${PIO_FILE})
ENDFOREACH()

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
pico_stdlib
hardware_pio
FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)

# add_compile_definitions(PICO_DEOPTIMIZED_DEBUG=1)

# Link Pico W only dependencies
if(PICO_BOARD STREQUAL "pico_w")
    target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_iperf
    )

    read_env_variable(WIFI_SSID "myWifi")
    read_env_variable(WIFI_PASSWORD "1234")
    read_env_variable(HTTP_URL "http://0.0.0.0:8000")
    read_env_variable(HTTPS_URL "http://0.0.0.0:8443")

    set(WIFI_COMPILE_OPTIONS -DWIFI_SSID=\"${WIFI_SSID}\" -DWIFI_PASSWORD=\"${WIFI_PASSWORD}\" -DHTTP_URL=\"${HTTP_URL}\" -DHTTPS_URL=\"${HTTPS_URL}\")
    target_compile_options(${PROJECT_NAME} PRIVATE ${WIFI_COMPILE_OPTIONS})

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)

        MG_ARCH=MG_ARCH_FREERTOS
        MG_ENABLE_LWIP=1
        MG_ENABLE_PACKED_FS=1
    )
endif(PICO_BOARD STREQUAL "pico_w")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bootloader)
bootloader_attach(${PROJECT_NAME})
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${mongoose_SOURCE_DIR}) # TODO: Move mongoose into library
