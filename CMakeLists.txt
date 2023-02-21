include(ExternalProject)
cmake_minimum_required(VERSION 3.10)

project(soloscuro VERSION 0.2 LANGUAGES C)

add_definitions(-DDEBUG)

add_subdirectory(ext/)

# I wish I know how to make this part of a subdir correctly, but I don't...
ExternalProject_Add(libadlmidi_proj
  GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI
  GIT_TAG master
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DlibADLMIDI_SHARED=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
)

ExternalProject_Get_property(libadlmidi_proj INSTALL_DIR)
ExternalProject_Get_property(libadlmidi_proj SOURCE_DIR)
ExternalProject_Get_property(libadlmidi_proj BINARY_DIR)
add_library(adl_lib SHARED IMPORTED )
#add_library(adl_lib STATIC IMPORTED )
add_dependencies(adl_lib libadlmidi_proj)
set_target_properties(adl_lib PROPERTIES IMPORTED_LOCATION ${BINARY_DIR}/libADLMIDI.so)
set (ADL_INC ${SOURCE_DIR}/include/)
# end external project...

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -fsanitize=address -g -fno-omit-frame-pointer")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -Wall -Werror -fsanitize=address -g -fno-omit-frame-pointer")

add_subdirectory(code-generation/powers/)
add_subdirectory(src/)
add_subdirectory(sdl/)

enable_testing()
add_subdirectory(unity/)
add_subdirectory(test/)

file(GLOB adl_libs ${BINARY_DIR}/libADLMIDI.so*)
install(FILES ${adl_libs} DESTINATION .)
install(FILES lua/main.lua DESTINATION lua)
install(FILES lua/base/1001.lua DESTINATION lua/base/1001.lua)
