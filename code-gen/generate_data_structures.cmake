include_guard ()

macro (code_gen_base_dir)
	get_directory_property (LISTFILE_STACK LISTFILE_STACK)
	list (POP_BACK LISTFILE_STACK _LIST_FILE)
	cmake_path (GET _LIST_FILE PARENT_PATH CODE_GEN_BASE_DIR)
endmacro ()

#[=======================================================================[.rst:
..function:: generate_data_structures

	Generate data structures from an IDL file using poly-scribe-code-gen.

	:param DEV_MODE: Whether to run in development mode (optional, defaults to OFF).
	:type DEV_MODE: bool
	:param IDL_FILE: Path to the IDL file.
	:type IDL_FILE: str
	:param OUTPUT_NAME: Name of the output file (optional).
	:type OUTPUT_NAME: str
	:param AUTHOR_NAME: Name of the author (optional).
	:type AUTHOR_NAME: str
	:param AUTHOR_MAIL: Email of the author (optional).
	:type AUTHOR_MAIL: str
	:param NAMESPACE: Namespace for the generated code (optional).
	:type NAMESPACE: str
	:param LICENCE: Licence for the generated code (optional).
	:type LICENCE: str

	This function generates data structures from the provided IDL file using the `poly-scribe-code-gen` tool.
	It accepts various options such as whether to run in development mode (`DEV_MODE`), the path to the IDL file (`IDL_FILE`), output file name (`OUTPUT_NAME`), author's name (`AUTHOR_NAME`), author's email (`AUTHOR_MAIL`), namespace for the generated code (`NAMESPACE`), and license for the generated code (`LICENCE`).

	Example usage::

	   generate_data_structures(
		  DEV_MODE ON
		  IDL_FILE "path/to/idl_file.idl"
		  OUTPUT_NAME "output_file.hpp"
		  AUTHOR_NAME "John Doe"
		  AUTHOR_MAIL "john@example.com"
		  NAMESPACE "my_namespace"
		  LICENCE "MIT"
	   )

	.. note:: Todo

		add option to generate in build dir or source dir, if source dir check if exists?

#]=======================================================================]
function (generate_data_structures)
	set (options)
	set (
		oneValueArgs
		DEV_MODE
		IDL_FILE
		OUTPUT_NAME
		AUTHOR_NAME
		AUTHOR_MAIL
		NAMESPACE
		LICENCE
		HEADER_DIR_VAR
	)
	set (multiValueArgs)
	cmake_parse_arguments (GEN_DATA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	code_gen_base_dir ()

	set (ADDITIONAL_DATA "{}")
	set (GEN_DATA_AUTHOR_NAME)
	string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_name" "\"${GEN_DATA_AUTHOR_NAME}\"")
	string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "author_email" "\"${GEN_DATA_AUTHOR_MAIL}\"")
	string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "licence" "\"${GEN_DATA_LICENCE}\"")
	string (JSON ADDITIONAL_DATA SET ${ADDITIONAL_DATA} "namespace" "\"${GEN_DATA_NAMESPACE}\"")

	set (ADDITIONAL_DATA_FILE ${PROJECT_BINARY_DIR}/generated/${GEN_DATA_OUTPUT_NAME}.json)
	file (WRITE ${ADDITIONAL_DATA_FILE} ${ADDITIONAL_DATA})

	include (${CODE_GEN_BASE_DIR}/../cmake/python_venv.cmake)

	setup_and_activate_python_venv ("venv-code-gen")

	execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip list OUTPUT_VARIABLE rv)

	if (NOT rv MATCHES poly-scribe-code-gen OR GEN_DATA_DEV_MODE)
		execute_process (COMMAND "${Python3_EXECUTABLE}" -m pip install "${CODE_GEN_BASE_DIR}/.")
	endif ()

	get_filename_component (GEN_DATA_IDL_FILE "${GEN_DATA_IDL_FILE}" REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

	execute_process (
		COMMAND "${Python3_EXECUTABLE}" -m poly_scribe_code_gen -a ${ADDITIONAL_DATA_FILE} -c
				${PROJECT_BINARY_DIR}/generated/${GEN_DATA_OUTPUT_NAME} ${GEN_DATA_IDL_FILE}
	)

	deactivate_python_venv ("venv-code-gen")

	set (
		${GEN_DATA_HEADER_DIR_VAR}
		${PROJECT_BINARY_DIR}/generated
		PARENT_SCOPE
	)

endfunction ()
