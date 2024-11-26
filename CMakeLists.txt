cmake_minimum_required(VERSION 3.15)

project(libcli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Git QUIET)

add_subdirectory(logger)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/logger/include
)

option(BUILD_TESTING "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Testing configuration
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
