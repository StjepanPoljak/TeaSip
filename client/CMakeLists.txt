#cmake_minimum_required(VERSION 3.10.2)

include_directories(include)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCES source/*.cpp)

add_executable(tcp-client ${SOURCES})


