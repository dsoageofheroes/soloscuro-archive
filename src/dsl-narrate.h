#ifndef DSL_NARRATE
#define DSL_NARRATE

#include <stdint.h>

enum {
    NAR_ADD_MENU,
    NAR_PORTRAIT,
    NAR_SHOW_TEXT,
    NAR_SHOW_MENU,
    NAR_EDIT_BOX
};

int8_t narrate_open(int16_t action, char *text, int16_t index);

#endif
