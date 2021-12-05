#ifndef LUA_ENTITY_H
#define LUA_ENTITY_H

#include "lua-inc.h"
#include "entity.h"

extern int sol_lua_entity_function(entity_t *entity, const char *func, lua_State *l);

#endif
