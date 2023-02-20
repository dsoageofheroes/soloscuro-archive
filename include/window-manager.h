#ifndef SOL_WINDOW_MANAGER_H
#define SOL_WINDOW_MANAGER_H

#include "mouse.h"
#include "input.h"
#include "entity-animation.h"

// wops = Window OPerationS
typedef struct sol_wops_s {
    void (*init) (const uint32_t x, const uint32_t y);
    void (*cleanup) ();
    void (*render) (void*);
    int (*mouse_movement) (const uint32_t x, const uint32_t y);
    int (*mouse_up) (const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
    int (*mouse_down) (const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
    int (*key_down) (const enum sol_entity_action_e);
    int (*key_press) (const sol_key_e e);
    void (*return_control) ();
    uint32_t (*get_width) ();
    uint32_t (*get_height) ();
    const char *name;
    uint8_t grey_out_map;
    void *data;
} sol_wops_t;

extern sol_status_t sol_window_init();
extern sol_status_t sol_window_render(const uint32_t xmappos, const uint32_t ymappos);
extern sol_status_t sol_window_load(int layer, sol_wops_t *window, const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_push(sol_wops_t *window, const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_toggle(sol_wops_t *window, const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_pop();
extern sol_status_t sol_window_handle_mouse(const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_handle_mouse_down(const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_handle_mouse_up(const sol_mouse_button_t button, const uint32_t x, const uint32_t y);
extern sol_status_t sol_window_handle_key_down(const enum sol_entity_action_e);
extern sol_status_t sol_window_handle_key_press(const enum sol_key_e e);
extern sol_status_t sol_window_load_region(const int region);
extern sol_status_t sol_window_clear();
extern sol_status_t sol_window_free();

#endif
