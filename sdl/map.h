#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>

typedef struct map_s {
    SDL_Texture **tiles;
    uint32_t num_tiles;
    uint32_t gff_file;
    uint32_t map_id;
    SDL_Texture **objs;
    SDL_Rect *obj_locs;
    uint16_t *flags;
    uint32_t num_objs;
} map_t;

void map_init(map_t *map);
void map_free(map_t *map);
void map_load_region(map_t *map, SDL_Renderer *renderer, int id);
void map_render(void *data, SDL_Renderer *renderer);
int map_handle_mouse(const uint32_t x, const uint32_t y);
int map_handle_mouse_click(const uint32_t x, const uint32_t y);

// From main.c
uint32_t getCameraX();
uint32_t getCameraY();

#endif
