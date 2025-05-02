cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# MIT License
# Copyright (c) 2024-2025 Tomáš Mark

# +-+-+-+-+-+-+-+-+-+
#   |g|t|e|s|t|s|   |
# +-+-+-+-+-+-+-+-+-+

set(TEST_NAME LibTester)
project(TEST_NAME LANGUAGES CXX)
include(CTest)
include(../../cmake/CPM.cmake)

# A. Way given by CPM.cmake
CPMAddPackage(
    NAME GTest
    GITHUB_REPOSITORY google/googletest
    VERSION 1.16.0
    OPTIONS "INSTALL_GTEST OFF")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# B. Way given via (Conan)
# find_package(GTest REQUIRED)

# configure the test executable
add_executable(TEST_NAME ${TEST_SOURCES})
target_link_libraries(TEST_NAME PRIVATE GTest::gtest GTest::gtest_main dotname::standalone_common)
set_target_properties(TEST_NAME PROPERTIES OUTPUT_NAME "${TEST_NAME}")
add_test(NAME TEST_NAME COMMAND TEST_NAME)
