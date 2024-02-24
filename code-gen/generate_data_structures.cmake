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
		OUTPUT_FILE
		AUTHOR_NAME
		AUTHOR_MAIL
		NAMESPACE
		LICENCE
		OUTPUT_HEADER_DIR
		IN_SOURCE_PATH
	)
	set (multiValueArgs)
	cmake_parse_arguments (GEN_DATA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (GEN_DATA_IN_SOURCE_PATH
		AND NOT EXISTS "${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_FILE}"
		AND NOT ${GEN_DATA_DEV_MODE}
	)
		message (
			FATAL_ERROR
				"The generated file does not exist in the provided path. And the development mode is off. Please provide a correct path or turn on the development mode so that it can be generated.\n\nPath: ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_FILE}"
		)
	endif ()

	set (GEN_DATA_INCLUDE_DIR ${GEN_DATA_IN_SOURCE_PATH})

	if (${GEN_DATA_DEV_MODE} OR NOT GEN_DATA_IN_SOURCE_PATH)
		code_gen_base_dir ()

		set (GEN_DATA_HEADER_REL ${TARGET_LIBRARY}/${GEN_DATA_OUTPUT_HEADER_DIR}/${GEN_DATA_OUTPUT_FILE})
		cmake_path (NORMAL_PATH GEN_DATA_HEADER_REL)
		cmake_path (GET GEN_DATA_HEADER_REL PARENT_PATH GEN_DATA_HEADER_REL_PATH)

		set (ADDITIONAL_DATA "{}")
		set (GEN_DATA_AUTHOR_NAME)
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_name" "\"${GEN_DATA_AUTHOR_NAME}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_email" "\"${GEN_DATA_AUTHOR_MAIL}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "licence" "\"${GEN_DATA_LICENCE}\"")
		string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "namespace" "\"${GEN_DATA_NAMESPACE}\"")

		set (ADDITIONAL_DATA_FILE ${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL_PATH}/${GEN_DATA_OUTPUT_FILE}.json)
		file (WRITE ${ADDITIONAL_DATA_FILE} ${ADDITIONAL_DATA})

		include (${CODE_GEN_BASE_DIR}/../cmake/python_venv.cmake)

		setup_and_activate_python_venv ("venv-code-gen")

		execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip list OUTPUT_VARIABLE rv)

		if ((NOT rv MATCHES poly-scribe-code-gen OR GEN_DATA_DEV_MODE) AND ${GEN_DATA_CALL_COUNT} EQUAL 0)
			math (EXPR GEN_DATA_CALL_COUNT "${GEN_DATA_CALL_COUNT}+1")
			set (
				GEN_DATA_CALL_COUNT
				${GEN_DATA_CALL_COUNT}
				CACHE INTERNAL "Internal variable counting the number of calls to generate_data_structures" FORCE
			)
			execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install "${CODE_GEN_BASE_DIR}/.")
		endif ()

		get_filename_component (GEN_DATA_IDL_FILE "${GEN_DATA_IDL_FILE}" REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

		execute_process (
			COMMAND "${Python3_EXECUTABLE}" -m poly_scribe_code_gen -a ${ADDITIONAL_DATA_FILE} -c
					${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL} ${GEN_DATA_IDL_FILE}
		)

		deactivate_python_venv ("venv-code-gen")

		if (GEN_DATA_IN_SOURCE_PATH)
			if (NOT EXISTS ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR})
				file (MAKE_DIRECTORY ${GEN_DATA_IN_SOURCE_PATH})
			endif ()

			file (COPY ${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL} DESTINATION ${GEN_DATA_IN_SOURCE_PATH}/${GEN_DATA_OUTPUT_HEADER_DIR})
		endif ()

		set (GEN_DATA_INCLUDE_DIR ${PROJECT_BINARY_DIR}/${TARGET_LIBRARY})
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

	install (
		DIRECTORY ${PROJECT_BINARY_DIR}/${GEN_DATA_HEADER_REL_PATH}/
		DESTINATION include/${GEN_DATA_HEADER_REL_PATH}
		COMPONENT "${TARGET_LIBRARY}_Development"
	)

endfunction ()
