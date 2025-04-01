cmake_minimum_required(VERSION 3.30)
project(chip8)

set(CMAKE_CXX_STANDARD 23)
set(SRC src/main.cpp src/chip8.cpp src/audio.cpp)

add_executable(chip8 ${SRC})

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
target_link_libraries(chip8 PRIVATE SDL3::SDL3)
include_directories(include ${SDL3_INCLUDE_DIRS})


target_compile_options(chip8 PRIVATE -Werror -Wall -Wextra)