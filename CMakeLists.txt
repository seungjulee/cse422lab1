cmake_minimum_required(VERSION 2.8.4)
project(xssh)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

set(SOURCE_FILES Shell.cpp External.cpp Command.cpp execvpe.c)
add_executable(xssh ${SOURCE_FILES})