/* This file is auto-generated. */
#ifndef WIZARD_ACID_ARROW_POWER_H
#define WIZARD_ACID_ARROW_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_acid_arrow_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_acid_arrow_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_acid_arrow_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 2;
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 4) + mod;
    effect_type = 0 | EFFECT_ACID | EFFECT_MAGIC;
    effect_apply_damage(source->entity, entity, damage, effect_type);
}

static int wizard_acid_arrow_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_acid_arrow_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 800;
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_acid_arrow_setup  (power_t *power) {
    power->name                 = "ACID ARROW";
    power->description          = spin_read_description(select_by_game(19, 19, 19));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 2;
    power->shape                = TARGET_ENEMY;
    power->cast_sound           = select_by_game(80, 185, 185);
    power->thrown_sound         = select_by_game(73, 255, 255);
    power->hit_sound            = select_by_game(94, 62, 62);
    power->actions.can_activate = wizard_acid_arrow_can_activate;
    power->actions.pay          = wizard_acid_arrow_pay;
    power->actions.apply        = wizard_acid_arrow_apply;
    power->actions.affect_power = wizard_acid_arrow_affect_power;
    powers_set_icon(power, select_by_game(21018, 21018, 21018));
    powers_set_cast(power, select_by_game(8000, 8000, 8000)); // OJFF
    powers_set_thrown(power, select_by_game(8114, 8100, 8100)); // OJFF.
    powers_set_hit(power, select_by_game(8200, 8200, 8200)); // OJFF
}

#endif
