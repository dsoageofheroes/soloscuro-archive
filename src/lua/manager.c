#include "sol-lua-manager.h"
#include "sol-lua-settings.h"
#include "gpl.h"
#include "gpl-state.h"

static lua_State *lua_state = NULL;
static sol_status_t sol_lua_run(const char *filename, const char *name);

extern sol_status_t sol_lua_get_state(lua_State **l) {
    if (!l) { return SOL_NULL_ARGUMENT; }
    if (lua_state) { *l = lua_state; return SOL_SUCCESS; }

    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    // inject the Lua state with everything.
    sol_lua_settings_register(lua_state);
    sol_gpl_state_register(lua_state);
    luaL_dostring(lua_state, "function init() end\n");
    luaL_dostring(lua_state, "function keydown() end\n");
    luaL_dostring(lua_state, "function keyup() end\n");
    luaL_dostring(lua_state, "function idle() end\n");

    *l = lua_state;
    return SOL_SUCCESS;
}

extern sol_status_t sol_lua_load(const char *filename) {
    return sol_lua_run(filename, "init");
}

// TODO: Refactor?
static int sol_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(lua_state, -1));
    return 0;
}

extern sol_status_t sol_lua_run_function(const char *function) {

    lua_getglobal(lua_state, function);
    if (lua_pcall(lua_state, 0, 0, 0)) {
        sol_lua_error("Can't call");
        lua_pop(lua_state, 1);
        return SOL_LUA_NO_FUNCTION;
    }

    return SOL_SUCCESS;
}

static sol_status_t sol_lua_run(const char *filename, const char *name) {
    lua_State *l;
    sol_lua_get_state(&l);

    if (luaL_loadfile(lua_state, filename)) {
        error("unable to open '%s'.\n", filename);
        return SOL_FILESYSTEM_ERROR;
    }

    if (lua_pcall(lua_state, 0, 0, 0)) { sol_lua_error("Can't prime"); return 1;}
    return sol_lua_run_function(name);
}

