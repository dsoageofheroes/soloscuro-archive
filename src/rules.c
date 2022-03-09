#include "class.h"
#include "alignment.h"
#include "stats.h"
#include "gff.h"
#include "race.h"
#include "rules.h"
#include "gpl.h"
#include "combat.h"
#include <stdlib.h>
#include <stdio.h>

static void set_psp(entity_t *pc) {
    pc->stats.high_psp = 0;
    if (pc->stats.con > 15) {
        pc->stats.high_psp += pc->stats.con - 15;
    }
    if (pc->stats.intel > 15) {
        pc->stats.high_psp += pc->stats.intel - 15;
    }
    if (pc->stats.wis >= 15) {
        pc->stats.high_psp += 20 + 2 * (pc->stats.wis - 15);
    }
    if (entity_has_class(pc, REAL_CLASS_PSIONICIST)) {
        int psi_level = 0;
        if (pc->class[0].class == REAL_CLASS_PSIONICIST) { psi_level = pc->class[0].level; }
        if (pc->class[1].class == REAL_CLASS_PSIONICIST) { psi_level = pc->class[1].level; }
        if (pc->class[2].class == REAL_CLASS_PSIONICIST) { psi_level = pc->class[2].level; }
        pc->stats.high_psp += 10 * (psi_level);
        if (pc->stats.wis > 15) {
            pc->stats.high_psp += (20 + 2 * (pc->stats.intel - 15)) * (psi_level - 1);
        }
    }
    pc->stats.psp = pc->stats.high_psp;
}

static void do_level_up(entity_t *pc, const uint32_t class_idx, const uint32_t class) {
    int clevel = pc->class[class_idx].level;
    int num_classes = 0;

    for (int i = 0; i < 3 && pc->class[i].class > -1; i++) {
        num_classes++;
    }

    // Roll the next HP:
    int current_hit_die = sol_dnd2e_class_total_hit_die(pc->class[class_idx].class, clevel);
    int next_hit_die = sol_dnd2e_class_total_hit_die(pc->class[class_idx].class, clevel + 1);
    //printf("%d: lvl = %d, chd = %d, nhd = %d\n", pc->class[class_idx].class, clevel, current_hit_die, next_hit_die);
    int hp = 0;
    if (next_hit_die > current_hit_die) {
        //printf("Need to increase HP by up to %d\n", sol_dnd2e_class_hp_die(pc->class[class_idx].class));
        hp = 1 + (rand() % sol_dnd2e_class_hp_die(pc->class[class_idx].class));
        hp += sol_dnd2e_hp_mod(&pc->stats);
        hp = (hp / num_classes) + (((hp % num_classes) > 0) ? 1 : 0);
        if (hp < 1) { hp = 1; }
    } else {
        hp = 2;
    }
    pc->stats.hp += hp;
    pc->stats.high_hp += hp;
    pc->class[class_idx].level++;

    //TODO: add level up spell slots correctly
    sol_dnd2e_class_update_max_spell_slots(pc);
    set_psp(pc);
}

void dnd2e_set_exp(entity_t *pc, const uint32_t amt) {
    pc->stats.hp = pc->stats.high_hp = 0;
    for (int i = 0; i < 3; i++) {
        if (pc->class[i].class > -1) {
            pc->class[i].level = 0;
            do_level_up(pc, i, pc->class[i].class);
        }
    }
    pc->class[0].current_xp = 0;
    pc->class[1].current_xp = 0;
    pc->class[2].current_xp = 0;
    dnd2e_award_exp(pc, amt);
}

void dnd2e_award_exp_to_class(entity_t *pc, const int index, const uint32_t amt) {
    pc->class[index].current_xp += amt;
    int next_level = sol_dnd2e_class_level(pc->class[index].class, pc->class[index].current_xp);
    while (next_level > pc->class[index].level) {
        do_level_up(pc, index, pc->class[index].level);
    }
}

void dnd2e_award_exp(entity_t *pc, const uint32_t amt) {
    int num_classes = 0;
    for (int i = 0; i < 3 && pc->class[i].level > -1; i++) {
        //printf("pc->leve[%d] = %d\n",i, pc->level[i]);
        num_classes++;
    }
    for (int i = 0; i < num_classes; i++) {
        dnd2e_award_exp_to_class(pc, i, amt / num_classes);
    }
}

int16_t dnd2e_get_ac_pc(entity_t *pc) {
    int ac_bonus = 0;

    if (!dnd2e_character_is_valid(pc)) { return 10; }

    if (pc->inv) {
        item_t *item = pc->inv;
        for (int i = 0; i <= SLOT_FOOT; i++) {
            if (item[i].ds_id) {
                ac_bonus += -item[i].ac;
                //printf("slot: %d, id: %d it1r: %d, ac: %d\n", i, item[i].id, item[i].item_index, ds_get_item1r(item[i].item_index)->base_AC);
            }
        }
    }
    return pc->stats.base_ac + sol_dnd2e_ac_mod(&pc->stats) + ac_bonus;
}

int16_t dnd2e_get_attack_num(const entity_t *pc, const item_t *item) {
    if (item == NULL || !item->ds_id) { return pc->stats.attacks[0].number; }
    // For some reason double attacks are stored for missiles...
    return item->attack.number + sol_dnd2e_class_attack_num(pc, item);
}

int16_t dnd2e_get_attack_sides_pc(const entity_t *pc, const item_t *item) {
    if (item == NULL || !item->ds_id) { return pc->stats.attacks[0].sides; }
    return item->attack.sides;
}

int16_t dnd2e_get_attack_die_pc(const entity_t *pc, const item_t *item) {
    if (item == NULL || !item->ds_id) { return pc->stats.attacks[0].num_dice; }
    return item->attack.num_dice;
}

enum {
    MATERIAL_WOOD,
    MATERIAL_BONE =  (1<<0),
    MATERIAL_STONE = (1<<1),
    MATERIAL_OBSIDIAN = (1<<2),
    MATERIAL_METAL = (1<<3),
    MATERIAL_LEATHER = (1<<3),
};

int16_t dnd2e_get_attack_mod_pc(const entity_t *pc, const item_t *item) {
    uint16_t material_mod = 0;

    if (!dnd2e_character_is_valid(pc)) { return 0; }

    if (item == NULL || !item->ds_id) {
        return pc->stats.attacks[0].bonus
            + sol_dnd2e_melee_damage_mod(&pc->stats);
    }

    switch(item->material) {
        case MATERIAL_WOOD: material_mod = -2; break;
        case MATERIAL_STONE:
        case MATERIAL_BONE:
        case MATERIAL_OBSIDIAN:
            material_mod = -1; break;
    }

    return item->attack.bonus + material_mod +
            + sol_dnd2e_melee_damage_mod(&pc->stats);
}

static int calc_starting_exp(entity_t *pc) {
    int num_classes = 0;
    int most_exp_class = -1;
    int most_exp = -1;
    int starting_level = -1;

    for (int i = 0; i < 3; i++) {
        if (pc->class[0].class >= 0 ) { num_classes++; }
    }

    starting_level = (num_classes > 1) ? 2 : 3;

    for (int i = 0; i < 3; i++) {
        int temp = sol_dnd2e_next_level_exp(pc->class[0].class, starting_level - 1);
        most_exp = most_exp > temp ? most_exp : temp;
    }

    return most_exp * starting_level * num_classes;
}

void dnd2e_randomize_stats_pc(entity_t *pc) {
    pc->stats.str = 10 + (rand() % 11);
    pc->stats.dex = 10 + (rand() % 11);
    pc->stats.con = 10 + (rand() % 11);
    pc->stats.intel = 10 + (rand() % 11);
    pc->stats.wis = 10 + (rand() % 11);
    pc->stats.cha = 10 + (rand() % 11);
    pc->stats.base_ac = 10;

    sol_dnd2e_race_apply_mods(pc);
    dnd2e_loop_creation_stats(pc);
    dnd2e_set_starting_level(pc);
}

void dnd2e_set_starting_level(entity_t *pc) {
    dnd2e_set_exp(pc, calc_starting_exp(pc)); // Also sets HP & PSP
}

static void adjust_creation_hp(entity_t *pc) {
    int min_hp = 0; // Right now min_hp is looking for max level
    int max_hp = 0;
    int num_levels = 0;

    if (pc->class[0].class > -1) {
        min_hp = (pc->class[0].level > min_hp) ? pc->class[0].level : min_hp;
        max_hp += sol_dnd2e_class_max_hp(pc->class[0].class, pc->class[0].level, sol_dnd2e_hp_mod(&pc->stats));
        num_levels++;
    }
    if (pc->class[1].class > -1) {
        min_hp = (pc->class[1].level > min_hp) ? pc->class[1].level : min_hp;
        max_hp += sol_dnd2e_class_max_hp(pc->class[1].class, pc->class[1].level, sol_dnd2e_hp_mod(&pc->stats));
        num_levels++;
    }
    if (pc->class[2].class > -1) {
        min_hp = (pc->class[2].level > min_hp) ? pc->class[2].level : min_hp;
        max_hp += sol_dnd2e_class_max_hp(pc->class[2].class, pc->class[2].level, sol_dnd2e_hp_mod(&pc->stats));
        num_levels++;
    }

    // min_hp is the max level, apply con bonus.
    min_hp += min_hp + min_hp * sol_dnd2e_hp_mod(&pc->stats);

    if (num_levels > 0) {
        max_hp += (num_levels - 1); // force rounding
        max_hp /= num_levels;
    }

    if (pc->stats.hp < min_hp) { pc->stats.high_hp = pc->stats.hp = max_hp; }
    if (pc->stats.hp > max_hp) { pc->stats.high_hp = pc->stats.hp = min_hp; }
    //printf("(%d, %d) %d %d %d\n", min_hp, max_hp, pc->class[0].class, pc->class[1].class, pc->class[2].class);
}

void dnd2e_loop_creation_stats(entity_t *pc) {
    if (!pc || pc->race < 0 || pc->race > RACE_THRIKREEN) { return; }

    sol_dnd2e_race_apply_initial_stats(pc);

    sol_dnd2e_class_apply_stats(pc, pc->class[0].class);
    sol_dnd2e_class_apply_stats(pc, pc->class[1].class);
    sol_dnd2e_class_apply_stats(pc, pc->class[2].class);

    adjust_creation_hp(pc);
    set_psp(pc);
}

int dnd2e_character_is_valid(const entity_t *pc) {
    if (!sol_dnd2e_stats_valid(&pc->stats)) { return 0; }
    if (!sol_dnd2e_is_class_allowed(pc->race, pc->class)) { return 0; }
    if (pc->gender != GENDER_MALE && pc->gender != GENDER_FEMALE) { return 0; }
    if (pc->alignment < LAWFUL_GOOD || pc->alignment > CHAOTIC_EVIL) { return 0; }
    if (pc->class[0].level < 1) { return 0; }
    if (pc->class[1].class > -1 && pc->class[1].level < 1) { return 0; }
    if (pc->class[2].class > -1 && pc->class[2].level < 1) { return 0; }
    if (pc->stats.magic_resistance < 0 || pc->stats.magic_resistance > 100) { return 0; }
    if (pc->allegiance != 1) { return 0; }
    if (pc->size < 0) { return 0; }
    // Not checked:
    // pc->id
    // pc->data1
    // pc->legalClass
    // pc->base_ac
    // pc->base_move
    // pc->num_blows
    // pc->num_attacks
    // pc->num_dice
    // pc->num_sides
    // pc->num_bonuses
    // pc->saving_throw
    // pc->spell_group
    // pc->high_level
    // pc->soundfx
    // pc->attack_sound
    // pc->psi_group
    // pc->pallette
    return 1;// passed the checks.
}

int dnd2e_psin_is_valid(ds_character_t *pc, psin_t *psi) {
    int num_psionics = 0;
    int is_psionicist = 0;

    for (int i = 0; i < 7; i++) {
        if (psi->types[i]) { num_psionics++; }
    }
    for (int i = 0; i < 3; i++) {
        if (pc->real_class[i] == REAL_CLASS_PSIONICIST) { is_psionicist = 1; }
    }

    if (is_psionicist) { return num_psionics == 3; }

    return num_psionics == 1;
}

int16_t dnd2e_get_move(entity_t *pc) {
    return pc->stats.base_move;
}

static int item_thac0_mod(item_t *item) {
    int thac0_mod = 0;

    switch(item->material) {
        case MATERIAL_WOOD: thac0_mod = 3; break;
        case MATERIAL_STONE:
        case MATERIAL_BONE:
            thac0_mod = 2; break;
        case MATERIAL_OBSIDIAN:
            thac0_mod = 1; break;
    }

    if (item->effect) {
        warn("need to added any effects on thac0 for %d (%s)\n", item->ds_id, item->name);
    }

    return thac0_mod;
}

int16_t dnd2e_get_thac0(entity_t *pc, int slot) {
    int class_thac0 = sol_dnd2e_class_thac0(pc);
    int weapon_mod = (pc->inv && pc->inv[slot].ds_id)
        ? item_thac0_mod(pc->inv + slot)
        : 0; // bare handed.
    int stat_mod = 0;

    if (slot != SLOT_MISSILE && slot != SLOT_AMMO) {
        stat_mod = sol_dnd2e_melee_hit_mod(&pc->stats);
    } else { // ASSUME missile/ammo
        stat_mod = sol_dnd2e_range_hit_mod(&pc->stats);
    }

    return class_thac0 + weapon_mod - stat_mod;
}
// END THAC0

extern int16_t dnd2e_dice_roll(const uint16_t num, const uint16_t sides) {
    int16_t sum = 0;

    for (uint16_t i = 0; i < num; i++) {
        sum += 1 + (rand() % sides);
    }

    return sum;
}

// Missle modification is in original DS engine, so is d10. Although DS was highest goes first, we are lowest.
// TODO: Add mods for race, spell efects, etc...
int dnd2e_roll_initiative(entity_t *entity) {
    return (rand() % 10) + sol_dnd2e_reaction_mod(&entity->stats);
}
int dnd2e_roll_sub_roll() {
    return rand();
}

int16_t dnd2e_calc_ac(entity_t *entity) {
    int ac_bonus = 0;

    if (entity->inv) {
        item_t *item = entity->inv;
        for (int i = 0; i <= SLOT_FOOT; i++) {
            if (item[i].ds_id) {
                ac_bonus += item[i].ac;
                //printf("slot: %d, id: %d it1r: %d, ac: %d\n", i, item[i].id, item[i].item_index, ds_get_item1r(item[i].item_index)->base_AC);
            }
        }
    }

    return entity->stats.base_ac + sol_dnd2e_ac_mod(&entity->stats) - ac_bonus;
}

// Combat
static int droll(const int max) {
    return 1 + (rand() % max);
}

static int16_t roll_damage_innate(innate_attack_t *attack) {
    int16_t amt = 0;

    for (int i = 0; i < attack->num_dice; i++) {
        amt += droll(attack->sides);
    }

    // TODO: SPECIALS?
    if (attack->special) {
        warn("melee special not implemented.\n");
    }

    return amt;
}

static int16_t roll_damage_weapon(item_t *item) {
    int16_t amt = 0;

    for (int i = 0; i < item->attack.num_dice; i++) {
        amt += droll(item->attack.sides);
    }

    amt += item->attack.bonus;

    // TODO: SPECIALS?
    if (item->effect) {
        warn("weapon special not implemented.\n");
    }

    return amt;
}

static void populate_melee_sequence(entity_t *source, uint8_t *seq, const int round) {
    int      pos         = 0;
    int      amt         = 0;
    int16_t  class_extra = 0;
    item_t  *item        = NULL;

    if (!source->inv) {
        // Usually monster so we go with innate attacks.
        amt = source->stats.attacks[0].number / 2;
        amt += ((round % 2 == 1) && (source->stats.attacks[0].number % 2 == 1)) ? 1 : 0;
        for (int i = 0; i < amt; i++) { seq[pos++] = SLOT_INNATE0; }

        amt = source->stats.attacks[1].number / 2;
        amt += ((round % 2 == 1) && (source->stats.attacks[1].number % 2 == 1)) ? 1 : 0;
        for (int i = 0; i < amt; i++) { seq[pos++] = SLOT_INNATE1; }

        amt = source->stats.attacks[2].number / 2;
        amt += ((round % 2 == 1) && (source->stats.attacks[2].number % 2 == 1)) ? 1 : 0;
        for (int i = 0; i < amt; i++) { seq[pos++] = SLOT_INNATE2; }

        seq[pos] = SLOT_END;
        return;
    }

    item = source->inv + SLOT_HAND0;
    if (item->name) {
        amt = sol_dnd2e_class_attack_num(source, item) / 2;
        amt += ((round % 2 == 1) && (sol_dnd2e_class_attack_num(source, item) % 2 == 1)) ? 1 : 0;
        for (int i = 0; i < amt; i++) { seq[pos++] = SLOT_HAND0; }
    } else {
        seq[pos++] = SLOT_INNATE0;
    }

    item = source->inv + SLOT_HAND1;
    if (!item->name && seq[pos - 1] == SLOT_INNATE0) {
        seq[pos++] = SLOT_INNATE1;
    } else if (item->attack.number > 0) {
        seq[pos++] = SLOT_HAND1;
    }

    seq[pos] = SLOT_END;
}

static void populate_missile_sequence(entity_t *source, uint8_t *seq, const int round) {
    int      pos         = 0;
    int      amt         = 0;
    int16_t  class_extra = 0;
    item_t *launcher = source->inv ? source->inv + SLOT_MISSILE : NULL;
    item_t *ammo = source->inv ? source->inv + SLOT_AMMO : NULL;

    if (!launcher || !launcher->name || !launcher->name[0]) { goto missile_seq_end; }
    amt = sol_dnd2e_class_attack_num(source, launcher) / 2;
    amt += ((round % 2 == 1) && (sol_dnd2e_class_attack_num(source, launcher) % 2 == 1)) ? 1 : 0;
    for (int i = 0; i < amt; i++) { seq[pos++] = SLOT_MISSILE; }

missile_seq_end:
    seq[pos] = SLOT_END;
}

static int get_next_attack(entity_t *source, const int attack_num, const int round, const uint16_t type) {
    int             current_count = 0;
    int             next_max = 0;
    int             to_add = 0;
    static uint8_t  attack_sequence[16];
    static uint16_t attack_type;

    if (attack_num == 0) {
        switch (type) {
            case EA_MELEE:
                populate_melee_sequence(source, attack_sequence, round); 
                attack_type = type;
                break;
            case EA_MISSILE:
                populate_missile_sequence(source, attack_sequence, round); 
                attack_type = type;
                break;
        }
    }

    if (attack_type != type) { return -1; }

    return attack_sequence[attack_num];
}

extern int16_t dnd2e_can_melee_again(entity_t *source, const int attack_num, const int round) {
    return get_next_attack(source, attack_num, round, EA_MELEE) != -1;
}

static sol_attack_t sol_dnd2e_attack(entity_t *source, entity_t *target, const int round, const int type) {
    static sol_attack_t invalid_attack = { -1, 0 };
    sol_attack_t attack = {0, 0};
    int16_t thac0 = 20, attack_slot, damage_source_slot;
    item_t *item;

    if (!source || !target || source->stats.combat.attack_num < 0) { return invalid_attack; }

    attack_slot = get_next_attack(source, source->stats.combat.attack_num, round, type);
    //printf("attack_slot = %d\n", attack_slot);
    if (attack_slot == SLOT_END) { return invalid_attack; }
    source->stats.combat.attack_num++;

    thac0 = dnd2e_get_thac0(source, attack_slot);
    //printf("thac0 = %d (%d)\n", thac0, dnd2e_calc_ac(target));

    if (droll(20) < thac0 - dnd2e_calc_ac(target)) {
        return attack; // miss!
    }

    damage_source_slot = (attack_slot == SLOT_MISSILE) ? SLOT_AMMO : attack_slot;
    item = sol_item_get((inventory_t*) source->inv, damage_source_slot);
    if (item) {
        //printf("ITEM: %s\n", item->name);
        //printf("damage mod = %d\n", sol_dnd2e_melee_damage_mod(&source->stats));
        attack.damage = roll_damage_weapon(item);
        attack.damage += (attack_slot == SLOT_MISSILE)
                ? sol_dnd2e_range_damage_mod(&source->stats)
                : sol_dnd2e_melee_damage_mod(&source->stats);
    } else {
        attack.damage = roll_damage_innate(source->stats.attacks + (attack_slot - SLOT_INNATE0));
    }

    return attack;
}

extern sol_attack_t sol_dnd2e_range_attack(entity_t *source, entity_t *target, const int round) {
    static sol_attack_t error = { -2, 0 };
    item_t *launcher = source->inv ? source->inv + SLOT_MISSILE : NULL;
    item_t *ammo = source->inv ? source->inv + SLOT_AMMO : NULL;

    if (!launcher || !ammo || !launcher->name || !ammo->name) { return error; }
    if (!launcher->name[0] || !ammo->name[0]) { return error;}
    if (launcher->attack.range <= 1) { return error; }
    if (launcher->attack.range < entity_distance(source, target)) { return error; }

    return sol_dnd2e_attack(source, target, round, EA_MISSILE);
}

// returns the amt of damage done. 0 means miss/absorbed, negative means attack is not legit, or out of round.
extern sol_attack_t sol_dnd2e_melee_attack(entity_t *source, entity_t *target, const int round) {
    return sol_dnd2e_attack(source, target, round, EA_MELEE);
}
