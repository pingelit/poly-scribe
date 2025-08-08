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
			"YAML_BUILD_SHARED_LIBS OFF"
)

CPMAddPackage (
	NAME jsoncons
	VERSION 1.3.0
	GITHUB_REPOSITORY danielaparker/jsoncons
	OPTIONS "JSONCONS_BUILD_TESTS OFF"
)

CPMAddPackage (
	NAME reflect-cpp
	GIT_TAG c5f0a241c7ec5b27996144f260660a504fe13ed1 # TODO: update once the new version is released
	GITHUB_REPOSITORY getml/reflect-cpp
	OPTIONS "REFLECTCPP_YAML ON" "REFLECTCPP_CBOR ON" "REFLECTCPP_UBJSON ON" "REFLECTCPP_INSTALL ON"
			"REFLECTCPP_BUILD_SHARED OFF"
	PATCHES "external-patches/reflect-cpp.patch"
)

target_link_libraries (reflectcpp PUBLIC jsoncons)
target_compile_features (reflectcpp PUBLIC cxx_std_20)

set_target_properties (reflectcpp yaml-cpp PROPERTIES FOLDER "poly-scribe/third-party")

add_library (poly-scribe INTERFACE)
add_library (poly-scribe::poly-scribe ALIAS poly-scribe)
target_link_libraries (poly-scribe INTERFACE reflectcpp yaml-cpp jsoncons)
target_include_directories (
	poly-scribe INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

# based on cereal's CMakeLists.txt
include (GNUInstallDirs)
include (CMakePackageConfigHelpers)

install (TARGETS poly-scribe EXPORT poly-scribeTargets)
install (DIRECTORY include/poly-scribe DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set (configFile ${CMAKE_CURRENT_BINARY_DIR}/poly-scribeConfig.cmake)
set (versionFile ${CMAKE_CURRENT_BINARY_DIR}/poly-scribeConfigVersion.cmake)
set (configInstallDestination ${CMAKE_INSTALL_LIBDIR}/cmake/poly-scribe)

configure_package_config_file (
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in ${configFile} INSTALL_DESTINATION ${configInstallDestination}
)
if (${CMAKE_VERSION} VERSION_GREATER 3.13)
	write_basic_package_version_file ("${versionFile}" COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
else ()
	write_basic_package_version_file ("${versionFile}" COMPATIBILITY SameMajorVersion)
endif ()

install (FILES ${configFile} ${versionFile} DESTINATION ${configInstallDestination})
install (
	EXPORT poly-scribeTargets
	NAMESPACE "poly-scribe::"
	DESTINATION ${configInstallDestination}
)

# Don't even look at tests if we're not top-level
if (NOT PROJECT_IS_TOP_LEVEL)
	return ()
endif ()

include (CTest)

add_subdirectory (test)

option (POLY_SCRIBE_BUILD_EXAMPLES "Build examples" ON)

if (POLY_SCRIBE_BUILD_EXAMPLES)
	add_executable (poly-scribe-example docs/assets/example.cpp)
	target_link_libraries (poly-scribe-example poly-scribe)
	target_include_directories (poly-scribe-example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/docs/assets)
	target_compile_features (poly-scribe-example PRIVATE cxx_std_20)
endif ()
