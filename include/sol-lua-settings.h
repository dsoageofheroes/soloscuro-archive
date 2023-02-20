#ifndef SOL_LUA_H
#define SOL_LUA_H

#include "lua-inc.h"

extern sol_status_t sol_lua_settings_register(lua_State *l);
extern sol_status_t sol_lua_register_globals();

#endif
