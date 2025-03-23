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
	NAME yaml-cpp
	GIT_TAG 0.8.0
	GITHUB_REPOSITORY jbeder/yaml-cpp
	OPTIONS "YAML_CPP_BUILD_TOOLS OFF" "YAML_CPP_INSTALL ON" "YAML_CPP_DISABLE_UNINSTALL ON"
			"YAML_CPP_FORMAT_SOURCE OFF" # some of these options are currently only on the master branch
)

CPMAddPackage (
	NAME jsoncons
	VERSION 1.3.0
	GITHUB_REPOSITORY danielaparker/jsoncons
	OPTIONS "JSONCONS_BUILD_TESTS OFF"
)

CPMAddPackage (
	NAME reflect-cpp
	VERSION 0.18.0
	GITHUB_REPOSITORY getml/reflect-cpp
	OPTIONS "REFLECTCPP_YAML ON" "REFLECTCPP_CBOR ON" "REFLECTCPP_UBJSON ON"
	PATCHES "external-patches/reflect-cpp.patch"
)

target_link_libraries (reflectcpp PUBLIC jsoncons)

set_target_properties (reflectcpp yaml-cpp PROPERTIES FOLDER "poly-scribe/third-party")

add_library (poly-scribe INTERFACE)
add_library (poly-scribe::poly-scribe ALIAS poly-scribe)
target_link_libraries (poly-scribe INTERFACE reflectcpp yaml-cpp jsoncons)
target_include_directories (
	poly-scribe INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

# Don't even look at tests if we're not top-level
if (NOT PROJECT_IS_TOP_LEVEL)
	return ()
endif ()

include (CTest)

add_subdirectory (test)
