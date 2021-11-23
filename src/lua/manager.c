#include "sol-lua-manager.h"
#include "sol-lua-settings.h"
#include "gpl.h"

static lua_State *lua_state = NULL;
static void sol_lua_run(const char *filename, const char *name);

extern lua_State* sol_lua_get_state() {
    if (lua_state) { return lua_state; }

    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    // inject the Lua state with everything.
    sol_lua_settings_register(lua_state);

    return lua_state;
}

extern int sol_lua_load(const char *filename) {
    sol_lua_run(filename, "init");
    return lua_toboolean(lua_state, -1);
}

// TODO: Refactor?
static int sol_lua_error(const char *msg) {
    error("%s: %s\n", msg, lua_tostring(lua_state, -1));
    return 0;
}

static void sol_lua_run(const char *filename, const char *name) {
    sol_lua_get_state();

    if (luaL_loadfile(lua_state, filename)) {
        error("unable to open '%s'.\n", filename);
        return;
    }

    if (lua_pcall(lua_state, 0, 0, 0)) { sol_lua_error("Can't prime"); }
    lua_getglobal(lua_state, name);
    if (lua_pcall(lua_state, 0, 1, 0)) { sol_lua_error("Can't call"); }
}

