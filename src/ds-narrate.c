#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl-manager.h"
#include "ds-narrate.h"
#include "dsl.h"
#include "dsl-var.h"
#include "../sdl/gameloop.h"
#include "port.h"
#include "replay.h"

#define MAX_MENUS (16)
#define MAX_LINE (128)

static size_t menu_pos = 0;
static uint32_t menu_addrs[MAX_MENUS];
static char menu_text[MAX_MENUS][MAX_LINE];

int8_t narrate_open(int16_t action, const char *text, int16_t index) {
    if (action == NAR_SHOW_TEXT) {
        if (strncmp("END", text, 3) == 0) {
            game_loop_wait_for_signal(WAIT_NARRATE_CONTINUE);
            return 0;
        }
        if (strncmp("CLOSE", text, 5) == 0) {
            port_narrate_close();
            return 0;
        }
    }
    port_narrate_open(action, text, index);
    switch(action) {
        case NAR_ADD_MENU:
            strncpy(menu_text[menu_pos], text, MAX_LINE);
            menu_addrs[menu_pos++] = index;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            break;
        case NAR_PORTRAIT:
            menu_pos = 0;
            //warn("I need to create narrate_box with portrait index %d, text = '%s'\n", index, text);
            break;
        case NAR_SHOW_TEXT:
            //warn("I need to show text (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_SHOW_MENU:
            //warn("I need to show menu (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_EDIT_BOX:
            //warn("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            error("narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }
    /*
    */
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

static int option_is_exit(const int option) {
    if (option >= menu_pos || option < 0) { return 0; }
    if (strncmp("Goodbye.", menu_text[option], 8) == 0) { return 1; }
    if (strncmp("END", menu_text[option], 3) == 0) { return 1; }
    if (strncmp("CLOSE", menu_text[option], 5) == 0) { return 1; }
    return 0;
}

int narrate_select_menu(int option) {
    int accum = !option_is_exit(option);
    char buf[1024];

    replay_print("rep.select_menu(%d)\n", option);
    if (option >= menu_pos || option < 0) {
        error("select_menu: Menu option %d selected, but only (0 - " PRI_SIZET ") available!\n", option, menu_pos - 1);
        return -1;
    }
    menu_pos = 0;
    port_narrate_clear();
    snprintf(buf, 1024, "func%d()\n", menu_addrs[option]);
    dsl_execute_string(buf);
    //game_loop_signal(WAIT_NARRATE_SELECT, accum);
    return accum;
}
