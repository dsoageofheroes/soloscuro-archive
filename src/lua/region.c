#include "player.h"
#include "region.h"
#include "region-manager.h"
#include "gff.h"
#include "port.h"
#include "arbiter.h"
#include "status.h"
#include "gpl-manager.h"
#include <string.h>
#include <float.h>
#include <ctype.h>

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

// this is just annoying lua...
static int region_set_flag_ids(lua_State *l) {
    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    size_t size, pos = 0, bpos = 0, i = 0;
    const int index = luaL_checkinteger(l, 1);
    const char *flags = luaL_checklstring (l, 2, &size);
    char buf[128];

    if (index < 0 || index >= MAP_ROWS) { return 0; }
    while (pos < size && pos < MAP_COLUMNS) {
        while (pos < size && isspace(*flags)) { flags++; pos++; }
        bpos = 0;
        while (bpos < 127 && pos < size && !isspace(*flags)) { buf[bpos++] = *flags; flags++; pos++; }
        buf[bpos] = '\0';
        region->flags[index][i++] = atol(buf);
        //printf("set[%d][%d] = %d\n", index, (int)i - 1, region->flags[index][i-1]);
    }

    return 0;
}

static int region_get_flag_id(lua_State *l) {
    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    const int x = luaL_checkinteger(l, 1);
    const int y = luaL_checkinteger(l, 2);

    if (x < 0 || x >= MAP_ROWS || y < 0 || y >= MAP_COLUMNS) { return 0; }
    lua_pushinteger(l, region->flags[x][y]);

    return 1;
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

    sol_entity_list_for_each(region->entities, dude) {
        if (!strcmp(name, dude->name)) {
            return sol_lua_load_entity (l, dude);
        }
    }

    lua_pushnil(l);
    return 1;
}

static int enter_combat (lua_State *l) {
    luaL_checktype(l, 1, LUA_TTABLE);
    sol_status_t status = SOL_SUCCESS;
    const int x = luaL_checkinteger(l, 2);
    const int y = luaL_checkinteger(l, 3);

    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    sol_status_warn(sol_arbiter_enter_combat(region, x, y), "Can't put entity in combat.");

    return 0;
}

static int add_entity (lua_State *l) {
    luaL_checktype(l, 1, LUA_TTABLE);

    sol_region_t *region = (sol_region_t*) lua_touserdata(l, lua_upvalueindex(1));
    sol_entity_t *entity = (sol_entity_t*) sol_lua_get_userdata(l, 1);

    //return sol_animate_shift_entity(region->entities, entity_list_add(region->entities, entity));
    sol_entity_list_add(region->entities, entity, NULL);
    return 0;
    //return sol_entity_list_add(region->entities, entity, NULL) == SOL_SUCCESS;
}


extern int sol_lua_region_function(sol_region_t *region, const char *func, lua_State *l) {
    if (!strcmp(func, "set_tile_id")) {
        return push_region_function(l, region, region_set_tile_id);
    }
    if (!strcmp(func, "set_flag_ids")) {
        return push_region_function(l, region, region_set_flag_ids);
    }
    if (!strcmp(func, "get_flag_id")) {
        return push_region_function(l, region, region_get_flag_id);
    }
    if (!strcmp(func, "find")) {
        return push_region_function(l, region, get_first_entity);
    }
    if (!strcmp(func, "enter_combat")) {
        return push_region_function(l, region, enter_combat);
    }
    if (!strcmp(func, "add_entity")) {
        return push_region_function(l, region, add_entity);
    }
    if (!strcmp(func, "run_mas")) {
        sol_gpl_lua_execute_script(region->map_id, 0, 1);
    }
    lua_pushinteger(l, 0);
    return 1;
}
