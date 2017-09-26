cmake_minimum_required(VERSION 3.5)
project(p1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c utils.c editor.c parser.c  )
add_executable(p1 ${SOURCE_FILES})
add_executable(p1_test test.c)
