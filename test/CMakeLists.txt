enable_testing ()

CPMAddPackage (
	NAME catch2
	GITHUB_REPOSITORY catchorg/Catch2
	VERSION 3.1.0
	OPTIONS "CATCH_INSTALL_DOCS OFF" "BUILD_SHARED_LIBS OFF"
)

include (${Catch2_SOURCE_DIR}/extras/Catch.cmake)

add_executable (tests dummy.cpp)
target_link_libraries (tests PRIVATE poly-scribe::poly-scribe Catch2::Catch2WithMain)

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
