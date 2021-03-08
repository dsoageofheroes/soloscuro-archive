#include "combat-animation.h"
#include "combat.h"
#include "dsl.h"
#include "port.h"
#include <stdlib.h>

static combat_animation_node_t *last;
static combat_animation_node_t *next_animation_head = NULL, *next_animation_tail = NULL;

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
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)
        || current_scmd == combat_get_scmd(COMBAT_SCMD_MELEE_DOWN)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    }

    return current_scmd;
}


static scmd_t* get_combat_scmd(scmd_t *current_scmd, enum combat_action_e action) {
    current_scmd = get_scmd(current_scmd, 0, 0);

    if (action == CA_MELEE) {
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

scmd_t* combat_animation_face_direction(scmd_t *current_scmd, const enum combat_action_e action) {
    switch (action) {
        case CA_WALK_DOWNLEFT:
        case CA_WALK_UPLEFT:
        case CA_WALK_LEFT:
            return combat_get_scmd(COMBAT_SCMD_STAND_LEFT);
        case CA_WALK_DOWNRIGHT:
        case CA_WALK_UPRIGHT:
        case CA_WALK_RIGHT:
            return combat_get_scmd(COMBAT_SCMD_STAND_RIGHT);
        case CA_WALK_UP:
            return combat_get_scmd(COMBAT_SCMD_STAND_UP);
        case CA_WALK_DOWN:
            return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
        default:
            return current_scmd;
    }
}

scmd_t* combat_animation_get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff,
        const enum combat_action_e action) {
    current_scmd = get_scmd(current_scmd, xdiff, ydiff);
    if (action != CA_NONE) { current_scmd = get_combat_scmd(current_scmd, action); }

    return current_scmd;
}

void combat_animation_add(enum combat_action_e action, entity_t *source, entity_t *target, const int32_t amt) {
    combat_animation_node_t *toadd = malloc(sizeof(combat_animation_node_t));
    toadd->ca.action = action;
    toadd->ca.source = source;
    toadd->ca.target = target;
    toadd->ca.amt = amt;
    toadd->next = NULL;
    if (!next_animation_head) {
        next_animation_head = next_animation_tail = toadd;
    } else {
        next_animation_tail->next = toadd;
    }
}

int combat_animation_has_more() {
    return next_animation_head != NULL;
}

static void apply_last(region_t *reg) {
    // This comes from last.
    switch(last->ca.action) {
        case CA_RED_DAMAGE:
            last->ca.target->stats.hp -= last->ca.amt;
            if (last->ca.target->stats.hp <= 0) {
                //printf("DYING\n");
                last->ca.target->combat_status = COMBAT_STATUS_DYING;
                combat_is_defeated(reg, last->ca.target);
            }
            break;
        default:
            break;
    }
}

int combat_animation_execute(region_t *reg) {
    if (!next_animation_head) {
        if (last) { // we just finished.
            apply_last(reg);
            free(last);
            last = NULL;
        }
        return 0;
    }
    entity_t *source = next_animation_head->ca.source;

    switch(next_animation_head->ca.action) {
        case CA_MELEE:
            source->sprite.scmd = get_combat_scmd(source->sprite.scmd, next_animation_head->ca.action);
            port_update_entity(source, 0, 0);
            break;
        case CA_RED_DAMAGE:
            source->sprite.scmd = get_scmd(source->sprite.scmd, 0, 0);
            port_update_entity(source, 0, 0);
            port_combat_action(&(next_animation_head->ca));
            break;
        default:
            error("unknown action %d!\n", next_animation_head->ca.action);
            break;
    }

    if (last) { free(last); }
    last = next_animation_head;
    next_animation_head = next_animation_head->next;
    if (!next_animation_head) { next_animation_tail = NULL; }

    return 1;
}

