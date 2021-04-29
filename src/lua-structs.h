#ifndef LUA_STRUCTS_H
#define LUA_STRUCTS_H

#include "lua-inc.h"

extern void lua_struct_register(lua_State *l);
const struct luaL_Reg* lua_struct_get_funcs();

#endif
