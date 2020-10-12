cmake_minimum_required(VERSION 3.10)

##########################################################################################
# DO NOT MODIFY                                                                         ##
#                                                                                       ##
# Download and unpack googletest at configure time                                      ##
SET(ENABLE_GTEST OFF CACHE BOOL "Enable GTest?")                                        ##
if(ENABLE_GTEST)                                                                        ##
                                                                                        ##
                                                                                        ##
  configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)                  ##
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .                    ##
    RESULT_VARIABLE result                                                              ##
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )                 ##
  if(result)                                                                            ##
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")                  ##
  endif()                                                                               ##
  execute_process(COMMAND ${CMAKE_COMMAND} --build .                                    ##
    RESULT_VARIABLE result                                                              ##
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )                 ##
  if(result)                                                                            ##
    message(FATAL_ERROR "Build step for googletest failed: ${result}")                  ##
  endif()                                                                               ##
                                                                                        ##
  # Prevent GoogleTest from overriding our compiler/linker options                      ##
  # when building with Visual Studio                                                    ##
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)                                    ##
                                                                                        ##
  # Add googletest directly to our build. This adds the following targets:              ##
  # gtest, gtest_main, gmock and gmock_main                                             ##
  add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"                                 ##
                   "${CMAKE_BINARY_DIR}/googletest-build"                               ##
                   EXCLUDE_FROM_ALL)                                                    ##
                                                                                        ##
  # The gtest/gtest_main targets carry header search path                               ##
  # dependencies automatically when using CMake 2.8.11 or                               ##
  # later. Otherwise we have to add them here ourselves.                                ##
  if (CMAKE_VERSION VERSION_LESS 2.8.11)                                                ##
    include_directories("${gtest_SOURCE_DIR}/include")                                  ##
  endif()                                                                               ##
                                                                                        ##
  include(GoogleTest)                                                                   ##
  enable_testing()                                                                      ##
                                                                                        ##
endif()                                                                                 ##
#                                                                                       ##
#                                                                                       ##
##########################################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message("Source Directory ${CMAKE_SOURCE_DIR}")
FILE(GLOB SRCFILES "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/src/*.cpp")
message("Files are ${SRCFILES}")
# set the project name
project(HeapDebugger)

# add the executable
add_executable(HeapDebugger ${SRCFILES})

##########################################################################################
# DO NOT MODIFY                                                                         ##
if(ENABLE_GTEST)                                                                        ##
  FILE(GLOB TESTFILES "${CMAKE_SOURCE_DIR}/test/*.h" "${CMAKE_SOURCE_DIR}/test/*.cpp")  ##
  message("Test files are ${TESTFILES}")                                                ##
                                                                                        ##
  # this is the file I want to exclude / remove from the list                           ##
  get_filename_component(full_path_test_cpp ${CMAKE_SOURCE_DIR}/src/main.cpp ABSOLUTE)  ##
  # Now I remove the main.cpp so that there are not two conflicting mains.              ##
  list(REMOVE_ITEM SRCFILES "${full_path_test_cpp}")                                    ##
  # Create a project with all the source files except main, including the test files.   ##
  add_executable(run_unit_test ${TESTFILES} ${SRCFILES})                                ##
  # Link GTest library against it                                                       ##
  target_link_libraries(run_unit_test gtest_main)                                       ##
  # Standard linking to gtest stuff.                                                    ##
  target_include_directories(run_unit_test PRIVATE  "${CMAKE_SOURCE_DIR}/src")          ##
  # Extra linking for the project.                                                      ##
  message("Linking gtest")                                                              ##
                                                                                        ##
  macro (create_test target)                                                            ##
    add_test (NAME ${target} COMMAND $<TARGET_FILE:${target}>)                          ##
  endmacro (create_test)                                                                ##
                                                                                        ##
  create_test(run_unit_test)                                                            ##
  #add_test(NAME example_test COMMAND example)                                          ##
endif()                                                                                 ##
##########################################################################################
