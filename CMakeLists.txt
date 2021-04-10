cmake_minimum_required(VERSION 3.17)
project(msh C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(msh
        src/main.c)
