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

extern float settings_set_zoom(const float _zoom) {
    if (zoom <= 0) { return zoom; }
    return (zoom = _zoom);
}

extern void setting_screen_set(const uint16_t _width, const uint16_t _height) {
    if (width < 1 || height < 1) { return; }
    width = _width;
    height = _height;
    error("MUST resize window!!!!\n");
}

extern int settings_in_debug() { return 0; }

extern void sol_set_debug(const int val) {
    debug = val;
}

extern void sol_debug(const char *file, const int line_num, const char *pretty, const char *str, ...) {
    if (debug) { return; }
    va_list argp;
    va_start(argp, str);
    printf("[%s:%d] %s: ", file, line_num, pretty);
    vprintf(str, argp);
}
