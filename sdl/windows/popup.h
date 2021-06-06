#ifndef POPUP_H
#define POPUP_H

#include <SDL2/SDL.h>
#include "../window-manager.h"

enum {
    POPUP_0,
    POPUP_1,
    POPUP_2,
    POPUP_CANCEL,
    POPUP_NOTHING
};

void popup_set_message(const char *msg);
void popup_set_option(const int idx, const char *msg);
uint8_t popup_get_selection();
void popup_clear_selection();

extern wops_t popup_window;

#endif
