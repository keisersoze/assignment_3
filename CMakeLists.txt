cmake_minimum_required(VERSION 3.13)
project(assignment_3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-O3")
include_directories(.)

add_executable(assignment_3
        thread_pool.h matrix_product.h matrix_sum.h matrix_expression.h example7.cc)
