cmake_minimum_required(VERSION 3.0.0)

project(TeaSip)

add_subdirectory(server)
target_include_directories(tcp-server BEFORE PUBLIC include)

add_subdirectory(client)
target_include_directories(tcp-client BEFORE PUBLIC include)


