#ifndef ANIMATE_H
#define ANIMATE_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "../src/dsl-scmd.h"

typedef struct animate_s {
    scmd_t *scmd;
    SDL_Texture **textures;
    SDL_Rect loc;
    uint16_t len;
    uint16_t pos;
    int16_t ticks_left;
    uint16_t flags;
    struct animate_s *next;
} animate_t;

#include "map.h"

struct map_s;

void animate_init();
void animate_clear();
animate_t* animate_add(struct map_s *map, SDL_Renderer *renderer, scmd_t *cmd, int id);
void animate_render(void *data, SDL_Renderer *renderer);
void animate_close();

#endif
