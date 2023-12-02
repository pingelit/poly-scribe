cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (
	poly-scribe
	LANGUAGES CXX
	VERSION 0.0.1
)

include (cmake/get_cpm.cmake)

CPMAddPackage (
	NAME cereal
	VERSION 1.3.2
	GITHUB_REPOSITORY USCiLab/cereal
	OPTIONS "SKIP_PERFORMANCE_COMPARISON ON" "BUILD_DOC OFF" "BUILD_SANDBOX OFF"
)
