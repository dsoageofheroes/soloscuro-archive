#ifndef SOL_MOUSE_H
#define SOL_MOUSE_H

#include "ssi-item.h"
#include "port.h"
#include "powers.h"

enum mouse_state {
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

extern void             sol_mouse_init();
extern void             sol_mouse_set_as_item(item_t *item);
extern power_t*         sol_mouse_get_power();
extern void             sol_mouse_free_item();
extern item_t*          sol_mouse_get_item();
extern void             sol_mouse_free();
extern enum mouse_state sol_mouse_get_state();
extern void             sol_mouse_set_state(const enum mouse_state state);
extern uint32_t         sol_mouse_getx();
extern uint32_t         sol_mouse_gety();

#endif
