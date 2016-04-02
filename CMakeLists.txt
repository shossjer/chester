#
cmake_minimum_required(VERSION 2.6)
project(chester)

# make debug build default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(MODE_DEBUG 1)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  set(MODE_RELEASE 1)
else()
  message(FATAL_ERROR "Build type ${CMAKE_BUILD_TYPE} not supported")
endif()

# gcc -xc++ -E -v /dev/null
set(CMAKE_INCLUDE_PATH "/usr/include/c++/4.8;/usr/include/x86_64-linux-gnu/c++/4.8;/usr/include/c++/4.8/backward;/usr/lib/gcc/x86_64-linux-gnu/4.8/include;/usr/local/include;/usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed;/usr/include/x86_64-linux-gnu;/usr/include")

# project information
message(STATUS "Using compiler ${CMAKE_CXX_COMPILER}")

message(STATUS "CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}")
message(STATUS "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")

message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_SYSTEM_PREFIX_PATH=${CMAKE_SYSTEM_PREFIX_PATH}")
message(STATUS "CMAKE_INCLUDE_PATH=${CMAKE_INCLUDE_PATH}")
message(STATUS "CMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_SYSTEM_INCLUDE_PATH}")
message(STATUS "CMAKE_SYSTEM_LIBRARY_PATH=${CMAKE_SYSTEM_LIBRARY_PATH}")

# the version number
set(CHESTER_VERSION_MAJOR 0)
set(CHESTER_VERSION_MINOR 1)

# compiler flags
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "")

# additional includes
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

# subdirectories
add_subdirectory("src/client")
add_subdirectory("src/common")
add_subdirectory("src/server")

# write config
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
