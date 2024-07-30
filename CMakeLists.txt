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
	NAME cereal
	VERSION 1.3.2
	GITHUB_REPOSITORY USCiLab/cereal
	OPTIONS "SKIP_PERFORMANCE_COMPARISON ON" "BUILD_DOC OFF" "BUILD_SANDBOX OFF" "CEREAL_INSTALL ON"
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

# based on cereal's CMakeLists.txt
include (GNUInstallDirs)
include (CMakePackageConfigHelpers)

install (TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)
install (DIRECTORY include/poly-scribe DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set (configFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
set (versionFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
set (configInstallDestination ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

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
	EXPORT ${PROJECT_NAME}Targets
	NAMESPACE "poly-scribe::"
	DESTINATION ${configInstallDestination}
)

# Don't even look at tests if we're not top-level
if (NOT PROJECT_IS_TOP_LEVEL)
	return ()
endif ()

include (CTest)

add_subdirectory (test)

if (EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
	file (CREATE_LINK ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json SYMBOLIC
		  COPY_ON_ERROR
	)
endif ()
