# Function to read environment variables from a .env file
function(read_env_variable VAR_NAME DEFAULT_VALUE)
    # Check if .env file exists
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env")
        message(WARNING "No .env file found.")
        set(${VAR_NAME} ${DEFAULT_VALUE} PARENT_SCOPE)
        return()
    endif()

    # Read the contents of the .env file
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.env" ENV_CONTENTS)

    # Split the contents into lines
    string(REGEX REPLACE "\n" ";" ENV_LIST "${ENV_CONTENTS}")

    # Initialize variable found flag
    set(VAR_FOUND FALSE)

    # Iterate over each line
    foreach(ENV_LINE ${ENV_LIST})
        # Split each line into variable and value
        string(REGEX MATCH "([^=]+)=(.*)" _ ${ENV_LINE})
        set(ENV_NAME ${CMAKE_MATCH_1})
        set(ENV_VALUE ${CMAKE_MATCH_2})

        # If the variable matches, set its value and exit loop
        if(${ENV_NAME} STREQUAL ${VAR_NAME})
            set(${VAR_NAME} ${ENV_VALUE} PARENT_SCOPE)
            set(VAR_FOUND TRUE)
            break()
        endif()
    endforeach()

    # If variable not found, set default value
    if(NOT VAR_FOUND)
        set(${VAR_NAME} ${DEFAULT_VALUE} PARENT_SCOPE)
        message(STATUS "Variable ${VAR_NAME} not found in .env file. Using default value: ${DEFAULT_VALUE}")
    endif()
endfunction()