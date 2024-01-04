cmake_minimum_required(VERSION 3.22)
project(hangman)

set(CMAKE_CXX_STANDARD_17)

add_executable(server server.cpp Player.cpp)
add_compile_options(-Wall -Wextra)