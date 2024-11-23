message(STATUS "Building tests")

enable_testing()

# Include FetchContent for downloading GTest
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(cli_tests ${CMAKE_SOURCE_DIR}/src/cli.cc cli_tests.cc)
add_executable(cli_collection_tests ${CMAKE_SOURCE_DIR}/src/cli.cc cli_collections_test.cc)

target_link_libraries(cli_collection_tests
  PRIVATE
  ${PROJECT_NAME}
  GTest::gtest_main
  GTest::gmock_main
  ${PROJECT_NAME}
)

target_link_libraries(cli_tests
  PRIVATE
  ${PROJECT_NAME}
  GTest::gtest_main
  GTest::gmock_main
  ${PROJECT_NAME}
)

target_include_directories(cli_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cli_collection_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)

gtest_discover_tests(cli_tests)
gtest_discover_tests(cli_collection_tests)

# Add custom target for running tests with detailed output
add_custom_target(test_verbose COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS cli_tests cli_collection_tests)
