/* This file is auto-generated. */
#ifndef WIZARD_CONJURE_GREATER_ELEMENTAL_POWER_H
#define WIZARD_CONJURE_GREATER_ELEMENTAL_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_conjure_greater_elemental_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_conjure_greater_elemental_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_conjure_greater_elemental_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 0;
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 0) + mod;
    effect_type = 0;
}

static int wizard_conjure_greater_elemental_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_conjure_greater_elemental_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 90;
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_conjure_greater_elemental_setup  (power_t *power) {
    power->name                 = "CONJURE GREATER ELEMENTAL";
    power->description          = spin_read_description(select_by_game(82, 82, 82));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 7;
    power->shape                = TARGET_SINGLE;
    power->cast_sound           = select_by_game(108, 108, 108);
    power->thrown_sound         = select_by_game(255, 255, 255);
    power->hit_sound            = select_by_game(8, 8, 8);
    power->actions.can_activate = wizard_conjure_greater_elemental_can_activate;
    power->actions.pay          = wizard_conjure_greater_elemental_pay;
    power->actions.apply        = wizard_conjure_greater_elemental_apply;
    power->actions.affect_power = wizard_conjure_greater_elemental_affect_power;
    powers_set_icon(power, select_by_game(21081, 21081, 21081));
    powers_set_cast(power, select_by_game(8309, 8309, 8309)); // OJFF
    powers_set_thrown(power, select_by_game(8099, 8099, 8099)); // OJFF.
    powers_set_hit(power, select_by_game(8300, 8300, 8300)); // OJFF
}

#endif
