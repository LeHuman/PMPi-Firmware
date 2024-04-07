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
    math(EXPR max_sz "${max_sz}/4096")

    set(${var} "(${max_sz}k) : [${flash_bar}${flash_bar_empty}] ${int}.${float}%" PARENT_SCOPE)
endfunction()

if(DEFINED USAGE_PATH_TO_FILE AND EXISTS "${USAGE_PATH_TO_FILE}")
    get_file_usage(__usage_bar ${USAGE_PATH_TO_FILE} ${USAGE_FILE_MAX_SIZE})
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo "\n${USAGE_PREFIX}${__usage_bar}\n")
endif(DEFINED USAGE_PATH_TO_FILE AND EXISTS "${USAGE_PATH_TO_FILE}")