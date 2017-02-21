cmake_minimum_required (VERSION 3.1)

# Compiler options for gcc on windows via MSYS
if(MSYS OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")
endif()

# TODO replace these relative paths with the proper cmake macros
add_executable(volume_tests volume_tests.cpp ../src/Volume.cpp ../src/Point.cpp)
target_include_directories(volume_tests PRIVATE "../src")
target_link_libraries(volume_tests "gtest")

add_executable(rtree_tests rtree_tests.cpp ../src/Volume.cpp ../src/Point.cpp)
target_include_directories(rtree_tests PRIVATE "../src")
target_link_libraries(rtree_tests "gtest")
