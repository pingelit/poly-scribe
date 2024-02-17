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
	DEV_MODE
	FALSE
	IDL_FILE
	integration.webidl
	OUTPUT_NAME
	"integration_data.h"
	AUTHOR_NAME
	"Pascal"
	AUTHOR_MAIL
	"ppa@akustik.rwth-aachen.de"
	NAMESPACE
	"integration_space"
	LICENCE
	"MIT"
	HEADER_DIR_VAR
	GENERATION
)

target_include_directories (
	tests PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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
