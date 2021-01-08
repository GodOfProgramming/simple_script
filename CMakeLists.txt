cmake_minimum_required(VERSION 3.16)

project(SimpleScript VERSION 0.0.1)

# set compiler options

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -pedantic -Werror)

# configure variables

set(EXE SimpleScript)
set(EXE_TEST SimpleScriptTest)

set(SRC src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${EXE} "${SRC}/main.cpp")
add_executable(${EXE_TEST} "${SRC}/main.test.cpp")

target_link_libraries(${EXE} pthread)
target_link_libraries(${EXE_TEST} ${GTEST_LIBRARIES} gcov pthread)

target_compile_options(${EXE_TEST} PUBLIC -g -O0 --coverage -fprofile-arcs -ftest-coverage)

# ss

target_include_directories(${EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${EXE} PUBLIC "${CMAKE_SOURCE_DIR}/src")

# test

target_include_directories(${EXE_TEST} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${EXE_TEST} PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(src)
