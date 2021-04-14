#include "lua-structs.h"
#include "ds-player.h"
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

#define GET_INTEGER_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushinteger(l, a->field); return 1; }
#define GET_STRING_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushstring(l, a->field); return 1; }
#define SET_INTEGER_TABLE(a, field, num) if (!strcmp(str, #field)) { a->field = num; return 0; }
#define SET_STRING_TABLE(a, field, num) if (!strcmp(str, #field)) { if (a->field) { free(a->field); } a->field = strdup(num); return 0; }


static int load_player (lua_State *l) {
    int n = luaL_checkinteger(l, 1);

    lua_newtable(l);
    lua_pushlightuserdata(l, player_get_entity(n));
    lua_setfield(l, -2, "ptr__");

    lua_pushliteral(l, "stats");
    lua_newtable(l);
    lua_pushlightuserdata(l, &(player_get_entity(n)->stats));
    lua_setfield(l, -2, "ptr__");
    luaL_setmetatable(l, "soloscuro.stats");
    lua_settable(l, -3);

    luaL_setmetatable(l, "soloscuro.entity");

    return 1;
}

static const struct luaL_Reg entity_lib [] = {
    //{"new", entity_new},
    {"load_player", load_player},
    {NULL, NULL}
};

static int entity_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, -3);
    entity_t *dude = lua_touserdata(l, -1);
    lua_pop(l, 1);

    //printf("indexing '%s' of object %p\n", str, dude);

    GET_STRING_TABLE(dude, name);
    GET_INTEGER_TABLE(dude, size);
    GET_INTEGER_TABLE(dude, race);
    GET_INTEGER_TABLE(dude, gender);
    GET_INTEGER_TABLE(dude, alignment);
    GET_INTEGER_TABLE(dude, allegiance);
    GET_INTEGER_TABLE(dude, object_flags);
    GET_INTEGER_TABLE(dude, region);
    GET_INTEGER_TABLE(dude, mapx);
    GET_INTEGER_TABLE(dude, mapy);
    GET_INTEGER_TABLE(dude, mapz);
    GET_INTEGER_TABLE(dude, sound_fx);
    GET_INTEGER_TABLE(dude, attack_sound);
    GET_INTEGER_TABLE(dude, combat_status);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int entity_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, -4);
    entity_t *dude = lua_touserdata(l, -1);
    lua_pop(l, 1);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(dude, size, num);
        SET_INTEGER_TABLE(dude, race, num);
        SET_INTEGER_TABLE(dude, gender, num);
        SET_INTEGER_TABLE(dude, alignment, num);
        SET_INTEGER_TABLE(dude, allegiance, num);
        SET_INTEGER_TABLE(dude, object_flags, num);
        SET_INTEGER_TABLE(dude, region, num);
        SET_INTEGER_TABLE(dude, mapx, num);
        SET_INTEGER_TABLE(dude, mapy, num);
        SET_INTEGER_TABLE(dude, mapz, num);
        SET_INTEGER_TABLE(dude, sound_fx, num);
        SET_INTEGER_TABLE(dude, attack_sound, num);
        SET_INTEGER_TABLE(dude, combat_status, num);
    } else if (lua_isstring(l, 3)) {
        const char *name = luaL_checkstring(l, 3);
        SET_STRING_TABLE(dude, name, name);
    }

    return 0;
}

static const luaL_Reg entity_methods[] = {
    {"__index",      entity_get},
    {"__newindex",   entity_set},
    {0,0}
};

static int stats_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, -3);
    stats_t *stats = lua_touserdata(l, -1);
    lua_pop(l, 1);

    GET_INTEGER_TABLE(stats, str);
    GET_INTEGER_TABLE(stats, dex);
    GET_INTEGER_TABLE(stats, con);
    GET_INTEGER_TABLE(stats, intel);
    GET_INTEGER_TABLE(stats, wis);
    GET_INTEGER_TABLE(stats, cha);
    GET_INTEGER_TABLE(stats, hp);
    GET_INTEGER_TABLE(stats, high_hp);
    GET_INTEGER_TABLE(stats, psp);
    GET_INTEGER_TABLE(stats, high_psp);
    GET_INTEGER_TABLE(stats, base_ac);
    GET_INTEGER_TABLE(stats, move);
    GET_INTEGER_TABLE(stats, base_move);
    GET_INTEGER_TABLE(stats, base_thac0);
    GET_INTEGER_TABLE(stats, magic_resistance);
    GET_INTEGER_TABLE(stats, special_defense);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int stats_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, -4);
    stats_t *stats = lua_touserdata(l, -1);
    lua_pop(l, 1);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(stats, str, num);
        SET_INTEGER_TABLE(stats, dex, num);
        SET_INTEGER_TABLE(stats, con, num);
        SET_INTEGER_TABLE(stats, intel, num);
        SET_INTEGER_TABLE(stats, wis, num);
        SET_INTEGER_TABLE(stats, cha, num);
        SET_INTEGER_TABLE(stats, hp, num);
        SET_INTEGER_TABLE(stats, high_hp, num);
        SET_INTEGER_TABLE(stats, psp, num);
        SET_INTEGER_TABLE(stats, high_psp, num);
        SET_INTEGER_TABLE(stats, base_ac, num);
        SET_INTEGER_TABLE(stats, move, num);
        SET_INTEGER_TABLE(stats, base_move, num);
        SET_INTEGER_TABLE(stats, base_thac0, num);
        SET_INTEGER_TABLE(stats, magic_resistance, num);
        SET_INTEGER_TABLE(stats, special_defense, num);
    }

    return 0;
}
static const luaL_Reg stats_methods[] = {
    {"__index",      stats_get},
    {"__newindex",   stats_set},
    {0,0}
};

extern void lua_struct_register(lua_State *l) {
    lua_newtable(l);
    luaL_setfuncs(l, entity_lib, 0);
    lua_setglobal(l, "soloscuro");

    luaL_newmetatable(l, "soloscuro.entity");
    luaL_setfuncs(l, entity_methods, 0);
    lua_setglobal(l, "entity");

    luaL_newmetatable(l, "soloscuro.stats");
    luaL_setfuncs(l, stats_methods, 0);
    lua_setglobal(l, "stats");
}
