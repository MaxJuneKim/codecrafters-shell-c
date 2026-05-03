cmake_minimum_required(VERSION 3.13)

project(codecrafters-shell)

file(GLOB_RECURSE SOURCE_FILES src/*.c)

set(CMAKE_C_STANDARD 23) # Enable the C23 standard

find_package(Threads)
add_executable(shell ${SOURCE_FILES})
# target_sources(shell PRIVATE ${SOURCE_FILES} PUBLIC headers)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

target_include_directories(shell PRIVATE headers)
target_include_directories(shell PRIVATE headers/Navigation)
target_link_libraries(shell PRIVATE readline Threads::Threads)