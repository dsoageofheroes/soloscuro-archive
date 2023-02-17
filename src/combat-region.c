#include "combat.h"
#include "combat-region.h"
#include "entity-list.h"
#include "gpl-state.h"
#include "region-manager.h"
#include "player.h"
#include "rules.h"

#include <stdio.h>
#include <string.h>

extern void sol_combat_region_init(combat_region_t *cr) {
    memset(cr, 0x0, sizeof(combat_region_t));
    sol_entity_list_init(cr->combatants);
    sol_entity_list_init(cr->round.entities);
}

extern int sol_combat_check_if_over(combat_region_t *cr) {
    entity_t *combatant;
    if (!cr || !cr->combatants.head) { return 1; }
    int allegiance = -1;

    sol_entity_list_for_each((&(cr->combatants)), combatant) {
        if (allegiance == -1) { allegiance = combatant->allegiance; }
        if (allegiance != combatant->allegiance) {
            return 0;
        }
    }

    // clear FIGHT
    sol_gpl_set_gname(GNAME_FIGHT, 0);
    return 1;
}

extern void sol_combat_clear(combat_region_t *cr) {
    if (!cr || !cr->combatants.head) { return; }
    sol_entity_list_clear((&(cr->combatants)));
    sol_entity_list_clear((&(cr->round.entities)));
}

extern entity_t* sol_combat_get_current(combat_region_t *cr) {
    if (!cr || !cr->round.entities.head) { return NULL; }

    return cr->round.entities.head->entity;
}

extern void sol_combat_next_combatant(combat_region_t *cr) {
    dude_t *dude = NULL;
    if (!cr) { return; }

    sol_entity_list_remove_entity(&cr->round.entities, cr->round.entities.head->entity);
    if (cr->round.entities.head) {
        dude = cr->round.entities.head->entity;
        dude->stats.combat.move = dnd2e_get_move(dude);
        dude->combat_status = EA_NONE;
        sol_gpl_set_gname(GNAME_FIGHT, dude->ds_id);
    } else {
        sol_gpl_set_gname(GNAME_FIGHT, 0);
    }
}

extern sol_status_t sol_combat_attempt_action(combat_region_t *cr, dude_t *dude) {
    if (!cr || !dude) { return SOL_NULL_ARGUMENT; }
    if (cr->round.entities.head->entity != dude) { return SOL_NOT_TURN; }

    if (dude->stats.combat.move <= 0) { return SOL_NO_MOVES_LEFT; }

    dude->stats.combat.move--;
    return SOL_SUCCESS;
}

// This is called right at the end of an animation.
extern int sol_combat_guard_check(combat_region_t *cr) {
    sol_region_t *reg;
    entity_t *dude = sol_combat_get_current(cr), *enemy = NULL;
    if (!dude) { return 0; }
      
    sol_entity_list_for_each((&(cr->combatants)), enemy) {
        //printf("(%d, %d) vs (%d, %d)\n", dude->mapx, dude->mapy, enemy->mapx, enemy->mapy);
        if (enemy == dude) { continue; }
        if (abs(enemy->mapx - dude->mapx) <= 1 && abs(enemy->mapy - dude->mapy) <= 1) {
            if (enemy->combat_status == EA_GUARD) {
                //printf("%s should attack %s!\n", enemy->name, dude->name);
                sol_region_manager_get_current(&reg);
                if (sol_combat_add_attack_animation(reg,
                              enemy, dude, NULL, EA_GUARD) == -2) {
                    enemy->combat_status = EA_NONE;
                    //printf("out of attacks.\n");
                }

                return 1;
            }
        }
    }
  
    return 0;
}

extern entity_t* sol_combat_get_closest_enemy(combat_region_t *cr, const int x, const int y) {
    entity_t *dude, *ret = NULL;
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

    return ret;
}
