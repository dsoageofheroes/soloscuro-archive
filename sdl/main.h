#ifndef MAIN_H
#define MAIN_H

void main_exit_system();
SDL_Renderer *main_get_rend();
const float main_get_zoom();
void main_set_ignore_repeat(int repeat);
void main_set_xscroll(int amt);
void main_set_yscroll(int amt);
void main_exit_game();
int main_player_freeze();

#endif
