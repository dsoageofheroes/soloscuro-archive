#include "combat.h"
#include "combat-region.h"
#include "entity-list.h"
#include "gpl-state.h"
#include "region-manager.h"
#include "player.h"
#include "rules.h"

#include <stdio.h>
#include <string.h>

extern sol_status_t sol_combat_region_init(sol_combat_region_t *cr) {
    memset(cr, 0x0, sizeof(sol_combat_region_t));
    sol_entity_list_init(cr->combatants);
    sol_entity_list_init(cr->round.entities);
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_check_if_over(sol_combat_region_t *cr) {
    sol_entity_t *combatant;
    if (!cr || !cr->combatants.head) { return SOL_SUCCESS; }
    int allegiance = -1;

    sol_entity_list_for_each((&(cr->combatants)), combatant) {
        if (allegiance == -1) { allegiance = combatant->allegiance; }
        if (allegiance != combatant->allegiance) {
            return SOL_IN_COMBAT;
        }
    }

    // clear FIGHT
    sol_gpl_set_gname(GNAME_FIGHT, 0);
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_clear(sol_combat_region_t *cr) {
    if (!cr || !cr->combatants.head) { return SOL_NULL_ARGUMENT; }
    sol_entity_list_clear((&(cr->combatants)));
    sol_entity_list_clear((&(cr->round.entities)));
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_get_current(sol_combat_region_t *cr, sol_entity_t **e) {
    if (!cr) { return SOL_NULL_ARGUMENT; }
    if (!cr->round.entities.head) { return SOL_NOT_IN_COMBAT; }

    *e = cr->round.entities.head->entity;
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_next_combatant(sol_combat_region_t *cr) {
    sol_dude_t *dude = NULL;
    if (!cr) { return SOL_NULL_ARGUMENT; }

    sol_entity_list_remove_entity(&cr->round.entities, cr->round.entities.head->entity);
    if (cr->round.entities.head) {
        dude = cr->round.entities.head->entity;
        dude->stats.combat.move = dnd2e_get_move(dude);
        dude->combat_status = EA_NONE;
        sol_gpl_set_gname(GNAME_FIGHT, dude->ds_id);
    } else {
        sol_gpl_set_gname(GNAME_FIGHT, 0);
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_combat_attempt_action(sol_combat_region_t *cr, sol_dude_t *dude) {
    if (!cr || !dude) { return SOL_NULL_ARGUMENT; }
    if (cr->round.entities.head->entity != dude) { return SOL_NOT_TURN; }

    if (dude->stats.combat.move <= 0) { return SOL_NO_MOVES_LEFT; }

    dude->stats.combat.move--;
    return SOL_SUCCESS;
}

// This is called right at the end of an animation.
extern sol_status_t sol_combat_guard_check(sol_combat_region_t *cr) {
    sol_region_t *reg;
    sol_entity_t *dude, *enemy = NULL;

    if (!cr) { return SOL_NULL_ARGUMENT; }
    if (sol_combat_get_current(cr, &dude) != SOL_SUCCESS) { return SOL_NOT_TURN;}
    if (!dude) { return SOL_NOT_TURN; }
      
    sol_entity_list_for_each((&(cr->combatants)), enemy) {
        //printf("(%d, %d) vs (%d, %d)\n", dude->mapx, dude->mapy, enemy->mapx, enemy->mapy);
        if (enemy == dude) { continue; }
        if (abs(enemy->mapx - dude->mapx) <= 1 && abs(enemy->mapy - dude->mapy) <= 1) {
            if (enemy->combat_status == EA_GUARD) {
                //printf("%s should attack %s!\n", enemy->name, dude->name);
                sol_region_manager_get_current(&reg);
                if (sol_combat_add_attack_animation(reg,
                              enemy, dude, NULL, EA_GUARD) != SOL_SUCCESS) {
                    enemy->combat_status = EA_NONE;
                    //printf("out of attacks.\n");
                }

                return SOL_SUCCESS;
            }
        }
    }
  
    return SOL_NOT_FOUND;
}

extern sol_status_t sol_combat_get_closest_enemy(sol_combat_region_t *cr, const int x, const int y, sol_entity_t **e) {
    sol_entity_t *dude, *ret = NULL;
    int       max = 9999999, slot;

    sol_entity_list_for_each((&(cr->combatants)), dude) {
        int dx = abs(dude->mapx - x);
        int dy = abs(dude->mapy - y);
        int tmax = dx > dy ? dx : dy;

        if (sol_player_get_slot(dude, &slot) == SOL_SUCCESS && slot >= 0) { continue; } 
        if (tmax == 0) { continue; }
        if (tmax < max) {
            max = tmax;
            ret = dude;
        }
    }

    *e = ret;
    return ret ? SOL_SUCCESS : SOL_NOT_FOUND;
}
