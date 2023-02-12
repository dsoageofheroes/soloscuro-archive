#ifndef SOL_MOUSE_H
#define SOL_MOUSE_H

#include "ssi-item.h"
#include "port.h"
#include "powers.h"

enum sol_mouse_state_e {
    MOUSE_POINTER,
    MOUSE_NO_POINTER,
    MOUSE_MELEE,
    MOUSE_NO_MELEE,
    MOUSE_RANGE,
    MOUSE_NO_RANGE,
    MOUSE_TALK,
    MOUSE_NO_TALK,
    MOUSE_POWER,
    MOUSE_NO_POWER,
    MOUSE_WAIT,
    MOUSE_ITEM,
};

typedef enum sol_mouse_button_e {
    SOL_MOUSE_BUTTON_LEFT,
    SOL_MOUSE_BUTTON_RIGHT,
    SOL_MOUSE_BUTTON_MIDDLE,
} sol_mouse_button_t;

extern sol_status_t sol_mouse_init();
extern sol_status_t sol_mouse_set_as_item(sol_item_t *item);
extern sol_status_t sol_mouse_free_item();
extern sol_status_t sol_mouse_free();
extern sol_status_t sol_mouse_set_state(const enum sol_mouse_state_e state);
extern sol_status_t sol_mouse_get_item(sol_item_t **item);
extern sol_status_t sol_mouse_get_power(power_t **power);
extern sol_status_t sol_mouse_get_state(enum sol_mouse_state_e *ms);

#endif
