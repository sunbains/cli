cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(EXAMPLES)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(cli_basic cli_basic.cc)
add_executable(cli_advanced cli_advanced.cc)

target_link_libraries(cli_basic PRIVATE )
target_link_libraries(cli_advanced PRIVATE )
