// This is the LUA for running main.lua, not to run the games GPL (see ds-state.c for that.)
#ifndef UI_LUA_H
#define UI_LUA_H

#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

int ui_lua_load_preload(const char *filename);
int ui_lua_load(const char *filename);
void ui_lua_close();
int ui_lua_keydown(const int key_code);
int ui_lua_keyup(const int key_code);
void uil_set_globals(lua_State *l);

#endif
