include_guard ()

# Check for CPM
file (GLOB CPM_MODULE_LOCATIONS ${CMAKE_BINARY_DIR}/cmake/CPM*.cmake)

if (NOT CPM_MODULE_LOCATIONS)
	set (CPM_DOWNLOAD_VERSION 0.40.8)
	set (CPM_MODULE_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

	message (STATUS "Downloading CPM.cmake")
	file (DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
		  ${CPM_MODULE_LOCATION}
	)
else ()
	list (GET CPM_MODULE_LOCATIONS -1 CPM_MODULE_LOCATION)
endif ()

include (${CPM_MODULE_LOCATION})
