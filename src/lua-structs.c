#include "lua-structs.h"
#include "ds-player.h"
#include "region.h"
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

#define GET_INTEGER_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushinteger(l, a->field); return 1; }
#define GET_STRING_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushstring(l, a->field); return 1; }
#define SET_INTEGER_TABLE(a, field, num) if (!strcmp(str, #field)) { a->field = num; return 0; }
#define SET_STRING_TABLE(a, field, num) if (!strcmp(str, #field)) { if (a->field) { free(a->field); } a->field = strdup(num); return 0; }

static void push_table_start(lua_State *l, void *data, const char *name) {
    lua_pushstring(l, name);
    lua_newtable(l);
    lua_pushlightuserdata(l, data);
    lua_setfield(l, -2, "ptr__");
}

static void push_table_end(lua_State *l, const char *metaname) {
    if (metaname) {
        luaL_setmetatable(l, metaname);
    }
    lua_settable(l, -3);
}

static void push_table(lua_State *l, void *data, const char *name, const char *metaname) {
    push_table_start(l, data, name);
    push_table_end(l, metaname);
}

static int load_player (lua_State *l) {
    int n = luaL_checkinteger(l, 1);

    lua_newtable(l); // entity table
    lua_pushlightuserdata(l, player_get_entity(n));
    lua_setfield(l, -2, "ptr__");

    push_table_start(l, &(player_get_entity(n)->stats), "stats");

    push_table(l, &(player_get_entity(n)->stats.attacks[0]), "attack1", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[1]), "attack2", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[2]), "attack3", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.saves), "saves", "soloscuro.saves");

    push_table_end(l, "soloscuro.stats");

    push_table(l, &(player_get_entity(n)->class[0]), "class1", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[1]), "class2", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[2]), "class3", "soloscuro.class");

    luaL_setmetatable(l, "soloscuro.entity"); // entity meta

    return 1;
}

static int create_region (lua_State *l) {
    lua_newtable(l); // entity table
    lua_pushlightuserdata(l, calloc(1, sizeof(region_t)));
    lua_setfield(l, -2, "ptr__");

    /*push_table_start(l, &(player_get_entity(n)->stats), "stats");

    push_table(l, &(player_get_entity(n)->stats.attacks[0]), "attack1", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[1]), "attack2", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[2]), "attack3", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.saves), "saves", "soloscuro.saves");

    push_table_end(l, "soloscuro.stats");

    push_table(l, &(player_get_entity(n)->class[0]), "class1", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[1]), "class2", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[2]), "class3", "soloscuro.class");
    */

    luaL_setmetatable(l, "soloscuro.region"); // entity meta

    return 1;
}

static void* get_userdata(lua_State *l, const int loc) {
    void *ret = NULL;
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, loc);
    ret = lua_touserdata(l, -1);
    lua_pop(l, 1);

    return ret;
}

static const struct luaL_Reg entity_lib [] = {
    //{"new", entity_new},
    {"load_player", load_player},
    {"create_region", create_region},
    {NULL, NULL}
};

static int entity_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    entity_t *dude = get_userdata(l, -3);

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
    entity_t *dude = get_userdata(l, -4);

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
    stats_t *stats = get_userdata(l, -3);

    //printf("indexing '%s' of stats %p\n", str, stats);
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
    stats_t *stats = get_userdata(l, -4);

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

static int attack_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    innate_attack_t *attack = get_userdata(l, -3);

    //printf("indexing '%s' of attack %p\n", str, attack);
    GET_INTEGER_TABLE(attack, number);
    GET_INTEGER_TABLE(attack, num_dice);
    GET_INTEGER_TABLE(attack, sides);
    GET_INTEGER_TABLE(attack, bonus);
    GET_INTEGER_TABLE(attack, special);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int attack_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    innate_attack_t *attack = get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(attack, number, num);
        SET_INTEGER_TABLE(attack, num_dice, num);
        SET_INTEGER_TABLE(attack, sides, num);
        SET_INTEGER_TABLE(attack, bonus, num);
        SET_INTEGER_TABLE(attack, special, num);
    }

    return 0;
}
static const luaL_Reg attack_methods[] = {
    {"__index",      attack_get},
    {"__newindex",   attack_set},
    {0,0}
};

static int saves_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    saving_throws_t *saves = get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(saves, paralysis);
    GET_INTEGER_TABLE(saves, wand);
    GET_INTEGER_TABLE(saves, petrify);
    GET_INTEGER_TABLE(saves, breath);
    GET_INTEGER_TABLE(saves, spell);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int saves_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    saving_throws_t *saves = get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(saves, paralysis, num);
        SET_INTEGER_TABLE(saves, wand, num);
        SET_INTEGER_TABLE(saves, petrify, num);
        SET_INTEGER_TABLE(saves, breath, num);
        SET_INTEGER_TABLE(saves, spell, num);
    }

    return 0;
}
static const luaL_Reg saves_methods[] = {
    {"__index",      saves_get},
    {"__newindex",   saves_set},
    {0,0}
};

static int class_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    class_t *class = get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(class, current_xp);
    GET_INTEGER_TABLE(class, high_xp);
    GET_INTEGER_TABLE(class, class);
    GET_INTEGER_TABLE(class, level);
    GET_INTEGER_TABLE(class, high_level);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int class_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    class_t *class = get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(class, current_xp, num);
        SET_INTEGER_TABLE(class, high_xp, num);
        SET_INTEGER_TABLE(class, class, num);
        SET_INTEGER_TABLE(class, level, num);
        SET_INTEGER_TABLE(class, high_level, num);
    }

    return 0;
}
static const luaL_Reg class_methods[] = {
    {"__index",      class_get},
    {"__newindex",   class_set},
    {0,0}
};

static int region_set_tile(lua_State *l) {
    region_t *region = get_userdata(l, 1);
    const int x = luaL_checkinteger(l, 2);
    const int y = luaL_checkinteger(l, 3);
    const int tile = luaL_checkinteger(l, 4);
    printf("%p (%d, %d) to %d!\n", region, x, y, tile);
    //uint8_t tiles[MAP_ROWS][MAP_COLUMNS];
    return 0;
}

static int region_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    region_t *region = get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(region, map_id);
    GET_INTEGER_TABLE(region, palette_id);
    if (!strcmp(str, "set_tile")) { lua_pushcfunction(l, region_set_tile); return 1; }
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int region_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    region_t *region = get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(region, map_id, num);
        SET_INTEGER_TABLE(region, palette_id, num);
    }

    return 0;
}
static const luaL_Reg region_methods[] = {
    {"__index",      region_get},
    {"__newindex",   region_set},
    {0,0}
};

static void setup_metatable(lua_State *l, const char *name, const luaL_Reg *methods, const char *global) {
    luaL_newmetatable(l, name);
    luaL_setfuncs(l, methods, 0);
    lua_setglobal(l, global);
}

extern void lua_struct_register(lua_State *l) {
    lua_newtable(l);
    luaL_setfuncs(l, entity_lib, 0);
    lua_setglobal(l, "soloscuro");

    setup_metatable(l, "soloscuro.entity", entity_methods, "entity__");
    setup_metatable(l, "soloscuro.stats", stats_methods, "stats__");
    setup_metatable(l, "soloscuro.attack", attack_methods, "attack__");
    setup_metatable(l, "soloscuro.saves", saves_methods, "saves__");
    setup_metatable(l, "soloscuro.class", class_methods, "class__");
    setup_metatable(l, "soloscuro.region", region_methods, "region__");
}
