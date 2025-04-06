cmake_minimum_required(VERSION 3.10)
project(CaveScroller C)

# Find SDL2
find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found. Install libsdl2-dev.")
endif()

# Find SDL2_mixer manually
find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer sdl2_mixer)
find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h PATH_SUFFIXES SDL2)

if(NOT SDL2_MIXER_LIBRARY OR NOT SDL2_MIXER_INCLUDE_DIR)
    message(FATAL_ERROR "SDL2_mixer not found. Install libsdl2-mixer-dev.")
endif()

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIR})

# Add executable with all source files
add_executable(CaveScroller main.c cave.c player1.c player2.c)

# Link libraries
target_link_libraries(CaveScroller ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARY} m)