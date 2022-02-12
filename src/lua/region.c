#include "player.h"
#include "region.h"
#include "region-manager.h"
#include "gff.h"
#include "port.h"
#include "arbiter.h"
#include <string.h>
#include <float.h>

extern char *strdup(const char *s); // Not in standard.

static int region_set_tile(lua_State *l) {
    //sol_region_t *region = get_userdata(l, 1);
    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    const int x = luaL_checkinteger(l, 2);
    const int y = luaL_checkinteger(l, 3);
    const int tile = luaL_checkinteger(l, 4);

    if (x > 0 && x < 99 && y > 0 && y < 128) {
        region->tiles[x-1][y-1] = tile;
    }
    //printf("%p (%d, %d) to %d!\n", region, x, y, tile);
    //uint8_t tiles[MAP_ROWS][MAP_COLUMNS];
    return 0;
}

static int region_set_tile_id(lua_State *l) {
    //sol_region_t *region = get_userdata(l, 1);
    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    region->sol.mid = luaL_checkinteger(l, 1);
    region->sol.tid = luaL_checkinteger(l, 2);

    //if (pos < 0 && pos > 255) {
        //return 0;
    //}

    //if (!region->tile_ids) {
        //region->num_tiles = 256;
        //region->tile_ids = calloc(1, region->num_tiles * sizeof(uint32_t));
    //}

    //region->tile_ids[pos] = id;
    //printf("%p (%d, %d) to %d!\n", region, x, y, tile);
    return 0;
}

static int push_region_function(lua_State *l, sol_region_t *region, int (*func)(lua_State *l)) {
    lua_pushlightuserdata(l, region);
    lua_pushcclosure(l, func, 1);
    return 1;
}

static int get_first_entity(lua_State *l) {
    dude_t *dude = NULL;
    const char *name = luaL_checkstring(l, 1);
    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));

    entity_list_for_each(region->entities, dude) {
        if (!strcmp(name, dude->name)) {
            return sol_lua_load_entity (l, dude);
        }
    }

    lua_pushnil(l);
    return 1;
}

static int enter_combat (lua_State *l) {
    luaL_checktype(l, 1, LUA_TTABLE);
    const int x = luaL_checkinteger(l, 2);
    const int y = luaL_checkinteger(l, 2);

    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    sol_arbiter_enter_combat(region, x, y);

    return 0;
}


extern int sol_lua_region_function(sol_region_t *region, const char *func, lua_State *l) {
    //if (!strcmp(func, "set_tile")) { lua_pushcfunction(l, region_set_tile); return 1; }
    if (!strcmp(func, "set_tile_id")) {
        return push_region_function(l, region, region_set_tile_id);
    }
    if (!strcmp(func, "find")) {
        return push_region_function(l, region, get_first_entity);
    }
    if (!strcmp(func, "enter_combat")) {
        return push_region_function(l, region, enter_combat);
    }
    //if (!strcmp(func, "test")) { lua_pushcfunction(l, region_test); return 1; }
    lua_pushinteger(l, 0);
    return 1;
}
