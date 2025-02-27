cmake_minimum_required(VERSION 3.27)
project(faux_combinator)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE src)

find_package(GTest REQUIRED)
add_executable(${PROJECT_NAME}_test test/faux_combinator.cpp)
include_directories(${GTest_INCLUDE_DIR} src)
target_link_libraries(${PROJECT_NAME}_test GTest::gtest_main)
add_test(${PROJECT_NAME}_test ${PROJECT_NAME}_test)
