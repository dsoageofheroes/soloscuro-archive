#include "sol-lua-manager.h"
#include "ds-load-save.h"
#include "port.h"
#include "gameloop.h"
#include "description.h"
#include "gpl.h"
#include "player.h"
#include "inventory.h"
#include "map.h"
#include "narrate.h"
#include "combat-status.h"
#include "window-main.h"
#include "view-character.h"
#include "new-character.h"
#include "sol-lua-manager.h"
#include "sol-lua-settings.h"
#include "region-manager.h"
#include "lua-inc.h"
#include <string.h>
#include <ctype.h>

#define MAX_SOL_FUNCS (1<<10)

//static lua_State *sol_lua = NULL;
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

    if (sol_load_character_charsave(slot, id) != SOL_SUCCESS) {
        lua_pushboolean(l, 0);
        return 1;
    }

    sol_player_load(slot);
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
    sol_window_toggle(&inventory_window, 0, 0);
    return 0;
}

static int change_region(lua_State *l) {
    sol_region_t* reg;
    sol_region_manager_get_region(luaL_checkinteger(l, 1), 0, &reg);
    if (!reg) { return lua_return_bool(l, 0); }
    sol_region_manager_set_current(reg);
    return lua_return_bool(l, 1);
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

static int debug_set_desc(lua_State *l) {
    const int type       = luaL_checkinteger(l, 1);
    const int level      = luaL_checkinteger(l, 2);
    const int idx        = luaL_checkinteger(l, 3);
    sol_power_list_t *powers = NULL;
    sol_power_instance_t *pi = NULL;

    //printf("debug_set_desc(%d, %d, %d)\n", type, level, idx);
    if (type == 0) {
        sol_wizard_get_spells(level, &powers);
    }

    if (!powers) { return 0; }
    pi = powers->head;

    for (int i = 0; pi && i < idx; i++) {
        pi = pi->next;
    }

    if (!pi) { return 0; }

    sol_description_set_message(pi->stats->description);
    sol_description_set_icon(pi->stats->icon_id);

    return 0;
}

static int load_window(lua_State *l) {
    const char *str = luaL_checkstring(l, 1);
    static char has_been_initialized = 0;

    if (!has_been_initialized) {
        port_init();
        has_been_initialized = 1;
    }

    if (!strcmp(str, "view")) {
        sol_window_toggle(&view_character_window, 0, 0);
    } else if (!strcmp(str, "inventory")) {
        sol_window_toggle(&inventory_window, 0, 0);
    } else if (!strcmp(str, "main")) {
        sol_window_toggle(&main_window, 0, 0);
    } else if (!strcmp(str, "character-creation")) {
        sol_window_toggle(&new_character_window, 0, 0);
    } else if (!strcmp(str, "map")) {
        sol_window_toggle(&map_window, 0, 0);
    } else if (!strcmp(str, "narrate")) {
        sol_window_toggle(&narrate_window, 0, 0);
    } else if (!strcmp(str, "combat")) {
        sol_window_toggle(&combat_status_window, 0, 0);
    } else if (!strcmp(str, "description")) {
        sol_window_toggle(&description_window, 0, 0);
    }

    return 0;
}

static int game_loop(lua_State *l) {
    sol_game_loop();
    return 0;
}

static int exit_game(lua_State *l) {
    port_set_config(CONFIG_EXIT, 1);
    return 0;
}

static const struct luaL_Reg sol_funcs[] = {
    {"load_charsave", sol_load_charsave},
    {"change_region", change_region},
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
    {"debug_set_desc", debug_set_desc},
    {"game_loop", game_loop},
    //{"run_browser", sol_run_browser},
    {"exit_game", exit_game},
    {NULL, NULL},
};

// Note make sure to call this after the soloscuro table has been created.
static void add_wizard_globals(lua_State *l) {
    char buf[128];
    sol_power_t *pw = NULL;

    for (int i = 0; i < WIZ_MAX; i++) {
        sol_wizard_get_spell(i, &pw);
        if (pw) {
            snprintf(buf, 128, "soloscuro.WIZ_%s=%d", pw->name, i);
            for (int j = 9; j < 128 && buf[j] != '=' && buf[j] != '\0'; j++) {
                if (isalpha(buf[j])) { buf[j] = toupper(buf[j]); }
                if (buf[j] == ' ') { buf[j] = '_'; }
            }
            luaL_dostring(l, buf);
        }
    }
}

extern sol_status_t sol_lua_settings_register(lua_State *l) {
    const struct luaL_Reg *r;
    set_globals(l);
    pos = 0;
    memset(sol_lib, 0x0, sizeof(sol_lib));
    lua_newtable(l);
    sol_lua_struct_get_funcs(&r);
    add_funcs(r);
    add_funcs(sol_funcs);
    sol_ds_get_lib(&r);
    add_funcs(r);

    luaL_setfuncs(l, sol_lib, 0);
    lua_setglobal(l, "soloscuro");

    sol_lua_struct_register(l);
    return SOL_SUCCESS;
}

extern sol_status_t sol_lua_register_globals() {
    lua_State *l;
    sol_lua_get_state(&l);
    add_wizard_globals(l);
    return SOL_SUCCESS;
}

static int sol_lua_error(const char *msg) {
    lua_State *l;
    sol_lua_get_state(&l);
    error("%s: %s\n", msg, lua_tostring(l, -1));
    return 0;
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
    lua_State *sol_lua;

    sol_lua_get_state(&sol_lua);
    if (lua_pcall(sol_lua, 0, 0, 0)) { sol_lua_error("Can't prime"); }

    lua_getglobal(sol_lua, "settings");
    lua_pushstring(sol_lua, "run");
    lua_gettable(sol_lua, -2);

    lua_str = luaL_checkstring(sol_lua, -1);
    
    gff_init();
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
}

extern sol_status_t sol_lua_load_preload(const char *filename) {
    lua_State *sol_lua;

    sol_lua_get_state(&sol_lua);
    if (luaL_loadfile(sol_lua, "solconfig.lua")) {
        write_generic_settings();

        if (luaL_loadfile(sol_lua, "solconfig.lua")) {
            error("unable to open '%s'.\n", "solconfig.lua");
            return SOL_FILESYSTEM_ERROR;
        }
    }

    load_game();

    return SOL_SUCCESS;
}

extern sol_status_t sol_lua_keydown(const int key_code) {
    lua_State *sol_lua;

    sol_lua_get_state(&sol_lua);
    if (!sol_lua) { return SOL_LUA_NO_INSTANCE; }

    lua_getglobal(sol_lua, "keydown");
    lua_pushnumber(sol_lua, key_code);
    if (lua_pcall(sol_lua, 1, 1, 0)) { return sol_lua_error("Can't call keydown()"); }

    return lua_toboolean(sol_lua, -1) ? SOL_SUCCESS : SOL_UNKNOWN_KEY;
}

extern sol_status_t sol_lua_keyup(const int key_code) {
    lua_State *sol_lua;

    sol_lua_get_state(&sol_lua);
    if (!sol_lua) { return SOL_LUA_NO_INSTANCE; }

    lua_getglobal(sol_lua, "keyup");
    lua_pushnumber(sol_lua, key_code);
    if (lua_pcall(sol_lua, 1, 1, 0)) { return sol_lua_error("Can't call keyup()"); }

    return lua_toboolean(sol_lua, -1) ? SOL_SUCCESS : SOL_UNKNOWN_KEY;
}

#define BUF_MAX (128)

static void set_globals(lua_State *l) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "PLAYER_LEFT = %d", PLAYER_LEFT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_RIGHT = %d", PLAYER_RIGHT); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_UP = %d", PLAYER_DOWN); luaL_dostring(l, buf);
    snprintf(buf, BUF_MAX, "PLAYER_DOWN = %d", PLAYER_UP); luaL_dostring(l, buf);
    sol_set_lua_globals(l);
}
