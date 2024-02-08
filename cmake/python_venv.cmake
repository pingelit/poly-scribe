# Function to setup and activate Python virtual environment
macro(setup_and_activate_python_venv VENV_NAME)
    message(STATUS "Setting up and activating Python virtual environment '${VENV_NAME}'")

    if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")
        # Find Python interpreter
        find_package(Python3 COMPONENTS Interpreter)
        
        # Create virtual environment
        execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")
        
        # Unset Python3_EXECUTABLE to prevent it from being used in further calls to find_package
        unset(Python3_EXECUTABLE)
    endif ()

    # Set Python3_FIND_VIRTUALENV to prioritize searching in virtual environment
    set(Python3_FIND_VIRTUALENV FIRST)

    # Set VIRTUAL_ENV environment variable
    set(ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")

    # Find Python interpreter within virtual environment
    find_package(Python3 COMPONENTS Interpreter)

    message(STATUS "Python interpreter path: ${Python3_EXECUTABLE}")
    message(STATUS "Python include directory: ${Python3_INCLUDE_DIR}")
    message(STATUS "Python library: ${Python3_LIBRARY}")
endmacro()

# Function to deactivate Python virtual environment
macro(deactivate_python_venv VENV_NAME)
    message(STATUS "Deactivating Python virtual environment '${VENV_NAME}'")

    # Check if virtual environment was activated before
    if(DEFINED ENV{VIRTUAL_ENV} AND "$ENV{VIRTUAL_ENV}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")
        # Unset VIRTUAL_ENV environment variable
        unset(ENV{VIRTUAL_ENV})
        
        # Find Python interpreter to restore previous settings
        set(Python3_FIND_VIRTUALENV STANDARD)

        unset(Python3_EXECUTABLE)
        find_package(Python3 COMPONENTS Interpreter)

        message(STATUS "Restoring Python interpreter path: ${Python3_EXECUTABLE}")
        message(STATUS "Restoring Python include directory: ${Python3_INCLUDE_DIR}")
        message(STATUS "Restoring Python library: ${Python3_LIBRARY}")
    else()
        message(WARNING "Virtual environment '${VENV_NAME}' not activated. Skipping deactivation.")
    endif()
endmacro()
