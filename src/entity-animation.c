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
    {.bmp_idx = 7, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 3, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_LAST, .xoffset = -1, .yoffset = 0, 0, 0, 0},
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
            return combat_get_scmd(COMBAT_SCMD_STAND_LEFT);
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_UPRIGHT:
        case EA_WALK_RIGHT:
            return combat_get_scmd(COMBAT_SCMD_STAND_RIGHT);
        case EA_WALK_UP:
            return combat_get_scmd(COMBAT_SCMD_STAND_UP);
        case EA_WALK_DOWN:
            return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
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
        port_play_sound_effect(target->attack_sound + 2);
        return;
    }
}

static void play_melee_sound(entity_t *source) {
    if (source->attack_sound) {
        port_play_sound_effect(source->attack_sound);
        return;
    }

    // sound 69: is Thri-keen melee sound
    if (source->race  == RACE_THRIKREEN) {
        port_play_sound_effect(69);
        return;
    }

    // sound 7: general melee sound.
    port_play_sound_effect(7);
}

static void play_damage_sound(entity_t *target) {
    if (!target) { return; }

    if (target->attack_sound) {
        port_play_sound_effect(target->attack_sound + 1);
    }

    // sound 67: is PC taking damage
    port_play_sound_effect(67);
}

// sound 63: is PC doing range attack
extern int entity_animation_execute(region_t *reg) {
    entity_animation_list_t list;
    list.head = next_animation_head;
    int ret = entity_animation_list_execute(&list, reg);
    next_animation_head = list.head;

    return ret;
}

entity_animation_list_t* entity_animation_list_create() {
    return calloc(1, sizeof(entity_animation_list_t));
}

void entity_animation_list_free(entity_animation_list_t *list) {
    if (!list) { return; }

    //TODO:
    if (list->head) {
        printf("NEED TO FREE!!!!!\n");
    }
}

void entity_animation_list_add(entity_animation_list_t *list, enum entity_action_e action,
        entity_t *source, entity_t *target, power_t *power, const int32_t amt) {
    if (!list) { return; }
    entity_animation_node_t *toadd = malloc(sizeof(entity_animation_node_t));
    entity_animation_node_t *rover = list->head;

    toadd->ca.action = action;
    toadd->ca.source = source;
    toadd->ca.target = target;
    toadd->ca.power  = power;
    toadd->ca.amt = amt;
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
            source->sprite.scmd = get_entity_scmd(source->sprite.scmd, list->head->ca.action);
            port_update_entity(source, 0, 0);
            break;
        case EA_RED_DAMAGE:
            play_damage_sound(target);
            source->sprite.scmd = get_scmd(source->sprite.scmd, 0, 0);
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
        default:
            error("unknown action %d!\n", list->head->ca.action);
            break;
    }

    last = list->head;
    list->head = list->head->next;
    if (last) { free(last); last = NULL; }

    return 1;
}
