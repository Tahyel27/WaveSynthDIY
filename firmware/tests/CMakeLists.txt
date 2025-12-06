cmake_minimum_required(VERSION 3.15)

project(synth_test CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent GTest from overriding /MD with /MT
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# 3. Download and make it available
FetchContent_MakeAvailable(googletest)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../src/SynthCore ${CMAKE_CURRENT_LIST_DIR}/build/SynthCore)

add_executable(testgen testgen.cpp)
target_link_libraries(testgen PRIVATE Engine)
target_include_directories(testgen PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

enable_testing()

add_executable(engine_tests engine_tests.cpp)
target_link_libraries(engine_tests PRIVATE Engine GTest::gtest_main)
target_include_directories(engine_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

include(GoogleTest)
gtest_discover_tests(engine_tests)