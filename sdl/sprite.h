#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>

#include "../src/gff-image.h"

typedef struct sprite_s {
    SDL_Texture **tex;
    SDL_Rect *loc;
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
uint16_t sprite_new(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
uint16_t sprite_append(uint16_t sprite_id, SDL_Renderer *renderer,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
void sprite_render(SDL_Renderer *renderer, const uint16_t sprite_id);
void sprite_render_flip(SDL_Renderer *renderer, const uint16_t sprite_id, SDL_RendererFlip flip);
uint16_t sprite_get_frame(const uint16_t id);
void sprite_set_frame(const uint16_t id, const uint16_t frame);
void sprite_set_alpha(const uint16_t id, const uint8_t alpha);
void sprite_render_box(SDL_Renderer *renderer, const uint16_t sprite_id,
        const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h);
int sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y);
void sprite_set_location(const uint16_t, const uint32_t, const uint32_t y);
void sprite_center(const int id, const int x, const int y, const int w, const int h);
void sprite_center_spr(const int dest, const int src);
uint32_t sprite_getx(const uint16_t id);
uint32_t sprite_gety(const uint16_t id);
uint32_t sprite_geth(const uint16_t id);
uint32_t sprite_get_max_height(const uint16_t id);
uint32_t sprite_getw(const uint16_t id);
uint32_t sprite_num_frames(const uint16_t id);
uint32_t sprite_get_xdiff_from_start(const uint16_t id);
// Free a sprite at an ID (do not use it again!)
void sprite_free(const uint16_t id);

#endif
