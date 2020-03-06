#ifndef ANIMATE_H
#define ANIMATE_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "map.h"
#include "../src/dsl-scmd.h"

typedef struct animate_s {
    scmd_t *scmd;
    SDL_Texture **textures;
    SDL_Rect loc;
    uint32_t len;
    uint16_t flags;
    struct animate_s *next;
} animate_t;

void animate_init();
void animate_clear();
void animate_add(map_t *map, SDL_Renderer *renderer, scmd_t *cmd, int id);
void animate_render(void *data, SDL_Renderer *renderer);
void animate_close();

#endif
