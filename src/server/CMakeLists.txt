
cmake_minimum_required(VERSION 2.6)
project(server)

# source listing
set(SERVER_SOURCE
  main.cpp
  )
# header listing
set(SERVER_HEADER
  )

set(SERVER_FILES ${SERVER_SOURCE} ${SERVER_HEADER})
# set target
add_executable(server ${SERVER_FILES})

add_dependencies(server common)
target_link_libraries(server common)

# find threads
#find_package(threads)
#target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(server pthread)
