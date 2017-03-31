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
 -Wno-shadow
 -Wwrite-strings
 -std=c++11
 -pthread
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(GENERATOR_SRC_LIST
    MysqlGenerator_unittest.cpp
)
aux_source_directory(./proto  GENERATOR_SRC_LIST)

set(INTERFACE_SRC_LIST
    MysqlInterface_unittest.cpp
)
aux_source_directory(./proto  INTERFACE_SRC_LIST)
include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable(generator_unittest ${GENERATOR_SRC_LIST})
target_link_libraries(generator_unittest  protobuf-mysql soul protobuf mysqlclient)

add_executable(interface_unittest ${INTERFACE_SRC_LIST})
target_link_libraries(interface_unittest  protobuf-mysql soul protobuf mysqlclient)
