#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <SDL2/SDL.h>

// sops = Screen OPerationS
typedef struct sops_s{
    void (*init) (SDL_Renderer*, const uint32_t x, const uint32_t y);
    void (*cleanup) ();
    void (*render) (void*, SDL_Renderer*);
    int (*mouse_movement) (const uint32_t x, const uint32_t y);
    int (*mouse_up) (const uint32_t button, const uint32_t x, const uint32_t y);
    int (*mouse_down) (const uint32_t button, const uint32_t x, const uint32_t y);
    void (*return_control) ();
    uint32_t (*get_width) ();
    uint32_t (*get_height) ();
    void *data;
} sops_t;

void screen_init(SDL_Renderer *renderer);
void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos);
void screen_load_screen(SDL_Renderer *renderer, int layer, sops_t *screen, const uint32_t x, const uint32_t y);
void screen_push_screen(SDL_Renderer *renderer, sops_t *screen, const uint32_t x, const uint32_t y);
void screen_toggle_screen(SDL_Renderer *renderer, sops_t *screen, const uint32_t x, const uint32_t y);
void screen_pop();
void screen_handle_mouse(const uint32_t x, const uint32_t y);
void screen_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y);
void screen_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y);
int screen_load_region(SDL_Renderer *renderer, const int region);
void screen_clear();
void screen_free();

// Utility function
SDL_Texture* create_texture(SDL_Renderer *renderer, const uint32_t gff_file,
        const uint32_t type, const uint32_t id, const uint32_t frame_id,
        const uint32_t palette_id, SDL_Rect *loc);

#endif
