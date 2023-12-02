cmake_minimum_required(VERSION 3.25)
project(minizip-cpp-tests CXX)
if(NOT MZPPTESTS)
    return()
endif()

if(NOT TARGET gtest)
    message(FATAL_ERROR "missing gtest target")
    return()
endif()

add_executable(mzpptests test.cpp)
target_link_libraries(mzpptests PRIVATE minizip-cpp gtest gtest_main)
target_compile_definitions(mzpptests PRIVATE MZPP_TEST_PATH=${CMAKE_CURRENT_LIST_DIR})

add_test(NAME mzpptests_run COMMAND mzpptests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
