#include "sol-lua.h"
#include "lua-structs.h"
#include "ds-load-save.h"
#include "port.h"
#include <string.h>

#define MAX_SOL_FUNCS (1<<10)

static struct luaL_Reg sol_lib [MAX_SOL_FUNCS];
static size_t pos = 0;

static int lua_return_bool(lua_State *l, int ret) {
    if (!ret) {
        lua_pushboolean(l, 0);
        return 1;
    }

    lua_pushboolean(l, 1);
    return 1;
}

static void add_funcs (const struct luaL_Reg *lib) {
    size_t i = 0;
    while (lib && lib[i].name) {
        sol_lib[pos++] = lib[i++];
    }
}

static int sol_load_charsave(lua_State *l) {
    lua_Integer slot = luaL_checkinteger(l, 1);
    lua_Integer id = luaL_checkinteger(l, 2);

    if (!ds_load_character_charsave(slot, id)) {
        lua_pushboolean(l, 0);
        return 1;
    }

    port_player_load(slot);
    lua_pushboolean(l, 1);
    return 1;
}

static int set_ignore_repeat(lua_State *l) {
    port_set_config(CONFIG_REPEAT, lua_toboolean(l, 1));
    return 0;
}

static int set_xscroll(lua_State *l) {
    port_set_config(CONFIG_XSCROLL, lua_tointeger(l, 1));
    return 0;
}

static int set_yscroll(lua_State *l) {
    port_set_config(CONFIG_YSCROLL, lua_tointeger(l, 1));
    return 0;
}

static int toggle_inventory(lua_State *l) {
    port_toggle_screen(SCREEN_INV);
    return 0;
}

static int load_region(lua_State *l) {
    return lua_return_bool(l,
        port_load_region(luaL_checkinteger(l, 1)));
}

static int set_player_frame_delay(lua_State *l) {
    port_set_config(CONFIG_PLAYER_FRAME_DELAY, lua_tointeger(l, 1));
    return 0;
}

static int set_player_move(lua_State *l) {
    port_set_config(CONFIG_PLAYER_SET_MOVE, lua_tointeger(l, 1));
    return 0;
}

static int player_move(lua_State *l) {
    port_set_config(CONFIG_PLAYER_MOVE, lua_tointeger(l, 1));
    return 0;
}

static int player_unmove(lua_State *l) {
    port_set_config(CONFIG_PLAYER_UNMOVE, lua_tointeger(l, 1));
    return 0;
}

static int set_quiet(lua_State *l) {
    port_set_config(CONFIG_SET_QUIET, lua_tointeger(l, 1));
    return 0;
}

static int load_directory(lua_State *l) {
    const char *str = luaL_checkstring(l, 1);
    gff_load_directory(str);
    return 0;
}

static int exit_game(lua_State *l) {
    port_set_config(CONFIG_EXIT, 1);
    return 0;
}

static const struct luaL_Reg sol_funcs[] = {
    {"load_charsave", sol_load_charsave},
    {"load_region", load_region},
    {"set_player_frame_delay", set_player_frame_delay},
    {"set_player_move", set_player_move},
    {"set_xscroll", set_xscroll},
    {"set_yscroll", set_yscroll},
    {"player_move", player_move},
    {"player_unmove", player_unmove},
    {"toggle_inventory", toggle_inventory},
    {"set_ignore_repeat", set_ignore_repeat},
    {"set_quiet", set_quiet},
    {"load_directory", load_directory},
    //{"run_browser", uil_run_browser},
    {"exit_game", exit_game},
    {NULL, NULL},
};

extern void sol_lua_register(lua_State *l) {
    pos = 0;
    memset(sol_lib, 0x0, sizeof(sol_lib));
    add_funcs(lua_struct_get_funcs());
    add_funcs(sol_funcs);

    lua_newtable(l);
    luaL_setfuncs(l, sol_lib, 0);
    lua_setglobal(l, "soloscuro");

    lua_struct_register(l);
}
