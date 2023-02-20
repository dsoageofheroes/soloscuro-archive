#ifndef SOL_LUA_MANAGER_H
#define SOL_LUA_MANAGER_H

#include "lua-inc.h"

extern sol_status_t sol_lua_get_state(lua_State **l);
extern sol_status_t sol_lua_struct_register(lua_State *l);
extern sol_status_t sol_lua_load_preload(const char *filename);
extern sol_status_t sol_lua_keyup(const int key_code);
extern sol_status_t sol_lua_keydown(const int key_code);
extern sol_status_t sol_lua_load(const char *filename);
extern sol_status_t sol_lua_run_function(const char *function);
extern sol_status_t sol_ds_get_lib(const struct luaL_Reg **l);
extern sol_status_t sol_lua_struct_get_funcs(const struct luaL_Reg **r);

// Evaluate for refactor
#endif
