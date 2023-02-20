#include "settings.h"
#include "gpl.h"
#include <stdarg.h>

static float    zoom = 2.0;
static uint16_t width = 800;
static uint16_t height = 600;
static int      debug = 0;

extern float settings_zoom() { return zoom; }
extern uint16_t settings_screen_width() { return width; }
extern uint16_t settings_screen_height() { return height; }
extern sol_status_t sol_settings_set_zoom(float val) { zoom = val; return SOL_SUCCESS; }

extern sol_status_t sol_set_debug(const int val) {
    debug = val;
    return SOL_SUCCESS;
}

extern sol_status_t sol_debug(const char *file, const int line_num, const char *pretty, const char *str, ...) {
    if (!debug) { return SOL_SUCCESS; }
    va_list argp;
    va_start(argp, str);
    printf("[%s:%d] %s: ", file, line_num, pretty);
    vprintf(str, argp);
    return SOL_SUCCESS;
}

extern uint16_t settings_ticks_per_move() { return 30; }

static sol_test_info_t sol_test_info = {0, NULL};

extern sol_status_t sol_set_lua_test(char failed, char *msg) {
    sol_test_info.failed = failed;
    sol_test_info.msg = msg;
    return SOL_SUCCESS;
}
