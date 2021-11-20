#include "effect.h"
#include "port.h"
#include "gpl.h"

#include <string.h>

// TODO: Finish the Implementation!
void effect_apply_damage(entity_t *source, entity_t *target, const int32_t damage, enum effect_type type) {
    if (type & EFFECT_MAGIC) {
        entity_animation_add(EA_MAGIC_DAMAGE, source, target, NULL, damage);
        entity_animation_add(EA_DAMAGE_APPLY, source, target, NULL, damage);
    } else {
        warn ("NEED TO implement effect_apply_damage!\n");
    }
}
