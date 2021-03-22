#include "../powers.h"

static int armor_can_activate (struct power_instance_s *source) {
    return 0;
}

static int armor_pay          (struct power_instance_s *source) {
    return 0;
}

static int armor_apply        (struct power_instance_s *source, entity_t *target) {
    return 0;
};

static int armor_still_active (struct power_instance_s *source, const int rounds_past) {
    return 0;
}

static int armor_affect_power (struct power_instance_s *target) {
    return 0;
}

power_t spell_armor = {
    .actions.can_activate = armor_can_activate,
    .actions.pay          = armor_pay,
    .actions.apply        = armor_apply,
    .actions.still_active = armor_still_active,
    .actions.affect_power = armor_affect_power,
};
