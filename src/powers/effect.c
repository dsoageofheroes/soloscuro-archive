#include "effect.h"
#include "port.h"
#include "gpl.h"
#include "region-manager.h"

#include <string.h>

// TODO: Finish the Implementation!
void sol_effect_apply_damage(entity_t *source, entity_t *target, const int32_t damage, enum effect_type type) {
    sol_region_t *reg = sol_region_manager_get_current();
    if (type & EFFECT_MAGIC) {
        entity_animation_list_add_effect(&(reg->actions), EA_MAGIC_DAMAGE, source, target, NULL, 30, damage);
        entity_animation_list_add_effect(&(reg->actions), EA_DAMAGE_APPLY, source, target, NULL, 0, damage);
    } else {
        warn ("NEED TO implement effect_apply_damage!\n");
    }
}
