#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>
#include "textbox.h"

void main_exit_system();
SDL_Renderer *main_get_rend();
SDL_Surface *main_get_screen();
const float main_get_zoom();
uint32_t main_get_width();
uint32_t main_get_height();
void main_center_on_player();
void main_set_browser_mode();
void main_set_ignore_repeat(int repeat);
void main_set_xscroll(int amt);
void main_set_yscroll(int amt);
void main_exit_game();
void main_set_textbox(textbox_t *tb);
int main_player_freeze();


#endif
