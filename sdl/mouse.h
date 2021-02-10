#ifndef SDL_MOUSE_H
#define SDL_MOUSE_H

#include <SDL2/SDL.h>
#include "../src/ds-item.h"
#include "sprite.h"

void mouse_init(SDL_Renderer *rend);
void mouse_set_as_item(ds1_item_t *item, uint16_t spr);
ds1_item_t* mouse_retreive_item(uint16_t *spr);
ds1_item_t* mouse_get_item();
void mouse_free();

#endif
