cmake_minimum_required(VERSION 3.31)
project(VOC_multipexer C)

set(CMAKE_C_STANDARD 11)

add_executable(VOC_multipexer main.c
        libraries/VOC_essentials.c
        libraries/VOC_essentials.h)
