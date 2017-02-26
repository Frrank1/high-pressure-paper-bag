cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Compiler options for gcc on windows via MSYS
if(MSYS OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -Og ")
	SET(GCC_COVERAGE_COMPILE_FLAGS "-coverage -fprofile-arcs -ftest-coverage")
	SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
	SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
endif()

# TODO replace these relative paths with the proper cmake macros
add_executable(run_tests run_tests.cpp rtree_tests.cpp volume_tests.cpp ../src/Volume.cpp ../src/Point.cpp ../src/Cluster.cpp)
target_include_directories(run_tests PRIVATE "../src")
target_link_libraries(run_tests "gtest" Threads::Threads)
set_target_properties(run_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
