add_library(common_api INTERFACE)
target_include_directories(common_api SYSTEM INTERFACE include)
target_link_libraries(common_api INTERFACE FreeRTOS-Kernel FreeRTOS-Kernel-Heap4 hardware_clocks pico_stdlib) # TODO: External heap selection
# target_compile_definitions(freertos_config PUBLIC projCOVERAGE_TEST=0)
