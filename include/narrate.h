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

extern sol_status_t sol_narrate_clear();
extern sol_status_t sol_narrate_is_open();
extern sol_status_t sol_narrate_open(int16_t action, const char *text, int16_t index);
extern sol_status_t sol_narrate_select_menu(uint32_t option, int *noption);
extern sol_status_t sol_narrate_ask_yes_no();

extern sol_wops_t narrate_window;

#endif
