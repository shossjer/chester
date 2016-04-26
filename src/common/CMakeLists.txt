
cmake_minimum_required(VERSION 2.6)
project(common)

# source listing
set(COMMON_SOURCE
  dummy.cpp
  )
# header listing
set(COMMON_HEADER
  Code.hpp
  Header.hpp
  Reader.hpp
  Serializer.hpp
  Structurer.hpp
  Writer.hpp
  messages.hpp
  network/byte_order.hpp
  )

set(COMMON_FILES ${COMMON_SOURCE} ${COMMON_HEADER})
# set target
add_library(common ${COMMON_FILES})
