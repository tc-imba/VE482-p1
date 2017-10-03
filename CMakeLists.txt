cmake_minimum_required(VERSION 2.7)
project(p1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c utils.c editor.c parser.c history.c )
add_executable(p1 ${SOURCE_FILES})
add_executable(p1_test test.c history.c)
