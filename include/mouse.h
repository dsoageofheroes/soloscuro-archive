#ifndef SDL_MOUSE_H
#define SDL_MOUSE_H

#include "../src/ssi-item.h"
#include "../sdl/sprite.h"
#include "../src/powers.h"

enum mouse_state {
    MOUSE_POINTER,
    MOUSE_NO_POINTER,
    MOUSE_MELEE,
    MOUSE_NO_MELEE,
    MOUSE_RANGE,
    MOUSE_NO_RANGE,
    MOUSE_TALK,
    MOUSE_NO_TALK,
    MOUSE_POWER,
    MOUSE_NO_POWER,
    MOUSE_WAIT,
    MOUSE_ITEM,
};

extern void             sol_mouse_init();
extern void             sol_mouse_set_as_item(item_t *item);
extern power_t*         sol_mouse_get_power();
extern void             sol_mouse_free_item();
extern item_t*          sol_mouse_get_item();
extern void             sol_mouse_free();
extern enum mouse_state sol_mouse_get_state();
extern void             sol_mouse_set_state(const enum mouse_state state);

#endif
