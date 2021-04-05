#ifndef SDL_MOUSE_H
#define SDL_MOUSE_H

#include <SDL2/SDL.h>
#include "../src/ds-item.h"
#include "sprite.h"
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

extern void mouse_init(SDL_Renderer *rend);
extern void mouse_set_as_item(item_t *item);
extern void mouse_set_as_power(power_t *pw);
extern power_t* mouse_get_power();
extern void mouse_free_item();
extern item_t* mouse_get_item();
extern void mouse_free();
extern enum mouse_state mouse_get_state();
extern void mouse_set_state(const enum mouse_state state);

#endif
