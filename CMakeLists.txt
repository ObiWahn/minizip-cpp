cmake_minimum_required(VERSION 3.25)
project(minizip-cpp C CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
option(MZPPTESTS ON)
include(FetchContent)

FetchContent_Declare(
  ZlibOfficial
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG  master
)
set(fetch_libs ZlibOfficial)

if(MZPPTESTS)
    include(CTest)
    FetchContent_Declare(
        GoogleTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG  v1.14.0
    )
    list(APPEND fetch_libs GoogleTest)
endif()

FetchContent_MakeAvailable(${fetch_libs})

get_target_property(ZLIB_SOURCE_DIR zlib SOURCE_DIR)

add_library(minizip
${ZLIB_SOURCE_DIR}/contrib/minizip/ioapi.c
#${ZLIB_SOURCE_DIR}/contrib/minizip/iowin32.c
${ZLIB_SOURCE_DIR}/contrib/minizip/miniunz.c
${ZLIB_SOURCE_DIR}/contrib/minizip/minizip.c
${ZLIB_SOURCE_DIR}/contrib/minizip/mztools.c
${ZLIB_SOURCE_DIR}/contrib/minizip/unzip.c
${ZLIB_SOURCE_DIR}/contrib/minizip/zip.c
)
target_include_directories(minizip PUBLIC ${ZLIB_SOURCE_DIR} SYSTEM)
target_link_libraries(minizip PUBLIC zlib)

add_library(minizip-cpp INTERFACE)
target_compile_features(minizip-cpp INTERFACE cxx_std_17)
target_include_directories(minizip-cpp INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(minizip-cpp INTERFACE minizip)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
