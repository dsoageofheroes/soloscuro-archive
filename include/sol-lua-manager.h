#ifndef SOL_LUA_MANAGER_H
#define SOL_LUA_MANAGER_H

#include "lua-inc.h"

extern lua_State* sol_lua_get_state();

// Evaluate for refactor
extern void sol_lua_register(lua_State *l);
extern void sol_lua_state_register(lua_State *l);
extern int sol_lua_load_preload(const char *filename);
extern int sol_lua_keyup(const int key_code);
extern int sol_lua_keydown(const int key_code);
extern int sol_lua_load(const char *filename);

extern void lua_struct_register(lua_State *l);
const struct luaL_Reg* lua_struct_get_funcs();
extern const struct luaL_Reg* sol_ds_get_lib();
#endif
