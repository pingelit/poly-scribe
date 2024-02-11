cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (
	poly-scribe
	LANGUAGES CXX
	VERSION 0.0.1
)

include (cmake/get_cpm.cmake)
include (cmake/ccache.cmake)
include (code-gen/generate_data_structures.cmake)

CPMAddPackage (
	NAME cereal
	VERSION 1.3.2
	GITHUB_REPOSITORY USCiLab/cereal
	OPTIONS "SKIP_PERFORMANCE_COMPARISON ON" "BUILD_DOC OFF" "BUILD_SANDBOX OFF"
)

add_library (${PROJECT_NAME} INTERFACE)
add_library (${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

target_link_libraries (${PROJECT_NAME} INTERFACE cereal::cereal)

target_compile_features (${PROJECT_NAME} INTERFACE cxx_std_17)

if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set_property (TARGET ${PROJECT_NAME} PROPERTY FOLDER poly-scribe)
endif ()

# Don't even look at tests if we're not top-level
if (NOT PROJECT_IS_TOP_LEVEL)
	return ()
endif ()

include (CTest)

add_subdirectory (test)

file (
	CREATE_LINK ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json SYMBOLIC
	COPY_ON_ERROR
)
