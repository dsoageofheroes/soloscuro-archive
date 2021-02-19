#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>

void trigger_init();
void trigger_cleanup();

typedef struct attack_trigger_s {
    uint32_t obj, file, addr, global;
} attack_trigger_t;

typedef struct noorders_trigger_s {
    uint32_t obj, file, addr;
} noorders_trigger_t;

typedef struct use_trigger_s {
    uint32_t obj, file, addr, global;
} use_trigger_t;

typedef struct look_trigger_s {
    uint32_t obj, file, addr, global;
} look_trigger_t;

typedef struct talkto_trigger_s {
    uint32_t obj, file, addr;
} talkto_trigger_t;

typedef struct usewith_trigger_s {
    uint32_t obj1, obj2, file, addr;
} usewith_trigger_t;

typedef struct tile_trigger_s {
    uint32_t x, y, file, addr, trip;
} tile_trigger_t;

typedef struct box_trigger_s {
    uint32_t x, y, w, h, file, addr, trip;
} box_trigger_t;

int add_attack_trigger(uint32_t obj, uint32_t file, uint32_t addr);
int add_attack_trigger_global(uint32_t obj, uint32_t file, uint32_t addr);
int add_use_trigger(uint32_t obj, uint32_t file, uint32_t addr);
int add_use_trigger_global(uint32_t obj, uint32_t file, uint32_t addr);
int add_look_trigger(uint32_t obj, uint32_t file, uint32_t addr);
int add_look_trigger_global(uint32_t obj, uint32_t file, uint32_t addr);
int add_noorders_trigger(uint32_t obj, uint32_t file, uint32_t addr);
int add_talkto_trigger(uint32_t obj, uint32_t file, uint32_t addr);
int add_usewith_trigger(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr);
int add_tile_trigger(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip);
int add_box_trigger(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t file, uint32_t addr, uint32_t trip);

talkto_trigger_t get_talkto_trigger(uint32_t obj);
look_trigger_t get_look_trigger(uint32_t obj);
void trigger_object_clear(const uint32_t obj);
void trigger_noorders();
void trigger_box_check(uint32_t x, uint32_t y);
int trigger_tile_check(uint32_t x, uint32_t y);

void talk_click(uint32_t obj);

#endif
