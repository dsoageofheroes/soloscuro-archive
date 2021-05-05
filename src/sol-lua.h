#ifndef SOL_LUA_H
#define SOL_LUA_H

#include "lua-inc.h"

extern void sol_lua_register(lua_State *l);
extern void sol_lua_close();
extern void sol_lua_state_register(lua_State *l);
extern int sol_lua_load_preload(const char *filename);
extern int sol_lua_keyup(const int key_code);
extern int sol_lua_keydown(const int key_code);
extern int sol_lua_load(const char *filename);

#endif
