cmake_minimum_required(VERSION 3.14)
project(untitled C)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(untitled main.c)
target_link_libraries(${PROJECT_NAME} pthread)
