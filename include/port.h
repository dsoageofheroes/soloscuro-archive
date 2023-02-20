/*
 * These are the functions needed to be implmeneted for porting to different systems.
 * This include both OS specific call and callbacks.
 */

// TODO FIXME PLEASE SUPER PLEASE
// O man does this need to be cleaned up, so much dead code and stale systems.
#ifndef PORT_H
#define PORT_H

#include "status.h"
#include "ssi-object.h"
#include "region.h"
#include "combat.h"
#include "gff.h"
#include "lua-inc.h"

#ifdef _WIN32
#  ifdef _WIN64
#    define PRI_SIZET "%I64u"
#  else
#    define PRI_SIZET "%I32u"
#  endif
#else
#  define PRI_SIZET "%zu"
#endif

#ifdef _WIN32
#  ifdef _WIN64
#    define PRI_LI "%I64d"
#  else
#    define PRI_LI "%I32d"
#  endif
#else
#  define PRI_LI "%lld"
#endif

#include "sprite.h"

extern sol_status_t sol_play_sound_effect(const uint16_t id);
extern sol_status_t sol_set_lua_globals(lua_State *l);
extern sol_status_t sol_mouse_set_as_power(sol_power_t *pw);
extern sol_status_t sol_draw_cone(int sx, int sy, int range);
extern uint32_t sol_get_camerax();
extern uint32_t sol_get_cameray();
extern sol_status_t sol_camera_scrollx(const int amt);
extern sol_status_t sol_camera_scrolly(const int amt);

typedef enum sol_game_config_e {
    CONFIG_REPEAT,
    CONFIG_XSCROLL,
    CONFIG_YSCROLL,
    CONFIG_PLAYER_FRAME_DELAY,
    CONFIG_PLAYER_SET_MOVE,
    CONFIG_PLAYER_MOVE,
    CONFIG_PLAYER_UNMOVE,
    CONFIG_SET_QUIET,
    CONFIG_EXIT,
    CONFIG_RUN_BROWSER,
} sol_game_config_t;

// TODO: clean these up at some point.
extern void port_start();
extern void port_init();
extern void port_close();
extern void port_set_config(sol_game_config_t gc, ssize_t val);
extern void port_handle_input();
extern void port_tick();
extern void port_start_display_frame();
extern void port_commit_display_frame();
extern void sol_center_on_player();

#endif
