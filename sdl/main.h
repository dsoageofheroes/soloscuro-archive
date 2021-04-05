#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>
#include "textbox.h"

extern void main_exit_system();
extern SDL_Renderer *main_get_rend();
extern SDL_Surface *main_get_screen();
extern float main_get_zoom();
extern uint32_t main_get_width();
extern uint32_t main_get_height();
extern void main_center_on_player();
extern int main_still_running();
extern int main_get_debug();
extern void main_set_browser_mode();
extern void main_set_ignore_repeat(int repeat);
extern void main_set_xscroll(int amt);
extern void main_set_yscroll(int amt);
extern void main_exit_game();
extern void main_set_textbox(textbox_t *tb);
extern int main_player_freeze();


#endif
