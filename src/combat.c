#include "combat.h"
#include "ds-player.h"
#include "region.h"
#include "dsl.h"
#include "port.h"
#include "rules.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//static combat_action_list_t action_list;
static int current_player = 0;
static int wait_on_player = 0;
static combat_action_t monster_actions[MAX_COMBAT_ACTIONS]; // list of actions for a monster's turn.
static int monster_step = -1; // keep track of what step of the action the monster is on.

typedef struct combat_entry_s {
    int initiative;
    int sub_roll; // used to break ties.
    combat_action_t current_action;
    entity_t *entity;
    struct combat_entry_s *next;
} combat_entry_t;

// For BFS
typedef struct action_node_s {
    int num_moves;
    uint16_t x, y;
    combat_action_t actions[MAX_COMBAT_ACTIONS];
    struct action_node_s *next;
} action_node_t;

static int in_combat = 0;
static int need_to_cleanup = 0; // combat is over, but things aren't clean yet...
static combat_entry_t *combat_order = NULL;
static combat_entry_t *current_turn = NULL;

static int is_combat_over(region_t *reg);

const enum combat_turn_t combat_player_turn() {
    if (!in_combat) { return NO_COMBAT; }
    if (current_player >= 0) { return PLAYER1_TURN + current_player; }

    return NONPLAYER_TURN;
}

entity_t* combat_get_current(combat_region_t *cr) {
    if (!current_turn) { return NULL; }
    return current_turn->entity;
}

void combat_init(combat_region_t *cr) {
    memset(cr, 0x0, sizeof(combat_region_t));
    cr->combatants = entity_list_create();
}

void combat_free(combat_region_t *cr) {
    //if (cr->combatants) {
        //entity_list_free(cr->combatants);
        //free(cr->combatants);
        //cr->combatants = NULL;
    //}
}

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
};

static scmd_t* get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff) {
    if (xdiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_LEFT); }
    if (xdiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT); }
    if (ydiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_UP); }
    if (ydiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_DOWN); }

    // xdiff and ydiff == 0.
    if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_LEFT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_LEFT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_RIGHT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_UP);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    }

    return current_scmd;
}

static int32_t ticks_per_game_round = 20;// For outside combat.

//TODO: Ignores walls, but that might be okay right now.
static int calc_distance_to_player(entity_t *entity) {
    int min = 9999999;
    int max;

    for (int i = 0; i < MAX_PCS; i++) {
        if (player_exists(i)) {
            entity_t *dude = player_get_entity(i);
            int xdiff = (entity->mapx - dude->mapx);
            int ydiff = (entity->mapy - dude->mapy);
            if (xdiff < 0) { xdiff *= -1;}
            if (ydiff < 0) { ydiff *= -1;}
            max = (xdiff > ydiff) ? xdiff : ydiff;
            min = (min < max) ? min : max;
        }
    }

    return min;
}

static int enemies_alive(combat_region_t *cr) {
    dude_t *enemy = NULL;
    entity_list_for_each(cr->combatants, enemy) {
        if (enemy->allegiance > 1) { // looks like > 1 is hostile.
            return 1; // need to check status as well.
        }
    }
    return 0;
}

static int initiative_is_less(combat_entry_t *n0, combat_entry_t *n1) {
    if (n0->initiative != n1->initiative) { return n0->initiative < n1->initiative; }
    return n0->sub_roll < n1->sub_roll;
}

// Add to the actual combat list.
static void add_to_combat(entity_t *entity) {
    combat_entry_t *node = malloc(sizeof(combat_entry_t));
    node->entity = entity;
    node->next = combat_order; // start up front.
    node->initiative = dnd2e_roll_initiative(entity);
    node->sub_roll = dnd2e_roll_sub_roll();
    node->current_action.action = CA_NONE;// Means they need to take their turn.
    //printf("rolled: %d (%d)\n", node->initiative, node->sub_roll);

    // if the node is first.
    if (combat_order == NULL || initiative_is_less(node, combat_order)) {
        combat_order = node;
        return;
    }

    // Not the first, so shift now.
    combat_entry_t *prev = combat_order;
    node->next = prev->next;
    prev->next = node;
    while(node->next && !initiative_is_less(node, node->next)) {
        prev->next = node->next;
        node->next = node->next->next;
        prev->next->next = node;
        prev = prev->next;
    }
}

static void enter_combat_mode(region_t *reg) {
    debug("Enter combat mode.\n");
    entity_t *combatant = NULL;

    combat_region_t *cr = &(reg->cr);
    if (!enemies_alive(cr)) {
        error("Called to enter combat, but no enemies? ignoring...\n");
        return;
    }

    in_combat = 1;

    // Freeze all combats.
    entity_list_for_each(cr->combatants, combatant) {
        combatant->sprite.scmd = get_scmd(combatant->sprite.scmd, 0, 0);
        port_update_entity(combatant, 0, 0);
    }

    // Right now players are not part of combat, so add them!
    for (int i = 0; i < MAX_PCS; i++) {
        if (ds_player_exists(i)) {
            combat_add(&(reg->cr), player_get_entity(i));
        }
    }

    // Now lets make an initiative list
    entity_list_for_each(cr->combatants, combatant) {
        add_to_combat(combatant);
    }

    // print to debug:
    combat_entry_t *rover = combat_order;
    while(rover) {
        //printf("initiative: %d (%d)\n", rover->initiative, rover->sub_roll);
        rover = rover->next;
    }

    port_enter_combat();
}

static int which_player(combat_entry_t *node) {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = player_get_entity(i);
        if (player == node->entity) { // Warning: pointer test, be teh same, not just a clone.
            return i;
        }
    }

    return -1;
}

static void next_turn() {
    if (!current_turn) { return; }

    current_turn = current_turn->next;
    wait_on_player = 0;
}

void combat_player_action(const combat_action_t action) {
    if (!wait_on_player || !current_turn) { return; }

    current_turn->current_action = action;

    switch(action.action) {
        case CA_GUARD: next_turn(); break;
        default:
            error("Unknown action %d\n", action.action);
    }
}

static void queue_add(action_node_t **head, action_node_t **tail, action_node_t *current, const enum combat_action_e action) {
    if (current->num_moves > 11) { return; }
    action_node_t *new = malloc(sizeof(action_node_t));
    memcpy(new, current, sizeof(action_node_t));
    new->actions[new->num_moves].action = action;
    new->next = NULL;
    new->num_moves++;

    switch (action) {
        case CA_WALK_LEFT: new->x -= 1; break;
        case CA_WALK_RIGHT: new->x += 1; break;
        case CA_WALK_UP: new->y -= 1; break;
        case CA_WALK_DOWN: new->y += 1; break;
        case CA_WALK_UPLEFT: new->x -= 1; new->y -= 1; break;
        case CA_WALK_UPRIGHT: new->x += 1; new->y -= 1; break;
        case CA_WALK_DOWNLEFT: new->x -= 1; new->y += 1; break;
        case CA_WALK_DOWNRIGHT: new->x += 1; new->y += 1; break;
        default: 
            break; // Do nothing right now...
    }

    if (!*tail) {
        *head = *tail = new;
    } else {
        (*tail)->next = new;
        *tail = new;
    }
}

static entity_t* player_exists_in_pos(region_t *reg, const uint16_t x, const uint16_t y) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (!ds_player_exists(i)) { continue; }
        entity_t *player = player_get_entity(i);
        //printf("(%d, %d) -> player(%d, %d)\n", x, y, player->mapx, player->mapy);
        if (player->mapx == x && player->mapy == y) { return player; }
    }

    return NULL;
}

static entity_t* player_to_attack(region_t *reg, action_node_t *node) {
    entity_t *player = NULL;
    if (!node) { return NULL; }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y + 0)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y + 0)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y + 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y + 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y - 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y - 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 0, node->y - 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 0, node->y + 1)) != NULL) {
        if (player->combat_status != COMBAT_STATUS_DYING) {
            return player;
        }
    }

    return NULL;
}

static void generate_monster_actions(region_t *reg) {
    // Start of AI, lets just go to the closest PC and attack.
    static uint8_t visit_flags[MAP_ROWS][MAP_COLUMNS];
    entity_t *player;
    memset(monster_actions, 0x0, sizeof(combat_action_t) * MAX_COMBAT_ACTIONS);
    memset(visit_flags, 0x0, sizeof(uint8_t) * MAP_ROWS * MAP_COLUMNS);
    action_node_t *rover = malloc(sizeof(action_node_t));
    memset(rover, 0x0, sizeof(action_node_t));
    action_node_t *queue_head, *queue_tail;
    queue_head = queue_tail = rover;
    rover->num_moves = 0;
    rover->x = current_turn->entity->mapx;
    rover->y = current_turn->entity->mapy;
    //printf("player = %d, name = %s\n", current_player, current_turn->combat->name);

    // BFS
    while (queue_head) {
        rover = queue_head;
        queue_head = queue_head->next;
        if (!queue_head) { queue_tail = NULL; }
        if (visit_flags[rover->x][rover->y]) { free(rover); continue; }
        visit_flags[rover->x][rover->y] = 1;// Mark as visited.

        if ((player = player_to_attack(reg, rover)) != NULL) {
            printf("CAN ATTACK!\n");
            for(int i = 0; i < rover->num_moves; i++) {
                monster_actions[i] = rover->actions[i];
                printf("move %d: %d\n", i, rover->actions[i].action);
            }
            monster_actions[rover->num_moves].action = CA_MELEE;
            monster_actions[rover->num_moves].target = player;
            rover->num_moves++;
            monster_actions[rover->num_moves].target = NULL;
            monster_actions[rover->num_moves].action = CA_END;
            //Free up the queue.
            free(rover);
            while(queue_head) {
                rover = queue_head;
                queue_head = queue_head->next;
                free(rover);
            }
            queue_tail = NULL;
            return;
        }

        queue_add(&queue_head, &queue_tail, rover, CA_WALK_LEFT);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_RIGHT);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_UP);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_DOWN);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_UPLEFT);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_UPRIGHT);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_DOWNLEFT);
        queue_add(&queue_head, &queue_tail, rover, CA_WALK_DOWNRIGHT);
        free(rover);
    }

    printf("NEED TO move and guard!\n");
}

static void set_current_scmd(region_t *reg) {
    combat_action_t *action = monster_actions + monster_step;
    int16_t xdiff = 0, ydiff = 0;

    switch (action->action) {
        case CA_WALK_LEFT: xdiff = -1; ydiff = 0; break;
        case CA_WALK_RIGHT: xdiff = 1; ydiff = 0; break;
        case CA_WALK_UP: xdiff = 0; ydiff = -1; break;
        case CA_WALK_DOWN: xdiff = 0; ydiff = 1; break;
        case CA_WALK_UPLEFT: xdiff = -1; ydiff = -1; break;
        case CA_WALK_UPRIGHT: xdiff = 1; ydiff = -1; break;
        case CA_WALK_DOWNLEFT: xdiff = -1; ydiff = 1; break;
        case CA_WALK_DOWNRIGHT: xdiff = 1; ydiff = 1; break;
        default: break;
    }

    //printf("(%d, %d) applying xdiff = %d, ydiff = %d\n", current_turn->entity->mapx, current_turn->entity->mapy, xdiff, ydiff);
    current_turn->entity->sprite.scmd = get_scmd(current_turn->entity->sprite.scmd, xdiff, ydiff);
    port_update_entity(current_turn->entity, xdiff, ydiff);
}

static void end_turn() {
    monster_step = -1;
    current_turn = current_turn->next;
    if (!current_turn) { current_turn = combat_order; }
}

static void check_and_perform_attack(region_t *reg) {
    combat_action_t *action = monster_actions + monster_step;
    switch (action->action) {
        case CA_MELEE:
            // DS Engine: Attacks usually based on weapons. If none, then bare hand.
            //            Monster's base weapon are considered plus for
            //            level 0-4: regular
            //            level 5-6: +1
            //            level 6-7: +2
            //            level 8-9: +3
            //            level 10+: +4
            // Monster data should not be in the entity.
            // For Now, 1d6, always hits. Need to add thac0 calculation.
            action->target->stats.hp -= 1 + (rand() % 6);
            //printf("hp after: %d\n", action->target->stats.hp);
            if (action->target->stats.hp <= 0) {
                //printf("DYING\n");
                action->target->combat_status = COMBAT_STATUS_DYING;
            }
            end_turn();
            break;
        default:
            break;
    }
}

static void do_combat_rounds(region_t *reg) {
    if (wait_on_player) { return; }
    //Need to start combat rounds.
    if (!current_turn) { current_turn = combat_order; }

    current_player = which_player(current_turn);
    printf("player = %d, name = %s\n", current_player, current_turn->entity->name);
    if (current_player >= 0) {
        wait_on_player = 1;
        return; // Need to wait on player input.
    }

    // Monster time.
    if (monster_step < 0) {
        generate_monster_actions(reg);
        monster_step = 0;
    }

    set_current_scmd(reg);

    check_and_perform_attack(reg);
    if (monster_step < 0) { return; } // turn ended
    monster_step++;
}

static int is_combat_over(region_t *reg) {
    combat_entry_t *rover = combat_order;
    uint8_t forces[10]; // represent each opposing force.
    uint8_t num_types = 0;
    memset(forces, 0x0, sizeof(forces));

    while (rover) {
        //printf("%s: alliegiance %d, hp: %d\n", rover->entity->name, rover->entity->allegiance, rover->entity->stats.hp);
        if (rover->entity->allegiance < 10
                && rover->entity->combat_status != COMBAT_STATUS_DYING) {
            forces[rover->entity->allegiance]++;
        }
        rover = rover->next;
    }

    for (int i = 0; i < 10; i++) {
        num_types += (forces[i] > 0) ? 1 : 0;
    }

    return num_types < 2;
}

void combat_update(region_t *reg) {
    if (reg == NULL) { return; }
    combat_region_t *cr = &(reg->cr);
    if (cr == NULL) { return; }
    int xdiff, ydiff;
    int posx, posy;

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    if (need_to_cleanup) { return; }

    if (in_combat) {
        in_combat = !is_combat_over(reg); // Just to check
        if (in_combat) {
            entity_t *combatant = NULL;
            entity_list_for_each(reg->cr.combatants, combatant) {
                port_update_entity(combatant, 0, 0);
            }
            do_combat_rounds(reg);
            return;
        }
        need_to_cleanup = 1;
        return;
        // We were in combat but now it is over. Need to clean up.
    }

    dude_t *bad_dude = NULL;
    entity_list_for_each(reg->cr.combatants, bad_dude) {
        if (bad_dude->abilities.hunt) {
            xdiff = player_get_entity(ds_player_get_active())->mapx - bad_dude->mapx;
            ydiff = player_get_entity(ds_player_get_active())->mapy - bad_dude->mapy;
            xdiff = (xdiff < 0) ? -1 : (xdiff > 0) ? 1 : 0;
            ydiff = (ydiff < 0) ? -1 : (ydiff > 0) ? 1 : 0;
            posx = bad_dude->mapx;
            posy = bad_dude->mapy;

            if (region_location_blocked(reg, posx + xdiff, posy + ydiff)
                    ){
                if (!region_location_blocked(reg, posx, posy + ydiff)) {
                    xdiff = 0;
                } else if (!region_location_blocked(reg, posx + xdiff, posy)) {
                    ydiff = 0;
                } else {
                    xdiff = ydiff = 0;
                }
            }
            bad_dude->sprite.scmd = get_scmd(bad_dude->sprite.scmd, xdiff, ydiff);
            if (calc_distance_to_player(bad_dude) < 5) {
                enter_combat_mode(reg);
                return;
            }
            port_update_entity(bad_dude, xdiff, ydiff);
        } else {
            if (bad_dude) {
                port_update_entity(bad_dude, 0, 0);
            }
        }
    }
}

scmd_t* combat_get_scmd(const enum combat_scmd_t type) {
    return combat_types[type];
}

ds1_combat_t* combat_get_combat( combat_region_t* cr, const uint32_t combat_id) {
    if (!cr || combat_id < 0 || combat_id >= MAX_COMBAT_OBJS) { return NULL; }
    return cr->combats + combat_id;
}

void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id) {
    if (!cr || combat_id < 0 || combat_id >= MAX_COMBAT_OBJS) { return; }
    cr->hunt[combat_id] = 1;
}

// This does not force into combat mode, simply add a combat to the current region.
uint32_t combat_add(combat_region_t *rc, entity_t *entity) {
    if (!rc || !entity) { return 0; }

    entity_list_add(rc->combatants, entity);
    return 1;
}
