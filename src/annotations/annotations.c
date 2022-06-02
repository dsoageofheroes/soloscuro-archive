#include "annotations.h"
#include "sol-lua-manager.h"
#include "gpl.h"

extern void sol_annotations_init() {
}

extern void sol_annotations_cleanup() {
}

extern int sol_annotations_call_func(const uint32_t file, const uint32_t func, int obj) {
    char buf[1024];
    lua_State *l = sol_lua_get_state();
    int ret = 0;

    sprintf(buf, "lua/base/%d.lua", file);
    if (ret = luaL_loadfile(l, buf)) {
        error("unable to open '%s'.\n", buf);
        printf("ret = %d\n", ret);
        return 1;
    }

    if (lua_pcall(l, 0, 0, 0)) {
        error("Can't prime");
        return 1;
    }

    sprintf(buf, "func%d", func);
    lua_getglobal(l, buf);
    lua_pushinteger(l, obj);
    if (lua_pcall(l, 1, 0, 0)) {
        error("Can't call");
        lua_pop(l, 1);
        lua_pop(l, 1);
        return 1;
    }

    return 0;
}
