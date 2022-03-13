#include "arbiter.h"
#include "combat.h"
#include "entity-animation.h"
#include "player.h"
#include "region.h"
#include "region-manager.h"
#include "gpl.h"
#include "port.h"
#include "rules.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int current_player = 0;
static int wait_on_player = 0;
static entity_action_t monster_actions[MAX_COMBAT_ACTIONS]; // list of actions for a monster's turn.
static int monster_step = -1; // keep track of what step of the action the monster is on.
static enum entity_action_e player_action;

typedef struct combat_entry_s {
    int initiative;
    int sub_roll; // used to break ties.
    int round; // which round we are in.
    int melee_actions, range_actions, spell_actions, psionic_actions;
    entity_action_t current_action;
    entity_t *entity;
    struct combat_entry_s *next;
} combat_entry_t;

// For BFS
typedef struct action_node_s {
    int num_moves;
    uint16_t x, y;
    entity_action_t actions[MAX_COMBAT_ACTIONS];
    struct action_node_s *next;
} action_node_t;

static combat_entry_t *combat_order = NULL;
static combat_entry_t *current_turn = NULL;
static combat_entry_t *defeated = NULL;

void sol_combat_free(combat_region_t *cr) {
    entity_list_free(&(cr->combatants));
}

static int32_t ticks_per_game_round = 20;// For outside combat.

static int initiative_is_less(combat_entry_t *n0, combat_entry_t *n1) {
    if (n0->initiative != n1->initiative) { return n0->initiative < n1->initiative; }
    return n0->sub_roll < n1->sub_roll;
}

// Add to the actual combat list.
static void add_to_combat(entity_t *entity) {
    combat_entry_t *node = calloc(1, sizeof(combat_entry_t));
    node->entity = entity;
    node->next = combat_order; // start up front.
    node->initiative = dnd2e_roll_initiative(entity);
    node->sub_roll = dnd2e_roll_sub_roll();
    node->current_action.action = EA_NONE;// Means they need to take their turn.

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

static int which_player(combat_entry_t *node) {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = sol_player_get(i);
        if (player == node->entity) { // Warning: pointer test, be teh same, not just a clone.
            return i;
        }
    }

    return -1;
}

void sol_combat_player_action(const entity_action_t action) {
    if (!wait_on_player || !current_turn) { return; }

    player_action = action.action;
}

static void queue_add(action_node_t **head, action_node_t **tail, action_node_t *current, const enum entity_action_e action) {
    action_node_t *new = malloc(sizeof(action_node_t));
    memcpy(new, current, sizeof(action_node_t));
    new->actions[new->num_moves].action = action;
    new->next = NULL;
    new->num_moves++;

    switch (action) {
        case EA_WALK_LEFT: new->x -= 1; break;
        case EA_WALK_RIGHT: new->x += 1; break;
        case EA_WALK_UP: new->y -= 1; break;
        case EA_WALK_DOWN: new->y += 1; break;
        case EA_WALK_UPLEFT: new->x -= 1; new->y -= 1; break;
        case EA_WALK_UPRIGHT: new->x += 1; new->y -= 1; break;
        case EA_WALK_DOWNLEFT: new->x -= 1; new->y += 1; break;
        case EA_WALK_DOWNRIGHT: new->x += 1; new->y += 1; break;
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

static entity_t* player_exists_in_pos(sol_region_t *reg, const uint16_t x, const uint16_t y) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (!sol_player_exists(i)) { continue; }
        entity_t *player = sol_player_get(i);
        //printf("(%d, %d) -> player(%d, %d)\n", x, y, player->mapx, player->mapy);
        if (player->mapx == x && player->mapy == y) { return player; }
    }

    return NULL;
}

static entity_t* player_to_attack(sol_region_t *reg, action_node_t *node) {
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

static entity_t* entity_at_location(const sol_region_t *reg, entity_t *entity, const int32_t x, const int32_t y) {
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude) {
        //printf("(%s: %d, %d) ?= (%s: %d, %d)\n", dude->name, dude->mapx, dude->mapy, entity->name, entity->mapx, entity->mapy);
        if (dude != entity && dude->mapx == x && dude->mapy == y) {
            return dude;
        }
    }

    return NULL;
}

static void generate_monster_move_attack_closest(sol_region_t *reg, entity_t *monster) {
    // Start of AI, lets just go to the closest PC and attack.
    static uint8_t visit_flags[MAP_ROWS][MAP_COLUMNS];
    entity_t *player;
    memset(monster_actions, 0x0, sizeof(entity_action_t) * MAX_COMBAT_ACTIONS);
    memset(visit_flags, 0x0, sizeof(uint8_t) * MAP_ROWS * MAP_COLUMNS);
    action_node_t *rover = malloc(sizeof(action_node_t));
    memset(rover, 0x0, sizeof(action_node_t));
    action_node_t *queue_head, *queue_tail;
    queue_head = queue_tail = rover;
    rover->num_moves = 0;
    rover->x = monster->mapx;
    rover->y = monster->mapy;
    //printf("player = %d, name = %s\n", current_player, current_turn->combat->name);

    // BFS
    while (queue_head) {
        rover = queue_head;
        queue_head = queue_head->next;
        if (!queue_head) { queue_tail = NULL; }
        if (rover->x >= MAP_ROWS || rover->y >= MAP_COLUMNS) {
            free(rover);
            continue;
        }
        if (visit_flags[rover->x][rover->y]) { free(rover); continue; }
        if (entity_at_location(reg, monster, rover->x, rover->y) != NULL) { free(rover); continue; }
        visit_flags[rover->x][rover->y] = 1;// Mark as visited.

        if ((player = player_to_attack(reg, rover)) != NULL) {
            for(int i = 0; i < rover->num_moves; i++) {
                monster_actions[i] = rover->actions[i];
                //printf("move %d: %d\n", i, rover->actions[i].action);
            }
            monster_actions[rover->num_moves].action = EA_MELEE;
            monster_actions[rover->num_moves].target = player;
            monster_actions[rover->num_moves].source = monster;
            rover->num_moves++;
            monster_actions[rover->num_moves].target = NULL;
            monster_actions[rover->num_moves].action = EA_END;
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

        if (rover->num_moves < (MAX_COMBAT_ACTIONS - 1)) {
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_LEFT);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_RIGHT);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_UP);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_DOWN);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_UPLEFT);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_UPRIGHT);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_DOWNLEFT);
            queue_add(&queue_head, &queue_tail, rover, EA_WALK_DOWNRIGHT);
        }
        free(rover);
    }

    printf("NEED TO move and guard!\n");
}

static void apply_action_animation(const enum entity_action_e action) {
    int16_t xdiff = 0, ydiff = 0;

    switch (action) {
        case EA_WALK_LEFT: xdiff = -1; ydiff = 0; break;
        case EA_WALK_RIGHT: xdiff = 1; ydiff = 0; break;
        case EA_WALK_UP: xdiff = 0; ydiff = -1; break;
        case EA_WALK_DOWN: xdiff = 0; ydiff = 1; break;
        case EA_WALK_UPLEFT: xdiff = -1; ydiff = -1; break;
        case EA_WALK_UPRIGHT: xdiff = 1; ydiff = -1; break;
        case EA_WALK_DOWNLEFT: xdiff = -1; ydiff = 1; break;
        case EA_WALK_DOWNRIGHT: xdiff = 1; ydiff = 1; break;
        default: break;
    }

    //printf("(%d, %d) applying xdiff = %d, ydiff = %d\n", current_turn->entity->mapx, current_turn->entity->mapy, xdiff, ydiff);
    current_turn->entity->anim.scmd = entity_animation_get_scmd(current_turn->entity,
            xdiff, ydiff, EA_NONE);
    entity_animation_update(current_turn->entity, xdiff, ydiff);
}

static void end_turn() {
    entity_t *entity = (current_turn) ? current_turn->entity : NULL;
    monster_step = -1;

    if (entity) {
        entity->anim.scmd = entity_animation_get_scmd(entity, 0, 0, EA_NONE);
    }
    current_turn = current_turn->next;
    /*
    combat_entry_t *rover = current_turn;
    printf("Current Order:\n");
    while (rover) {
        printf("%s\n", rover->entity->name);
        rover = rover->next;
    }
    */
    player_action = EA_NONE;
}


static entity_t* entity_in_way(sol_region_t *reg, entity_t *entity, const enum entity_action_e action) {
    int xdiff = 0, ydiff = 0;

    switch(action) {
        case EA_WALK_DOWNLEFT:  xdiff = -1; ydiff = 1; break;
        case EA_WALK_DOWN:      xdiff = 0; ydiff = 1; break;
        case EA_WALK_DOWNRIGHT: xdiff = 1; ydiff = 1; break;
        case EA_WALK_UPLEFT:    xdiff = -1; ydiff = -1; break;
        case EA_WALK_UP:        xdiff = 0; ydiff = -1; break;
        case EA_WALK_UPRIGHT:   xdiff = 1; ydiff = -1; break;
        case EA_WALK_LEFT:      xdiff = -1; ydiff = 0; break;
        case EA_WALK_RIGHT:     xdiff = 1; ydiff = 0; break;
        default:
            return NULL;
    }

    return entity_at_location(reg, entity, entity->mapx + xdiff, entity->mapy + ydiff);
}

static void player_melee(sol_region_t *reg, entity_t* entity, entity_t *enemy) {
/*
    //int amt = 1 + (rand() % 6);
    //int amt = 100; // FTW!
    //combat_animation_add(EA_MELEE, current_turn->entity, NULL, 0);
    //combat_animation_add(EA_RED_DAMAGE, current_turn->entity, enemy, amt);
    int16_t amt = dnd2e_melee_attack(entity, enemy, current_turn->melee_actions++, current_turn->round);
    entity_animation_add(EA_MELEE, entity, NULL, NULL, 0);
    //printf("amt = %d!\n", amt);
    if (amt > 0) {
        entity_animation_add(EA_RED_DAMAGE, entity, enemy, NULL, amt);
    }
    wait_on_player = dnd2e_can_melee_again(entity, current_turn->melee_actions, current_turn->round);
    player_action = EA_NONE;
    //wait_on_player = 0;
*/
}

static void next_round() {
    combat_entry_t *rover = combat_order;

    while (rover) {
        rover->entity->stats.combat.move = rover->entity->stats.base_move;
        rover->melee_actions = rover->range_actions = rover->spell_actions = rover->psionic_actions = 0;
        rover->round++;
        rover = rover->next;
    }

    current_turn = combat_order;
}

static entity_action_t clear = { NULL, NULL, 0, EA_NONE };

static void monster_set_animation(entity_t *monster, entity_action_t *action) {
    int32_t xdiff = 0, ydiff = 0;
    switch(action->action) {
        case EA_WALK_UPLEFT:
            ydiff = -1; xdiff = -1; break;
        case EA_WALK_UPRIGHT:
            ydiff = -1; xdiff = 1; break;
        case EA_WALK_DOWNLEFT:
            ydiff = 1; xdiff = -1; break;
        case EA_WALK_DOWNRIGHT:
            ydiff = 1; xdiff = 1; break;
        case EA_WALK_LEFT:
            ydiff = 0; xdiff = -1; break;
        case EA_WALK_RIGHT:
            ydiff = 0; xdiff = 1; break;
        case EA_WALK_UP:
            ydiff = -1; xdiff = 0; break;
        case EA_WALK_DOWN:
            ydiff = 1; xdiff = 0; break;
    }
    monster->mapx += xdiff;
    monster->mapy += ydiff;
    monster->anim.destx += (xdiff * 32);
    monster->anim.desty += (ydiff * 32);
}

extern void sol_combat_add_attack_animation(sol_region_t *reg, dude_t *dude, entity_t *target,
                                        power_t *power, enum entity_action_e action) {
    sol_attack_t     attack;
    combat_region_t *cr = sol_arbiter_combat_region(reg);

    attack = sol_arbiter_entity_attack(dude, target, cr->round.num, action);

    if (attack.damage == -2) { return; } // not your move!

    if (attack.damage <= -1) {
        dude->stats.combat.move = 0;
        return;
    }

    entity_animation_list_add_effect(&(dude->actions), action, dude, target, power, 30, 30);
    entity_animation_list_add(&(dude->actions), EA_NONE, dude, NULL, NULL, 1);
    entity_animation_list_add(&reg->actions, EA_WAIT_ON_ENTITY, dude, NULL, NULL, 30);

    if (attack.damage > 0) {
        entity_animation_list_add_effect(&reg->actions, EA_RED_DAMAGE, dude, target, NULL, 30, attack.damage);
        entity_animation_list_add_effect(&reg->actions, EA_DAMAGE_APPLY, dude, target, NULL, 0, attack.damage);
    }
}

static void monster_action(sol_region_t *reg, entity_t *monster) {
    entity_action_t *action;

    if (monster_step < 0) {
        generate_monster_move_attack_closest(reg, monster);
        monster_step = 0;
    }

    action = monster_actions + monster_step;
    if (monster_step >= 0 && monster_actions[monster_step].action == EA_MELEE) {
        sol_combat_add_attack_animation(reg, monster, action->target, action->power, action->action);

        if (action->target->stats.hp <= 0) {
            printf("Need to continue attack.\n");
            generate_monster_move_attack_closest(reg, monster);
            return;
        } 

        return;
    }

    if (action->action == EA_END) {
        monster->stats.combat.move = 0;
        return;
    }

    entity_animation_list_add_effect(&(monster->actions), action->action,
        action->source, action->target, action->power, 30, action->damage);
    monster_set_animation(monster, action);
    monster_step++;
    monster->stats.combat.move--;
}

extern void sol_combat_update(sol_region_t *reg) {
    entity_t *combatant;
    combat_region_t *cr;
    if (reg == NULL) { return; }

    cr = sol_arbiter_combat_region(reg);
    if (cr == NULL) { return; }

    if (sol_combat_check_if_over(cr)) {
        sol_combat_clear(cr);
        return;
    }

    combatant = sol_combat_get_current(cr);
    if (!combatant) {
        sol_arbiter_next_round(cr);
        combatant = sol_combat_get_current(cr);
    }
    //printf("combatant = %s: %d\n", combatant->name, combatant->stats.num_attacks);

    if (combatant->stats.combat.move <= 0) {
        sol_combat_next_combatant(cr);
        monster_step = -1;
        return;
    }

    if (sol_player_get_slot(combatant) >= 0) { // It is an active player's turn
        return;
    }

    // still waiting on last animation to complete.
    if (!entity_animation_list_empty(&combatant->actions)
        || !entity_animation_list_empty(&reg->actions)
        ) { return; }

    monster_action(reg, combatant);
}

// This does not force into combat mode, simply add a combat to the current region.
extern uint32_t combat_add(combat_region_t *rc, entity_t *entity) {
    if (!rc || !entity) { return 0; }

    entity_list_add(&(rc->combatants), entity);
    return 1;
}

extern int sol_combat_activate_power(power_t *pw, entity_t *source, entity_t *target, const int32_t x, const int32_t y) {
    if (!pw || !source) { return 0; }
    sol_region_t *reg = sol_region_manager_get_current();
    power_instance_t pi;
    if (!reg) { error ("current region is null!\n"); }

    powers_load(pw);

    memset(&pi, 0x0, sizeof(power_instance_t));
    pi.entity = source;
    if (!pw->actions.pay(&pi, pw->level)) { return 0; }
    // We are now committed to make this work.

    if (!target) { // time to make a fake target for the power.
        target = entity_create_fake(x, y);
    }

    if (pw->cast.scmd) {
        entity_animation_list_add(&(reg->actions), EA_POWER_CAST, source, target, pw, 30);
    }

    if (pw->thrown.scmd) {
        entity_animation_list_add(&(reg->actions), EA_POWER_THROW, source, target, pw, 30);
    }

    if (pw->hit.scmd) {
        entity_animation_list_add(&(reg->actions), EA_POWER_HIT, source, target, pw, 30);
    }

    switch (pw->shape) {
        case TARGET_NONE: break;
        case TARGET_SINGLE: // check AOE for circle (IE: Fireball)
            if (pw->aoe > 1) {
                warn("single target with aoe not implemented.\n");
                break;
            }
        case TARGET_ANY:
        case TARGET_ALLY:
        case TARGET_SELF:
        case TARGET_ENEMY:
            entity_animation_list_add(&(source->actions), EA_POWER_APPLY, source, target, pw, 0);
            break;
        case TARGET_CONE:
        case TARGET_RECTANGLE:
        case TARGET_MULTI:
        default:
            warn("pw->shape %d not implemented\n", pw->shape);
    }
    return 1;
}
