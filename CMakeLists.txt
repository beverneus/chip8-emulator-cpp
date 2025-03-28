cmake_minimum_required(VERSION 3.30)
project(chip8)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8 src/main.cpp)
target_link_libraries(chip8 ${SDL2_LIBRARIES})