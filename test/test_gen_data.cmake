cmake_minimum_required (VERSION 3.19)

# include the file that contains the function to be tested
include (${FUNCTION_FILE})

set (PROJECT_BINARY_DIR "${CMAKE_SOURCE_DIR}/cmake_testing")

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
