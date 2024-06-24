add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE include) # The config file directory
# target_compile_definitions(freertos_config PUBLIC projCOVERAGE_TEST=0)
