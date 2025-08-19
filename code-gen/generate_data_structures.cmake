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

	Generate data structures from an IDL file using the poly-scribe code generator.

    :param TARGET_LIBRARY: The target library to which the generated data structures should be added.
    :type TARGET_LIBRARY: str
    :param DEV_MODE: If set to ON, the code generator will run in development mode
        and will not check if the generated files already exist.
    :type DEV_MODE: bool
    :param IDL_FILE: The path to the IDL file to be processed.
    :type IDL_FILE: str
    :param AUTHOR_NAME: The name of the author of the IDL file.
    :type AUTHOR_NAME: str
    :param AUTHOR_MAIL: The email of the author of the IDL file.
    :type AUTHOR_MAIL: str
    :param NAMESPACE: The namespace to be used in the generated code.
    :type NAMESPACE: str
    :param LICENCE: The licence to be used in the generated code.
    :type LICENCE: str
    :param USE_IN_SOURCE: If set to ON, the generated files will be copied to the source directory.
    :type USE_IN_SOURCE: bool
    :param IN_SOURCE_PATH: The path to the source directory where the generated files should be copied.
    :type IN_SOURCE_PATH: str
    :param OUTPUT_HEADER_DIR: The directory where the generated header files should be placed.
    :type OUTPUT_HEADER_DIR: str
    :param OUTPUT_CPP: The name of the generated C++ file.
    :type OUTPUT_CPP: str
    :param OUTPUT_MATLAB: The directory where the generated MATLAB files should be placed.
    :type OUTPUT_MATLAB: str
    :param OUTPUT_MATLAB_VAR: The variable name to store the path to the generated MATLAB files.
    :type OUTPUT_MATLAB_VAR: str
    :param OUTPUT_PYTHON: The name of the generated Python file.
    :type OUTPUT_PYTHON: str
    :param OUTPUT_PYTHON_VAR: The variable name to store the path to the generated Python file.
    :type OUTPUT_PYTHON_VAR: str
    :param OUTPUT_PYTHON_PKG: The name of the generated Python package.
    :type OUTPUT_PYTHON_PKG: str
    :param OUTPUT_PYTHON_PKG_VAR: The variable name to store the path to
        the generated Python package.
    :type OUTPUT_PYTHON_PKG_VAR: str
    :param OUTPUT_SCHEMA: The name of the generated schema file.
    :type OUTPUT_SCHEMA: str
    :param OUTPUT_SCHEMA_CLASS: The class name to be used in the generated schema file.
    :type OUTPUT_SCHEMA_CLASS: str
    :param OUTPUT_SCHEMA_VAR: The variable name to store the path to the generated schema file.
    :type OUTPUT_SCHEMA_VAR: str
    :return: None
    :rtype: None

    .. example::

        generate_data_structures(
            my_library
            DEV_MODE ON
            IDL_FILE my_idl_file.idl
            AUTHOR_NAME "John Doe"
            AUTHOR_MAIL "john@doe.com"
            NAMESPACE "my_namespace"
            LICENCE "MIT"
            USE_IN_SOURCE ON
            IN_SOURCE_PATH "src/my_namespace"
            OUTPUT_HEADER_DIR "include/my_namespace"
            OUTPUT_CPP "my_data_structures.h"
        )

	.. note:: in_source and dev_mode
		| in_source | dev_mode | result
		|-----------|----------|-------------------------------------------|
		| ON        | ON       | generate in build and copy to source      |
		| ON        | OFF      | check if in source, generate if necessary |
		| OFF       | ON       | generate in build                         |
		| OFF       | OFF      | generate in build                         |

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
		OUTPUT_PYTHON
		OUTPUT_PYTHON_VAR
		OUTPUT_PYTHON_PKG
		OUTPUT_PYTHON_PKG_VAR
		OUTPUT_SCHEMA
		OUTPUT_SCHEMA_CLASS
		OUTPUT_SCHEMA_VAR
	)
	set (multiValueArgs)
	cmake_parse_arguments (GEN_DATA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# further input parsing
	if (NOT GEN_DATA_USE_IN_SOURCE AND GEN_DATA_IN_SOURCE_PATH)
		set (GEN_DATA_USE_IN_SOURCE ON)
	endif ()

	# if the GEN_DATA_IN_SOURCE_PATH is not absolute, we assume it is relative to the current source dir
	if (GEN_DATA_IN_SOURCE_PATH AND NOT IS_ABSOLUTE "${GEN_DATA_IN_SOURCE_PATH}")
		if (GEN_DATA_IN_SOURCE_PATH STREQUAL ".")
			set (GEN_DATA_IN_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
		else ()
			set (GEN_DATA_IN_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${GEN_DATA_IN_SOURCE_PATH})
		endif ()
	endif ()

	if (GEN_DATA_USE_IN_SOURCE AND NOT GEN_DATA_IN_SOURCE_PATH)
		set (GEN_DATA_IN_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
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
		AND NOT GEN_DATA_DEV_MODE
		AND GEN_DATA_OUTPUT_PYTHON
		AND NOT EXISTS "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_PYTHON}"
	)
		message (
			WARNING
				"USE_IN_SOURCE is enabled, but \"${GEN_DATA_OUTPUT_PYTHON}\" does not exist. We will try to generate it."
		)
		set (GEN_DATA_NEEDS_GENERATION ON)
	endif ()

	if (GEN_DATA_USE_IN_SOURCE
		AND NOT GEN_DATA_DEV_MODE
		AND GEN_DATA_OUTPUT_SCHEMA
		AND NOT EXISTS "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_SCHEMA}"
	)
		message (
			WARNING
				"USE_IN_SOURCE is enabled, but \"${GEN_DATA_OUTPUT_SCHEMA}\" does not exist. We will try to generate it."
		)
		set (GEN_DATA_NEEDS_GENERATION ON)
	endif ()

	if (GEN_DATA_OUTPUT_SCHEMA AND NOT GEN_DATA_OUTPUT_SCHEMA_CLASS)
		message (FATAL_ERROR "OUTPUT_SCHEMA_CLASS must be provided if OUTPUT_SCHEMA is set")
	endif ()

	if (GEN_DATA_OUTPUT_SCHEMA AND NOT (GEN_DATA_OUTPUT_PYTHON OR GEN_DATA_OUTPUT_PYTHON_PKG))
		message (FATAL_ERROR "OUTPUT_PYTHON or OUTPUT_PYTHON_PKG must be provided if OUTPUT_SCHEMA is set")
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

	if (GEN_DATA_NEEDS_GENERATION OR GEN_DATA_DEV_MODE)
		find_package (Python3 COMPONENTS Interpreter)

		if (NOT Python3_FOUND)
			message (FATAL_ERROR "Python not found, cannot generate data structures")
		endif ()

		code_gen_base_dir ()

		get_filename_component (GEN_DATA_IDL_FILE_NAME "${GEN_DATA_IDL_FILE}" NAME_WE)

		set (GEN_DATA_OUTPUT_BASE_DIR ${PROJECT_BINARY_DIR}/poly_gen/${GEN_DATA_IDL_FILE_NAME})

		set (ADDITIONAL_DATA "{}")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_name" "\"${GEN_DATA_AUTHOR_NAME}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_email" "\"${GEN_DATA_AUTHOR_MAIL}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "licence" "\"${GEN_DATA_LICENCE}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "package" "\"${GEN_DATA_NAMESPACE}\"")

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

		if (GEN_DATA_OUTPUT_SCHEMA)
			set (GEN_DATA_SCHEMA_ARG --schema ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_SCHEMA}
									 ${GEN_DATA_OUTPUT_SCHEMA_CLASS}
			)
		endif ()

		if (GEN_DATA_OUTPUT_PYTHON)
			set (GEN_DATA_PYTHON_ARG --py ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON})
		endif ()

		if (GEN_DATA_OUTPUT_PYTHON_PKG)
			set (GEN_DATA_PYTHON_PKG_ARG --py-package ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON_PKG})
		endif ()

		execute_process (
			COMMAND
				"${Python3_EXECUTABLE}" -m poly_scribe_code_gen -a ${ADDITIONAL_DATA_FILE} ${GEN_DATA_CPP_ARG}
				${GEN_DATA_MATLAB_ARG} ${GEN_DATA_PYTHON_ARG} ${GEN_DATA_PYTHON_PKG_ARG} ${GEN_DATA_SCHEMA_ARG}
				${GEN_DATA_IDL_FILE}
			RESULT_VARIABLE result ERROR_VARIABLE error_output
		)

		if (NOT result EQUAL 0)
			message (FATAL_ERROR "Failed to execute poly_scribe_code_gen: ${error_output}")
		endif ()

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

			if (GEN_DATA_OUTPUT_PYTHON)
				if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_PYTHON})
					file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
				endif ()

				file (COPY ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON}
					  DESTINATION ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_PYTHON}
				)
			endif ()

			if (GEN_DATA_OUTPUT_PYTHON_PKG)
				if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_PYTHON_PKG})
					file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
				endif ()

				file (COPY ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON_PKG}
					  DESTINATION ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_PYTHON_PKG}
				)
			endif ()

			if (GEN_DATA_OUTPUT_SCHEMA)
				if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_SCHEMA})
					file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
				endif ()

				get_filename_component (GEN_DATA_OUTPUT_SCHEMA_PARENT_DIR ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_SCHEMA} DIRECTORY)

				file (COPY ${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_SCHEMA}
					  DESTINATION ${GEN_DATA_OUTPUT_SCHEMA_PARENT_DIR}
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
			set (
				${GEN_DATA_OUTPUT_PYTHON_VAR}
				${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON}
				PARENT_SCOPE
			)
			set (
				${GEN_DATA_OUTPUT_PYTHON_PKG_VAR}
				${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_PYTHON_PKG}
				PARENT_SCOPE
			)
			set (
				${GEN_DATA_OUTPUT_SCHEMA_VAR}
				${GEN_DATA_OUTPUT_BASE_DIR}/${GEN_DATA_OUTPUT_SCHEMA}
				PARENT_SCOPE
			)
		endif ()
	endif ()

	if (TARGET ${TARGET_LIBRARY})
		get_property (
			type
			TARGET ${TARGET_LIBRARY}
			PROPERTY TYPE
		)

		if (${type} STREQUAL "INTERFACE_LIBRARY")
			target_include_directories (${TARGET_LIBRARY} INTERFACE $<BUILD_INTERFACE:${GEN_DATA_INCLUDE_DIR}>)
			target_link_libraries (${TARGET_LIBRARY} INTERFACE poly-scribe::poly-scribe)
			target_compile_features (${TARGET_LIBRARY} INTERFACE cxx_std_20)
		else ()
			target_include_directories (${TARGET_LIBRARY} PUBLIC $<BUILD_INTERFACE:${GEN_DATA_INCLUDE_DIR}>)
			target_link_libraries (${TARGET_LIBRARY} PUBLIC poly-scribe::poly-scribe)
			target_compile_features (${TARGET_LIBRARY} PUBLIC cxx_std_20)
		endif ()
	endif ()

	# install ( DIRECTORY ${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL_PATH}/ DESTINATION
	# include/${GEN_DATA_HEADER_REL_PATH} COMPONENT "${TARGET_LIBRARY}_Development" )

endfunction ()
