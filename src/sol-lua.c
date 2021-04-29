#include "sol-lua.h"
#include "lua-structs.h"
#include "ds-load-save.h"
#include <string.h>

#define MAX_SOL_FUNCS (1<<10)

static struct luaL_Reg sol_lib [MAX_SOL_FUNCS];
static size_t pos = 0;

static void add_funcs (const struct luaL_Reg *lib) {
    size_t i = 0;
    while (lib && lib[i].name) {
        printf("->%s\n", lib[i].name);
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

    //player_load(slot, main_get_zoom());
    //player_load_graphics(slot);
    lua_pushboolean(l, 1);
    return 1;
}

static const struct luaL_Reg sol_funcs[] = {
    {"load_charsave", sol_load_charsave},
    //{"load_region", uil_load_region},
    //{"set_player_frame_delay", uil_set_player_frame_delay},
    //{"set_player_move", uil_set_player_move},
    //{"set_xscroll", uil_set_xscroll},
    //{"set_yscroll", uil_set_yscroll},
    //{"player_move", uil_player_move},
    //{"player_unmove", uil_player_unmove},
    //{"toggle_inventory", uil_toggle_inventory},
    //{"set_ignore_repeat", uil_set_ignore_repeat},
    //{"set_quiet", uil_set_quiet},
    //{"load_directory", uil_load_directory},
    //{"run_browser", uil_run_browser},
    //{"exit_game", uil_exit_game},
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
