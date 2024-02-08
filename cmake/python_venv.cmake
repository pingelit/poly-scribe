include_guard ()
#[=======================================================================[.rst:
..macro:: setup_and_activate_python_venv

	Setup and activate Python virtual environment.

	:param VENV_NAME: Name of the virtual environment.
	:type VENV_NAME: str

	This macro sets up and activates a Python virtual environment.
	If the virtual environment does not exist, it creates one using the `venv` module.
	It then sets the `VIRTUAL_ENV` environment variable to point to the created virtual environment.
	Additionally, it finds the Python interpreter within the virtual environment and sets the `Python3_EXECUTABLE`, `Python3_INCLUDE_DIR`, and `Python3_LIBRARY` variables.

	Example usage::

	  setup_and_activate_python_venv("venv")

#]=======================================================================]
macro (setup_and_activate_python_venv VENV_NAME)
	message (STATUS "Setting up and activating Python virtual environment '${VENV_NAME}'")

	if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")
		# Find Python interpreter
		find_package (Python3 COMPONENTS Interpreter)

		# Create virtual environment
		execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")

		# Unset Python3_EXECUTABLE to prevent it from being used in further calls to find_package
		unset (Python3_EXECUTABLE)
	endif ()

	# Set Python3_FIND_VIRTUALENV to prioritize searching in virtual environment
	set (Python3_FIND_VIRTUALENV FIRST)

	# Set VIRTUAL_ENV environment variable
	set (ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")

	# Find Python interpreter within virtual environment
	find_package (Python3 COMPONENTS Interpreter)

	message (STATUS "Python interpreter path: ${Python3_EXECUTABLE}")
	message (STATUS "Python include directory: ${Python3_INCLUDE_DIR}")
	message (STATUS "Python library: ${Python3_LIBRARY}")
endmacro ()

#[=======================================================================[.rst:
..macro:: deactivate_python_venv

	Deactivate Python virtual environment.

	:param VENV_NAME: Name of the virtual environment to deactivate.
	:type VENV_NAME: str

	This macro deactivates a previously activated Python virtual environment.
	It first checks if the virtual environment with the given name was activated.
	If it was, it unsets the `VIRTUAL_ENV` environment variable to deactivate the virtual environment.
	Additionally, it restores the previous Python interpreter settings by setting `Python3_FIND_VIRTUALENV` to `STANDARD` and finding the Python interpreter outside the virtual environment.

	Example usage::

	  deactivate_python_venv("venv")

#]=======================================================================]
macro (deactivate_python_venv VENV_NAME)
	message (STATUS "Deactivating Python virtual environment '${VENV_NAME}'")

	# Check if virtual environment was activated before
	if (DEFINED ENV{VIRTUAL_ENV} AND "$ENV{VIRTUAL_ENV}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/${VENV_NAME}")
		# Unset VIRTUAL_ENV environment variable
		unset (ENV{VIRTUAL_ENV})

		# Find Python interpreter to restore previous settings
		set (Python3_FIND_VIRTUALENV STANDARD)

		unset (Python3_EXECUTABLE)
		find_package (Python3 COMPONENTS Interpreter)

		message (STATUS "Restoring Python interpreter path: ${Python3_EXECUTABLE}")
		message (STATUS "Restoring Python include directory: ${Python3_INCLUDE_DIR}")
		message (STATUS "Restoring Python library: ${Python3_LIBRARY}")
	else ()
		message (WARNING "Virtual environment '${VENV_NAME}' not activated. Skipping deactivation.")
	endif ()
endmacro ()
