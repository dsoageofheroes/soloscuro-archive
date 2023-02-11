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

// Narrate functions

extern void port_swap_enitity(int obj_id, entity_t *dude);

extern sol_status_t sol_play_sound_effect(const uint16_t id);

extern void port_load_item(item_t *item);
extern int port_load_region(const int region);

extern void port_start();
extern void port_init();
extern void port_close();

typedef enum game_config_e {
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
} game_config_t;

extern void port_set_config(game_config_t gc, ssize_t val);

extern void port_set_lua_globals(lua_State *l);
extern void port_entity_update_scmd(entity_t *entity);

extern void port_handle_input();
extern void port_tick();
extern void port_window_render();
extern void port_start_display_frame();
extern void port_commit_display_frame();

extern void sol_mouse_set_as_power(power_t *pw);
extern void sol_draw_cone(int sx, int sy, int range);
extern uint32_t sol_get_camerax();
extern uint32_t sol_get_cameray();

extern void sol_center_on_player();
extern void sol_camera_scrollx(const int amt);
extern void sol_camera_scrolly(const int amt);

#endif
