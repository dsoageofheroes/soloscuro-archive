#ifndef ANIMATE_H
#define ANIMATE_H

#include <SDL2/SDL.h>
#include "../src/animation.h"
#include "map.h"

void animate_init();
void animate_clear();
void animate_close();

void animate_list_render(SDL_Renderer *renderer);
animate_sprite_node_t *animate_list_add(animate_sprite_t *anim, const int zpos);
extern animate_sprite_node_t* animate_list_node_add(animate_sprite_node_t *node, const int zpos);
void animate_list_remove(animate_sprite_node_t *node, const int zpos);
void animate_shift_node(animate_sprite_node_t *an, const int zpos);
void animate_set_animation(animate_sprite_t *as, scmd_t *scmd, const uint32_t ticks_per_move);
void animate_sprite_node_free(animate_sprite_node_t *node);

#endif
