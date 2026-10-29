cmake_minimum_required(VERSION 3.28)
project(Lab1_OS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(parent main.c)
add_executable(child1 child1.c)
add_executable(child2 child2.c)

target_compile_definitions(parent PRIVATE
        CHILD1_PATH="$<TARGET_FILE:child1>"
        CHILD2_PATH="$<TARGET_FILE:child2>"
)

add_dependencies(parent child1 child2)