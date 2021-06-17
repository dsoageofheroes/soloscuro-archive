#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <SDL2/SDL.h>

// wops = Window OPerationS
typedef struct wops_s{
    void (*init) (SDL_Renderer*, const uint32_t x, const uint32_t y);
    void (*cleanup) ();
    void (*render) (void*, SDL_Renderer*);
    int (*mouse_movement) (const uint32_t x, const uint32_t y);
    int (*mouse_up) (const uint32_t button, const uint32_t x, const uint32_t y);
    int (*mouse_down) (const uint32_t button, const uint32_t x, const uint32_t y);
    void (*return_control) ();
    uint32_t (*get_width) ();
    uint32_t (*get_height) ();
    const char *name;
    uint8_t grey_out_map;
    void *data;
} wops_t;

void window_init(SDL_Renderer *renderer);
void window_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos);
void window_load(SDL_Renderer *renderer, int layer, wops_t *window, const uint32_t x, const uint32_t y);
void window_push(SDL_Renderer *renderer, wops_t *window, const uint32_t x, const uint32_t y);
void window_toggle(SDL_Renderer *renderer, wops_t *window, const uint32_t x, const uint32_t y);
void window_pop();
void window_handle_mouse(const uint32_t x, const uint32_t y);
void window_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y);
void window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y);
int window_load_region(SDL_Renderer *renderer, const int region);
void window_clear();
void window_free();

// Utility function
SDL_Texture* create_texture(SDL_Renderer *renderer, const uint32_t gff_file,
        const uint32_t type, const uint32_t id, const uint32_t frame_id,
        const uint32_t palette_id, SDL_Rect *loc);

#endif
