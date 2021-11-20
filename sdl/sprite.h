#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>

typedef struct sprite_s {
    SDL_Texture **tex;
    SDL_Rect *loc;
    uint16_t pos, len;
    uint8_t in_use;
} sprite_t;

// Needed to intialize the sprite lists.
void sprite_init();

#endif
