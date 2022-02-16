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

static int is_combat_over(sol_region_t *reg);

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
    //if (reg->flags[x][y]) { return 1; }
    /*
    entity_list_for_each(reg->cr.combatants, dude) {
        //printf("(%s: %d, %d) ?= (%s: %d, %d)\n", dude->name, dude->mapx, dude->mapy, entity->name, entity->mapx, entity->mapy);
        if (dude != entity && dude->mapx == x && dude->mapy == y) {
            return dude;
        }
    }
    */

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
    port_update_entity(current_turn->entity, xdiff, ydiff);
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

static void move_entity(sol_region_t *reg, entity_t *entity, const enum entity_action_e action) {
    entity_t *enemy;

    switch(action) {
        case EA_NONE:
            entity->anim.scmd = entity_animation_get_scmd(entity, 0, 0, EA_NONE);
            port_update_entity(entity, 0, 0);
            ticks_per_game_round = 0;
            break;
        case EA_WALK_DOWNLEFT:
        case EA_WALK_DOWN:
        case EA_WALK_DOWNRIGHT:
        case EA_WALK_UPLEFT:
        case EA_WALK_UP:
        case EA_WALK_UPRIGHT:
        case EA_WALK_LEFT:
        case EA_WALK_RIGHT:
            enemy = entity_in_way(reg, entity, action);
            if (enemy && enemy->allegiance != entity->allegiance) {
                entity->anim.scmd = entity_animation_face_direction(entity->anim.scmd, action);
                player_melee(reg, entity, enemy);
                return;
            }
            if (entity->stats.move) {
                entity->stats.move--;
                apply_action_animation(action);
            }
            ticks_per_game_round = 30;
            break;
        default:
            warn("Unimplemented action: %d\n", action);
            break;
    }
}

static void next_round() {
    combat_entry_t *rover = combat_order;

    while (rover) {
        rover->entity->stats.move = rover->entity->stats.base_move;
        rover->melee_actions = rover->range_actions = rover->spell_actions = rover->psionic_actions = 0;
        rover->round++;
        rover = rover->next;
    }

    current_turn = combat_order;
}

/*
static void do_combat_rounds(sol_region_t *reg) {
    //Need to start combat rounds.
    if (!current_turn) {
        next_round();
    }
    entity_t *monster = current_turn->entity;

    current_player = which_player(current_turn);
    //printf("player = %d, name = %s\n", current_player, current_turn->entity->name);
    if (current_player >= 0) {
        wait_on_player = 1;
        move_entity(reg, sol_player_get(current_player), player_action);
        return; // Need to wait on player input.
    }

    // Monster time.
    if (monster_step < 0) {
        //generate_monster_actions(reg);
        monster_step = 0;
    }

    apply_action_animation(monster_actions[monster_step].action);

    check_and_perform_attack(reg);
    if (monster_step < 0) { return; } // turn ended
    monster->stats.move--;
    monster_step++;

}
*/

static int is_combat_over(sol_region_t *reg) {
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

// decide if the current turn is over and ready next.
static void check_current_turn() {
    if (!entity_animation_has_more()) {
        if (!dnd2e_can_melee_again(current_turn->entity, current_turn->melee_actions, current_turn->round)) {
            end_turn();
            return;
        }
        monster_step = -1;
    }
}

static void do_player_turn(sol_region_t *reg) {
    if (ticks_per_game_round > 0) {
        ticks_per_game_round--;
        return;
    }

    move_entity(reg, sol_player_get(current_player), player_action);
    // Players move quicker.
    if (ticks_per_game_round > 0) { ticks_per_game_round = 20; }
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

static void monster_action(sol_region_t *reg, entity_t *monster) {
    entity_action_t *action;
    sol_attack_t attack;
    combat_region_t *cr = sol_arbiter_combat_region(reg);
    if (monster_step < 0) {
        generate_monster_move_attack_closest(reg, monster);
        monster_step = 0;
    }

    if (monster_step > 0 && monster_actions[monster_step - 1].action == EA_MELEE) {
        action = monster_actions + monster_step - 1;
        entity_animation_list_add(&(monster->actions), EA_NONE, monster, NULL, NULL, 1);
        if (action->target->stats.hp <= 0) {
            printf("Need to continue attack.\n");
            generate_monster_move_attack_closest(reg, monster);
            return;
        }
        attack = sol_arbiter_enemy_melee_attack(action->source, action->target, cr->round.attack_count++, cr->round.num);
        if (attack.damage > -1) {
            if (attack.damage > 0) {
                entity_animation_list_add_effect(&reg->actions, EA_RED_DAMAGE, action->source, action->target, NULL, 30, attack.damage);
                entity_animation_list_add_effect(&reg->actions, EA_DAMAGE_APPLY, action->source, action->target, NULL, 0, attack.damage);
            }
            // ADD melee calculations.
            return;
        }
    }
    action = monster_actions + monster_step;
    if (action->action == EA_END) {
        monster->stats.move = 0;
        return;
    }

    entity_animation_list_add_effect(&(monster->actions), action->action,
        action->source, action->target, action->power, 30, action->damage);
    monster_set_animation(monster, action);
    monster_step++;
    monster->stats.move--;
}

extern void sol_combat_update(sol_region_t *reg) {
    entity_t *combatant;
    if (reg == NULL) { return; }

    combatant = sol_combat_get_current(sol_arbiter_combat_region(reg));
    if (sol_player_get_slot(combatant) >= 0) { // It is an active player's turn
        return;
    }

    // still waiting on last animation to complete.
    if (!entity_animation_list_empty(&combatant->actions)
        || !entity_animation_list_empty(&reg->actions)
        ) { return; }

    monster_action(reg, combatant);

    if (combatant->stats.move <= 0) {
        sol_combat_next_combatant(sol_arbiter_combat_region(reg));
        return;
    }


    /*
    combat_region_t *cr = &(reg->cr);
    if (cr == NULL) { return; }

    if (wait_on_player) {
        if (entity_animation_region_execute(reg)) {
            check_current_turn();
            return;
        }
        do_player_turn(reg);
        return;
    }

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    if (in_combat) {
        if (entity_animation_region_execute(reg)) {
            check_current_turn();
            return;
        }
        //port_combat_action(&clear);

        in_combat = !is_combat_over(reg); // Just to check
        entity_t *combatant = NULL;
        if (in_combat) {
            entity_list_for_each(reg->cr.combatants, combatant) {
                port_update_entity(combatant, 0, 0);
            }
            do_combat_rounds(reg);
            return;
        } else {
            entity_list_for_each(reg->cr.combatants, combatant) {
                entity_list_remove(reg->cr.combatants, entity_list_find(reg->cr.combatants, combatant));
            }
            port_exit_combat();
        }
        return;
        // We were in combat but now it is over. Need to clean up.
    }
    */
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
    if (!reg) { error ("current region is null!\n"); }

    powers_load(pw);

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
