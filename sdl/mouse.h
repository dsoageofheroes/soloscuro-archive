#ifndef SDL_MOUSE_H
#define SDL_MOUSE_H

#include <SDL2/SDL.h>
#include "../src/ds-item.h"
#include "sprite.h"

void mouse_init(SDL_Renderer *rend);
void mouse_set_as_item(item_t *item);
void mouse_free_item();
item_t* mouse_get_item();
void mouse_free();

#endif
