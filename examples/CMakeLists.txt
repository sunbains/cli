cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(EXAMPLES)

add_executable(cli_basic cli_basic.cc)
add_executable(cli_advanced cli_advanced.cc)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/logger/include)

target_include_directories(cli_basic PRIVATE ${INCLUDE_DIRS})
target_include_directories(cli_advanced PRIVATE ${INCLUDE_DIRS})

target_link_libraries(cli_basic INTERFACE)
target_link_libraries(cli_advanced INTERFACE)
