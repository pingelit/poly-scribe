cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project (
	poly-scribe
	LANGUAGES CXX
	VERSION 0.0.1
)

include (cmake/get_cpm.cmake)
include (cmake/ccache.cmake)
include (code-gen/generate_data_structures.cmake)

CPMAddPackage (
	NAME reflect-cpp
	VERSION 0.16.0
	GITHUB_REPOSITORY getml/reflect-cpp
)

add_library (poly-scribe INTERFACE)
add_library (poly-scribe::poly-scribe ALIAS poly-scribe)
target_link_libraries (poly-scribe INTERFACE reflectcpp yaml-cpp)
target_include_directories (
	poly-scribe INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

# Don't even look at tests if we're not top-level
if (NOT PROJECT_IS_TOP_LEVEL)
	return ()
endif ()

include (CTest)

add_subdirectory (test)
