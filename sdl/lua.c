#include <SDL2/SDL.h>
#include "lua.h"
#include "screen-manager.h"
#include "../src/dsl.h"
#include "main.h"
#include "player.h"
#include "../src/ds-load-save.h"
#include "screens/inventory.h"

static lua_State *ui_lua = NULL;
static void ui_lua_state_register(lua_State *l);

static int ui_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(ui_lua, -1));
    return 0;
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

int ui_lua_keydown(const int key_code) {
    if (!ui_lua) { return 0; }

    lua_getglobal(ui_lua, "keydown");
    lua_pushnumber(ui_lua, key_code);
    if (lua_pcall(ui_lua, 1, 1, 0)) { return ui_lua_error("Can't call keydown()"); }

    return lua_toboolean(ui_lua, -1);
}

int ui_lua_keyup(const int key_code) {
    if (!ui_lua) { return 0; }

    lua_getglobal(ui_lua, "keyup");
    lua_pushnumber(ui_lua, key_code);
    if (lua_pcall(ui_lua, 1, 1, 0)) { return ui_lua_error("Can't call keyup()"); }

    return lua_toboolean(ui_lua, -1);
}

void ui_lua_close() {
    if (ui_lua) {
        lua_close(ui_lua);
        ui_lua = NULL;
    }
}

static int uil_toggle_inventory(lua_State *l) {
    screen_toggle_screen(main_get_rend(), &inventory_screen, 0, 0);
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

static int uil_set_player_frame_delay(lua_State *l) {
    player_set_delay(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_set_player_move(lua_State *l) {
    player_set_move(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_player_move(lua_State *l) {
    player_move(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_player_unmove(lua_State *l) {
    player_unmove(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_set_xscroll(lua_State *l) {
    main_set_xscroll(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_set_yscroll(lua_State *l) {
    main_set_yscroll(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_set_ignore_repeat(lua_State *l) {
    main_set_ignore_repeat(lua_toboolean(l, 1));
    return 0;
}

static int uil_exit_game(lua_State *l) {
    main_exit_game();
    return 0;
}

static const struct luaL_Reg ds_funcs[] = {
    {"load_charsave", uil_load_charsave},
    {"load_region", uil_load_region},
    {"set_player_frame_delay", uil_set_player_frame_delay},
    {"set_player_move", uil_set_player_move},
    {"set_xscroll", uil_set_xscroll},
    {"set_yscroll", uil_set_yscroll},
    {"player_move", uil_player_move},
    {"player_unmove", uil_player_unmove},
    {"toggle_inventory", uil_toggle_inventory},
    {"set_ignore_repeat", uil_set_ignore_repeat},
    {"exit_game", uil_exit_game},
    {NULL, NULL},
};

#define BUF_MAX (128)

static void set_globals(lua_State *l) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "PLAYER_LEFT = %d", PLAYER_LEFT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_RIGHT = %d", PLAYER_RIGHT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_UP = %d", PLAYER_DOWN); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_DOWN = %d", PLAYER_UP); luaL_dostring(l, buf);
    uil_set_globals(l);
}

static void ui_lua_state_register(lua_State *l) {
    set_globals(l);
    lua_newtable(l);
    luaL_setfuncs(l, ds_funcs, 0);
    lua_setglobal(l, "ds");
}
