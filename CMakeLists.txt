cmake_minimum_required(VERSION 3.30)
project(RealTimeProject1 C)

set(CMAKE_C_STANDARD 11)
include_directories(include)

add_executable(RealTimeProject1 main.c src/config.c)
