
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
message(STATUS "Looking for wolfSSL")
find_library(LIBRARY_WOLFSSL wolfssl)
message(STATUS "Looking for wolfSSL - ${LIBRARY_WOLFSSL}")
target_link_libraries(client ${LIBRARY_WOLFSSL})

message(STATUS "Looking for z")
find_library(LIBRARY_Z z)
message(STATUS "Looking for z - ${LIBRARY_Z}")
target_link_libraries(client ${LIBRARY_Z})
