#ifndef ADD_LOAD_SAVE_H
#define ADD_LOAD_SAVE_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"

enum {
    ACTION_NONE,
    ACTION_ADD,
    ACTION_SAVE,
    ACTION_LOAD,
};

int add_load_save_get_action();
uint32_t add_load_save_get_selection();

extern sops_t als_screen;

#endif
