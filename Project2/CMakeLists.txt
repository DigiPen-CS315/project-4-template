cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FILE(GLOB SRCFILES "*.h" "*.cpp")

# set the project name
project(HeapDebugger)


# add the executable
add_executable(HeapDebugger ${SRCFILES})

