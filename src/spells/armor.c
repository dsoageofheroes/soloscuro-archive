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

extern void spell_armor_attach (power_t *spell) {
    spell->actions.can_activate = armor_can_activate;
    spell->actions.pay          = armor_pay;
    spell->actions.apply        = armor_apply;
    spell->actions.still_active = armor_still_active;
    spell->actions.affect_power = armor_affect_power;
}
