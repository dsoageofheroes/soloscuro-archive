#ifndef SCRREN_NARRATE_H
#define SCRREN_NARRATE_H

#include "window-manager.h"

enum {
    NAR_ADD_MENU,
    NAR_PORTRAIT,
    NAR_SHOW_TEXT,
    NAR_SHOW_MENU,
    NAR_EDIT_BOX
};

void narrate_init(const uint32_t x, const uint32_t y);
void narrate_render(void *data);
int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y);
int narrate_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void narrate_clear();
void narrate_free();
int narrate_is_open();
extern int8_t narrate_open(int16_t action, const char *text, int16_t index);
extern int narrate_select_menu(uint32_t option);
extern int    sol_ui_narrate_ask_yes_no();
extern int8_t sol_ui_narrate_open(int16_t action, const char *text, int16_t index);
extern void   sol_ui_narrate_clear();
extern void   sol_ui_narrate_close();

extern sol_wops_t narrate_window;

#endif
