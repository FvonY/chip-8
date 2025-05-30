cmake_minimum_required(VERSION 3.10)
project(chip8 C)

set(CMAKE_C_STANDARD 99)

# Find all .c files in src/
# file(GLOB SRC_FILES src/*.c)

add_executable(chip8 main.c stack.c)

# Optional: add include directories
target_include_directories(chip8 PRIVATE include)

