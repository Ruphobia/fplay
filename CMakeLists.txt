cmake_minimum_required(VERSION 3.10)
project(HelloPixels C)

# Find SDL2 package
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add executable
add_executable(HelloPixels main.c)

# Link SDL2
target_link_libraries(HelloPixels ${SDL2_LIBRARIES})