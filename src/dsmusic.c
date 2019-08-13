#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "gff.h"
#include "music.h"

static int lua_music_init(lua_State *L) {    
    size_t sflen;
    const char *sfpath = luaL_checklstring(L, 1, &sflen);
    music_init(sfpath);
    return 0;
}

static int lua_music_shutdown(lua_State *L) {
    music_shutdown();
    return 0;
}

int lua_music_play(lua_State *L) {
    size_t midilen;
    const char *midipath = luaL_checklstring(L, 1, &midilen);

    music_play(midipath);

    return 0;
}

int lua_music_play_data(lua_State *L) {
    size_t midilen;
    const char *midi_data = luaL_checklstring(L, 1, &midilen);

    music_play_mem(midi_data, midilen);
    return 0;
}

int lua_music_stop(lua_State *L) {
    music_stop();
    return 0;
}

//library to be registered
static const struct luaL_Reg lslib [] = {
      {"music_init", lua_music_init},
      {"music_play", lua_music_play},
      {"music_play_data", lua_music_play_data},
      {"music_stop", lua_music_stop},
      {"music_shutdown", lua_music_shutdown},
      {NULL, NULL}  /* sentinel */
    };

// How Lua registers the functions.
int luaopen_dsmusic (lua_State *L){
    luaL_register(L, "dsmusic", lslib);
    return 1;
}

