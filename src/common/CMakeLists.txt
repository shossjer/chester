
cmake_minimum_required(VERSION 2.6)
project(common)

# source listing
set(COMMON_SOURCE
  network/initiate.cpp
  )
# header listing
set(COMMON_HEADER
  network/initiate.hpp
  )

set(COMMON_FILES ${COMMON_SOURCE} ${COMMON_HEADER})
# set target
add_library(common ${COMMON_FILES})
