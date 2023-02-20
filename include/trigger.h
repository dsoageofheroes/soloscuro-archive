#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "entity.h"
#include "region.h"

extern sol_status_t sol_trigger_init();
extern sol_status_t sol_trigger_cleanup();
extern sol_status_t sol_trigger_set_region(sol_region_t *reg);
extern sol_status_t sol_trigger_noorders_enable();
extern sol_status_t sol_trigger_noorders_disable();
extern sol_status_t sol_trigger_use(uint32_t obj);
extern sol_status_t sol_trigger_object_clear(const uint32_t obj);
extern sol_status_t sol_trigger_noorders(uint32_t x, uint32_t y);
extern sol_status_t sol_trigger_box_check(uint32_t x, uint32_t y);
extern sol_status_t sol_trigger_noorders_event();
extern sol_status_t sol_trigger_noorders_check();
extern sol_status_t sol_trigger_noorders_entity_check(sol_entity_t *entity);
extern sol_status_t sol_trigger_los_check(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param);
extern sol_status_t sol_trigger_deserialize(char *data);
extern sol_status_t sol_trigger_talk_click(uint32_t obj);
extern sol_status_t sol_write_triggers(FILE *file);
extern sol_status_t sol_trigger_tile_check(uint32_t x, uint32_t y);
extern sol_status_t sol_trigger_add_attack(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_attack_global(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_use(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_use_global(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_look(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_look_global(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_noorders(uint32_t obj, uint32_t file, uint32_t addr, int trigger, int run);
extern sol_status_t sol_trigger_add_talkto(uint32_t obj, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_usewith(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr);
extern sol_status_t sol_trigger_add_tile(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip);
extern sol_status_t sol_trigger_add_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                                uint32_t file, uint32_t addr, uint32_t trip);
extern sol_status_t sol_trigger_add_los(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param);
extern sol_status_t sol_trigger_has_talkto(uint32_t obj);
extern sol_status_t sol_trigger_has_look(uint32_t obj);
extern sol_status_t sol_trigger_has_use(uint32_t obj);

// Deprecated
extern char*            sol_trigger_serialize(size_t *len);


#endif
