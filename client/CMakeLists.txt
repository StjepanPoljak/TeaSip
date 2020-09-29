#cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCES source/*.cpp)

find_package(Threads)

add_executable(tcp-client ${SOURCES})

target_link_libraries(tcp-client ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(tcp-client PRIVATE include)


