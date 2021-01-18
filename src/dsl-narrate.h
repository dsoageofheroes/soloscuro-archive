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

enum {
    NARRATE_CLOSE
};


int8_t narrate_open(int16_t action, const char *text, int16_t index);
int narrate_select_menu(int option);

#endif
