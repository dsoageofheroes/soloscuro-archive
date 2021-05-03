#include <SDL2/SDL.h>
#include "screen-manager.h"
#include "../src/dsl.h"
#include "main.h"
#include "uil.h"
#include "player.h"
#include "../src/ds-load-save.h"
#include "screens/inventory.h"
#include "../src/lua-inc.h"
#include "../src/lua-structs.h"
#include "../src/sol-lua.h"

static lua_State *ui_lua = NULL;
static void ui_lua_state_register(lua_State *l);
extern void uil_set_globals(lua_State *l);

static int ui_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(ui_lua, -1));
    return 0;
}

static void ui_lua_run(const char *filename, const char *name) {
    ui_lua = luaL_newstate();
    luaL_openlibs(ui_lua);

    ui_lua_state_register(ui_lua);
    sol_lua_register(ui_lua);

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

extern int ui_lua_load_preload(const char *filename) {
    ui_lua = luaL_newstate();
    luaL_openlibs(ui_lua);
    //lua_struct_register(ui_lua);
    sol_lua_register(ui_lua);

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

extern int ui_lua_load(const char *filename) {
    ui_lua_run(filename, "init");
    if (ui_lua) {
        return lua_toboolean(ui_lua, -1);
    }

    return 0;
}

extern int ui_lua_keydown(const int key_code) {
    if (!ui_lua) { return 0; }

    lua_getglobal(ui_lua, "keydown");
    lua_pushnumber(ui_lua, key_code);
    if (lua_pcall(ui_lua, 1, 1, 0)) { return ui_lua_error("Can't call keydown()"); }

    return lua_toboolean(ui_lua, -1);
}

extern int ui_lua_keyup(const int key_code) {
    if (!ui_lua) { return 0; }

    lua_getglobal(ui_lua, "keyup");
    lua_pushnumber(ui_lua, key_code);
    if (lua_pcall(ui_lua, 1, 1, 0)) { return ui_lua_error("Can't call keyup()"); }

    return lua_toboolean(ui_lua, -1);
}

extern void ui_lua_close() {
    if (ui_lua) {
        lua_close(ui_lua);
        ui_lua = NULL;
    }
}

static const struct luaL_Reg ds_funcs[] = {
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
