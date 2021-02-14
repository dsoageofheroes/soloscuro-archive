#include "lua.h"
#include "screen-manager.h"
#include "../src/dsl.h"
#include "main.h"
#include "player.h"
#include "../src/ds-load-save.h"

static lua_State *ui_lua = NULL;
static void ui_lua_state_register(lua_State *l);

static void ui_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(ui_lua, -1));
}

int ui_lua_load(const char *filename) {
    ui_lua = luaL_newstate();
    luaL_openlibs(ui_lua);

    ui_lua_state_register(ui_lua);

    if (luaL_loadfile(ui_lua, filename)) {
        error("unable to open '%s'.\n", filename);
        ui_lua_close();
        return 0;
    }

    if (lua_pcall(ui_lua, 0, 0, 0)) { ui_lua_error("Can't prime init()"); }
    lua_getglobal(ui_lua, "init");
    if (lua_pcall(ui_lua, 0, 1, 0)) { ui_lua_error("Can't call init()"); }

    return lua_toboolean(ui_lua, -1);
}

void ui_lua_close() {
    if (ui_lua) {
        lua_close(ui_lua);
        ui_lua = NULL;
    }
}

static int uil_toggle_inventory(lua_State *l) {
    //lua_Integer id = luaL_checkinteger(l, 1);
    //if (id < 0 || id >= MAX_GFLAGS) {
        //printf("ERROR: " PRI_LI " is out of range for global flags!\n", id);
        //exit(1);
    //}
    //lua_pushinteger(l, dsl_global_flags[id]);
    //screen_toggle_screen(renderer, &inventory_screen, 0, 0);
    printf("HERE!\n");
    return 0;
}

static int lua_return_bool(lua_State *l, int ret) {
    if (!ret) {
        lua_pushboolean(l, 0);
        return 1;
    }

    lua_pushboolean(l, 1);
    return 1;
}

static int uil_load_charsave(lua_State *l) {
    lua_Integer slot = luaL_checkinteger(l, 1);
    lua_Integer id = luaL_checkinteger(l, 2);

    if (!ds_load_character_charsave(slot, id)) {
        lua_pushboolean(l, 0);
        return 1;
    }

    player_load(main_get_rend(), slot, main_get_zoom());
    player_load_graphics(main_get_rend());
    lua_pushboolean(l, 1);
    return 1;
}

static int uil_load_region(lua_State *l) {
    return lua_return_bool(l,
        screen_load_region(main_get_rend(), luaL_checkinteger(l, 1)));
}

static const struct luaL_Reg ds_funcs[] = {
    {"load_charsave", uil_load_charsave},
    {"load_region", uil_load_region},
    {"toggle_inventory", uil_toggle_inventory},
    {NULL, NULL},
};
/*
*/

#define BUF_SIZE (1<<10)

static void set_globals(lua_State *l) {
    char buf[BUF_SIZE];
    snprintf(buf, BUF_SIZE, "GLOBAL = %d\n", 33);
    luaL_dostring(l, buf);
}

static void ui_lua_state_register(lua_State *l) {
    set_globals(l);
    lua_newtable(l);
    luaL_setfuncs(l, ds_funcs, 0);
    lua_setglobal(l, "ds");
}
