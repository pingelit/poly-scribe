include_guard ()

#[=======================================================================[.rst:
..command:: target_enable_clang_tidy

	Enable clang-tidy static analyser for the given target.
	Optionally treat warnings as errors.

	The implementation is based on the implementation of https://github.com/aminya/project_options

	::

		target_enable_clang_tidy(target_name ON)

#]=======================================================================]
macro (target_enable_clang_tidy TARGET WARNINGS_AS_ERRORS)
	find_program (CLANGTIDY clang-tidy)
	if (CLANGTIDY)

		# construct the clang-tidy command line
		set (CLANG_TIDY_CALL ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)

		# clang-tidy fails with MSVC since the exception handling option gets striped see
		# https://gitlab.kitware.com/cmake/cmake/-/issues/20512
		if (MSVC)
			list (APPEND CLANG_TIDY_CALL -extra-arg=/EHsc)
		endif ()

		# set warnings as errors
		if (${WARNINGS_AS_ERRORS})
			list (APPEND CLANG_TIDY_CALL -warnings-as-errors=*)
		endif ()

		# set C++ standard
		if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
			if ("${CMAKE_CXX_CLANG_TIDY_DRIVER_MODE}" STREQUAL "cl")
				set (CLANG_TIDY_CALL ${CLANG_TIDY_CALL} -extra-arg=/std:c++${CMAKE_CXX_STANDARD})
			else ()
				set (CLANG_TIDY_CALL ${CLANG_TIDY_CALL} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
			endif ()
		endif ()

		set_target_properties (${TARGET} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CALL}")
	else ()
		message (WARNING "clang-tidy requested but executable not found")
	endif ()
endmacro ()
