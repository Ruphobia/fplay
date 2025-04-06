cmake_minimum_required(VERSION 3.10)
project(PitfallClone C)

set(CMAKE_C_STANDARD 11)

# Find SDL2
find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found. Please install libsdl2-dev.")
endif()

# Manually find SDL2_mixer
find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer)
find_path(SDL2_MIXER_INCLUDE_DIR NAMES SDL_mixer.h PATH_SUFFIXES SDL2)

if (NOT SDL2_MIXER_LIBRARY OR NOT SDL2_MIXER_INCLUDE_DIR)
    message(FATAL_ERROR "SDL2_mixer not found. Please install libsdl2-mixer-dev.")
endif()

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIR})

# Add executable
add_executable(PitfallClone main.c scroller.c)

# Link libraries
target_link_libraries(PitfallClone ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARY})