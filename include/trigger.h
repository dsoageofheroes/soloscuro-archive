#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct attack_trigger_s {
    uint32_t obj, file, addr, global;
} attack_trigger_t;

typedef struct noorders_trigger_s {
    uint32_t obj, file, addr;
    uint8_t need_to_run; // flag if we need to run/execute at the next oppurtunity.
    uint8_t trigger_on_tile; // flag if we need to run/execute next tile trigger.
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

extern void             sol_trigger_init();
extern void             sol_trigger_cleanup();

extern int              sol_trigger_add_attack(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_attack_global(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_use(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_use_global(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_look(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_look_global(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_noorders(uint32_t obj, uint32_t file, uint32_t addr, int trigger, int run);
extern int              sol_trigger_add_talkto(uint32_t obj, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_usewith(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr);
extern int              sol_trigger_add_tile(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip);
extern int              sol_trigger_add_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                                uint32_t file, uint32_t addr, uint32_t trip);

extern talkto_trigger_t sol_trigger_get_talkto(uint32_t obj);
extern look_trigger_t   sol_trigger_get_look(uint32_t obj);
extern void             sol_trigger_object_clear(const uint32_t obj);
extern void             sol_trigger_enable_object(const uint32_t obj);
extern void             sol_trigger_noorders(uint32_t x, uint32_t y);
extern void             sol_trigger_noorders_enable_all();
extern void             sol_trigger_box_check(uint32_t x, uint32_t y);
extern int              sol_trigger_tile_check(uint32_t x, uint32_t y);
extern void             sol_trigger_end_combat();

extern char*            sol_trigger_serialize(size_t *len);
extern void             sol_trigger_deserialize(char *data);

extern void             sol_trigger_talk_click(uint32_t obj);

extern void             sol_write_triggers(FILE *file);

#endif
