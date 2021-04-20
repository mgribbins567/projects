cmake_minimum_required(VERSION 3.15)
project(Project2 C)

set(CMAKE_C_STANDARD 90)

add_executable(Project2 main.c futo.c)

set(CMAKE_C_FLAGS "-Wall -pedantic")