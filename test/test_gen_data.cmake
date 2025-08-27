cmake_minimum_required (VERSION 3.19)

# include the file that contains the function to be tested
include (${FUNCTION_FILE})

set (PROJECT_BINARY_DIR "${CMAKE_SOURCE_DIR}/cmake_testing")
set (inital_source_dir "${CMAKE_SOURCE_DIR}")
set (CMAKE_SOURCE_DIR "${PROJECT_BINARY_DIR}/cmake_testing_source")
set (CMAKE_CURRENT_SOURCE_DIR "${CMAKE_SOURCE_DIR}")

# ######################################################################################################################
# Test the generate_data_structures function with a simple webidl file We use the standard mode and expect the files to
# be generated in the binary directory. We also test that C++, Python and schema files are generated correctly.
# Furthermore, we test that the author_name, author_mail, namespace and licence are set correctly in the generated JSON
# file.
# ######################################################################################################################

set (expected_author_name "Test Author")
set (expected_author_email "test@test.com")
set (expected_namespace "TestNamespace")
set (expected_licence "MIT")

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	OUTPUT_PYTHON_PKG
	python/integration_data
	OUTPUT_PYTHON_PKG_VAR
	PYTHON_PKG_GENERATED
	OUTPUT_SCHEMA
	schema/integration_schema.json
	OUTPUT_SCHEMA_VAR
	SCHEMA_GENERATED
	OUTPUT_SCHEMA_CLASS
	IntegrationTest
)

# Check if the file PROJECT_BINARY_DIR/poly_gen/integration/integration.json exists
set (expected_json_file "${PROJECT_BINARY_DIR}/poly_gen/integration/integration.json")
if (NOT EXISTS "${expected_json_file}")
	message (SEND_ERROR "Expected JSON file does not exist: ${expected_json_file}")
endif ()

# open the generated JSON file and check its content, author_name, author_mail, namespace, licence should be set
# correctly
file (READ "${expected_json_file}" json_content)
string (JSON author_email GET "${json_content}" author_email)
string (JSON author_name GET "${json_content}" author_name)
string (JSON licence GET "${json_content}" licence)
string (JSON package_name GET "${json_content}" package)

# Check if the values are as expected
if (NOT author_email STREQUAL "${expected_author_email}")
	message (SEND_ERROR "Expected author_email to be '${expected_author_email}', but got '${author_email}'")
endif ()
if (NOT author_name STREQUAL "${expected_author_name}")
	message (SEND_ERROR "Expected author_name to be '${expected_author_name}', but got '${author_name}'")
endif ()
if (NOT licence STREQUAL "${expected_licence}")
	message (SEND_ERROR "Expected licence to be '${expected_licence}', but got '${licence}'")
endif ()
if (NOT package_name STREQUAL "${expected_namespace}")
	message (SEND_ERROR "Expected package_name to be '${expected_namespace}', but got '${package_name}'")
endif ()

# Check if the generated header file exists
set (expected_header_file "${PROJECT_BINARY_DIR}/poly_gen/integration/integration/integration.hpp")
if (NOT EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected header file does not exist: ${expected_header_file}")
endif ()

# Check if the generated Python package exists
set (expected_python_package "${PROJECT_BINARY_DIR}/poly_gen/integration/python/integration_data")
if (NOT EXISTS "${expected_python_package}")
	message (SEND_ERROR "Expected Python package does not exist: ${expected_python_package}")
endif ()

# Check if the generated Python package contains the expected __init__.py file
set (expected_python_init_file "${expected_python_package}/src/${expected_namespace}/__init__.py")
if (NOT EXISTS "${expected_python_init_file}")
	message (SEND_ERROR "Expected Python __init__.py file does not exist: ${expected_python_init_file}")
endif ()

# Check if the generated schema file exists
set (expected_schema_file "${PROJECT_BINARY_DIR}/poly_gen/integration/schema/integration_schema.json")
if (NOT EXISTS "${expected_schema_file}")
	message (SEND_ERROR "Expected schema file does not exist: ${expected_schema_file}")
endif ()

# todo test that the output vars are set correctly

# delete all files under PROJECT_BINARY_DIR/
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")

# ######################################################################################################################
# Test the generate_data_structures function with a simple webidl file in dev mode We use the dev mode and expect the
# files to be generated in the binary directory.
# ######################################################################################################################

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	DEV_MODE
	ON
)

# Check if the generated header file exists in dev mode
if (NOT EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected header file does not exist in dev mode: ${expected_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")

# ######################################################################################################################
# Test the generate_data_structures function with a simple webidl file with USE_IN_SOURCE set to TRUE We expect the
# files to be generated in the bin directory and then copied to the source directory as they don't exist there yet. Then
# we delete the files in the binary directory. In a second run, the files should do not have to be generated again in
# the binary directory as they already exist in the source directory.
# ######################################################################################################################

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	USE_IN_SOURCE
	TRUE
)

# Check if the generated header file exists in the source directory
set (expected_source_header_file "${CMAKE_SOURCE_DIR}/integration/integration.hpp")
if (NOT EXISTS "${expected_source_header_file}")
	message (SEND_ERROR "Expected source header file does not exist: ${expected_source_header_file}")
endif ()

# Check if the generated header file exists in the binary directory
if (NOT EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected binary header file does not exist: ${expected_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	USE_IN_SOURCE
	TRUE
)

# Check if the generated header file exists in the source directory
if (NOT EXISTS "${expected_source_header_file}")
	message (SEND_ERROR "Expected source header file does not exist: ${expected_source_header_file}")
endif ()

# Check if the generated header file exists in the binary directory
if (EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected binary header file does exist but should not: ${expected_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${CMAKE_SOURCE_DIR}")

# ######################################################################################################################
# Test the function with USE_IN_SOURCE set to TRUE and DEV_MODE set to TRUE, we expect the files to be generated in the
# bin directory and then copied to the source directory as they don't exist there yet. In a second run, the files should
# again be generated in the binary directory when they were deleted after the first run. No matter if the files exist in
# the source directory or not.
# ######################################################################################################################

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	USE_IN_SOURCE
	TRUE
	DEV_MODE
	ON
)

# Check if the generated header file exists in the source directory
if (NOT EXISTS "${expected_source_header_file}")
	message (SEND_ERROR "Expected source header file does not exist: ${expected_source_header_file}")
endif ()

# Check if the generated header file exists in the binary directory
if (NOT EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected binary header file does not exist: ${expected_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	USE_IN_SOURCE
	TRUE
	DEV_MODE
	ON
)

# Check if the generated header file exists in the source directory
if (NOT EXISTS "${expected_source_header_file}")
	message (SEND_ERROR "Expected source header file does not exist: ${expected_source_header_file}")
endif ()

# Check if the generated header file exists in the binary directory
if (NOT EXISTS "${expected_header_file}")
	message (SEND_ERROR "Expected binary header file does not exist: ${expected_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${CMAKE_SOURCE_DIR}")

# remove the generated header file
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")

# ######################################################################################################################
# Test the generate_data_structures function with a simple webidl file with an absolute in source path
# ######################################################################################################################

set (expected_absolute_source_dir "${CMAKE_SOURCE_DIR}/absolute_in_source")

generate_data_structures (
	null_lib
	IDL_FILE
	"${CMAKE_CURRENT_LIST_DIR}/integration.webidl"
	AUTHOR_NAME
	${expected_author_name}
	AUTHOR_MAIL
	${expected_author_email}
	NAMESPACE
	${expected_namespace}
	LICENCE
	${expected_licence}
	OUTPUT_HEADER_DIR
	"integration"
	OUTPUT_CPP
	"integration.hpp"
	USE_IN_SOURCE
	TRUE
	IN_SOURCE_PATH
	"${expected_absolute_source_dir}"
)

# Check if the generated header file exists in the absolute source directory
set (expected_absolute_source_header_file "${expected_absolute_source_dir}/integration/integration.hpp")
if (NOT EXISTS "${expected_absolute_source_header_file}")
	message (SEND_ERROR "Expected absolute source header file does not exist: ${expected_absolute_source_header_file}")
endif ()

# remove the generated header file
file (REMOVE_RECURSE "${PROJECT_BINARY_DIR}/poly_gen/integration")
file (REMOVE_RECURSE "${expected_absolute_source_dir}")
