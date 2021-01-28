#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>

#include "../src/gff-image.h"

typedef struct sprite_s {
    SDL_Texture **tex;
    SDL_Rect loc;
    uint16_t pos, len;
    uint8_t in_use;
} sprite_t;

#define SPRITE_ERROR (0xFFFF)

// Needed to intialize the sprite lists.
void sprite_init();

// Create a sprite and return its ID
uint16_t sprite_create(SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
void sprite_render(SDL_Renderer *renderer, const uint16_t sprite_id);
uint16_t sprite_get_frame(const uint16_t id);
void sprite_set_frame(const uint16_t id, const uint16_t frame);
int sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y);
// Free a sprite at an ID (do not use it again!)
void sprite_free(const uint16_t id);

#endif
