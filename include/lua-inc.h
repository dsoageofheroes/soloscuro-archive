// helper to include lua for the various compilers...
#ifdef _WIN32
#include "lua5.3/lua.h"
#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"
#else
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#endif

#include "entity.h"

extern int   sol_lua_load_entity (lua_State *l, dude_t *dude);
extern void* sol_lua_get_userdata(lua_State *l, const int loc);
extern void  sol_lua_dumpstack (lua_State *L);
