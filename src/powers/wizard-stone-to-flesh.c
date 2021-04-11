/* This file is auto-generated. */
#ifndef WIZARD_STONE_TO_FLESH_POWER_H
#define WIZARD_STONE_TO_FLESH_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_stone_to_flesh_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_stone_to_flesh_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_stone_to_flesh_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 0;
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 1) + mod;
    effect_type = 0;
    effect_apply_damage(source->entity, entity, damage, effect_type);
}

static int wizard_stone_to_flesh_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_stone_to_flesh_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 0 + (120 * entity_get_wizard_level(entity));
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_stone_to_flesh_setup  (power_t *power) {
    power->name                 = "STONE TO FLESH";
    power->description          = spin_read_description(select_by_game(-1, 79, 79));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 6;
    power->shape                = TARGET_ALLY;
    power->cast_sound           = select_by_game(255, 255, 255);
    power->thrown_sound         = select_by_game(255, 255, 255);
    power->hit_sound            = select_by_game(9, 9, 9);
    power->actions.can_activate = wizard_stone_to_flesh_can_activate;
    power->actions.pay          = wizard_stone_to_flesh_pay;
    power->actions.apply        = wizard_stone_to_flesh_apply;
    power->actions.affect_power = wizard_stone_to_flesh_affect_power;
    powers_set_icon(power, select_by_game(21078, 21078, 21078));
    powers_set_cast(power, select_by_game(8003, 8003, 8003)); // OJFF
    powers_set_thrown(power, select_by_game(8053, 8053, 8053)); // OJFF.
    powers_set_hit(power, select_by_game(8305, 8305, 8305)); // OJFF
}

#endif
