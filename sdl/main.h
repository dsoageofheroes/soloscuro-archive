#ifndef MAIN_H
#define MAIN_H

void main_exit_system();
SDL_Renderer *main_get_rend();
SDL_Surface *main_get_screen();
const float main_get_zoom();
void main_set_browser_mode();
void main_set_ignore_repeat(int repeat);
void main_set_xscroll(int amt);
void main_set_yscroll(int amt);
void main_exit_game();
int main_player_freeze();

#endif
