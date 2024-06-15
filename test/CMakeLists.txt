enable_testing ()

CPMAddPackage (
	NAME catch2
	GITHUB_REPOSITORY catchorg/Catch2
	VERSION 3.5.1
	OPTIONS "CATCH_INSTALL_DOCS OFF" "BUILD_SHARED_LIBS OFF"
)

include (${Catch2_SOURCE_DIR}/extras/Catch.cmake)
include (../cmake/clang-tidy.cmake)
include (../cmake/system-link.cmake)

add_executable (
	tests scribe-wrapper.cpp tags.cpp container-wrapper.cpp pointer-wrapper.cpp map-wrapper.cpp integration.cpp
)
target_link_libraries (tests PRIVATE poly-scribe::poly-scribe)
target_link_system_libraries (tests PUBLIC Catch2::Catch2 Catch2::Catch2WithMain cereal::cereal)

target_enable_clang_tidy (tests TRUE)

generate_data_structures (
	tests
	DEV_MODE
	FALSE
	IDL_FILE
	integration.webidl
	OUTPUT_CPP
	"integration_data.hpp"
	AUTHOR_NAME
	"Pascal"
	AUTHOR_MAIL
	"ppa@akustik.rwth-aachen.de"
	NAMESPACE
	"integration_space"
	LICENCE
	"MIT"
	OUTPUT_HEADER_DIR
	poly-scribe-structs
	OUTPUT_MATLAB
	matlab
	OUTPUT_MATLAB_VAR
	MATLAB_GENERATED
	OUTPUT_PYTHON
	integration_data.py
	OUTPUT_PYTHON_VAR
	PYTHON_GENERATED
	OUTPUT_HEADER_DIR
	poly-scribe-structs
)

catch_discover_tests (
	tests
	TEST_PREFIX
	"unittests."
	REPORTER
	XML
	OUTPUT_DIR
	.
	OUTPUT_PREFIX
	"unittests."
	OUTPUT_SUFFIX
	.xml
)

# check if matlab is available
find_package (Matlab COMPONENTS MAIN_PROGRAM)

if (Matlab_FOUND)
	matlab_add_unit_test (
		NAME
		"unittests.matlab"
		UNITTEST_FILE
		integrationTest.m
		ADDITIONAL_PATH
		"${MATLAB_GENERATED}"
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
	)
endif ()

setup_and_activate_python_venv ("venv-test")

execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install -r "${CMAKE_CURRENT_SOURCE_DIR}/test_requirements.txt")

list (APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/venv-test/Scripts")

# partly inspired by https://github.com/python-cmake/pytest-cmake
find_program (PYTEST_EXECUTABLE NAMES pytest)

add_test (
	NAME unittests.python
	COMMAND "${PYTEST_EXECUTABLE}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

get_filename_component (PYTHON_GENERATED_PARENT ${PYTHON_GENERATED} DIRECTORY)

set_tests_properties (unittests.python PROPERTIES ENVIRONMENT "PYTHONPATH=${PYTHON_GENERATED_PARENT}")

list (REMOVE_ITEM CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/venv-test/Scripts")

deactivate_python_venv ("venv-test")
