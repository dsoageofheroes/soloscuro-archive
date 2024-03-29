#include "entity-animation.h"
#include "combat.h"
#include "gpl.h"
#include "port.h"
#include "arbiter.h"
#include "region-manager.h"
#include "player.h"
#include "combat-status.h"
#include <stdlib.h>
#include <math.h>

static void clear_scmd_status(sol_entity_t *entity);

static sol_entity_animation_node_t *next_animation_head = NULL;

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

static scmd_t door_open[] = {
    {.bmp_idx = 0, .delay = 14, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 16, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 16, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 14, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t door_close[] = {
    {.bmp_idx = 3, .delay = 14, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 16, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 16, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 0, .delay = 14, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
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
    cast_scmd,
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

extern sol_status_t sol_combat_get_scmd(const sol_combat_scmd_t type, scmd_t **s) {
    if (!s) { return SOL_NULL_ARGUMENT; }
    *s = combat_types[type];
    return SOL_SUCCESS;
}

static scmd_t* get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff) {
    if (xdiff < 0) { return combat_types[COMBAT_SCMD_MOVE_LEFT]; }
    if (xdiff > 0) { return combat_types[COMBAT_SCMD_MOVE_RIGHT]; }
    if (ydiff < 0) { return combat_types[COMBAT_SCMD_MOVE_UP]; }
    if (ydiff > 0) { return combat_types[COMBAT_SCMD_MOVE_DOWN]; }

    // xdiff and ydiff == 0.
    if (current_scmd == combat_types[COMBAT_SCMD_MOVE_LEFT] 
        || current_scmd == combat_types[COMBAT_SCMD_MELEE_LEFT]) {
        return combat_types[COMBAT_SCMD_STAND_LEFT];
    } else if (current_scmd == combat_types[COMBAT_SCMD_MOVE_RIGHT]
        || current_scmd == combat_types[COMBAT_SCMD_MELEE_RIGHT]) {
        return combat_types[COMBAT_SCMD_STAND_RIGHT];
    } else if (current_scmd == combat_types[COMBAT_SCMD_MOVE_UP]
        || current_scmd == combat_types[COMBAT_SCMD_MELEE_UP]) {
        return combat_types[COMBAT_SCMD_STAND_UP];
    } else if (current_scmd == combat_types[COMBAT_SCMD_MOVE_DOWN] || current_scmd == combat_types[COMBAT_SCMD_MELEE_DOWN]) {
        return combat_types[COMBAT_SCMD_STAND_DOWN];
    }

    return current_scmd;
}

static scmd_t* get_entity_scmd(scmd_t *current_scmd, enum sol_entity_action_e action) {
    current_scmd = get_scmd(current_scmd, 0, 0);

    if (action == EA_MELEE) {
        if (current_scmd == combat_types[COMBAT_SCMD_STAND_LEFT]) {
            return combat_types[COMBAT_SCMD_MELEE_LEFT];
        } else if (current_scmd == combat_types[COMBAT_SCMD_STAND_RIGHT]) {
            return combat_types[COMBAT_SCMD_MELEE_RIGHT];
        } else if (current_scmd == combat_types[COMBAT_SCMD_STAND_UP]) {
            return combat_types[COMBAT_SCMD_MELEE_UP];
        } else if (current_scmd == combat_types[COMBAT_SCMD_STAND_DOWN]) {
            return combat_types[COMBAT_SCMD_MELEE_DOWN];
        }
    } 
    switch(action) {
        case EA_DOOR_OPEN: return door_open;
        case EA_DOOR_CLOSE: return door_close;
        default:
            break;
    }

    return current_scmd;
}

extern sol_status_t sol_entity_animation_face_direction(scmd_t *current_scmd, const enum sol_entity_action_e action, scmd_t **ret) {
    switch (action) {
        case EA_WALK_DOWNLEFT:
        case EA_WALK_UPLEFT:
        case EA_WALK_LEFT:
            *ret = combat_stand_left;
            return SOL_SUCCESS;
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_UPRIGHT:
        case EA_WALK_RIGHT:
            *ret = combat_stand_right;
            return SOL_SUCCESS;
        case EA_WALK_UP:
            *ret = combat_stand_up;
            return SOL_SUCCESS;
        case EA_WALK_DOWN:
            *ret = combat_stand_down;
            return SOL_SUCCESS;
        default:
            *ret = current_scmd;
            return SOL_SUCCESS;
    }

    return SOL_NOT_FOUND;
}

extern sol_status_t sol_entity_animation_get_scmd(struct sol_entity_s *entity, const int xdiff, const int ydiff,
        const enum sol_entity_action_e action, scmd_t **ret) {
    if (entity == NULL) { return SOL_NULL_ARGUMENT; }
    if (action != EA_NONE) {
        *ret = get_entity_scmd(entity->anim.scmd, action);
        return SOL_SUCCESS;
    }
    switch(entity->direction) {
        case EA_WALK_RIGHT: *ret = combat_stand_right; return SOL_SUCCESS;
        case EA_WALK_UP: *ret = combat_stand_up; return SOL_SUCCESS;
        case EA_WALK_DOWN: *ret = combat_stand_down; return SOL_SUCCESS;
    }

    *ret = combat_stand_left;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_animation_add(enum sol_entity_action_e action, sol_entity_t *source, sol_entity_t *target,
        sol_power_t *power, const int32_t amt) {
    sol_status_t status;
    sol_entity_animation_list_t list;
    list.head = next_animation_head;
    status = sol_entity_animation_list_add(&list, action, source, target, power, amt);
    next_animation_head = list.head;
    return status;
}

static void play_death_sound(sol_entity_t *target) {
    if (!target) { return; }

    if (target->attack_sound) {
        sol_play_sound_effect(target->attack_sound + 2);
        return;
    }
}

/*
static void play_melee_sound(sol_entity_t *source) {
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
*/

static void play_damage_sound(sol_entity_t *target) {
    if (!target) { return; }

    if (target->attack_sound) {
        sol_play_sound_effect(target->attack_sound + 1);
        return;
    }

    // sound 67: is PC taking damage
    sol_play_sound_effect(67);
}

static int region_damage_execute(sol_power_t *pw, sol_region_t *reg) {
    if (!reg || !reg->actions.head) { return 0; }
    sol_entity_action_t *action = &(reg->actions.head->ca);

    //sol_entity_t *source = action->source;
    sol_entity_t *target = action->target;

    switch(action->action) {
        case EA_MELEE:
            error("NEED TO IMPLEMENT MELEE animation!\n");
            //play_melee_sound(source);
            //source->anim.scmd = get_entity_scmd(source->anim.scmd, list->head->ca.action);
            //port_update_entity(source, 0, 0);
            break;
        case EA_RED_DAMAGE:
        case EA_BIG_RED_DAMAGE:
        case EA_GREEN_DAMAGE:
        case EA_MAGIC_DAMAGE:
        case EA_BROWN_DAMAGE:
            if (action->start_amt == action->amt) {
                play_damage_sound(target);
            }
            break;
        case EA_DAMAGE_APPLY: // handled in region_animation_last_check.
            break;
        default:
            error("unknown action %d!\n", action->action);
            break;
    }

    //last = list->head;
    //list->head = list->head->next;
    //if (last) { free(last); last = NULL; }

    return 1;
}

// This is called right before the animation is freed.
// It has already been removed from the region's list.
static void region_animation_last_check(sol_region_t *reg, sol_entity_animation_node_t *todelete) {
    sol_power_instance_t pi;
    sol_entity_t *target;
    sol_entity_action_t *action;
    sol_combat_region_t *cr = NULL;
    int slot;

    switch(todelete->ca.action) {
        case EA_POWER_HIT:
            if (sol_arbiter_hits(todelete) == SOL_SUCCESS) {
                pi.entity = todelete->ca.source;
                pi.item = NULL;
                pi.stats = todelete->ca.power;
                todelete->ca.power->actions.apply(&pi, todelete->ca.target);
            }
            break;
        case EA_DAMAGE_APPLY:
            sol_combat_clear_damage();
            target = todelete->ca.target;
            action = &(todelete->ca);
            target->stats.hp -= action->damage;
            if (target->stats.hp <= 0) {
                target->combat_status = COMBAT_STATUS_DYING;
                play_death_sound(target);
                warn("NEED TO IMPLEMENT death animation!\n");
                sol_arbiter_combat_region(reg, &cr);
                if (!sol_entity_list_remove_entity(&cr->combatants, target)) {
                    error("Unable to remove entity from combat region!\n");
                }
                if (!sol_entity_list_remove_entity(&cr->round.entities, target)) {
                    error("Unable to remove entity from combat round!\n");
                }
                if (!sol_entity_list_remove_entity(reg->entities, target)) {
                    error("Unable to remove entity from region!\n");
                }
                // Only free if not a player.
                if (sol_player_get_slot(target, &slot) == SOL_SUCCESS && slot < 0) {
                    sol_entity_free(target);
                }
                target = NULL;
            }
            break;
        default:
            break;
    }
}

static int frame_offset(const sol_entity_t *source, const sol_entity_t *dest) {
    int diffx = abs(dest->mapx - source->mapx);
    int diffy = abs(dest->mapy - source->mapy);
    float angle = atan ((float)diffx / (float)diffy);

    int offset = angle * 4.0 / 1.58; // 1.57 is max
    if (offset > 3) {offset = 3; }

    //printf("angle = %f, offset = %d\n", angle, offset);
    if (dest->mapy > source->mapy) { // above
        if (dest->mapx < source->mapx) { // right
            return 8 + offset;
        } else { // left
            return 8 - offset;
        }
    } else { // dest->mapy <= source->mapy // below
        if (dest->mapx < source->mapx) { // right
            return 15 - offset;
        } else { // left
            return 0 + offset;
        }
    }
    return 1;
}

// sound 63: is PC doing range attack
extern sol_status_t sol_entity_animation_region_execute(sol_region_t *reg) {
    sol_power_t pw;
    sol_sprite_info_t info;
    if (!reg)               { return SOL_NULL_ARGUMENT;}
    if (!reg->actions.head) { return SOL_NOT_EMPTY; }
    sol_entity_action_t *action = &(reg->actions.head->ca);

    if (action->action == EA_WAIT_ON_ENTITY) {
        // If the source died or is done with the animation.
        if (!action->source || !action->source->actions.head) {
            sol_entity_animation_node_t *tmp = reg->actions.head;
            reg->actions.head = reg->actions.head->next;
            free (tmp);
            return SOL_SUCCESS;
        }
        // Execute that entity's animation and get out.
        //printf("source = %s\n", action->source->name);
        sol_entity_animation_execute(action->source);
        return SOL_SUCCESS;
    }

    if (!action->power && !action->damage) {
        //entity_animation_list_add_effect(&(reg->actions), EA_MAGIC_DAMAGE, source, target, NULL, 0, damage);
        error("Only handle power actions at the moment!");
        return SOL_NOT_IMPLEMENTED;
    }

    if (action->start_amt == action->amt && action->action == EA_POWER_THROW) {
        sol_status_check(sol_sprite_get_info(action->power->thrown.spr, &info), "Unable to get action sprite info.");
        int frame_count = info.num_frames;
        if (frame_count != 9) {
            //TODO: fix this for testing (this error happens all over unit tests.)
            error("Unknown frame for thrown animation! need to code...\n");
            //exit(1);
        }
        sol_play_sound_effect(action->power->thrown_sound);
        // Need to set correct direction);
        action->power->thrown.scmd = throw_scmd + frame_offset(action->source, action->target);
    }

    if (action->damage) {
        action->amt--;
        if (action->amt == 0) {
            sol_entity_animation_list_remove_current(&reg->actions);
            action = &(reg->actions.head->ca);
            if (action->action == EA_DAMAGE_APPLY) {
                region_damage_execute(&pw, reg);
                region_animation_last_check(reg, reg->actions.head);
                sol_entity_animation_list_remove_current(&reg->actions);
            }
        }
        return SOL_SUCCESS;
    }
    sol_power_t *power = reg->actions.head->ca.power;

    for (int i = 0; i < action->speed; i++) {
        //printf("action->amt = %d (%d), ticks = %d, scmd_pos = %d of %d\n", action->amt, action->start_amt,
            //action->ticks, action->scmd_pos, entity->anim.scmd[action->scmd_pos].delay);
        action->amt--;
        action->ticks++;

        animate_sprite_t *as =
            (action->action == EA_POWER_CAST) ? &(power->cast) :
            (action->action == EA_POWER_THROW) ? &(power->thrown) :
            (action->action == EA_POWER_HIT) ? &(power->hit) :
            &(power->cast);
        if (as->scmd[action->scmd_pos].delay < action->ticks) {
            //printf("region_execute: scnd_pos = %d\n", action->scmd_pos);
            action->scmd_pos = ssi_scmd_next_pos(as->scmd, action->scmd_pos);
            //sol_sprite_render_flip(cmap->region->actions.head->ca.power->cast.spr, 0, 0);
            sol_sprite_set_frame(as->spr, as->scmd[action->scmd_pos].bmp_idx);
            //port_entity_update_scmd(entity);
            action->ticks = 0;
        }

        if (action->start_amt >= 0 && action->amt <= 0) {
            sol_entity_animation_node_t *tmp = reg->actions.head;
            reg->actions.head = reg->actions.head->next;
            region_animation_last_check(reg, tmp);
            free (tmp);
            return SOL_SUCCESS;
        }
    }

    return SOL_SUCCESS;
}

static int entity_animation_check_update(sol_entity_t *entity, const int16_t xdiff, const int16_t ydiff) {
    sol_region_t *reg;

    sol_region_manager_get_current(&reg);
    if (sol_region_location_blocked(reg, entity->mapx + xdiff, entity->mapy + ydiff)) {
        return 0;
    }

    sol_entity_animation_update(entity, xdiff, ydiff);
    return 1;
}

extern sol_status_t sol_entity_animation_update(sol_entity_t *entity, const int16_t xdiff, const int16_t ydiff) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    animate_sprite_t *as = &(entity->anim);
    const float zoom = settings_zoom();
    //printf("cur:%d %d\n", as->x, as->y);
    //printf("dest: %d, %d\n", as->destx, as->desty);
    as->x = as->destx;
    as->y = as->desty;
    entity->mapx += xdiff;
    entity->mapy += ydiff;
    as->destx = entity->mapx * 16 * settings_zoom();
    as->desty = entity->mapy * 16 * settings_zoom();
    if (as->w > 16 * zoom) {
        //printf("width = %d\n", as->w);
        as->destx -= (as->w - 16 * zoom) / 2;
    }
    as->desty -= as->h - (16 * zoom);
    return SOL_SUCCESS;
}


extern sol_status_t sol_entity_animation_list_empty(sol_entity_animation_list_t *list) {
    return (list && list->head) ? SOL_NOT_EMPTY : SOL_SUCCESS;
}

static void set_anim(sol_entity_t *entity) {
    sol_entity_action_t *action = &(entity->actions.head->ca);
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

static scmd_t* get_melee_scmd(sol_entity_t *dude) {
    switch(dude->direction) {
        case EA_WALK_RIGHT: return combat_melee_right;
        case EA_WALK_UP: return combat_melee_up;
        case EA_WALK_DOWN: return combat_melee_down;
    }

    return combat_melee_left;
}

static scmd_t* entity_get_next_scmd(sol_entity_t *entity, const enum sol_entity_action_e action) {
    switch (action) {
        case EA_WALK_UPLEFT:
        case EA_WALK_DOWNLEFT:
        case EA_WALK_LEFT: entity->direction = EA_WALK_LEFT; return combat_move_left;
        case EA_WALK_UPRIGHT:
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_RIGHT: entity->direction = EA_WALK_RIGHT; return combat_move_right;
        case EA_WALK_UP: entity->direction = EA_WALK_UP; return combat_move_up;
        case EA_WALK_DOWN: entity->direction = EA_WALK_DOWN; return combat_move_down;
        case EA_POWER_CAST: return combat_types[COMBAT_POWER_CAST];
        case EA_MELEE: return get_melee_scmd(entity);
        case EA_NONE:
            switch(entity->direction) {
                case EA_WALK_RIGHT: return combat_stand_right;
                case EA_WALK_UP: return combat_stand_up;
                case EA_WALK_DOWN: return combat_stand_down;
            }
            return combat_stand_left;
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

static int apply_action(sol_entity_t *entity, sol_entity_action_t *action) {
    int ret = 1;

    switch (action->action) {
        case EA_WALK_LEFT:      ret = entity_animation_check_update(entity, -1, 0); break;
        case EA_WALK_RIGHT:     ret = entity_animation_check_update(entity, 1, 0); break;
        case EA_WALK_UP:        ret = entity_animation_check_update(entity, 0, -1); break;
        case EA_WALK_DOWN:      ret = entity_animation_check_update(entity, 0, 1); break;
        case EA_WALK_UPLEFT:    ret = entity_animation_check_update(entity, -1, -1); break;
        case EA_WALK_UPRIGHT:   ret = entity_animation_check_update(entity, 1, -1); break;
        case EA_WALK_DOWNLEFT:  ret = entity_animation_check_update(entity, -1, 1); break;
        case EA_WALK_DOWNRIGHT: ret = entity_animation_check_update(entity, 1, 1); break;
        case EA_NONE:
                                entity->anim.scmd = entity_get_next_scmd(entity, action->action);
                                clear_scmd_status(entity);
                                sol_sprite_set_frame_keep_loc(entity->anim.spr, entity->anim.scmd[entity->anim.pos].bmp_idx);
                                break;
        default:
                                error("Unhandled action: %d\n", action->action);
                                //exit(1);
    }

    //printf("(%d, %d)\n", entity->mapx, entity->mapy);
    //printf("(%d, %d) -> (%d, %d)\n", anim->x, anim->y, anim->destx, anim->desty);
    return ret;
}

static void update_camera(sol_entity_t *entity, sol_entity_action_t *action) {
    sol_sprite_info_t info;
    sol_status_check(sol_sprite_get_info(entity->anim.spr, &info), "Unable to get entity sprite info");
    int x = info.x;
    int y = info.y;
    const int buf = 16 * 6 * settings_zoom();

    //printf("%s: x = %d, y = %d, buf = %d, screen = %d, %d\n", entity->name, x, y, buf, settings_screen_width(), settings_screen_height());
    //printf("(%d, %d), frame: %d\n", entity->mapx, entity->mapy, sol_sprite_get_frame(entity->anim.spr));
    if (x < buf) {
        sol_camera_scrollx(x - buf);
    }
    if (x > (settings_screen_width() - buf)) {
        sol_camera_scrollx(x - (settings_screen_width() - buf));
    }
    if (y < buf) {
        sol_camera_scrolly(y - buf);
    }
    if (y > (settings_screen_height() - buf)) {
        sol_camera_scrolly(y - (settings_screen_height() - buf));
    }
}

static void clear_scmd_status(sol_entity_t *entity) {
    entity->anim.flags = 0x0;
    entity->anim.pos = 0;
    entity->anim.flags = 0;
    entity->anim.left_over = 0x0;
    entity->anim.movex = entity->anim.movey = 0;
}

extern sol_status_t sol_entity_animation_execute(sol_entity_t *entity) {
    if (!entity)               { return SOL_NULL_ARGUMENT; }
    if (!entity->actions.head) { return SOL_NOT_EMPTY; }

    sol_entity_t *active;
    sol_entity_action_t *action = &(entity->actions.head->ca);
    sol_status_t status = SOL_SUCCESS;

    if (action->ticks == 0 && action->amt == action->start_amt) {
        if (!apply_action(entity, action)) {
            sol_entity_animation_list_free(&entity->actions);
            return SOL_UNKNOWN_ERROR;
        }
    }

    sol_player_get_active(&active);
    if (entity == active) {
        update_camera(entity, action);
    }

    for (int i = 0; i < action->speed && entity->anim.scmd; i++) {
        //printf("action->amt = %d (%d), action = %d, ticks = %d, scmd_pos = %d of %d\n",
            //action->amt, action->start_amt, action->action,
            //action->ticks, action->scmd_pos, entity->anim.scmd[action->scmd_pos].delay);
        if (action->start_amt != -1 && action->amt == action->start_amt) {
            entity->anim.scmd = entity_get_next_scmd(entity, action->action);
            clear_scmd_status(entity);
            set_anim(entity);
        }

        action->amt--;
        action->ticks++;

        if (entity->anim.scmd[action->scmd_pos].delay < action->ticks) {
            //if (entity->name) { printf("->%s: ticks, amt = %d\n", entity->name, action->amt); }
            if (entity->anim.scmd[action->scmd_pos].flags & SCMD_LAST) {
                sol_entity_animation_list_remove_current(&entity->actions);
                return SOL_SUCCESS;
            }
            entity->anim.pos = action->scmd_pos = ssi_scmd_next_pos(entity->anim.scmd, action->scmd_pos);
            //printf("HERE: %d\n", entity->anim.pos);
            sol_entity_update_scmd(entity);
            //sprite_set_frame(entity->anim.spr, entity->anim.scmd[entity->anim.pos].bmp_idx);
            action->ticks = 0;
        }

        if (action->start_amt >= 0 && action->amt <= 0) {
            // SCMD are a special action that we assume is a loop.
            if (action->action == EA_SCMD) { action->amt = action->start_amt; continue;}

            sol_entity_animation_node_t *to_delete = entity->actions.head;
            entity->actions.head = entity->actions.head->next;
            if (!entity->actions.head) {
                sol_entity_animation_face_direction(entity->anim.scmd, to_delete->ca.action, &entity->anim.scmd);
                clear_scmd_status(entity);
            }
            free (to_delete);
            return SOL_SUCCESS;
        }

        if ((status = sol_animate_sprite_tick(action, entity))) {
            return status;
        }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_animation_list_remove_current(sol_entity_animation_list_t *list) {
    if (!list)       { return SOL_NULL_ARGUMENT;}
    if (!list->head) { return SOL_NOT_EMPTY; }

    sol_entity_animation_node_t *to_delete = list->head;
    list->head = list->head->next;
    free(to_delete);

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_animation_list_remove_references(sol_entity_animation_list_t *list, sol_entity_t *dead) {
    if (!list || !dead) { return SOL_NULL_ARGUMENT; }

    for (sol_entity_animation_node_t *rover = list->head; rover ; rover = rover->next) {
        if (rover->ca.source == dead) { rover->ca.source = NULL; }
        if (rover->ca.target == dead) { rover->ca.target = NULL; }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_animation_list_free(sol_entity_animation_list_t *list) {
    if (!list) { return SOL_NULL_ARGUMENT; }

    if (list->head) {
        sol_entity_animation_node_t *to_delete = NULL;
        for (sol_entity_animation_node_t *rover = list->head; rover ; rover = rover->next) {
            if (to_delete) { free(to_delete); }
            to_delete = rover;
        }
        free(to_delete);
        list->head = NULL;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_animation_list_add_effect(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        sol_entity_t *source, sol_entity_t *target, sol_power_t *power, const int32_t amt, const int damage) {
     return sol_entity_animation_list_add_speed(list, action, source, target, power, amt, 1, damage);
}

extern sol_status_t sol_entity_animation_list_add(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        sol_entity_t *source, sol_entity_t *target, sol_power_t *power, const int32_t amt) {
     return sol_entity_animation_list_add_speed(list, action, source, target, power, amt, 1, 0);
}
extern sol_status_t sol_entity_animation_list_add_speed(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        sol_entity_t *source, sol_entity_t *target, struct sol_power_s *power, const int32_t amt, const int32_t speed,
        const int32_t damage) {
    if (!list) { return SOL_NULL_ARGUMENT; }
    //printf("add_speed: %s, %d\n", source ? source->name : "?", speed);
    sol_entity_animation_node_t *toadd = malloc(sizeof(sol_entity_animation_node_t));
    sol_entity_animation_node_t *rover = list->head;

    toadd->ca.action = action;
    toadd->ca.source = source;
    toadd->ca.target = target;
    toadd->ca.power  = power;
    toadd->ca.amt = amt;
    toadd->ca.start_amt = amt;
    toadd->ca.ticks = 0;
    toadd->ca.scmd_pos = 0;
    toadd->ca.speed = speed;
    toadd->ca.damage = damage;
    toadd->next = NULL;

    if (!list->head) {
        list->head = toadd;
    } else {
        while (rover->next) { rover = rover->next; }
        rover->next = toadd;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_animation_render(const sol_entity_action_t *ea) {
    if (ea == NULL) { return SOL_NULL_ARGUMENT; }
    sol_sprite_t spr = SPRITE_ERROR;
    scmd_t *scmd;
    sol_sprite_info_t info;

    switch (ea->action) {
        case EA_POWER_APPLY:
        case EA_POWER_CAST:
            spr = ea->power->cast.spr;
            sol_sprite_center_spr(spr, ea->source->anim.spr);
            break;
        case EA_POWER_THROW:
            spr = ea->power->thrown.spr;
            scmd = ea->power->thrown.scmd + ea->scmd_pos;
            float percent_there = (float)(ea->start_amt - ea->amt) / (float)ea->start_amt;
            sol_status_check(sol_sprite_get_info(ea->source->anim.spr, &info), "Unable to get entity source sprite info");
            int32_t sx = info.x;
            int32_t sy = info.y;
            sol_status_check(sol_sprite_get_info(ea->target->anim.spr, &info), "Unable to get entity target sprite info");
            int32_t tx = info.x;
            int32_t ty = info.y;
            int32_t x = abs(sx - tx);
            int32_t y = abs(sy - ty);

            x = tx < sx ? sx - (percent_there) * x : sx + (percent_there) * x;
            y = ty < sy ? sy - (percent_there) * y : sy + (percent_there) * y;

            sol_sprite_set_location(spr, x, y);
            sol_sprite_render_flip(spr, scmd->flags & SCMD_XMIRROR, scmd->flags & SCMD_YMIRROR);
            return SOL_SUCCESS;
        case EA_POWER_HIT:
            spr = ea->power->hit.spr;
            sol_sprite_center_spr(spr, ea->target->anim.spr);
            return SOL_SUCCESS;
        case EA_RED_DAMAGE:
        case EA_BIG_RED_DAMAGE:
        case EA_GREEN_DAMAGE:
        case EA_MAGIC_DAMAGE:
        case EA_BROWN_DAMAGE:
            sol_combat_action(ea);
            return SOL_SUCCESS;
        case EA_WAIT_ON_ENTITY:
            return SOL_WAIT_ACTIONS; // don't render!
        default:
            spr = ea->source->anim.spr;
        break;
    }

    sol_sprite_render_flip(spr, 0, 0);
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_update_scmd_info(sol_entity_t *dude) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    if (dude->anim.scmd_info.gff_idx >= 0) { return SOL_OUT_OF_RANGE; }

    for (int i = 0; i < 20; i++) {
        if (dude->anim.scmd == combat_types[i]) {
            dude->anim.scmd_info.res_id = i;
            return SOL_SUCCESS;
        }
    }

    return SOL_NOT_FOUND;
}
