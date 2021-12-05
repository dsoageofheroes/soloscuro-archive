#ifndef LUA_REGION_H
#define LUA_REGION_H

#include "lua-inc.h"
#include "region.h"

extern int sol_lua_region_function(sol_region_t *reg, const char *func, lua_State *l);

#endif
