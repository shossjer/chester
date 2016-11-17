
cmake_minimum_required(VERSION 2.6)
project(client)

# source listing
set(CLIENT_SOURCE
  main.cpp
  )
# header listing
set(CLIENT_HEADER
  )

set(CLIENT_FILES ${CLIENT_SOURCE} ${CLIENT_HEADER})
# set target
add_executable(client ${CLIENT_FILES})

add_dependencies(client common)
target_link_libraries(client common)

# dependencies
message(STATUS "Looking for z")
find_library(LIBRARY_Z z)
message(STATUS "Looking for z - ${LIBRARY_Z}")

message(STATUS "Looking for zdll")
find_library(LIBRARY_ZDLL zdll)
message(STATUS "Looking for zdll - ${LIBRARY_ZDLL}")

message(STATUS "Looking for Windows")
find_file(FILE_WINDOWS_H windows.h)
message(STATUS "Looking for Windows - ${FILE_WINDOWS_H}")

if(LIBRARY_Z)
  target_link_libraries(client ${LIBRARY_Z})
elseif(LIBRARY_ZDLL)
  target_link_libraries(client ${LIBRARY_ZDLL})
else()
  message(FATAL_ERROR "Needs zlib")
endif()

if(LIBRARY_ZDLL OR
   FILE_WINDOWS_H)
  message(STATUS "Client is Windows")
  set(CLIENT_IS_WIN32 1 PARENT_SCOPE)
endif()
