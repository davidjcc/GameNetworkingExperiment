cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Pong)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

add_subdirectory(src/client)
add_subdirectory(src/server)

