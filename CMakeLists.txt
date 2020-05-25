# Minimum CMake version.
cmake_minimum_required(VERSION 3.7.2)

# Create project.
project(BigNumbers)

# Headers directory
include_directories(include)

# Adding source files.
# set(SOURCES source/main.c) # - Manually.
file(GLOB SOURCES "source/*.c") # - Automatically.

add_executable(BigNumbers ${SOURCES}) # Using variable SOURCES.

# Flags for builds
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wpedantic -Wextra -fexceptions -O0 -g3 -ggdb --std=c11")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -O3 --std=c11")

# Linking
target_link_libraries(BigNumbers m)