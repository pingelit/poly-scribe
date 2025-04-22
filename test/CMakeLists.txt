enable_testing ()

CPMAddPackage (
	NAME catch2
	GITHUB_REPOSITORY catchorg/Catch2
	VERSION 3.5.1
	OPTIONS "CATCH_INSTALL_DOCS OFF" "BUILD_SHARED_LIBS OFF"
)

include (${Catch2_SOURCE_DIR}/extras/Catch.cmake)

add_executable (cpp_gen_compiles cpp_gen_compiles.cpp)
target_link_libraries (cpp_gen_compiles PRIVATE Catch2::Catch2WithMain)

add_executable (lang_integration lang_integration.cpp)

set_target_properties (cpp_gen_compiles lang_integration Catch2 Catch2WithMain PROPERTIES FOLDER "poly-scribe/testing")

add_library (integration_data INTERFACE)

generate_data_structures (
	integration_data
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
	# OUTPUT_MATLAB
	# matlab
	# OUTPUT_MATLAB_VAR
	# MATLAB_GENERATED
	OUTPUT_PYTHON_PKG
	integration_data
	OUTPUT_PYTHON_PKG_VAR
	PYTHON_PKG_GENERATED
	OUTPUT_SCHEMA
	integration_schema.json
	OUTPUT_SCHEMA_VAR
	SCHEMA_GENERATED
	OUTPUT_SCHEMA_CLASS
	IntegrationTest
	OUTPUT_HEADER_DIR
	poly-scribe-structs
	DEV_MODE
	TRUE
)

target_link_libraries (cpp_gen_compiles PRIVATE integration_data)
target_link_libraries (lang_integration PRIVATE integration_data)

catch_discover_tests (
	cpp_gen_compiles
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
# find_package (Matlab COMPONENTS MAIN_PROGRAM)

# if (Matlab_FOUND)
# 	matlab_add_unit_test (
# 		NAME
# 		"unittests.matlab"
# 		UNITTEST_FILE
# 		integrationTest.m
# 		ADDITIONAL_PATH
# 		"${MATLAB_GENERATED}"
# 		WORKING_DIRECTORY
# 		${CMAKE_CURRENT_SOURCE_DIR}
# 	)
# endif ()

setup_and_activate_python_venv ("venv-test")

execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install ${PYTHON_PKG_GENERATED})
execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install pytest)

list (APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/venv-test/Scripts")

# partly inspired by https://github.com/python-cmake/pytest-cmake
find_program (PYTEST_EXECUTABLE NAMES pytest)

add_test (
	NAME unittests.python
	COMMAND "${PYTEST_EXECUTABLE}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_tests_properties (
	unittests.python
	PROPERTIES
		ENVIRONMENT
		"SCHEMA_FILE=${SCHEMA_GENERATED};CPP_EXE=$<TARGET_FILE:lang_integration>;TMP_DIR=${CMAKE_CURRENT_BINARY_DIR};MATLAB=${MATLAB_GENERATED}"
)

list (REMOVE_ITEM CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/venv-test/Scripts")

deactivate_python_venv ("venv-test")
