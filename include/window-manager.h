#ifndef SOL_WINDOW_MANAGER_H
#define SOL_WINDOW_MANAGER_H

#include "mouse.h"
#include "../src/entity-animation.h"

// wops = Window OPerationS
typedef struct sol_wops_s {
    void (*init) (const uint32_t x, const uint32_t y);
    void (*cleanup) ();
    void (*render) (void*);
    int (*mouse_movement) (const uint32_t x, const uint32_t y);
    int (*mouse_up) (const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
    int (*mouse_down) (const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
    int (*key_down) (const enum entity_action_e);
    void (*return_control) ();
    uint32_t (*get_width) ();
    uint32_t (*get_height) ();
    const char *name;
    uint8_t grey_out_map;
    void *data;
} sol_wops_t;

void sol_window_init();
void sol_window_render(const uint32_t xmappos, const uint32_t ymappos);
void sol_window_load(int layer, sol_wops_t *window, const uint32_t x, const uint32_t y);
void sol_window_push(sol_wops_t *window, const uint32_t x, const uint32_t y);
void sol_window_toggle(sol_wops_t *window, const uint32_t x, const uint32_t y);
void sol_window_pop();
void sol_window_handle_mouse(const uint32_t x, const uint32_t y);
void sol_window_handle_mouse_down(const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
void sol_window_handle_mouse_up(const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
int  sol_window_handle_key_down(const enum entity_action_e);
int  sol_window_load_region(const int region);
void sol_window_clear();
void sol_window_free();

#endif
