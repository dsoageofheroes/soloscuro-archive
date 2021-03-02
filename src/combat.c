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
//static int monster_move = -1;

typedef struct combat_entry_s {
    int initiative;
    int sub_roll; // used to break ties.
    combat_action_t current_action;
    region_object_t *robj;
    ds1_combat_t *combat;
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

static int is_combat_over(ds_region_t *reg);

const enum combat_turn_t combat_player_turn() {
    if (!in_combat) { return NO_COMBAT; }
    if (current_player >= 0) { return PLAYER1_TURN + current_player; }

    return NONPLAYER_TURN;
}

void combat_init(combat_region_t *cr) {
    memset(cr, 0x0, sizeof(combat_region_t));
}

void combat_free(combat_region_t *rc) {
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

static int location_is_blocked(dsl_region_t *reg, const uint32_t x, const uint32_t y) {
    return ds_region_location_blocked(reg, x, y)
        || dsl_region_is_block(reg, y, x + 1)
        || dsl_region_has_object(reg, x, y);
}

//TODO: Ignores walls, but that might be okay right now.
static int calc_distance_to_player(region_object_t *robj) {
    int xdiff = (robj->mapx - ds_player_get_pos(ds_player_get_active())->xpos);
    int ydiff = (robj->mapy - ds_player_get_pos(ds_player_get_active())->ypos);

    if (xdiff < 0) { xdiff *= -1;}
    if (ydiff < 0) { ydiff *= -1;}

    return xdiff > ydiff ? xdiff : ydiff;
}

static int enemies_alive(combat_region_t *cr) {
    for (int i = 0; i < cr->pos + 1; i++) {
        if (cr->combats[i].allegiance > 1) { // looks like > 1 is hostile.
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
static void add_to_combat(region_object_t *robj, ds1_combat_t *combat) {
    combat_entry_t *node = malloc(sizeof(combat_entry_t));
    node->robj = robj;
    node->combat = combat;
    node->next = combat_order; // start up front.
    node->initiative = dnd2e_roll_initiative(&(combat->stats));
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

static void enter_combat_mode(dsl_region_t *reg) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    printf("Enter combat mode.\n");
    combat_region_t *cr = &(reg->cr);
    if (!enemies_alive(cr)) {
        error("Called to enter combat, but no enemies? ignoring...\n");
        return;
    }

    in_combat = 1;

    // Freeze all combats.
    for (int i = 0; i < cr->pos + 1; i++) {
        cr->robjs[i]->scmd = get_scmd(cr->robjs[i]->scmd, 0, 0);
        port_update_obj(cr->robjs[i], 0, 0);
    }

    port_enter_combat();

    // Right now players are not part of combat, so add them!
    for (int i = 0; i < MAX_PCS; i++) {
        if (ds_player_exists(i)) {
            combat_add(&(dsl_region_get_current()->cr), ds_player_get_robj(i), ds_player_get_combat(i));
            port_add_obj(ds_player_get_robj(i), pal);
        }
    }

    // Now lets make an initiative list
    for (int i = 0; i < cr->pos + 1; i++) {
        add_to_combat(cr->robjs[i], cr->combats + i);
    }

    // print to debug:
    combat_entry_t *rover = combat_order;
    while(rover) {
        printf("initiative: %d (%d)\n", rover->initiative, rover->sub_roll);
        rover = rover->next;
    }
}

static int which_player(combat_entry_t *node) {
    for (int i = 0; i < MAX_PCS; i++) {
        region_object_t *player = ds_player_get_robj(i);
        if (player == node->robj) { // Warning: pointer test, but be same robj, not just a clone.
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

static int player_exists_in_pos(ds_region_t *reg, const uint16_t x, const uint16_t y) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (!ds_player_exists(i)) { continue; }
        region_object_t *player = ds_player_get_robj(i);
        //printf("(%d, %d) -> player(%d, %d)\n", x, y, player->mapx, player->mapy);
        if (player->mapx == x && player->mapy == y) { return i; }
    }

    return -1;
}

// This needs to be fixed becuase of our combat regions are made...
static ds1_combat_t* find_combat(ds1_combat_t *combat) {
    combat_entry_t *rover = combat_order;
    while(rover) {
        if (!strcmp(rover->combat->name, combat->name)) { return rover->combat; }
        rover = rover->next;
    }
    
    return NULL;
}

static int player_to_attack(ds_region_t *reg, action_node_t *node) {
    int player = -1;
    if (!node) { return -1; }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y + 0)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y + 0)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y + 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y + 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 1, node->y - 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x - 1, node->y - 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 0, node->y - 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }
    if ((player = player_exists_in_pos(reg, node->x + 0, node->y + 1)) != -1) {
        if (find_combat(ds_player_get_combat(player))->status != COMBAT_STATUS_DYING) {
            return player;
        }
    }

    return -1;
}


static void generate_monster_actions(ds_region_t *reg) {
    // Start of AI, lets just go to the closest PC and attack.
    static uint8_t visit_flags[MAP_ROWS][MAP_COLUMNS];
    int player;
    memset(monster_actions, 0x0, sizeof(combat_action_t) * MAX_COMBAT_ACTIONS);
    memset(visit_flags, 0x0, sizeof(uint8_t) * MAP_ROWS * MAP_COLUMNS);
    action_node_t *rover = malloc(sizeof(action_node_t));
    memset(rover, 0x0, sizeof(action_node_t));
    action_node_t *queue_head, *queue_tail;
    queue_head = queue_tail = rover;
    rover->num_moves = 0;
    rover->x = current_turn->robj->mapx;
    rover->y = current_turn->robj->mapy;
    //printf("player = %d, name = %s\n", current_player, current_turn->combat->name);

    // BFS
    while (queue_head) {
        rover = queue_head;
        queue_head = queue_head->next;
        if (!queue_head) { queue_tail = NULL; }
        if (visit_flags[rover->x][rover->y]) { free(rover); continue; }
        visit_flags[rover->x][rover->y] = 1;// Mark as visited.

        if ((player = player_to_attack(reg, rover)) != -1) {
            printf("CAN ATTACK!\n");
            for(int i = 0; i < rover->num_moves; i++) {
                monster_actions[i] = rover->actions[i];
                printf("move %d: %d\n", i, rover->actions[i].action);
            }
            monster_actions[rover->num_moves].action = CA_MELEE;
            //monster_actions[rover->num_moves].target_combat = ds_player_get_combat(player);
            monster_actions[rover->num_moves].target_combat = find_combat(ds_player_get_combat(player));
            monster_actions[rover->num_moves].target_robj = ds_player_get_robj(player);
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
    //typedef struct action_node_s {
        //combat_action_t action;
        //struct action_node_s *next;
    //} action_node_t;
}

static void set_current_scmd(ds_region_t *reg) {
    combat_action_t *action = monster_actions + monster_step;
    uint16_t xdiff = 0, ydiff = 0;

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

    printf("(%d, %d) applying xdiff = %d, ydiff = %d\n", current_turn->robj->mapx, current_turn->robj->mapy, xdiff, ydiff);
    current_turn->robj->scmd = get_scmd(current_turn->robj->scmd, xdiff, ydiff);
    port_update_obj(current_turn->robj, xdiff, ydiff);
//static combat_action_t monster_actions[MAX_COMBAT_ACTIONS]; // list of actions for a monster's turn.
}

static void end_turn() {
    monster_step = -1;
    current_turn = current_turn->next;
    if (!current_turn) { current_turn = combat_order; }
}

static void check_and_perform_attack(ds_region_t *reg) {
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
            // It appears that I need to load the monster data for charrecs to calc numdice, numattacks, adds, etc...
            // For Now, 1d6, always hits. Need to add thaco calculation.
            printf("hp before: %d\n", action->target_combat->hp);
            action->target_combat->hp -= 1 + (rand() % 6);
            printf("hp after: %d\n", action->target_combat->hp);
            if (action->target_combat->hp <= 0) {
                printf("DYING\n");
                action->target_combat->status = COMBAT_STATUS_DYING;
            }
            end_turn();
            break;
        default:
            break;
    }
}

static void do_combat_rounds(ds_region_t *reg) {
    if (wait_on_player) { return; }
    //Need to start combat rounds.
    if (!current_turn) { current_turn = combat_order; }

    current_player = which_player(current_turn);
    printf("player = %d, name = %s\n", current_player, current_turn->combat->name);
    if (current_player >= 0) {
        wait_on_player = 1;
        return; // Need to wait on player input.
    }

    // Monster time.
    if (monster_step < 0) {
        printf("BLAH!\n");
        generate_monster_actions(reg);
        monster_step = 0;
    }

    set_current_scmd(reg);

    check_and_perform_attack(reg);
    if (monster_step < 0) { return; } // turn ended
    monster_step++;
}

static int is_combat_over(ds_region_t *reg) {
    combat_entry_t *rover = combat_order;
    uint8_t forces[10]; // represent each opposing force.
    uint8_t num_types = 0;
    memset(forces, 0x0, sizeof(forces));

    while (rover) {
        //printf("%s: alliegiance %d, hp: %d\n", rover->combat->name, rover->combat->allegiance, rover->combat->hp);

        if (rover->combat->allegiance < 10
                && rover->combat->status != COMBAT_STATUS_DYING) {
            forces[rover->combat->allegiance]++;
        }
        rover = rover->next;
    }

    for (int i = 0; i < 10; i++) {
        num_types += (forces[i] > 0) ? 1 : 0;
    }

    return num_types < 2;
}

void combat_update(ds_region_t *reg) {
    if (reg == NULL) { return; }
    combat_region_t *cr = &(reg->cr);
    player_pos_t* pc = ds_player_get_pos(ds_player_get_active());
    if (cr == NULL || pc == NULL) { return; }
    int xdiff, ydiff;
    int posx, posy;

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    if (need_to_cleanup) { return; }

    if (in_combat) {
        in_combat = !is_combat_over(reg); // Just to check
        if (in_combat) {
            for (int i = 0; i < cr->pos + 1; i++) {
                port_update_obj(cr->robjs[i], 0, 0);
            }
            do_combat_rounds(reg);
            return;
        }
        need_to_cleanup = 1;
        printf("HERE!!!!!!!!!!!!!!!!!\n");
        return;
        // We were in combat but now it is over. Need to clean up.
    }

    for (int i = 0; i < cr->pos + 1; i++) {
        if (cr->hunt[i]) {
            xdiff = pc->xpos - cr->robjs[i]->mapx;
            ydiff = pc->ypos - cr->robjs[i]->mapy;
            xdiff = (xdiff < 0) ? -1 : (xdiff > 0) ? 1 : 0;
            ydiff = (ydiff < 0) ? -1 : (ydiff > 0) ? 1 : 0;
            posx = cr->robjs[i]->mapx;
            posy = cr->robjs[i]->mapy;

            //printf("pos = (%d, %d)\n", posx, posy);

            if (location_is_blocked(reg, posx + xdiff, posy + ydiff)
                    ){
                if (!location_is_blocked(reg, posx, posy + ydiff)) {
                    xdiff = 0;
                } else if (!location_is_blocked(reg, posx + xdiff, posy)) {
                    ydiff = 0;
                } else {
                    xdiff = ydiff = 0;
                }
            }
            cr->robjs[i]->scmd = get_scmd(cr->robjs[i]->scmd, xdiff, ydiff);
            if (calc_distance_to_player(cr->robjs[i]) < 5) {
                enter_combat_mode(reg);
                return;
            }
            port_update_obj(cr->robjs[i], xdiff, ydiff);
        } else {
            if (cr->robjs[i]) {
                port_update_obj(cr->robjs[i], 0, 0);
            }
        }
    }
}

scmd_t* combat_get_scmd(const enum combat_scmd_t type) {
    return combat_types[type];
}

static uint32_t find_next_pos(combat_region_t *rc) {
    size_t searched = 0;
    while (rc->combats[rc->pos].hp) {
        rc->pos = (rc->pos + 1) % MAX_COMBAT_OBJS;
        searched++;
        if (searched > MAX_COMBAT_OBJS) { return COMBAT_ERROR; }
    }
    return rc->pos;
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
uint32_t combat_add(combat_region_t *rc, region_object_t *robj, ds1_combat_t *combat) {
    size_t i;
    if (!rc || !robj || !combat) { return COMBAT_ERROR; }
    if (find_next_pos(rc) == COMBAT_ERROR) { return COMBAT_ERROR; }

    memcpy(rc->combats + rc->pos, combat, sizeof(ds1_combat_t) - 16); // Don't copy the name over!
    for (i = 0; i < 17 && combat->name[i]; i++) {
        rc->combats[rc->pos].name[i] = combat->name[i];
    }
    // Force the name to be null-terminated.
    i = i >= 17 ? 16 : i;
    combat->name[i] = '\0';
    //printf("Added %s\n", combat->name);
    rc->robjs[rc->pos] = robj;
    return rc->pos;
}
