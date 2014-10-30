cmake_minimum_required (VERSION 2.6)
project (xssh)
add_executable(xssh command.cpp external.cpp shell.cpp)
list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")