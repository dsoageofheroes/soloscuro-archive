#include "sol-lua-manager.h"
#include "sol-lua-settings.h"
#include "gpl.h"
#include "gpl-state.h"

static lua_State *lua_state = NULL;
static int sol_lua_run(const char *filename, const char *name);

extern lua_State* sol_lua_get_state() {
    if (lua_state) { return lua_state; }

    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    // inject the Lua state with everything.
    sol_lua_settings_register(lua_state);
    gpl_state_register(lua_state);
    luaL_dostring(lua_state, "function init() end\n");
    luaL_dostring(lua_state, "function keydown() end\n");
    luaL_dostring(lua_state, "function keyup() end\n");
    luaL_dostring(lua_state, "function idle() end\n");

    return lua_state;
}

extern int sol_lua_load(const char *filename) {
    return sol_lua_run(filename, "init");
}

// TODO: Refactor?
static int sol_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(lua_state, -1));
    return 0;
}

extern int sol_lua_run_function(const char *function) {

    lua_getglobal(lua_state, function);
    if (lua_pcall(lua_state, 0, 0, 0)) {
        sol_lua_error("Can't call");
        lua_pop(lua_state, 1);
        return 1;
    }

    return 0;
}

static int sol_lua_run(const char *filename, const char *name) {
    sol_lua_get_state();

    if (luaL_loadfile(lua_state, filename)) {
        error("unable to open '%s'.\n", filename);
        return 1;
    }

    if (lua_pcall(lua_state, 0, 0, 0)) { sol_lua_error("Can't prime"); return 1;}
    return sol_lua_run_function(name);
}

