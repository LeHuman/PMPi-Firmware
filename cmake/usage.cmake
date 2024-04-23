cmake_minimum_required(VERSION 3.14...3.22)

# Calculate the usage percentage of a file in relation to a maximum size in bytes and return a formatted visual
# TODO: Use platform specific commands to get file size
function(get_file_usage var filename max_sz)
    # Function to duplicate a character a certain number of times
    function(duplicate_char input_char count output_var)
        set(output "")
        math(EXPR iterations "${count}-1")
        foreach(_ RANGE ${iterations})
            set(output "${output}${input_char}")
        endforeach()
        set(${output_var} "${output}" PARENT_SCOPE)
    endfunction()

    file(READ "${filename}" content HEX)
    string(LENGTH "${content}" content_length)

    math(EXPR content_length "${content_length} / 2")
    math(EXPR content_length "10000 * ${content_length} / ${max_sz}")
    string(SUBSTRING ${content_length} 0 1 ticks)
    string(SUBSTRING ${content_length} 0 2 int)
    string(SUBSTRING ${content_length} 2 2 float)
    duplicate_char("=" "${ticks}" flash_bar)
    math(EXPR ticks "10-${ticks}")
    duplicate_char(" " "${ticks}" flash_bar_empty)
    math(EXPR max_sz "${max_sz}/1024")

    set(${var} "(${max_sz}k) : [${flash_bar}${flash_bar_empty}] ${int}.${float}%" PARENT_SCOPE)
endfunction()

if(DEFINED USAGE_PATH_TO_FILE AND EXISTS "${USAGE_PATH_TO_FILE}")
    get_file_usage(__usage_bar ${USAGE_PATH_TO_FILE} ${USAGE_FILE_MAX_SIZE})
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo "\n${USAGE_PREFIX} ${__usage_bar}\n")
endif(DEFINED USAGE_PATH_TO_FILE AND EXISTS "${USAGE_PATH_TO_FILE}")

function(usage_print usage_project_name usage_file_path usage_max_size usage_output_prefix)
    add_custom_target("USAGE_PRINT_${usage_project_name}"
        COMMAND ${CMAKE_COMMAND} 
            -DTARGET_NAME=${usage_project_name}
            # -DTARGET_PATH=${CMAKE_CURRENT_SOURCE_DIR}
            -DUSAGE_PATH_TO_FILE=${usage_file_path}
            -DUSAGE_FILE_MAX_SIZE=${usage_max_size}
            -DUSAGE_PREFIX=${usage_output_prefix}
            -P ${CMAKE_SOURCE_DIR}/cmake/usage.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_dependencies(${usage_project_name} "USAGE_PRINT_${usage_project_name}")
endfunction()
