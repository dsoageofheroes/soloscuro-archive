#ifndef ADD_LOAD_SAVE_H
#define ADD_LOAD_SAVE_H

#include "window-manager.h"

typedef enum sol_als_action_e {
    ACTION_NONE,
    ACTION_ADD,
    ACTION_SAVE,
    ACTION_LOAD,
    ACTION_DROP
} sol_als_action_t;

extern int      sol_add_load_save_get_action();
extern uint32_t sol_add_load_save_get_selection();
extern void     sol_add_load_save_set_mode(const sol_als_action_t _mode);

extern sol_wops_t als_window;

#endif
