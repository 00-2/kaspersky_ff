cmake_minimum_required(VERSION 3.23)
project(kaspersky_ff C)

set(CMAKE_C_STANDARD 11)

add_executable(kaspersky_ff main.c)
set_target_properties(kaspersky_ff PROPERTIES OUTPUT_NAME "better_name")