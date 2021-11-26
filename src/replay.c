#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "lua-inc.h"
#include "replay.h"
#include "gameloop.h"
#include "trigger.h"
#include "narrate.h"

static FILE *replay = NULL; // to write to
static FILE *replay_file = NULL; // to read from
static lua_State *replay_lua = NULL;
static int replay_mode = 0;

void replay_init(const char *path) {
    replay = fopen(path, "w+");
}

void replay_print(const char *str, ...) {
    va_list args;
    if (replay) {
        va_start(args, str);
        vfprintf(replay, str, args);
        fflush(replay);
        va_end(args);
    }
}

int in_replay_mode() {
    return replay_mode;
}

static int replay_change_region(lua_State *l) {
    //lua_Integer region = luaL_checkinteger(l, 1);
    printf("change region\n");
    return 0;
}

static int replay_talk_click(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    //printf("REPLAY: clicking ob obj: %lld\n", obj);
    fflush(stdout);
    sol_trigger_talk_click(obj);
    return 0;
}

static int replay_select_menu(lua_State *l) {
    lua_Integer menu_id = luaL_checkinteger(l, 1);
    //printf("REPLAY: selecting menu: %lld\n", menu_id);
    fflush(stdout);
    narrate_select_menu(menu_id);
    return 0;
}

static int replay_signal(lua_State *l) {
    lua_Integer signal = luaL_checkinteger(l, 1);
    lua_Integer _accum = luaL_checkinteger(l, 2);

    //printf("REPLAY: signal: %lld, %lld\n", signal, _accum);
    fflush(stdout);
    //printf("NEED TO GET game_loop_signal back!\n");
    sol_game_loop_signal(signal, _accum);

    return 0;
}

static const struct luaL_Reg replay_state_lib[] = {
    {"change_region", replay_change_region},
    {"talk_click", replay_talk_click},
    {"select_menu", replay_select_menu},
    {"signal", replay_signal},
    {NULL, NULL},
};

#define LINE_SIZE (1024)

int replay_next() {
    char line[LINE_SIZE];

    if (!replay_file || !replay_lua) {
        printf("exiting replay...\n");
        return 0;
    }

    if (fgets(line, LINE_SIZE, replay_file)) {
        //render();
        //tick();
        if (luaL_dostring(replay_lua, line)) {
            printf("Error: unable to execute line '%s'.\n", line);
            printf("error: %s\n", lua_tostring(replay_lua, -1));
            replay_mode = 0;
            return 0;
        }
    } else {
        replay_mode = 0;
    }

    return 1;
}

void replay_game(const char *file) {
    replay_file = fopen(file, "r+");

    if (!replay_file) {
        printf("Unable to open '%s'\n", file);
        exit(1);
    }

    replay_mode = 1;
    replay_lua = luaL_newstate();
    luaL_openlibs(replay_lua);
    lua_newtable(replay_lua);
    luaL_setfuncs(replay_lua, replay_state_lib, 0);
    lua_setglobal(replay_lua, "rep");

    //replay_continue();
    //printf("EXITING REPLAY CONTINUE\n");
    //replay_mode = 0;

    //lua_close(replay_lua);
    //fclose(replay_file);

    //replay_lua = NULL;
    //replay_file = NULL;

    //printf("ENTERING NORMAL GAME LOOP!!!!\n");
    //game_loop();
}

void replay_cleanup() {
    if (replay) {
        fclose(replay);
    }
}
