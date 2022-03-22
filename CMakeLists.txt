cmake_minimum_required(VERSION 3.10)

project(soloscuro VERSION 0.2 LANGUAGES C)
enable_testing()

set(libADLMIDI_SHARED ON)
add_subdirectory(ext/)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -g -fno-omit-frame-pointer")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address -g -fno-omit-frame-pointer")

add_subdirectory(code-generation/powers/)
add_subdirectory(src/)
add_subdirectory(sdl/)
add_subdirectory(unity/)
add_subdirectory(test/)
