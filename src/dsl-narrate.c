#include <stdio.h>
#include <stdlib.h>
#include "dsl-narrate.h"
#include "dsl.h"
#include "dsl-var.h"

int8_t narrate_open(int16_t action, char *text, int16_t index) {
    switch(action) {
        case NAR_ADD_MENU:
            warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            break;
        case NAR_PORTRAIT:
            warn("I need to create narrate_box with portrait index %d, text = '%s'\n", index, text);
            break;
        case NAR_SHOW_TEXT:
            warn("I need to show text (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_SHOW_MENU:
            warn("I need to show menu (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_EDIT_BOX:
            warn("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            error("narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }
    /*
    lua_getglobal(lua_state, "dsl");
    lua_getfield(lua_state, -1, "narrate_open2");
    //lua_getglobal(lua_state, "dsl.narrate_open2");
    //lua_getfield(lua_state, LUA_GLOBALSINDEX, "narrate_open");
    lua_pushnumber(lua_state, 1);
    lua_pushnumber(lua_state, 2);
    lua_pushnumber(lua_state, 3);
    if (lua_pcall(lua_state, 3, 0, 0) != 0) {
        error("error running function `dsl.narrate_open': %s\n", lua_tostring(lua_state, -1));
    } else {
        error("NON error running function `dsl.narrate_open': %s\n", lua_tostring(lua_state, -1));
    }
    */
    return 0;
}
