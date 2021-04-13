#include "lua-structs.h"
#include "ds-player.h"

/*
static int new_entity (lua_State *L) {
      int n = luaL_checkinteger(L, 1);
      size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);

      NumArray *a = malloc(nbytes);
      lua_pushlightuserdata (L, a);
      //NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);

      luaL_getmetatable(L, "LuaBook.array");
      lua_setmetatable(L, -2);
      a->size = n;
      return 1;  
}
*/

static int load_player (lua_State *l) {
    int n = luaL_checkinteger(l, 1);

    lua_pushlightuserdata (l, player_get_entity(n));
    luaL_getmetatable(l, "soloscuro.entity");
    lua_setmetatable(l, -2);

    return 1;
}

static const struct luaL_Reg entity_lib [] = {
    //{"new", entity_new},
    {"load_player", load_player},
    {NULL, NULL}
};

extern void lua_struct_register(lua_State *l) {
    luaL_newmetatable(l, "soloscuro.entity");
    lua_pushstring(l, "__index");
    lua_pushvalue(l, -2);  /* pushes the metatable */
    lua_settable(l, -3);  /* metatable.__index = metatable */

    luaL_setfuncs(l, entity_lib, 0);
    lua_setglobal(l, "entity");
}
