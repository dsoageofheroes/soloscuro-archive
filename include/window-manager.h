#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "../src/entity-animation.h"

// wops = Window OPerationS
typedef struct wops_s {
    void (*init) (const uint32_t x, const uint32_t y);
    void (*cleanup) ();
    void (*render) (void*);
    int (*mouse_movement) (const uint32_t x, const uint32_t y);
    int (*mouse_up) (const uint32_t button, const uint32_t x, const uint32_t y);
    int (*mouse_down) (const uint32_t button, const uint32_t x, const uint32_t y);
    int (*key_down) (const enum entity_action_e);
    void (*return_control) ();
    uint32_t (*get_width) ();
    uint32_t (*get_height) ();
    const char *name;
    uint8_t grey_out_map;
    void *data;
} wops_t;

void window_init();
void window_render(const uint32_t xmappos, const uint32_t ymappos);
void window_load(int layer, wops_t *window, const uint32_t x, const uint32_t y);
void window_push(wops_t *window, const uint32_t x, const uint32_t y);
void window_toggle(wops_t *window, const uint32_t x, const uint32_t y);
void window_pop();
void window_handle_mouse(const uint32_t x, const uint32_t y);
void window_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y);
void window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y);
int window_handle_key_down(const enum entity_action_e);
int window_load_region(const int region);
void window_clear();
void window_free();

#endif
