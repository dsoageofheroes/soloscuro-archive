#include <stdint.h>
#include "region.h"

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

void port_swap_objs(int obj_id, region_object_t *obj) {
}

void port_add_obj(region_object_t *obj) {
}

void port_animate_obj(region_object_t *obj) {
}

void port_update_obj(region_object_t *obj, const uint16_t xdiff, const uint16_t ydiff) {
    if (!obj) { return; }
    obj->mapx += xdiff;
    obj->mapy += ydiff;
}

void port_add_entity(entity_t *entity, gff_palette_t *pal) {
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

void port_change_region(dsl_region_t *reg) {
}
