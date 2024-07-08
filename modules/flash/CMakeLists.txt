cmake_minimum_required(VERSION 3.14)

# ---- Project ----
project(Flash C CXX ASM)

# ---- Add source files ----
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.S")
file(GLOB_RECURSE pios CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.pio")

# ---- Create executable ----
add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})

FOREACH(PIO_FILE ${pios})
    pico_generate_pio_header(${PROJECT_NAME} ${PIO_FILE})
ENDFOREACH()

target_link_libraries(${PROJECT_NAME} PUBLIC pico_stdlib SPI)
