cmake_minimum_required(VERSION 3.10)

project(soloscuro VERSION 0.2)

add_subdirectory(src/)

set(libADLMIDI_SHARED ON)
add_subdirectory(ext/libadlmidi)
add_subdirectory(ext/)

add_subdirectory(sdl/)
