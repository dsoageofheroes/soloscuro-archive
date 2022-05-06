#include <stdint.h>
#include "region.h"
#include "gpl.h"
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

void port_swap_enitity(int obj_id, entity_t *dude) {
}

int port_ask_yes_no() {
    return 1;
}

void port_change_region(sol_region_t *reg) {
}

extern void port_load_item(item_t *item) {
}

//extern void port_load_sprite(sprite_info_t *spr, gff_palette_t *pal, const int gff_index, const int type, const uint32_t id) {
//}

extern void port_combat_action(entity_action_t *ca) {
}

extern void port_play_sound_effect(const uint16_t id) {
}

extern void port_start() {
}

extern void port_set_config(game_config_t gc, ssize_t val) {
}

extern void port_player_load(const int slot) {
}

extern int port_load_region(const int region) {
    return 1;
}

extern void port_set_lua_globals(lua_State *l) {
}

extern void port_init() {
}

extern void port_close() {
}

extern uint32_t port_sprite_geth(const uint16_t id) {
    return id;
}

void             sol_mouse_init(){}
void             sol_mouse_set_as_item(item_t *item){}
power_t*         sol_mouse_get_power(){return NULL;}
void             sol_mouse_free_item(){}
item_t*          sol_mouse_get_item(){return NULL;}
void             sol_mouse_free(){}
enum mouse_state sol_mouse_get_state(){ return 0;}
void             sol_mouse_set_state(const enum mouse_state state){}

static int sprite_id = 0;
void sol_sprite_init() {}
sol_sprite_t sol_sprite_new(gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {return ++sprite_id;}
uint16_t sol_sprite_append(uint16_t sprite_id, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {return ++sprite_id;}
sol_sprite_t sol_sprite_create_from_data(unsigned char *data, const uint32_t w, const uint32_t h) {return ++sprite_id;}
void sol_sprite_set_location(const sol_sprite_t id, const uint32_t x, const uint32_t y) {}
void sol_sprite_render(const sol_sprite_t id) {}
uint32_t sol_sprite_getx(const sol_sprite_t id) {return 0;}
uint32_t sol_sprite_gety(const sol_sprite_t id) {return 0;}
uint32_t sol_sprite_getw(const sol_sprite_t id) {return 0;}
uint32_t sol_sprite_geth(const sol_sprite_t id) {return 0;}
void sol_sprite_set_alpha(const sol_sprite_t id, const uint8_t alpha) {}
void sol_sprite_free(const sol_sprite_t id) {}
void sol_sprite_set_frame(const uint16_t id, const uint16_t frame) {}
void sol_sprite_set_frame_keep_loc(const uint16_t id, const uint16_t frame) {}
uint16_t sol_sprite_get_frame(const uint16_t id) {return 0;}
int sol_sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y) {return 0;}
void sol_print_line_len(const sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len) {}
void sol_font_render_center(sol_font_t font, const char *str, const uint16_t x, const uint16_t y, const uint16_t w) {}
void sol_sprite_center_spr(const int dest, const int src) {}
void sol_sprite_render_box(const uint16_t sprite_id, const uint16_t x,
    const uint16_t y, const uint16_t w, const uint16_t h) {}
void sol_sprite_center(const int id, const int x, const int y, const int w, const int h) {}
uint32_t sol_sprite_num_frames(const uint16_t id) {return 1;}
void sol_sprite_render_flip(const uint16_t sprite_id, const int horizontal_flip, const int vertical_flip) {}
void sol_sprite_print(const sol_sprite_t sprite_id) {}

void sol_mouse_set_as_power(power_t *pw) {}
void sol_draw_cone(int sx, int sy, int range) {}
uint32_t sol_get_camerax() {return 100;}
uint32_t sol_get_cameray() {return 100;}

void sol_center_on_player() {}

void sol_background_load_region(sol_region_t *_region) {}
void sol_background_apply_alpha(const uint8_t alpha) {}
void sol_background_render() {}
void sol_background_free() {}

void  sol_audio_init() {}
void  sol_audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id) {}
void  sol_audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id, const float volume) {}
float sol_audio_get_xmi_volume() {return 50;}
void  sol_audio_set_xmi_volume(const float vol) {}
float sol_audio_get_voc_volume() {return 50;}
void  sol_audio_set_voc_volume(const float vol) {}
void  sol_audio_cleanup() {}

uint32_t sol_font_pixel_width(sol_font_t font, const char *text, const uint32_t len) { return 8; }
uint32_t sol_font_pixel_height(sol_font_t font) { return 8; }

void port_handle_input() {}
void port_tick() {}
void port_window_render() {}
void port_start_display_frame() {}
void port_commit_display_frame() {}

void       sol_textbox_set_current(textbox_t *tb) {}
void sol_font_free() {}
void sol_play_sound_effect(const uint16_t id) {}
void       sol_textbox_render(textbox_t *tb) {}
void port_entity_update_scmd(entity_t *entity) {}
extern void sol_camera_scrollx(const int amt) {}
extern void sol_camera_scrolly(const int amt) {}
