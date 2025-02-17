cmake_minimum_required(VERSION 3.10)
project(forword)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and build GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
)

# Add test executable
add_executable(forword_test
    tests/test_forword.cpp
)

# Link against GTest and pthread
target_link_libraries(forword_test
    GTest::gtest_main
    pthread
)

# Enable testing
enable_testing()
include(GoogleTest)
gtest_discover_tests(forword_test) 