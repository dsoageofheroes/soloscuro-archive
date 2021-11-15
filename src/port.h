/*
 * These are the functions needed to be implmeneted for porting to different systems.
 * This include both OS specific call and callbacks.
 */
#ifndef PORT_H
#define PORT_H

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

typedef enum window_e {
    WINDOW_VIEW,
    WINDOW_INVENTORY,
    WINDOW_MAIN,
    WINDOW_CHARACTER_CREATION,
    WINDOW_MAP,
    WINDOW_NARRATE,
    WINDOW_COMBAT,
} window_t;

// Narrate functions
extern int8_t port_narrate_open(int16_t action, const char *text, int16_t index);
extern void port_narrate_clear();
extern void port_narrate_close();

extern int port_ask_yes_no();
extern void port_enter_combat();
extern void port_exit_combat();

extern void port_add_entity(entity_t *entity, gff_palette_t *pal);
extern void port_remove_entity(entity_t *entity);
extern void port_animate_entity(entity_t *obj);
extern void port_swap_enitity(int obj_id, entity_t *dude);
extern void port_update_entity(entity_t *entity, const uint16_t xdiff, const uint16_t ydiff);
extern void port_load_sprite(animate_sprite_t *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load);
extern void port_free_sprite(sprite_info_t *spr);
extern int port_valid_sprite(sprite_info_t *spr);

extern void port_play_sound_effect(const uint16_t id);

extern void port_change_region(region_t *reg);
extern void port_combat_action(entity_action_t *ca);
extern void port_load_item(item_t *item);
extern void port_free_item(item_t *item);
extern void port_player_load(const int slot);
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

extern void port_toggle_window(const window_t window);
extern void port_load_window(const window_t window);
extern void port_set_lua_globals(lua_State *l);
extern void port_game_loop();
extern uint32_t port_sprite_geth(const uint16_t id);
extern void port_place_entity(entity_t *entity);
extern void port_entity_update_scmd(entity_t *entity);

extern void port_handle_input();
extern void port_tick();
extern void port_window_render();
extern void port_start_display_frame();
extern void port_commit_display_frame();

#endif
