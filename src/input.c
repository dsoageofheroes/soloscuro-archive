#include "input.h"
#include "combat.h"
#include <stdio.h>

extern void sol_key_down(const sol_key_e key) {
    switch(key) {
        case SOLK_g: sol_combat_guard(NULL); break;
        default: break;
    }
}
