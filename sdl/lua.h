// This is the LUA for running main.lua, not to run the games GPL (see ds-state.c for that.)
#ifndef UI_LUA_H
#define UI_LUA_H

#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

int ui_lua_load(const char *filename);
void ui_lua_close();

#endif
