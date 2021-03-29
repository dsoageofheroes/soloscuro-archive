/* This file is auto-generated. */
#ifndef WIZARD_PSIONIC_DAMPENER_POWER_H
#define WIZARD_PSIONIC_DAMPENER_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

extern int wizard_psionic_dampener_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

extern int wizard_psionic_dampener_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

extern void wizard_psionic_dampener_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 0;
    mod = (1 + 1) * caster_level;
    damage = dnd2e_dice_roll(num_dice, 0) + mod;
    effect_type = 0 | EFFECT_MAGIC;
}

extern int wizard_psionic_dampener_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

extern int wizard_psionic_dampener_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 0 + (60 * entity_get_wizard_level(entity));
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_psionic_dampener_setup  (power_t *power) {
    power->name                 = "PSIONIC DAMPENER";
    power->description          = spin_read_description(select_by_game(0, 49, 49));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->shape                = TARGET_ENEMY;
    power->cast_sound           = select_by_game(10, 10, 10);
    power->thrown_sound         = select_by_game(255, 255, 255);
    power->hit_sound            = select_by_game(12, 12, 12);
    power->actions.can_activate = wizard_psionic_dampener_can_activate;
    power->actions.pay          = wizard_psionic_dampener_pay;
    power->actions.apply        = wizard_psionic_dampener_apply;
    power->actions.affect_power = wizard_psionic_dampener_affect_power;
    powers_set_icon(power, select_by_game(21048, 21048, 21048));
    powers_set_cast(power, select_by_game(8307, 8307, 8307)); // OJFF
    powers_set_thrown(power, select_by_game(8215, 8215, 8215)); // OJFF.
    powers_set_hit(power, select_by_game(-1, -1, -1)); // OJFF
}

#endif
