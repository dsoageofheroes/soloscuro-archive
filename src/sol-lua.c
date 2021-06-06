#include "sol-lua.h"
#include "lua-structs.h"
#include "ds-load-save.h"
#include "port.h"
#include "dsl.h"
#include "player.h"
#include <string.h>

#define MAX_SOL_FUNCS (1<<10)

static lua_State *sol_lua = NULL;
static struct luaL_Reg sol_lib [MAX_SOL_FUNCS];
static size_t pos = 0;

static void set_globals(lua_State *l);

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
    //port_toggle_screen(SCREEN_INV);
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

static int load_window(lua_State *l) {
    const char *str = luaL_checkstring(l, 1);
    printf("str = '%s'\n", str);
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
    {"load_window", load_window},
    //{"run_browser", sol_run_browser},
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

static int sol_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(sol_lua, -1));
    return 0;
}

static void sol_lua_run(const char *filename, const char *name) {
    sol_lua = luaL_newstate();
    luaL_openlibs(sol_lua);

    set_globals(sol_lua);
    sol_lua_register(sol_lua);

    if (luaL_loadfile(sol_lua, filename)) {
        error("unable to open '%s'.\n", filename);
        sol_lua_close();
        return;
    }

    if (lua_pcall(sol_lua, 0, 0, 0)) { sol_lua_error("Can't prime"); }
    lua_getglobal(sol_lua, name);
    if (lua_pcall(sol_lua, 0, 1, 0)) { sol_lua_error("Can't call"); }
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
    if (lua_pcall(sol_lua, 0, 0, 0)) { sol_lua_error("Can't prime"); }

    lua_getglobal(sol_lua, "settings");
    lua_pushstring(sol_lua, "run");
    lua_gettable(sol_lua, -2);

    lua_str = luaL_checkstring(sol_lua, -1);
    
    if (!strcmp(lua_str, "ds1")) {
        lua_pop(sol_lua, 1);
        lua_pushstring(sol_lua, "ds1_location");
        lua_gettable(sol_lua, -2);
        lua_str = luaL_checkstring(sol_lua, -1);
        gff_load_directory(lua_str);
    } else if (!strcmp(lua_str, "browse-ds1")) {
        lua_pop(sol_lua, 1);
        lua_pushstring(sol_lua, "ds1_location");
        lua_gettable(sol_lua, -2);
        lua_str = luaL_checkstring(sol_lua, -1);
        gff_load_directory(lua_str);
        port_set_config(CONFIG_RUN_BROWSER, 1);
    } else if (!strcmp(lua_str, "browse-ds2")) {
        lua_pop(sol_lua, 1);
        lua_pushstring(sol_lua, "ds2_location");
        lua_gettable(sol_lua, -2);
        lua_str = luaL_checkstring(sol_lua, -1);
        gff_load_directory(lua_str);
        port_set_config(CONFIG_RUN_BROWSER, 1);
    } else if (!strcmp(lua_str, "browse-dso")) {
        lua_pop(sol_lua, 1);
        lua_pushstring(sol_lua, "dso_location");
        lua_gettable(sol_lua, -2);
        lua_str = luaL_checkstring(sol_lua, -1);
        gff_load_directory(lua_str);
        port_set_config(CONFIG_RUN_BROWSER, 1);
    }

    sol_lua_close();
}

extern int sol_lua_load_preload(const char *filename) {
    sol_lua = luaL_newstate();
    luaL_openlibs(sol_lua);
    //lua_struct_register(sol_lua);
    sol_lua_register(sol_lua);

    if (luaL_loadfile(sol_lua, "solconfig.lua")) {
        write_generic_settings();

        if (luaL_loadfile(sol_lua, "solconfig.lua")) {
            sol_lua_close();
            error("unable to open '%s'.\n", "solconfig.lua");
            return 0;
        }
    }

    load_game();

    //sol_lua_run(filename, "preload");
    return 0;
}

extern int sol_lua_load(const char *filename) {
    sol_lua_run(filename, "init");
    if (sol_lua) {
        return lua_toboolean(sol_lua, -1);
    }

    return 0;
}

extern int sol_lua_keydown(const int key_code) {
    if (!sol_lua) { return 0; }

    lua_getglobal(sol_lua, "keydown");
    lua_pushnumber(sol_lua, key_code);
    if (lua_pcall(sol_lua, 1, 1, 0)) { return sol_lua_error("Can't call keydown()"); }

    return lua_toboolean(sol_lua, -1);
}

extern int sol_lua_keyup(const int key_code) {
    if (!sol_lua) { return 0; }

    lua_getglobal(sol_lua, "keyup");
    lua_pushnumber(sol_lua, key_code);
    if (lua_pcall(sol_lua, 1, 1, 0)) { return sol_lua_error("Can't call keyup()"); }

    return lua_toboolean(sol_lua, -1);
}

extern void sol_lua_close() {
    if (sol_lua) {
        lua_close(sol_lua);
        sol_lua = NULL;
    }
}

#define BUF_MAX (128)

static void set_globals(lua_State *l) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "PLAYER_LEFT = %d", PLAYER_LEFT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_RIGHT = %d", PLAYER_RIGHT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_UP = %d", PLAYER_DOWN); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_DOWN = %d", PLAYER_UP); luaL_dostring(l, buf);
    port_set_lua_globals(l);
}
