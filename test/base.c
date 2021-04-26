#include <stdint.h>
#include "region.h"
#include "gff-image.h"

void game_loop_signal(int signal, int _accum) {
}

int game_loop_wait_for_signal(int signal) {
    return 0;
}

int8_t port_narrate_open(int16_t action, const char *text, int16_t index) {
    return 0;
}

void port_narrate_clear() {
}

void port_narrate_close() {
}

void port_add_entity(entity_t *entity, gff_palette_t *pal) {
}

extern void port_remove_entity(entity_t *entity) {
}

void port_animate_entity(entity_t *obj) {
}

void port_swap_enitity(int obj_id, entity_t *dude) {
}

void port_update_entity(entity_t *entity, const uint16_t xdiff, const uint16_t ydiff) {
    if (!entity) { return; }
    entity->mapx += xdiff;
    entity->mapy += ydiff;
}

int port_ask_yes_no() {
    return 1;
}

void port_enter_combat() {
}

void port_exit_combat() {
}

void port_change_region(region_t *reg) {
}

extern void port_load_item(item_t *item) {
}

extern void port_free_item(item_t *item) {
}

extern void port_load_sprite(sprite_info_t *spr, gff_palette_t *pal, const int gff_index, const int type, const uint32_t id) {
}

extern void port_free_sprite(sprite_info_t *spr) {
}

extern void port_combat_action(entity_action_t *ca) {
}

extern void port_play_sound_effect(const uint16_t id) {
}

extern void port_start() {
}
