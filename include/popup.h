#ifndef POPUP_H
#define POPUP_H

#include "window-manager.h"

typedef enum sol_popup_e {
    POPUP_0,
    POPUP_1,
    POPUP_2,
    POPUP_CANCEL,
    POPUP_NOTHING
} sol_popup_t;

extern sol_status_t sol_popup_quick_message(const char *msg);
extern sol_status_t sol_popup_set_message(const char *msg);
extern sol_status_t sol_popup_set_option(const sol_popup_t idx, const char *msg);
extern sol_status_t sol_popup_clear_selection();
extern sol_status_t sol_popup_get_selection(uint8_t *sel);

extern sol_wops_t popup_window;

#endif
