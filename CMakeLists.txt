cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message("Source Directory ${CMAKE_SOURCE_DIR}")
FILE(GLOB SRCFILES "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/src/*.cpp")
message("Files are ${SRCFILES}")
# set the project name
project(HeapDebugger)


# add the executable
add_executable(HeapDebugger ${SRCFILES})

