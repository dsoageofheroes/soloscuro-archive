/*
 * These are the functions needed to be implmeneted for porting to different systems.
 * This include both OS specific call and callbacks.
 */

// TODO FIXME PLEASE SUPER PLEASE
// O man does this need to be cleaned up, so much dead code and stale systems.
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

#define sol_sprite_t uint32_t

typedef enum sol_font_e {
    FONT_YELLOW,
    FONT_GREY,
    FONT_GREYLIGHT,
    FONT_BLACK,
    FONT_BLACKDARK,
    FONT_RED,
    FONT_REDDARK,
    FONT_BLUE,
    NUM_FONTS
} sol_font_t;

typedef struct sol_dim_s {
    uint16_t x, y, w, h;
} sol_dim_t;

// Narrate functions

extern void port_remove_entity(entity_t *entity); // We need to remove this...
extern void port_swap_enitity(int obj_id, entity_t *dude);
extern void port_load_sprite(animate_sprite_t *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load);
extern void port_free_sprite(sprite_info_t *spr);
extern int port_valid_sprite(sprite_info_t *spr);

extern void sol_play_sound_effect(const uint16_t id);

extern void port_change_region(sol_region_t *reg);
extern void port_load_item(item_t *item);
extern void port_free_item(item_t *item);
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

#define SPRITE_ERROR (0xFFFF)

extern void sol_sprite_init();
extern sol_sprite_t sol_sprite_new(gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
extern uint16_t sol_sprite_append(uint16_t sprite_id, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
extern sol_sprite_t sol_sprite_create_from_data(unsigned char *data, const uint32_t w, const uint32_t h);
extern void sol_sprite_set_location(const sol_sprite_t id, const uint32_t x, const uint32_t y);
extern void sol_sprite_render(const sol_sprite_t id);
extern uint32_t sol_sprite_getx(const sol_sprite_t id);
extern uint32_t sol_sprite_gety(const sol_sprite_t id);
extern uint32_t sol_sprite_getw(const sol_sprite_t id);
extern uint32_t sol_sprite_geth(const sol_sprite_t id);
extern void sol_sprite_set_alpha(const sol_sprite_t id, const uint8_t alpha);
extern void sol_sprite_free(const sol_sprite_t id);
extern void sol_sprite_set_frame(const uint16_t id, const uint16_t frame);
extern uint16_t sol_sprite_get_frame(const uint16_t id);
extern int sol_sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y);
extern void sol_print_line_len(const sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len);
extern void sol_font_render_center(sol_font_t font, const char *str, const uint16_t x, const uint16_t y, const uint16_t w);
extern void sol_sprite_center_spr(const int dest, const int src);
extern void sol_sprite_render_box(const uint16_t sprite_id, const uint16_t x,
    const uint16_t y, const uint16_t w, const uint16_t h);
extern void sol_sprite_center(const int id, const int x, const int y, const int w, const int h);
extern uint32_t sol_sprite_num_frames(const uint16_t id);
extern void sol_sprite_render_flip(const uint16_t sprite_id, const int horizontal_flip, const int vertical_flip);
extern void sol_sprite_print(const sol_sprite_t sprite_id);

extern void sol_mouse_set_as_power(power_t *pw);
extern void sol_draw_cone(int sx, int sy, int range);
extern uint32_t sol_get_camerax();
extern uint32_t sol_get_cameray();

extern void sol_center_on_player();

#endif
