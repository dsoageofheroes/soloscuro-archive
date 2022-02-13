cmake_minimum_required(VERSION 3.10)

project(soloscuro VERSION 0.2 LANGUAGES C)
enable_testing()

set(libADLMIDI_SHARED ON)
add_subdirectory(ext/)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_subdirectory(src/)
add_subdirectory(sdl/)
add_subdirectory(unity/)
add_subdirectory(test/)
