#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdlib.h>
#include <stdint.h>
#include "sol_textbox.h"

int textbox_handle_keydown(sol_textbox_t *tb, SDL_Keysym);
int textbox_handle_keyup(sol_textbox_t *tb, SDL_Keysym);

#endif
