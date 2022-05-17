#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

typedef struct sol_test_info_s {
    char failed;
    char *msg;
} sol_test_info_t;

extern float           settings_zoom();
extern float           settings_set_zoom(const float zoom);
extern uint16_t        settings_screen_width();
extern uint16_t        settings_screen_height();
extern void            setting_screen_set(const uint16_t width, const uint16_t height);
extern int             settings_in_debug();
extern uint16_t        settings_ticks_per_move();
extern void            sol_debug(const char *file, const int line_num, const char *pretty, const char *str, ...);
extern void            sol_set_debug(const int val);
extern void            sol_set_lua_test(char passed, char *msg);
extern sol_test_info_t sol_get_lua_test();
extern void            sol_settings_set_zoom(float val);

#endif
