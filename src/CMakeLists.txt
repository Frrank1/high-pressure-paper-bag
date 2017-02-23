cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_STANDARD 14)

# Compiler options for gcc (tested on windows via MSYS)
if(MSYS OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")

	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto")
	# Not sure if useful in this case, needs testing.
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")
	# For local use builds only
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

endif()

project (low-pressure-riot)

add_executable(graph graph.cpp GasGraph.cpp)
set_target_properties(graph PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(space space.cpp GasGraph.cpp GasSpace.cpp Volume.cpp Point.cpp score.cpp Cluster.cpp)
set_target_properties(space PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
