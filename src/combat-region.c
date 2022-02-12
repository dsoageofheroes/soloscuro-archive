#include "combat-region.h"
#include "entity-list.h"

extern int sol_combat_check_if_over(combat_region_t *cr) {
    entity_t *combatant;
    if (!cr || !cr->combatants.head) { return 1; }
    int allegiance = -1;

    entity_list_for_each((&(cr->combatants)), combatant) {
        if (allegiance == -1) { allegiance = combatant->allegiance; }
        if (allegiance != combatant->allegiance) {
            return 0;
        }
    }

    return 1;
}

extern void sol_combat_clear(combat_region_t *cr) {
    if (!cr || !cr->combatants.head) { return; }
    entity_list_clear((&(cr->combatants)));
}
