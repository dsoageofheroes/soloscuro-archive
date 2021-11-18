#include "entity-animation.h"
#include "combat.h"
#include "dsl.h"
#include "port.h"
#include <stdlib.h>

static entity_animation_node_t *last;
static entity_animation_node_t *next_animation_head = NULL;

static scmd_t combat_move_down[] = {
    {.bmp_idx = 3, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_up[] = {
    {.bmp_idx = 7, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_right[] = {
    {.bmp_idx = 11, .delay = 7, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = 0x0, .xoffset = 8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_melee_down[] = {
    {.bmp_idx = 15, .delay = 10, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 16, .delay = 10, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 17, .delay = 10, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_melee_up[] = {
    {.bmp_idx = 18, .delay = 10, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 19, .delay = 10, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 20, .delay = 10, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_melee_right[] = {
    {.bmp_idx = 21, .delay = 10, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 22, .delay = 10, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 23, .delay = 10, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_melee_left[] = {
    {.bmp_idx = 21, .delay = 10, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 22, .delay = 10, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 23, .delay = 10, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_left[] = {
    {.bmp_idx = 11, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_down[] = {
    {.bmp_idx = 3, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_up[] = {
    {.bmp_idx = 7, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_right[] = {
    {.bmp_idx = 11, .delay = 7, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = 0x0, .xoffset = 8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_left[] = {
    {.bmp_idx = 11, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_down[] = {
    {.bmp_idx = 0, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_up[] = {
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_right[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_left[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t cast_scmd[] = {
    {.bmp_idx = 0, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 8, .flags = 0x0, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 8, .flags = 0x0, .xoffset = -10, .yoffset = -10, 0, 0, 0},
    {.bmp_idx = 3, .delay = 7, .flags = SCMD_JUMP, .xoffset = -9, .yoffset = -10, 0, 0, 0},
};

static scmd_t throw_scmd[] = {
    {.bmp_idx = 0, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 7, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 7, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t throw_anim_scmd[] = {
    {.bmp_idx = 0, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 7, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 11, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 15, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 16, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 17, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 18, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 19, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 20, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 21, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 22, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 23, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 24, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 25, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 26, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 27, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 28, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 29, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 30, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 31, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 32, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 33, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 34, .delay = 5, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 35, .delay = 5, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 31, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 30, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 29, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 28, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 27, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 26, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 25, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 24, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 23, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 22, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 21, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 20, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 19, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 18, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 17, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 16, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 15, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 11, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 7, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 5, .flags = SCMD_XMIRROR, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 0, .delay = 5, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
};

static scmd_t *combat_types[] = {
    combat_stand_down,
    combat_stand_up,
    combat_stand_right,
    combat_stand_left,
    combat_move_down,
    combat_move_up,
    combat_move_right,
    combat_move_left,
    player_move_down,
    player_move_up,
    player_move_right,
    player_move_left,
    combat_melee_down,
    combat_melee_up,
    combat_melee_right,
    combat_melee_left,
    cast_scmd,
    throw_scmd + 0,
    throw_scmd + 1,
    throw_scmd + 2,
    throw_scmd + 3,
    throw_scmd + 4,
    throw_scmd + 5,
    throw_scmd + 6,
    throw_scmd + 7,
    throw_scmd + 8,
    throw_scmd + 9,
    throw_scmd + 10,
    throw_scmd + 11,
    throw_scmd + 12,
    throw_scmd + 13,
    throw_scmd + 14,
    throw_scmd + 15,
    throw_anim_scmd + 0,
    throw_anim_scmd + 4,
    throw_anim_scmd + 8,
    throw_anim_scmd + 12,
    throw_anim_scmd + 16,
    throw_anim_scmd + 20,
    throw_anim_scmd + 24,
    throw_anim_scmd + 28,
    throw_anim_scmd + 32,
    throw_anim_scmd + 36,
    throw_anim_scmd + 40,
    throw_anim_scmd + 44,
    throw_anim_scmd + 48,
    throw_anim_scmd + 52,
    throw_anim_scmd + 56,
    throw_anim_scmd + 60,
};

scmd_t* combat_get_scmd(const enum combat_scmd_t type) {
    return combat_types[type];
}

static scmd_t* get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff) {
    if (xdiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_LEFT); }
    if (xdiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT); }
    if (ydiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_UP); }
    if (ydiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_DOWN); }

    // xdiff and ydiff == 0.
    if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_LEFT) 
        || current_scmd == combat_get_scmd(COMBAT_SCMD_MELEE_LEFT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_LEFT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT)
        || current_scmd == combat_get_scmd(COMBAT_SCMD_MELEE_RIGHT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_RIGHT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)
        || current_scmd == combat_get_scmd(COMBAT_SCMD_MELEE_UP)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_UP);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_DOWN)
        || current_scmd == combat_get_scmd(COMBAT_SCMD_MELEE_DOWN)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    }

    return current_scmd;
}

static scmd_t* get_entity_scmd(scmd_t *current_scmd, enum entity_action_e action) {
    current_scmd = get_scmd(current_scmd, 0, 0);

    if (action == EA_MELEE) {
        if (current_scmd == combat_get_scmd(COMBAT_SCMD_STAND_LEFT)) {
            return combat_get_scmd(COMBAT_SCMD_MELEE_LEFT);
        } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_STAND_RIGHT)) {
            return combat_get_scmd(COMBAT_SCMD_MELEE_RIGHT);
        } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_STAND_UP)) {
            return combat_get_scmd(COMBAT_SCMD_MELEE_UP);
        } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_STAND_DOWN)) {
            return combat_get_scmd(COMBAT_SCMD_MELEE_DOWN);
        }
    } 

    return current_scmd;
}

extern scmd_t* entity_animation_face_direction(scmd_t *current_scmd, const enum entity_action_e action) {
    switch (action) {
        case EA_WALK_DOWNLEFT:
        case EA_WALK_UPLEFT:
        case EA_WALK_LEFT:
            return combat_stand_left;
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_UPRIGHT:
        case EA_WALK_RIGHT:
            return combat_stand_right;
        case EA_WALK_UP:
            return combat_stand_up;
        case EA_WALK_DOWN:
            return combat_stand_down;
        default:
            return current_scmd;
    }
}

extern scmd_t* entity_animation_get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff,
        const enum entity_action_e action) {
    current_scmd = get_scmd(current_scmd, xdiff, ydiff);
    if (action != EA_NONE) { current_scmd = get_entity_scmd(current_scmd, action); }

    return current_scmd;
}

extern void entity_animation_add(enum entity_action_e action, entity_t *source, entity_t *target,
        power_t *power, const int32_t amt) {
    entity_animation_list_t list;
    list.head = next_animation_head;
    entity_animation_list_add(&list, action, source, target, power, amt);
    next_animation_head = list.head;
}

extern int entity_animation_has_more() {
    return next_animation_head != NULL;
}

static void play_death_sound(entity_t *target) {
    if (!target) { return; }

    if (target->attack_sound) {
        sol_play_sound_effect(target->attack_sound + 2);
        return;
    }
}

static void play_melee_sound(entity_t *source) {
    if (source->attack_sound) {
        sol_play_sound_effect(source->attack_sound);
        return;
    }

    // sound 69: is Thri-keen melee sound
    if (source->race  == RACE_THRIKREEN) {
        sol_play_sound_effect(69);
        return;
    }

    // sound 7: general melee sound.
    sol_play_sound_effect(7);
}

static void play_damage_sound(entity_t *target) {
    if (!target) { return; }

    if (target->attack_sound) {
        sol_play_sound_effect(target->attack_sound + 1);
    }

    // sound 67: is PC taking damage
    sol_play_sound_effect(67);
}

// sound 63: is PC doing range attack
extern int entity_animation_region_execute(region_t *reg) {
    entity_animation_list_t list;
    list.head = next_animation_head;
    int ret = entity_animation_list_execute(&list, reg);
    next_animation_head = list.head;

    return ret;
}

static void set_anim(entity_t *entity) {
    entity_action_t *action = &(entity->actions.head->ca);
    const int distance = 32;
    const int ticks_per_move = action->start_amt;
    entity->anim.flags = 0x0;
    entity->anim.left_over = 0x0;
    switch (action->action) {
        case EA_WALK_UPLEFT:
        case EA_WALK_UPRIGHT:
        case EA_WALK_DOWNLEFT:
        case EA_WALK_DOWNRIGHT:
            entity->anim.movex = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);
            entity->anim.movey = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);
            break;
        case EA_WALK_LEFT:
        case EA_WALK_RIGHT:
            entity->anim.movex = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);
            entity->anim.movey = 0.0;
            break;
        case EA_WALK_UP:
        case EA_WALK_DOWN:
            entity->anim.movey = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);
            entity->anim.movex = 0.0;
            break;
        default:
            entity->anim.movex = entity->anim.movey = 0;
            break;
    }
}

static scmd_t* entity_get_next_scmd(const entity_t *entity, const enum entity_action_e action) {
    switch (action) {
        case EA_WALK_UPLEFT:
        case EA_WALK_DOWNLEFT:
        case EA_WALK_LEFT: return combat_move_left;
        case EA_WALK_UPRIGHT:
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_RIGHT: return combat_move_right;
        case EA_WALK_UP: return combat_move_up;
        case EA_WALK_DOWN: return combat_move_down;
        ////case EA_SCMD:
            //printf("EA_SCMD! %p\n", entity->actions.head);
            //entity->actions.head->ca.scmd_pos = 
                //ssi_scmd_next_pos(entity->anim.scmd, entity->actions.head->ca.scmd_pos);
            //printf("EA_SCMD!\n");
            //break;
        default:
            break;
    }
    return entity->anim.scmd;
}

extern int entity_animation_list_start_scmd(struct entity_s *entity) {
    if (!entity || !entity->anim.scmd) { return 0; }

    entity_animation_list_add(&entity->actions, EA_SCMD, NULL, NULL,
        NULL, ssi_scmd_total_delay(entity->anim.scmd, 0));
    set_anim(entity);

    return 1;
}

extern int entity_animation_execute(entity_t *entity) {
    if (!entity || !entity->actions.head) { return 0; }
    entity_action_t *action = &(entity->actions.head->ca);

    for (int i = 0; i < action->speed; i++) {
        //printf("action->amt = %d (%d), ticks = %d, scmd_pos = %d of %d\n", action->amt, action->start_amt,
            //action->ticks, action->scmd_pos, entity->anim.scmd[action->scmd_pos].delay);
        if (action->start_amt != -1 && action->amt == action->start_amt) {
            entity->anim.scmd = entity_get_next_scmd(entity, action->action);
            set_anim(entity);
        }

        action->amt--;
        action->ticks++;

        if (entity->anim.scmd[action->scmd_pos].delay < action->ticks) {
            //if (entity->name) { printf("->%s: ticks, amt = %d\n", entity->name, action->amt); }
            entity->anim.pos = action->scmd_pos = ssi_scmd_next_pos(entity->anim.scmd, action->scmd_pos);
            port_entity_update_scmd(entity);
            action->ticks = 0;
        }

        if (action->start_amt >= 0 && action->amt <= 0) {
            entity->anim.x = entity->anim.destx;
            entity->anim.y = entity->anim.desty;
            entity_animation_node_t *to_delete = entity->actions.head;
            entity->actions.head = entity->actions.head->next;
            if (!entity->actions.head) {
                entity->anim.scmd = entity_animation_face_direction(entity->anim.scmd, to_delete->ca.action);
                //entity->anim.scmd = combat_stand_left;
                entity->anim.flags = 0x0;
                entity->anim.pos = 0;
                entity->anim.flags = 0;
                entity->anim.left_over = 0x0;
                entity->anim.movex = entity->anim.movey = 0;
            }
            free (to_delete);
            return 1;
        }

        animate_sprite_tick(action, entity);
    }

    return 1;
}

entity_animation_list_t* entity_animation_list_create() {
    return calloc(1, sizeof(entity_animation_list_t));
}

void entity_animation_list_free(entity_animation_list_t *list) {
    if (!list) { return; }

    if (list->head) {
        entity_animation_node_t *to_delete = NULL;
        for (entity_animation_node_t *rover = list->head; rover ; rover = rover->next) {
            if (to_delete) { free(to_delete); }
            to_delete = rover;
        }
        free(to_delete);
        list->head = NULL;
    }
}

void entity_animation_list_add(entity_animation_list_t *list, enum entity_action_e action,
        entity_t *source, entity_t *target, power_t *power, const int32_t amt) {
     entity_animation_list_add_speed(list, action, source, target, power, amt, 1);
}
extern void entity_animation_list_add_speed(entity_animation_list_t *list, enum entity_action_e action,
        struct entity_s *source, struct entity_s *target, struct power_s *power, const int32_t amt, const int32_t speed) {
    if (!list) { return; }
    entity_animation_node_t *toadd = malloc(sizeof(entity_animation_node_t));
    entity_animation_node_t *rover = list->head;

    toadd->ca.action = action;
    toadd->ca.source = source;
    toadd->ca.target = target;
    toadd->ca.power  = power;
    toadd->ca.amt = amt;
    toadd->ca.start_amt = amt;
    toadd->ca.ticks = 0;
    toadd->ca.scmd_pos = 0;
    toadd->ca.speed = speed;
    toadd->next = NULL;

    if (!list->head) {
        list->head = toadd;
    } else {
        while (rover->next) { rover = rover->next; }
        rover->next = toadd;
    }
}

extern int entity_animation_list_execute(entity_animation_list_t *list, region_t *reg) {
    if (!list || !list->head) { return 0; }

    entity_t *source = list->head->ca.source;
    entity_t *target = list->head->ca.target;

    switch(list->head->ca.action) {
        case EA_MELEE:
            play_melee_sound(source);
            source->anim.scmd = get_entity_scmd(source->anim.scmd, list->head->ca.action);
            port_update_entity(source, 0, 0);
            break;
        case EA_RED_DAMAGE:
        case EA_BIG_RED_DAMAGE:
        case EA_GREEN_DAMAGE:
        case EA_MAGIC_DAMAGE:
        case EA_BROWN_DAMAGE:
            play_damage_sound(target);
            source->anim.scmd = get_scmd(source->anim.scmd, 0, 0);
            port_update_entity(source, 0, 0);
            port_combat_action(&(list->head->ca));
            break;
        case EA_DAMAGE_APPLY:
            target->stats.hp -= list->head->ca.amt;
            if (target->stats.hp <= 0) {
                target->combat_status = COMBAT_STATUS_DYING;
                play_death_sound(target);
                combat_is_defeated(reg, target);
            }
            break;
        case EA_POWER_CAST:
        case EA_POWER_THROW:
        case EA_POWER_HIT:
            port_combat_action(&(list->head->ca));
            break;
        case EA_POWER_APPLY:
            break;
        default:
            error("unknown action %d!\n", list->head->ca.action);
            break;
    }

    last = list->head;
    list->head = list->head->next;
    if (last) { free(last); last = NULL; }

    return 1;
}
