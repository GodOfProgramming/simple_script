cmake_minimum_required(VERSION 3.16)

project(SimpleScript VERSION 0.0.1)

# set compiler options

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SHARED_COMPILE_OPTS -Wall -Wextra -pedantic -Werror)

# configure variables

set(PROJECT_NAME_TEST "${PROJECT_NAME}Test")

add_executable(${PROJECT_NAME} "src/main.cpp")

add_executable(${PROJECT_NAME_TEST} "src/main.test.cpp")

target_compile_options(${PROJECT_NAME} PUBLIC ${SHARED_COMPILE_OPTS} -O3)

target_compile_options(${PROJECT_NAME_TEST} PUBLIC ${SHARED_COMPILE_OPTS} -g -O0 --coverage -fprofile-arcs -ftest-coverage)

# add sources

add_subdirectory(lib)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} pthread)

target_link_libraries(${PROJECT_NAME_TEST} gcov pthread)

# ss

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src")

# test

target_include_directories(${PROJECT_NAME_TEST} PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(${PROJECT_NAME_TEST} PUBLIC "${CMAKE_SOURCE_DIR}/src")
