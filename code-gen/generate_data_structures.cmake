include_guard (GLOBAL)

macro (code_gen_base_dir)
	get_directory_property (LISTFILE_STACK LISTFILE_STACK)
	list (POP_BACK LISTFILE_STACK _LIST_FILE)
	cmake_path (GET _LIST_FILE PARENT_PATH CODE_GEN_BASE_DIR)
endmacro ()

set (
	GEN_DATA_CALL_COUNT
	0
	CACHE INTERNAL "Internal variable counting the number of calls to generate_data_structures" FORCE
)

#[=======================================================================[.rst:
..function:: generate_data_structures

	Generate data structures from an IDL file using poly-scribe-code-gen.

	:param DEV_MODE: Whether to run in development mode (optional, defaults to OFF).
	:type DEV_MODE: bool
	:param IDL_FILE: Path to the IDL file.
	:type IDL_FILE: str
	:param OUTPUT_FILE: Name of the output file (optional).
	:type OUTPUT_FILE: str
	:param AUTHOR_NAME: Name of the author (optional).
	:type AUTHOR_NAME: str
	:param AUTHOR_MAIL: Email of the author (optional).
	:type AUTHOR_MAIL: str
	:param NAMESPACE: Namespace for the generated code (optional).
	:type NAMESPACE: str
	:param LICENCE: Licence for the generated code (optional).
	:type LICENCE: str

	This function generates data structures from the provided IDL file using the `poly-scribe-code-gen` tool.
	It accepts various options such as whether to run in development mode (`DEV_MODE`), the path to the IDL file (`IDL_FILE`), output file name (`OUTPUT_FILE`), author's name (`AUTHOR_NAME`), author's email (`AUTHOR_MAIL`), namespace for the generated code (`NAMESPACE`), and license for the generated code (`LICENCE`).

	Example usage::

	   generate_data_structures(
		  DEV_MODE ON
		  IDL_FILE "path/to/idl_file.idl"
		  OUTPUT_FILE "output_file.hpp"
		  AUTHOR_NAME "John Doe"
		  AUTHOR_MAIL "john@example.com"
		  NAMESPACE "my_namespace"
		  LICENCE "MIT"
	   )

	.. note:: Todo

		add option to generate in build dir or source dir, if source dir check if exists?

#]=======================================================================]
function (generate_data_structures TARGET_LIBRARY)
	set (options)
	set (
		oneValueArgs
		DEV_MODE
		IDL_FILE
		AUTHOR_NAME
		AUTHOR_MAIL
		NAMESPACE
		LICENCE
		USE_IN_SOURCE
		IN_SOURCE_PATH
		OUTPUT_HEADER_DIR
		OUTPUT_CPP
		OUTPUT_MATLAB
		OUTPUT_MATLAB_VAR
	)
	set (multiValueArgs)
	cmake_parse_arguments (GEN_DATA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# further input parsing
	if (NOT GEN_DATA_USE_IN_SOURCE AND GEN_DATA_IN_SOURCE_PATH)
		set (GEN_DATA_USE_IN_SOURCE ON)
	endif ()

	if (GEN_DATA_USE_IN_SOURCE AND NOT GEN_DATA_IN_SOURCE_PATH)
		set (GEN_DATA_IN_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
	endif ()

	if (GEN_DATA_DEV_MODE)
		set (GEN_DATA_USE_IN_SOURCE OFF)
	endif ()

	set (GEN_DATA_NEEDS_GENERATION OFF)

	if (GEN_DATA_USE_IN_SOURCE
		AND NOT GEN_DATA_DEV_MODE
		AND GEN_DATA_OUTPUT_CPP
		AND NOT EXISTS "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_CPP}"
	)
		message (
			WARNING
				"USE_IN_SOURCE is enabled, but \"${GEN_DATA_OUTPUT_CPP}\" does not exist. We will try to generate it."
		)
		set (GEN_DATA_NEEDS_GENERATION ON)
	endif ()

	if (GEN_DATA_USE_IN_SOURCE
		AND GEN_DATA_OUTPUT_MATLAB
		AND NOT GEN_DATA_DEV_MODE
	)
		if (NOT EXISTS "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_MATLAB}")
			message (
				WARNING
					"USE_IN_SOURCE is enabled, but the folder \"${GEN_DATA_OUTPUT_MATLAB}\" does not exist. We will try to generate it."
			)
			set (GEN_DATA_NEEDS_GENERATION ON)
		else ()
			file (GLOB matlab_files "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_MATLAB}/*.m")
			list (LENGTH matlab_files matlab_file_count)

			if (${matlab_file_count} EQUAL 0)
				message (
					WARNING
						"USE_IN_SOURCE is enabled, but the folder \"${GEN_DATA_OUTPUT_MATLAB}\" is empty. We will try to generate it."
				)
				set (GEN_DATA_NEEDS_GENERATION ON)
			endif ()
		endif ()
	endif ()

	if (NOT GEN_DATA_USE_IN_SOURCE)
		set (GEN_DATA_NEEDS_GENERATION ON)
	endif ()

	set (GEN_DATA_INCLUDE_DIR ${GEN_DATA_IN_SOURCE_PATH})

	if (GEN_DATA_NEEDS_GENERATION)
		find_package (Python3 COMPONENTS Interpreter)

		if (NOT Python3_FOUND)
			message (FATAL_ERROR "Python not found, cannot generate data structures")
		endif ()

		code_gen_base_dir ()

		get_filename_component (GEN_DATA_IDL_FILE_NAME "${GEN_DATA_IDL_FILE}" NAME_WE)

		set (GEN_DATA_OUTPUT_BASE_DIR ${PROJECT_BINARY_DIR}/poly_gen/${GEN_DATA_IDL_FILE_NAME})

		set (ADDITIONAL_DATA "{}")
		set (GEN_DATA_AUTHOR_NAME)
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_name" "\"${GEN_DATA_AUTHOR_NAME}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_email" "\"${GEN_DATA_AUTHOR_MAIL}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "licence" "\"${GEN_DATA_LICENCE}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "namespace" "\"${GEN_DATA_NAMESPACE}\"")

		set (ADDITIONAL_DATA_FILE ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_IDL_FILE_NAME}.json)
		file (WRITE ${ADDITIONAL_DATA_FILE} ${ADDITIONAL_DATA})

		get_filename_component (POLY_SCRIBE_BASE_DIR "${CODE_GEN_BASE_DIR}/.." REALPATH)

		include (${POLY_SCRIBE_BASE_DIR}/cmake/python_venv.cmake)

		setup_and_activate_python_venv ("venv-code-gen")

		execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip list OUTPUT_VARIABLE rv)

		if (${CMAKE_SOURCE_DIR} STREQUAL ${POLY_SCRIBE_BASE_DIR} AND GEN_DATA_DEV_MODE)
			set (POLY_SCRIBE_DEV_MODE TRUE)
		endif ()

		if ((NOT rv MATCHES poly-scribe-code-gen OR POLY_SCRIBE_DEV_MODE) AND ${GEN_DATA_CALL_COUNT} EQUAL 0)
			math (EXPR GEN_DATA_CALL_COUNT "${GEN_DATA_CALL_COUNT}+1")
			set (
				GEN_DATA_CALL_COUNT
				${GEN_DATA_CALL_COUNT}
				CACHE INTERNAL "Internal variable counting the number of calls to generate_data_structures" FORCE
			)
			execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install "${CODE_GEN_BASE_DIR}/.")
		endif ()

		get_filename_component (GEN_DATA_IDL_FILE "${GEN_DATA_IDL_FILE}" REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

		if (GEN_DATA_OUTPUT_CPP)
			set (GEN_DATA_CPP_ARG --cpp
								  ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_CPP}
			)
		endif ()

		if (GEN_DATA_OUTPUT_MATLAB)
			set (GEN_DATA_MATLAB_ARG --matlab ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_MATLAB})
		endif ()

		execute_process (
			COMMAND "${Python3_EXECUTABLE}" -m poly_scribe_code_gen -a ${ADDITIONAL_DATA_FILE} ${GEN_DATA_CPP_ARG}
					${GEN_DATA_MATLAB_ARG} ${GEN_DATA_IDL_FILE}
		)

		deactivate_python_venv ("venv-code-gen")

		if (GEN_DATA_USE_IN_SOURCE)
			if (GEN_DATA_OUTPUT_CPP)
				if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR})
					file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
				endif ()

				file (COPY ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_CPP}
					  DESTINATION ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR}
				)
			endif ()

			if (GEN_DATA_OUTPUT_MATLAB)
				if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_MATLAB})
					file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
				endif ()

				file (COPY ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_MATLAB}/
					  DESTINATION ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_MATLAB}
				)
			endif ()
		endif ()

		if (NOT GEN_DATA_USE_IN_SOURCE)
			set (GEN_DATA_INCLUDE_DIR ${GEN_DATA_OUTPUT_BASE_DIR})
			set (
				${GEN_DATA_OUTPUT_MATLAB_VAR}
				${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_MATLAB}
				PARENT_SCOPE
			)
		endif ()
	endif ()

	get_property (
		type
		TARGET ${TARGET_LIBRARY}
		PROPERTY TYPE
	)

	if (${type} STREQUAL "INTERFACE_LIBRARY")
		target_include_directories (${TARGET_LIBRARY} INTERFACE $<BUILD_INTERFACE:${GEN_DATA_INCLUDE_DIR}>)
		target_link_libraries (${TARGET_LIBRARY} PUBLIC poly-scribe::poly-scribe)
	else ()
		target_include_directories (${TARGET_LIBRARY} PUBLIC $<BUILD_INTERFACE:${GEN_DATA_INCLUDE_DIR}>)
		target_link_libraries (${TARGET_LIBRARY} PUBLIC poly-scribe::poly-scribe)
	endif ()

	# install ( DIRECTORY ${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL_PATH}/ DESTINATION
	# include/${GEN_DATA_HEADER_REL_PATH} COMPONENT "${TARGET_LIBRARY}_Development" )

endfunction ()
