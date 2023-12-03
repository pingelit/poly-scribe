# Enable cache if available
find_program (CACHE_BINARY NAMES ccache)
if (CACHE_BINARY)
	message (STATUS "ccache found and enabled")
	set (
		CMAKE_CXX_COMPILER_LAUNCHER
		${CACHE_BINARY}
		CACHE FILEPATH "CXX compiler cache used"
	)
	set (
		CMAKE_C_COMPILER_LAUNCHER
		${CACHE_BINARY}
		CACHE FILEPATH "C compiler cache used"
	)
else ()
	message (WARNING "ccache is was not found. Not using it")
endif ()
