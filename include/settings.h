#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "status.h"

typedef struct sol_test_info_s {
    char failed;
    char *msg;
} sol_test_info_t;

extern sol_status_t sol_debug(const char *file, const int line_num, const char *pretty, const char *str, ...);
extern sol_status_t sol_set_debug(const int val);
extern sol_status_t sol_set_lua_test(char passed, char *msg);
extern sol_status_t sol_settings_set_zoom(float val);

// Keeping old interface for now.
// TODO: Revisit later if needed.
extern float    settings_zoom();
extern uint16_t settings_screen_width();
extern uint16_t settings_screen_height();
extern uint16_t settings_ticks_per_move();

#endif
