#include <stdint.h>
#include "region.h"
#include "gpl.h"
#include "audio.h"
#include "gpl-manager.h"
#include "sol-lua-manager.h"
#include "port.h"
#include "mouse.h"
#include "sol_textbox.h"
#include "gff-image.h"

void game_loop_signal(int signal, int _accum) {
}

int game_loop_wait_for_signal(int signal) {
    return 0;
}

int8_t port_narrate_open(int16_t action, const char *text, int16_t index) {
    return 0;
}

void port_swap_enitity(int obj_id, sol_entity_t *dude) {
}

int port_ask_yes_no() {
    return 1;
}

void port_change_region(sol_region_t *reg) {
}

//extern void port_load_sprite(sprite_info_t *spr, gff_palette_t *pal, const int gff_index, const int type, const uint32_t id) {
//}

extern sol_status_t port_play_sound_effect(const uint16_t id) {
}

extern void port_start() {
}

extern void port_set_config(sol_game_config_t gc, ssize_t val) {
}

extern void port_player_load(const int slot) {
}

extern int port_load_region(const int region) {
    return 1;
}

extern sol_status_t sol_set_lua_globals(lua_State *l) {
}

extern void port_init() {
}

extern void port_close() {
}

extern uint32_t port_sprite_geth(const uint16_t id) {
    return id;
}

sol_status_t sol_mouse_init(){}
sol_status_t sol_mouse_set_as_item(sol_item_t *item){}
sol_status_t sol_mouse_free_item(){}
sol_status_t sol_mouse_free(){}
sol_status_t sol_mouse_set_state(const enum sol_mouse_state_e state){}
sol_status_t sol_mouse_get_power(sol_power_t **p){*p = NULL; return SOL_SUCCESS;}
sol_status_t sol_mouse_get_item(sol_item_t **i){*i = NULL; return SOL_SUCCESS;}
sol_status_t sol_mouse_get_state(enum sol_mouse_state_e *ms){ *ms = 0; return SOL_SUCCESS;}

static int sprite_id = 0;
extern sol_sprite_t sol_sprite_init() { return SOL_SUCCESS; }
extern sol_status_t sol_sprite_new(gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id,
        sol_sprite_t *spr) {
    *spr =  ++sprite_id;
    return SOL_SUCCESS;
}
extern sol_status_t sol_sprite_append(sol_sprite_t sprite_id, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {return SOL_SUCCESS;}
extern sol_status_t sol_sprite_create_from_data(unsigned char *data, const uint32_t w, const uint32_t h, sol_sprite_t *spr) {
    *spr = ++sprite_id;
    return SOL_SUCCESS;
}
extern sol_status_t sol_sprite_set_location(const sol_sprite_t id, const uint32_t x, const uint32_t y) {return SOL_SUCCESS;}
extern sol_status_t sol_sprite_render(const sol_sprite_t id) {return SOL_SUCCESS;}
extern sol_status_t sol_sprite_get_info(const sol_sprite_t id, sol_sprite_info_t *loc) {
    loc->x = loc->y = loc->w = loc->h = 100;
    loc->num_frames = 1;
    return SOL_SUCCESS;
}
sol_status_t sol_sprite_set_alpha(const sol_sprite_t id, const uint8_t alpha) {return SOL_SUCCESS;}
sol_status_t sol_sprite_free(const sol_sprite_t id) {return SOL_SUCCESS;}
sol_status_t sol_sprite_set_frame(const sol_sprite_t id, const uint16_t frame) {return SOL_SUCCESS;}
sol_status_t sol_sprite_increment_frame(const sol_sprite_t id, const int16_t amt) {return SOL_SUCCESS;}
sol_status_t sol_sprite_set_frame_keep_loc(const sol_sprite_t id, const uint16_t frame) {return SOL_SUCCESS;}
sol_status_t sol_sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y) {return SOL_SPRITE_NOT_IN_RECT;}
sol_status_t sol_sprite_center_spr(const int dest, const int src) {return SOL_SUCCESS;}
sol_status_t sol_sprite_center(const int id, const int x, const int y, const int w, const int h) {return SOL_SUCCESS;}
sol_status_t sol_sprite_render_box(const uint16_t sprite_id, const uint16_t x,
    const uint16_t y, const uint16_t w, const uint16_t h) {return SOL_SUCCESS;}
sol_status_t sol_sprite_render_flip(const uint16_t sprite_id, const int horizontal_flip, const int vertical_flip) {return SOL_SUCCESS;}
sol_status_t sol_sprite_set_color_mod(const uint16_t sprite_id, const uint8_t r, const uint8_t g, const uint8_t b) {return SOL_SUCCESS;}
sol_status_t sol_sprite_print(const sol_sprite_t sprite_id) {return SOL_SUCCESS;}

sol_status_t sol_print_line_len(const sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len) {return SOL_SUCCESS;}
sol_status_t sol_font_render_center(sol_font_t font, const char *str, const uint16_t x, const uint16_t y, const uint16_t w) {return SOL_SUCCESS;}

extern sol_status_t sol_mouse_set_as_power(sol_power_t *pw) {}
extern sol_status_t sol_draw_cone(int sx, int sy, int range) {}
uint32_t sol_get_camerax() {return 100;}
uint32_t sol_get_cameray() {return 100;}

void sol_center_on_player() {}

sol_status_t sol_background_load_region(sol_region_t *_region) {return SOL_SUCCESS;}
sol_status_t sol_background_apply_alpha(const uint8_t alpha) {return SOL_SUCCESS;}
sol_status_t sol_background_render() {return SOL_SUCCESS;}
sol_status_t sol_background_free() {return SOL_SUCCESS;}

sol_status_t sol_audio_init() {return SOL_SUCCESS;}
sol_status_t sol_audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id) {return SOL_SUCCESS;}
sol_status_t sol_audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id, const float volume) {return SOL_SUCCESS;}
sol_status_t sol_audio_get(sol_audio_stats_t **stats) { return SOL_SUCCESS; }
sol_status_t sol_audio_set_xmi_volume(const float vol) {return SOL_SUCCESS;}
sol_status_t sol_audio_set_voc_volume(const float vol) {return SOL_SUCCESS;}
sol_status_t sol_audio_cleanup() {return SOL_SUCCESS;}

uint32_t sol_font_pixel_width(sol_font_t font, const char *text, const uint32_t len) { return 8; }
uint32_t sol_font_pixel_height(sol_font_t font) { return 8; }

void port_handle_input() {}
void port_tick() {}
void port_window_render() {}
void port_start_display_frame() {}
void port_commit_display_frame() {}

extern sol_status_t sol_textbox_set_current(sol_textbox_t *tb) {return SOL_SUCCESS;}
void sol_font_free() {}
sol_status_t sol_play_sound_effect(const uint16_t id) {return SOL_SUCCESS; }
sol_status_t sol_textbox_render(sol_textbox_t *tb) {return SOL_SUCCESS; }
extern sol_status_t sol_camera_scrollx(const int amt) {return SOL_SUCCESS;}
extern sol_status_t sol_camera_scrolly(const int amt) {return SOL_SUCCESS;}
extern uint16_t     sol_sprite_get_frame_count(const uint16_t id){ return 0;}
