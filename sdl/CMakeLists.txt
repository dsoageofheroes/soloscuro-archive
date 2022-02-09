include (FindLua)
include (../cmake/FindLibSndFile.cmake)
include (../cmake/FindSDL2TTF.cmake)
find_package(ZLIB REQUIRED )
find_package(SDL2 REQUIRED)

set(SOLOSCURO_SDL_SRCS
    main.c
    browse.c
    textbox.c
    utils.c
    mouse.c
    background.c
    sprite.c
    lua-generate-globals.c
    font.c
    window.c
    audio.c
)

include_directories(sdl)
include_directories(../include)
include_directories(../ext)
include_directories(../ext/libadlmidi/include)

add_library(adl_lib SHARED IMPORTED )
set_target_properties(adl_lib PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libadlmidi/libADLMIDI.so )

add_executable(soloscuro_bin ${SOLOSCURO_SDL_SRCS})

target_link_libraries(soloscuro_bin soloscuro)
target_link_libraries(soloscuro_bin m)
target_link_libraries(soloscuro_bin spng)
target_link_libraries(soloscuro_bin xmi2mid)
target_link_libraries(soloscuro_bin adl_lib)
target_link_libraries(soloscuro_bin ${LUA_LIBRARIES})
target_link_libraries(soloscuro_bin ${ZLIB_LIBRARIES})
target_link_libraries(soloscuro_bin ${SDL2_LIBRARIES})
target_link_libraries(soloscuro_bin ${LIBSNDFILE_LIBRARIES})
target_link_libraries(soloscuro_bin ${SDL2TTF_LIBRARY})

set_target_properties(soloscuro_bin PROPERTIES OUTPUT_NAME soloscuro)
