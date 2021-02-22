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

static void ui_lua_run(const char *filename, const char *name) {
    ui_lua = luaL_newstate();
    luaL_openlibs(ui_lua);

    ui_lua_state_register(ui_lua);

    if (luaL_loadfile(ui_lua, filename)) {
        error("unable to open '%s'.\n", filename);
        ui_lua_close();
        return;
    }

    if (lua_pcall(ui_lua, 0, 0, 0)) { ui_lua_error("Can't prime"); }
    lua_getglobal(ui_lua, name);
    if (lua_pcall(ui_lua, 0, 1, 0)) { ui_lua_error("Can't call"); }

}

static void write_generic_settings() {
    FILE *file = fopen("solconfig.lua", "w");

    fprintf(file, "settings = {}\n");
    fprintf(file, "settings[\"ds1_location\"] = \"ds1/\"\n");
    fprintf(file, "settings[\"ds2_location\"] = \"ds2/\"\n");
    fprintf(file, "settings[\"dso_location\"] = \"dso/\"\n");
    fprintf(file, "settings[\"run\"] = \"ds1\"\n");
    fprintf(file, "--settings[\"run\"] = \"browse-ds1\"\n");
    fprintf(file, "--settings[\"run\"] = \"browse-ds2\"\n");
    fprintf(file, "--settings[\"run\"] = \"browse-dso\"\n");

    fclose(file);
}

static void load_game() {
    const char *lua_str = NULL;
    if (lua_pcall(ui_lua, 0, 0, 0)) { ui_lua_error("Can't prime"); }

    lua_getglobal(ui_lua, "settings");
    lua_pushstring(ui_lua, "run");
    lua_gettable(ui_lua, -2);

    lua_str = luaL_checkstring(ui_lua, -1);
    
    if (!strcmp(lua_str, "ds1")) {
        lua_pop(ui_lua, 1);
        lua_pushstring(ui_lua, "ds1_location");
        lua_gettable(ui_lua, -2);
        lua_str = luaL_checkstring(ui_lua, -1);
        gff_load_directory(lua_str);
    } else if (!strcmp(lua_str, "browse-ds1")) {
        lua_pop(ui_lua, 1);
        lua_pushstring(ui_lua, "ds1_location");
        lua_gettable(ui_lua, -2);
        lua_str = luaL_checkstring(ui_lua, -1);
        gff_load_directory(lua_str);
        main_set_browser_mode();
    } else if (!strcmp(lua_str, "browse-ds2")) {
        lua_pop(ui_lua, 1);
        lua_pushstring(ui_lua, "ds2_location");
        lua_gettable(ui_lua, -2);
        lua_str = luaL_checkstring(ui_lua, -1);
        gff_load_directory(lua_str);
        main_set_browser_mode();
    } else if (!strcmp(lua_str, "browse-dso")) {
        lua_pop(ui_lua, 1);
        lua_pushstring(ui_lua, "dso_location");
        lua_gettable(ui_lua, -2);
        lua_str = luaL_checkstring(ui_lua, -1);
        gff_load_directory(lua_str);
        main_set_browser_mode();
    }

    ui_lua_close();
}

int ui_lua_load_preload(const char *filename) {
    ui_lua = luaL_newstate();
    luaL_openlibs(ui_lua);

    if (luaL_loadfile(ui_lua, "solconfig.lua")) {
        write_generic_settings();

        if (luaL_loadfile(ui_lua, "solconfig.lua")) {
            ui_lua_close();
            error("unable to open '%s'.\n", "solconfig.lua");
            return 0;
        }
    }

    load_game();

    //ui_lua_run(filename, "preload");
    return 0;
}

int ui_lua_load(const char *filename) {
    ui_lua_run(filename, "init");
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

static int uil_set_quiet(lua_State *l) {
    dsl_set_quiet(luaL_checkinteger(l, 1));
    return 0;
}

static int uil_load_directory(lua_State *l) {
    const char *str = luaL_checkstring(l, 1);
    printf("str = %s\n", str);
    gff_load_directory(str);
    return 0;
}

extern void browse_loop(SDL_Surface*, SDL_Renderer *rend);
static int uil_run_browser(lua_State *l) {
    browse_loop(main_get_screen(), main_get_rend());
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
    {"set_quiet", uil_set_quiet},
    {"load_directory", uil_load_directory},
    {"run_browser", uil_run_browser},
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
