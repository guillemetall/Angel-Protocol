cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(angel)

# hardening:compile-flags
add_compile_options(
    -Wall -Wextra -Wvla -Wshadow -Wconversion -Wsign-conversion -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wstack-usage=2048 -fstack-protector-strong
)
