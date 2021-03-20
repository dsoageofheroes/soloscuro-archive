#include "combat.h"
#include "combat-animation.h"
#include "ds-player.h"
#include "region.h"
#include "dsl.h"
#include "port.h"
#include "rules.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int current_player = 0;
static int wait_on_player = 0;
static combat_action_t monster_actions[MAX_COMBAT_ACTIONS]; // list of actions for a monster's turn.
static int monster_step = -1; // keep track of what step of the action the monster is on.
static enum combat_action_e player_action;

typedef struct combat_entry_s {
    int initiative;
    int sub_roll; // used to break ties.
    int melee_actions, range_actions, spell_actions, psionic_actions;
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
static combat_entry_t *combat_order = NULL;
static combat_entry_t *current_turn = NULL;
static combat_entry_t *defeated = NULL;

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
    if (cr->combatants) {
        entity_list_free(cr->combatants);
        cr->combatants = NULL;
    }
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
    node->current_action.action = CA_NONE;// Means they need to take their turn.

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

static int get_dist(entity_t *entity, const uint16_t x, const uint16_t y) {
    int xdiff = abs(entity->mapx - x);
    int ydiff = abs(entity->mapy - y);

    return (xdiff > ydiff) ? xdiff : ydiff;
}

extern int combat_initiate(region_t *reg, const uint16_t x, const uint16_t y) {
    const int dist = 10; // distance of the sphere;
    dude_t *enemy = NULL;

    entity_list_for_each(reg->entities, enemy) {
        if (enemy->name && get_dist(enemy, x, y) <= dist) {
            entity_list_add(reg->cr.combatants, enemy);
        }
    }

    // Freeze all combats.
    entity_list_for_each(reg->cr.combatants, enemy) {
        enemy->sprite.scmd = combat_animation_get_scmd(enemy->sprite.scmd, 0, 0, CA_NONE);
        port_update_entity(enemy, 0, 0);
    }

    for (int i = 0; i < MAX_PCS; i++) {
        if (player_exists(i)) {
            entity_list_add(reg->cr.combatants, player_get_entity(i));
        }
    }

    in_combat = 1;

    entity_list_for_each(reg->cr.combatants, enemy) {
        add_to_combat(enemy);
    }

    port_enter_combat();

    /*
    combat_entry_t *rover = combat_order;
    while(rover) {
        printf("%s: %p\n", rover->entity->name, rover->entity->sprite.scmd);
        rover = rover->next;
    }
    */

    return in_combat;
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

void combat_player_action(const combat_action_t action) {
    if (!wait_on_player || !current_turn) { return; }

    player_action = action.action;
}

static void queue_add(action_node_t **head, action_node_t **tail, action_node_t *current, const enum combat_action_e action) {
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
        if (!player_exists(i)) { continue; }
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

static entity_t* entity_at_location(const region_t *reg, entity_t *entity, const int32_t x, const int32_t y) {
    dude_t *dude = NULL;
    //if (reg->flags[x][y]) { return 1; }
    entity_list_for_each(reg->cr.combatants, dude) {
        //printf("(%s: %d, %d) ?= (%s: %d, %d)\n", dude->name, dude->mapx, dude->mapy, entity->name, entity->mapx, entity->mapy);
        if (dude != entity && dude->mapx == x && dude->mapy == y) {
            return dude;
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
        if (entity_at_location(reg, current_turn->entity, rover->x, rover->y) != NULL) { free(rover); continue; }
        visit_flags[rover->x][rover->y] = 1;// Mark as visited.

        if ((player = player_to_attack(reg, rover)) != NULL) {
            for(int i = 0; i < rover->num_moves; i++) {
                monster_actions[i] = rover->actions[i];
                //printf("move %d: %d\n", i, rover->actions[i].action);
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

static void apply_action_animation(const enum combat_action_e action) {
    int16_t xdiff = 0, ydiff = 0;

    switch (action) {
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
    current_turn->entity->sprite.scmd = combat_animation_get_scmd(current_turn->entity->sprite.scmd,
            xdiff, ydiff, CA_NONE);
    port_update_entity(current_turn->entity, xdiff, ydiff);
}

static void end_turn() {
    entity_t *entity = (current_turn) ? current_turn->entity : NULL;
    monster_step = -1;

    if (entity) {
        entity->sprite.scmd = combat_animation_get_scmd(entity->sprite.scmd, 0, 0, CA_NONE);
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
    player_action = CA_NONE;
}

extern void combat_is_defeated(region_t *reg, entity_t *dude) {
    combat_entry_t *prev = NULL;
    combat_entry_t *rover = combat_order;
    if (!dude) { return; }

    while (rover && rover->entity != dude) { // Warning: pointer comparison
        prev = rover;
        rover = rover->next;
    }

    if (!rover) {
        error ("Unable to remove %s from combat.\n", dude->name);
        return;
    }

    // You defeated yourself...
    if (current_turn && current_turn->entity == dude) {
        current_turn = prev;
    }

    if (prev) {
        prev->next = rover->next;
    } else {
        combat_order = combat_order->next;
    }

    rover->next = defeated;
    if (defeated) { defeated->next = rover; }

    entity_list_remove(reg->cr.combatants, entity_list_find(reg->cr.combatants, dude));

    port_remove_entity(dude);

}

// DS Engine: Attacks usually based on weapons. If none, then bare hand.
//            Monster's base weapon are considered plus for
//            level 0-4: regular
//            level 5-6: +1
//            level 6-7: +2
//            level 8-9: +3
//            level 10+: +4
// Monster data should be in the entity.
// For Now, 1d6, always hits. Need to add thac0 calculation.
static void perform_enemy_melee_attack() {
    entity_t *source = current_turn->entity;
    entity_t *target = monster_actions[monster_step].target;

    int16_t amt = dnd2e_melee_attack(source, target, current_turn->melee_actions++);
    combat_animation_add(CA_MELEE, source, NULL, 0);
    //printf("amt = %d!\n", amt);
    if (amt > 0) {
        combat_animation_add(CA_RED_DAMAGE, source, target, amt);
    }
}

static void check_and_perform_attack(region_t *reg) {
    combat_action_t *action = monster_actions + monster_step;

    switch (action->action) {
        case CA_MELEE:
            perform_enemy_melee_attack();
            break;
        default:
            break;
    }
}

static entity_t* entity_in_way(region_t *reg, entity_t *entity, const enum combat_action_e action) {
    int xdiff = 0, ydiff = 0;

    switch(action) {
        case CA_WALK_DOWNLEFT:  xdiff = -1; ydiff = 1; break;
        case CA_WALK_DOWN:      xdiff = 0; ydiff = 1; break;
        case CA_WALK_DOWNRIGHT: xdiff = 1; ydiff = 1; break;
        case CA_WALK_UPLEFT:    xdiff = -1; ydiff = -1; break;
        case CA_WALK_UP:        xdiff = 0; ydiff = -1; break;
        case CA_WALK_UPRIGHT:   xdiff = 1; ydiff = -1; break;
        case CA_WALK_LEFT:      xdiff = -1; ydiff = 0; break;
        case CA_WALK_RIGHT:     xdiff = 1; ydiff = 0; break;
        default:
            return NULL;
    }

    return entity_at_location(reg, entity, entity->mapx + xdiff, entity->mapy + ydiff);
}

static void player_melee(region_t *reg, entity_t* entity, entity_t *enemy) {
    //int amt = 1 + (rand() % 6);
    int amt = 100; // FTW!
    combat_animation_add(CA_MELEE, current_turn->entity, NULL, 0);
    combat_animation_add(CA_RED_DAMAGE, current_turn->entity, enemy, amt);
    wait_on_player = 0;
}

static void move_entity(region_t *reg, entity_t *entity, const enum combat_action_e action) {
    entity_t *enemy;

    switch(action) {
        case CA_NONE:
            entity->sprite.scmd = combat_animation_get_scmd(entity->sprite.scmd, 0, 0, CA_NONE);
            port_update_entity(entity, 0, 0);
            ticks_per_game_round = 0;
            break;
        case CA_WALK_DOWNLEFT:
        case CA_WALK_DOWN:
        case CA_WALK_DOWNRIGHT:
        case CA_WALK_UPLEFT:
        case CA_WALK_UP:
        case CA_WALK_UPRIGHT:
        case CA_WALK_LEFT:
        case CA_WALK_RIGHT:
            enemy = entity_in_way(reg, entity, action);
            if (enemy && enemy->allegiance != entity->allegiance) {
                entity->sprite.scmd = combat_animation_face_direction(entity->sprite.scmd, action);
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
        rover = rover->next;
    }

    current_turn = combat_order;
}

static void do_combat_rounds(region_t *reg) {
    //Need to start combat rounds.
    if (!current_turn) {
        next_round();
    }
    entity_t *monster = current_turn->entity;

    current_player = which_player(current_turn);
    //printf("player = %d, name = %s\n", current_player, current_turn->entity->name);
    if (current_player >= 0) {
        wait_on_player = 1;
        move_entity(reg, player_get_entity(current_player), player_action);
        return; // Need to wait on player input.
    }

    // Monster time.
    if (monster_step < 0) {
        generate_monster_actions(reg);
        monster_step = 0;
    }

    apply_action_animation(monster_actions[monster_step].action);

    check_and_perform_attack(reg);
    if (monster_step < 0) { return; } // turn ended
    monster->stats.move--;
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

static int melee_count(entity_t *entity) {
    if (!entity->inv) {
        int amt = entity->stats.attacks[0].number;
        amt += entity->stats.attacks[1].number;
        amt += entity->stats.attacks[2].number;
        return amt;
    }
    warn("NOT TAKING INTO ACCOUNT INVENTORY FOR CREATURES!!! FIX THIS !!!");
    return 0;
}

// decide if the current turn is over and ready next.
static void check_current_turn() {
    if (!combat_animation_has_more()) {
        if (melee_count(current_turn->entity) <= current_turn->melee_actions) {
            end_turn();
            return;
        }
        monster_step = -1;
    }
}

static void do_player_turn(region_t *reg) {
    if (ticks_per_game_round > 0) {
        ticks_per_game_round--;
        return;
    }

    move_entity(reg, player_get_entity(current_player), player_action);
    // Players move quicker.
    if (ticks_per_game_round > 0) { ticks_per_game_round = 20; }
}

static combat_action_t clear = { CA_NONE, NULL, NULL, 0 };

void combat_update(region_t *reg) {
    if (reg == NULL) { return; }
    combat_region_t *cr = &(reg->cr);
    if (cr == NULL) { return; }

    if (wait_on_player) {
        do_player_turn(reg);
        return;
    }

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    if (in_combat) {
        if (combat_animation_execute(reg)) {
            check_current_turn();
            return;
        }
        port_combat_action(&clear);

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
}

void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id) {
    if (!cr || combat_id < 0 || combat_id >= MAX_COMBAT_OBJS) { return; }
    cr->hunt[combat_id] = 1;
}

// This does not force into combat mode, simply add a combat to the current region.
extern uint32_t combat_add(combat_region_t *rc, entity_t *entity) {
    if (!rc || !entity) { return 0; }

    entity_list_add(rc->combatants, entity);
    return 1;
}
