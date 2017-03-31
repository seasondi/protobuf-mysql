PROJECT(test)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_COMPILER "g++")

set(CXX_FLAGS
 -g
 -Wall
 -Wextra
 -Werror
 -Wno-conversion
 -Wno-unused-parameter
 -Wno-old-style-cast
 -Wno-sign-compare
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -std=c++11
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

aux_source_directory(.  SRC_LIST)
include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_library(protobuf-mysql ${SRC_LIST})
target_link_libraries(protobuf-mysql  soul libprotobuf.a mysqlclient)
