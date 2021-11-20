#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEVELS (10)
#define MAX_POWERS (100)
static int32_t wizard_powers[MAX_LEVELS][MAX_POWERS];
static int32_t priest_powers[MAX_LEVELS][MAX_POWERS];
static int32_t psionic_powers[MAX_LEVELS][MAX_POWERS];
static int32_t innate_powers[MAX_POWERS];

static char power_names[1024][64];
static const char *base_path = "powers";
static int debug = 0;

/* DS1 Notes:
 * It appears Monster Summoning I, II, III, and Evard's Black Tentacles are unique to DS1.
 * There is a collision between Hold person and hold person/mammal, both should map to the same spell.
 * Same with Dispel Magic
 */
enum effect_type {
    EFFECT_POISON     = 0x0001,
    EFFECT_FIRE       = 0x0002,
    EFFECT_COLD       = 0x0004,
    EFFECT_BLUNT      = 0x0008,
    EFFECT_CUTTING    = 0x0010,
    EFFECT_POINTED    = 0x0020,
    EFFECT_ACID       = 0x0040,
    EFFECT_ELECTRIC   = 0x0080,
    EFFECT_DRAINING   = 0x0100,
    EFFECT_MAGIC      = 0x0200,
    EFFECT_MENTAL     = 0x0400,
    EFFECT_DEATH      = 0x0800,
    EFFECT_PLUS1      = 0x1000,
    EFFECT_PLUS2      = 0x2000,
    EFFECT_METAL      = 0x4000,
    EFFECT_OVER2      = 0x8000, // More than a +2
    EFFECT_MELEE      = 0x10000,
    EFFECT_MISSILE    = 0x20000,
};

static FILE *ds1_file = NULL;
static FILE *ds2_file = NULL;
static FILE *dso_file = NULL;
static FILE *dso_extra_file = NULL;
static const char *ds1_power_names[];
static size_t ds1_pos = 0;
static size_t dso_pos = 0;

static int ds1_has_power(const char *name);

typedef struct damage_info_s {
    uint8_t plus      : 5;
    uint8_t dice_plus : 3;
    uint8_t div       : 3;
    uint8_t dice      : 5;
    uint8_t sides     : 4;
    uint8_t level     : 4;
    uint8_t savable   : 1;
    int8_t save_mod   : 4;
    uint8_t save_type : 3;
} __attribute__ ((__packed__)) damage_info_t;

typedef struct power_info_s {
    uint8_t data0;
    int16_t range;
    uint8_t range_per_level;
    uint8_t dur;
    uint16_t dur_per_level;
    int16_t dur_multiplier;
    uint16_t area;
    uint8_t area_per_level;
    uint8_t target;
    int16_t cast;
    uint8_t cast_sound;
    int8_t thrown;
    uint16_t special;
    uint8_t thrown_sound;
    int16_t hit;
    uint8_t hit_sound;
    int8_t aoe_id;
    uint8_t data1;
    int8_t effect;
    uint16_t effect_type;
    damage_info_t damage;

} __attribute__ ((__packed__)) power_info_t;

typedef struct power_entry_s {
    power_info_t info;
    char name[32];
    char name2[9];
} __attribute__ ((__packed__)) power_entry_t;

static power_info_t *ds1powers;
static int ds1_has_powerp(const power_entry_t *pw);

#include "tables.c"

static power_info_t ds1pw;
static power_entry_t ds2pw;
static power_entry_t dsopw;

static char test_buf[256];
static void generate_power(power_entry_t pw);
static void generate_tables();

const char *saves[] = {
    "NONE", "Poison", "Wands", "Petrification", "Breath", "Spells", "Paralyze", "Death", "Magic"
};

static char save_buf[256];
static char* get_save() {
    if (ds1pw.damage.savable != ds2pw.info.damage.savable || ds2pw.info.damage.savable != dsopw.info.damage.savable
        || ds1pw.damage.save_mod != ds2pw.info.damage.save_mod || ds2pw.info.damage.save_mod != dsopw.info.damage.save_mod
        || ds1pw.damage.save_type != ds2pw.info.damage.save_type || ds2pw.info.damage.save_type != dsopw.info.damage.save_type) {
        fprintf(stderr, "%s has different saves (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.damage.save_type,
                ds2pw.info.damage.save_type,
                dsopw.info.damage.save_type
               );
    }
    if (!dsopw.info.damage.savable) {
        return "N/A";
    }

    if (dsopw.info.damage.save_mod) {
        if (dsopw.info.damage.save_mod > 0) {
            snprintf(save_buf, 255, "%s + %d", saves[dsopw.info.damage.save_type], dsopw.info.damage.save_mod);
        } else {
            snprintf(save_buf, 255, "%s - %d", saves[dsopw.info.damage.save_type], abs(dsopw.info.damage.save_mod));
        }
    } else {
        snprintf(save_buf, 255, "%s", saves[dsopw.info.damage.save_type]);
    }
    return save_buf;
}

static char damage[256];
static char* get_damage(damage_info_t d) {
    int pos = 0;

    if (d.div > 1) {
        pos += snprintf(damage + pos, 255 - pos, "(%d * ((%d + caster level) / %d) + %d)d",
                d.level, d.dice_plus, d.div, d.dice);
    } else {
        if (d.dice_plus) {
            if (d.div == 1) {
                pos += snprintf(damage + pos, 255 - pos, "(%d * caster level + %d)d",
                        d.dice_plus, d.dice);
            } else {
                pos += snprintf(damage + pos, 255 - pos, "%dd",
                        d.dice);
            }
        } else {
            if (d.dice > 0) {
                pos += snprintf(damage + pos, 255 - pos, "%dd", d.dice);
            } else {
                return "N/A";
            }
        }
    }
    pos += snprintf(damage + pos, 255 - pos, "%d", d.sides);

    if (d.plus) {
        if (d.div > 1) {
            pos += snprintf(damage + pos, 255 - pos, " + %d * ((%d + caster level) / %d)",
                    d.level, d.plus, d.div);
        } else {
            if (d.div == 1) {
                pos += snprintf(damage + pos, 255 - pos, " + (%d + %d) * caster level)",
                        d.level, d.plus);
            } else {
                pos += snprintf(damage + pos, 255 - pos, " + %d", d.plus);
            }
        }
    } else if (d.div) {
        if (d.div == 1) {
            if (d.level) {
                pos += snprintf(damage + pos, 255 - pos, " + (%d + %d) * caster level",
                    d.level, d.div);
            } else {
                pos += snprintf(damage + pos, 255 - pos, " + %d * caster level",
                    d.div);
            }
        }
    }

    return damage;
}

static char effect_type[256];
static char* get_effect_type() {
    int pos = 0;
    if (ds1pw.effect_type != ds2pw.info.effect_type || ds2pw.info.effect_type != dsopw.info.effect_type) {
        fprintf(stderr, "%s has different effect_type (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.effect_type,
                ds2pw.info.effect_type,
                dsopw.info.effect_type
               );
    }
    if (dsopw.info.effect_type & 0x0001) {
        pos += snprintf(effect_type + pos, 255 - pos, "Poison ");
    }
    if (dsopw.info.effect_type & 0x0002) {
        pos += snprintf(effect_type + pos, 255 - pos, "Fire ");
    }
    if (dsopw.info.effect_type & 0x0004) {
        pos += snprintf(effect_type + pos, 255 - pos, "Cold ");
    }
    if (dsopw.info.effect_type & 0x0008) {
        pos += snprintf(effect_type + pos, 255 - pos, "Blunt ");
    }
    if (dsopw.info.effect_type & 0x0010) {
        pos += snprintf(effect_type + pos, 255 - pos, "Cutting ");
    }
    if (dsopw.info.effect_type & 0x0020) {
        pos += snprintf(effect_type + pos, 255 - pos, "Pointed ");
    }
    if (dsopw.info.effect_type & 0x0040) {
        pos += snprintf(effect_type + pos, 255 - pos, "Acid ");
    }
    if (dsopw.info.effect_type & 0x0080) {
        pos += snprintf(effect_type + pos, 255 - pos, "Electric ");
    }
    if (dsopw.info.effect_type & 0x0100) {
        pos += snprintf(effect_type + pos, 255 - pos, "Draining ");
    }
    if (dsopw.info.effect_type & 0x0200) {
        pos += snprintf(effect_type + pos, 255 - pos, "Magic ");
    }
    if (dsopw.info.effect_type & 0x0400) {
        pos += snprintf(effect_type + pos, 255 - pos, "Mental ");
    }
    if (dsopw.info.effect_type & 0x0800) {
        pos += snprintf(effect_type + pos, 255 - pos, "Death ");
    }
    if (dsopw.info.effect_type & 0x1000) {
        pos += snprintf(effect_type + pos, 255 - pos, "+1 ");
    }
    if (dsopw.info.effect_type & 0x2000) {
        pos += snprintf(effect_type + pos, 255 - pos, "+2 ");
    }
    if (dsopw.info.effect_type & 0x4000) {
        pos += snprintf(effect_type + pos, 255 - pos, "metal ");
    }
    if (dsopw.info.effect_type & 0x8000) {
        pos += snprintf(effect_type + pos, 255 - pos, ">+2 ");
    }

    return effect_type;
}

char effect_id[256];
static char* get_effect() {
    snprintf(effect_id, 255, "%d", dsopw.info.effect);

    return effect_id;
}

static char aoe_id[256];
static char* get_aoe_id() {
    if (ds1pw.aoe_id != ds2pw.info.aoe_id || ds2pw.info.aoe_id != dsopw.info.aoe_id) {
        fprintf(stderr, "%s has different aoe_id (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.aoe_id,
                ds2pw.info.aoe_id,
                dsopw.info.aoe_id
               );
    }

    snprintf(aoe_id, 255, "%d", dsopw.info.aoe_id);

    return aoe_id;
}

static char target_buf[256];
static char* get_target() {
    if (ds1pw.target != ds2pw.info.target || ds2pw.info.target != dsopw.info.target) {
        fprintf(stderr, "%s has different target (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.target,
                ds2pw.info.target,
                dsopw.info.target
               );
    }

    switch (dsopw.info.target) {
        case 0: snprintf(target_buf, 255, "No target"); break;
        case 1: snprintf(target_buf, 255, "Single"); break;
        case 2: snprintf(target_buf, 255, "Line"); break;
        case 3: snprintf(target_buf, 255, "Ally"); break;
        case 4: snprintf(target_buf, 255, "Enemy"); break;
        case 5: snprintf(target_buf, 255, "Anyone"); break;
        case 6: snprintf(target_buf, 255, "Cone/direction"); break;
        case 7: snprintf(target_buf, 255, "Self"); break;
        case 8: snprintf(target_buf, 255, "Two?"); break;
        default: snprintf(target_buf, 255, "? (%d)", dsopw.info.target); break;
    }

    return target_buf;
}

static char hit_buf[256];
static char* get_hit() {
    if (ds1pw.hit != ds2pw.info.hit || ds2pw.info.hit != dsopw.info.hit) {
        fprintf(stderr, "%s has different hit (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.hit,
                ds2pw.info.hit,
                dsopw.info.hit
               );
    }
    if (ds2pw.info.hit_sound != dsopw.info.hit_sound) {
        fprintf(stderr, "%s has different hit_sound (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.hit_sound,
                ds2pw.info.hit_sound,
                dsopw.info.hit_sound
               );
    }
    if (!ds1_pos) {
        snprintf(hit_buf, 255, "%d, --- , %d",
            dsopw.info.hit, dsopw.info.hit_sound
            );
    } else {
        snprintf(hit_buf, 255, "%d, %d, %d",
            dsopw.info.hit, ds1pw.hit_sound, dsopw.info.hit_sound
            );
    }
    return hit_buf;
}

static char special_buf[256];
static char* get_special() {
    char pos = 0;
    if (ds1pw.special != ds2pw.info.special || ds2pw.info.special != dsopw.info.special) {
        fprintf(stderr, "%s has different special (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.special,
                ds2pw.info.special,
                dsopw.info.special
               );
    }
    if (dsopw.info.special & 0x0001) {
        pos += snprintf(special_buf + pos, 255 - pos, "Summon ");
    }
    if (dsopw.info.special & 0x0002) {
        pos += snprintf(special_buf + pos, 255 - pos, "Enchant ");
    }
    if (dsopw.info.special & 0x0004) {
        pos += snprintf(special_buf + pos, 255 - pos, "Charm ");
    }
    if (dsopw.info.special & 0x0008) {
        pos += snprintf(special_buf + pos, 255 - pos, "Illusion "); // can disbelieve
    }
    if (dsopw.info.special & 0x0010) {
        pos += snprintf(special_buf + pos, 255 - pos, "wis-adj "); // apply wisdom adjustment to save
    }
    if (dsopw.info.special & 0x0020) {
        pos += snprintf(special_buf + pos, 255 - pos, "mind/wis-roll "); // must roll wisdom to save, undead immune
    }
    if (dsopw.info.special & 0x0040) {
        pos += snprintf(special_buf + pos, 255 - pos, "Roll-hit "); // must roll to hit to work.
    }
    if (dsopw.info.special & 0x0080) {
        pos += snprintf(special_buf + pos, 255 - pos, "Gaze-attack ");
    }
    if (dsopw.info.special & 0x0100) {
        pos += snprintf(special_buf + pos, 255 - pos, "Fear-attack ");
    }
    if (dsopw.info.special & 0x0200) {
        pos += snprintf(special_buf + pos, 255 - pos, "(0x200?) ");
    }
    if (dsopw.info.special & 0x0400) {
        pos += snprintf(special_buf + pos, 255 - pos, "Fire-immune ");
    }
    if (dsopw.info.special & 0x0800) {
        pos += snprintf(special_buf + pos, 255 - pos, "Water-immune ");
    }
    if (dsopw.info.special & 0x1000) {
        pos += snprintf(special_buf + pos, 255 - pos, "Earth-immune ");
    }
    if (dsopw.info.special & 0x2000) {
        pos += snprintf(special_buf + pos, 255 - pos, "Air-immune ");
    }
    if (dsopw.info.special & 0x4000) {
        pos += snprintf(special_buf + pos, 255 - pos, "is-attack "); // cause chang ein allegiance
    }
    return special_buf;
}

static char thrown_buf[256];
static char* get_thrown() {
    if (ds1pw.thrown != ds2pw.info.thrown || ds2pw.info.thrown != dsopw.info.thrown) {
        fprintf(stderr, "%s has different thrown (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.thrown,
                ds2pw.info.thrown,
                dsopw.info.thrown
               );
    }
    if (ds2pw.info.thrown_sound != dsopw.info.thrown_sound) {
        fprintf(stderr, "%s has different thrown_sound (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.thrown_sound,
                ds2pw.info.thrown_sound,
                dsopw.info.thrown_sound
               );
    }
    if (!ds1_pos) {
        snprintf(thrown_buf, 255, "%d, --- , %d",
            dsopw.info.thrown, dsopw.info.thrown_sound
            );
    } else {
        snprintf(thrown_buf, 255, "%d, %d, %d",
            dsopw.info.thrown, ds1pw.thrown_sound, dsopw.info.thrown_sound
            );
    }
    return thrown_buf;
}

static char cast_buf[256];
static char* get_cast(power_entry_t pw) {
    if (ds1pw.cast != ds2pw.info.cast || ds2pw.info.cast != dsopw.info.cast) {
        fprintf(stderr, "%s has different cast (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.cast,
                ds2pw.info.cast,
                dsopw.info.cast
               );
    }
    //if (ds1pw.cast_sound != ds2pw.info.cast_sound || ds2pw.info.cast_sound != dsopw.info.cast_sound) {
    if (ds2pw.info.cast_sound != dsopw.info.cast_sound) {
        fprintf(stderr, "%s has different cast_sound (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.cast_sound,
                ds2pw.info.cast_sound,
                dsopw.info.cast_sound
               );
    }
    if (!ds1_pos) {
        snprintf(cast_buf, 255, "%d, --- , %d",
            pw.info.cast, pw.info.cast_sound
            );
    } else {
        snprintf(cast_buf, 255, "%d, %d, %d",
            pw.info.cast, ds1pw.cast_sound, pw.info.cast_sound
            );
    }
    return cast_buf;
}

static char area_buf[256];
static char* get_area(power_entry_t pw) {
    if (ds1pw.area != ds2pw.info.area || ds2pw.info.area != dsopw.info.area) {
        fprintf(stderr, "%s has different areas (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.area,
                ds2pw.info.area,
                dsopw.info.area
               );
    }
    snprintf(area_buf, 255, "%d + %d per level",
            pw.info.area, pw.info.area_per_level
            );

    return area_buf;
}

static char duration_buf[256];
static char* get_duration(power_entry_t pw) {
    if (ds1pw.dur != ds2pw.info.dur || ds2pw.info.dur != dsopw.info.dur) {
        fprintf(stderr, "%s has different durations (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.dur,
                ds2pw.info.dur,
                dsopw.info.dur
               );
    }
    if (ds1pw.dur_per_level != ds2pw.info.dur_per_level || ds2pw.info.dur_per_level != dsopw.info.dur_per_level) {
        fprintf(stderr, "%s has different dur_per_level (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.dur_per_level,
                ds2pw.info.dur_per_level,
                dsopw.info.dur_per_level
               );
    }
    if (ds1pw.dur_multiplier != ds2pw.info.dur_multiplier || ds2pw.info.dur_multiplier != dsopw.info.dur_multiplier) {
        fprintf(stderr, "%s has different dur_multiplier (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.dur_multiplier,
                ds2pw.info.dur_multiplier,
                dsopw.info.dur_multiplier
               );
    }
    if (pw.info.dur_multiplier == 0) {
        snprintf(duration_buf, 255, "N/A");
    } else if (pw.info.dur_multiplier == -9999) {
        snprintf(duration_buf, 255, "indefinite");
    } else {
        snprintf(duration_buf, 255, "%dd%d + %d %s",
            pw.info.dur & 0x0F,
            pw.info.dur >> 4,
            pw.info.dur_per_level,
            (pw.info.dur_multiplier == 60)
                ? "round per level"
            : (pw.info.dur_multiplier == 1)
                ? "round per level"
            : (pw.info.dur_multiplier == -1)
                ? "interaction"
            : (pw.info.dur_multiplier == 3600)
                ? "* 6 turns"
            : (pw.info.dur_multiplier == 600)
                ? "* 1 turns"
                : "?"
            );
    }
    return duration_buf;
}

static char range_buf[256];
static char* get_range(power_entry_t pw) {
    if (ds1pw.range != ds2pw.info.range || ds2pw.info.range != dsopw.info.range) {
        fprintf(stderr, "%s has different range (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.range,
                ds2pw.info.range,
                dsopw.info.range
               );
    }
    if (pw.info.range == -2) { return "touch"; }
    if (pw.info.range == -1) { return "self"; }
    if (pw.info.range < 0) { return "unknown"; }
    if (pw.info.range_per_level > 0) {
        snprintf(range_buf, 255, "%d feet + %d per level", pw.info.range, pw.info.range_per_level);
    } else {
        snprintf(range_buf, 255, "%d feet", pw.info.range);
    }
    return range_buf;
}

const char *ai[]  = {
    "attack enemy",
        "help allies",
        "attack all",
        "effect all"
};

static void generate_power_csv() {
    if (!debug) { return; }
        printf("%s, %s", dsopw.name, dsopw.name2);
        printf(", %s", get_range(dsopw));
        printf(", %s", get_duration(dsopw));
        printf(", %s", get_area(dsopw));
        printf(", %s", get_target());
        printf(", %s", get_cast(dsopw));
        printf(", %s", get_special());
        printf(", %s", get_thrown());
        printf(", %s", get_hit());
        printf(", %s", get_aoe_id());
        printf(", %s", get_effect());
        printf(", %s", get_effect_type());

        if (!ds1_pos) {
            printf(", ---");
        } else {
            printf(", %s", get_damage(ds1pw.damage));
        }
        printf(", %s", get_damage(ds2pw.info.damage));
        printf(", %s", get_damage(dsopw.info.damage));
        printf(", %s", get_save());

        if (!ds1_pos) {
            printf(", ---");
        } else {
            printf(", %s %d times",
                ai[ds1pw.data0 & 0x03],
                ds1pw.data0 >> 2);
        }
        printf(", %s %d, %s %d times",
                ai[ds2pw.info.data0 & 0x03],
                ds2pw.info.data0 >> 2,
                ai[dsopw.info.data0 & 0x03],
                dsopw.info.data0 >> 2
              );
        printf("\n");
}

static void add_to_power(int32_t powers[MAX_POWERS], int32_t power) {
    size_t pos = 0;
    while(powers[pos] >= 0) { pos++; }
    powers[pos] = power;
}

int get_level() {
    if (dso_pos < 12) { return 1;
    } else if (dso_pos < 25) { return 2;
    } else if (dso_pos < 39) { return 3;
    } else if (dso_pos < 56) { return 4;
    } else if (dso_pos < 68) { return 5;
    } else if (dso_pos < 81) { return 6;
    } else if (dso_pos < 92) { return 7;
    } else if (dso_pos < 102) { return 8;
    } else if (dso_pos < 111) { return 9;
    } else if (dso_pos < 115) { return 10;
    } else if (dso_pos < 127) { return 0;
    } else if (dso_pos < 143) { return 2;
    } else if (dso_pos < 165) { return 3;
    } else if (dso_pos < 189) { return 4;
    } else if (dso_pos < 208) { return 5;
    } else if (dso_pos < 216) { return 6;
    } else if (dso_pos < 231) { return 7;
    } else if (dso_pos < 232) { return 8;
    } else if (dso_pos < 233) { return 9;
    } else if (dso_pos < 234) { return 10;
    } else if (dso_pos == 345) { return 3;
    } else if (dso_pos == 346) { return 4;
    } else if (dso_pos == 347) { return 5;
    } else if (dso_pos == 348) { return 4;
    }

    return 0;
}

void add_power() {
    if (dso_pos < 12) { add_to_power(wizard_powers[0], dso_pos);
    } else if (dso_pos < 25) { add_to_power(wizard_powers[1], dso_pos);
    } else if (dso_pos < 39) { add_to_power(wizard_powers[2], dso_pos);
    } else if (dso_pos < 56) { add_to_power(wizard_powers[3], dso_pos);
    } else if (dso_pos < 68) { add_to_power(wizard_powers[4], dso_pos);
    } else if (dso_pos < 81) { add_to_power(wizard_powers[5], dso_pos);
    } else if (dso_pos < 92) { add_to_power(wizard_powers[6], dso_pos);
    } else if (dso_pos < 102) { add_to_power(wizard_powers[7], dso_pos);
    } else if (dso_pos < 111) { add_to_power(wizard_powers[8], dso_pos);
    } else if (dso_pos < 115) { add_to_power(wizard_powers[9], dso_pos);
    } else if (dso_pos < 127) { add_to_power(priest_powers[0], dso_pos);
    } else if (dso_pos < 143) { add_to_power(priest_powers[1], dso_pos);
    } else if (dso_pos < 165) { add_to_power(priest_powers[2], dso_pos);
    } else if (dso_pos < 189) { add_to_power(priest_powers[3], dso_pos);
    } else if (dso_pos < 208) { add_to_power(priest_powers[4], dso_pos);
    } else if (dso_pos < 216) { add_to_power(priest_powers[5], dso_pos);
    } else if (dso_pos < 231) { add_to_power(priest_powers[6], dso_pos);
    } else if (dso_pos < 232) { add_to_power(priest_powers[7], dso_pos);
    } else if (dso_pos < 233) { add_to_power(priest_powers[8], dso_pos);
    } else if (dso_pos < 234) { add_to_power(priest_powers[9], dso_pos);
    } else if (dso_pos < 238) { add_to_power(psionic_powers[0], dso_pos); // Kinetic Sciences
    } else if (dso_pos < 241) { add_to_power(psionic_powers[1], dso_pos); // Kinetic Devotions
    } else if (dso_pos < 244) { add_to_power(psionic_powers[2], dso_pos); // Metabolism Sciences
    } else if (dso_pos < 255) { add_to_power(psionic_powers[3], dso_pos); // Metabolism Devotions
    } else if (dso_pos < 260) { add_to_power(psionic_powers[4], dso_pos); // Telepathic Sciences
    } else if (dso_pos < 269) { add_to_power(psionic_powers[5], dso_pos); // Telepathic Devotions
    } else if (dso_pos < 345) { add_to_power(innate_powers, dso_pos); // Monster/innate powers
    } else if (dso_pos == 345) { add_to_power(wizard_powers[2], dso_pos); // Monster Summoning I
    } else if (dso_pos == 346) { add_to_power(wizard_powers[3], dso_pos); // Monster Summoning II
    } else if (dso_pos == 347) { add_to_power(wizard_powers[4], dso_pos); // Monster Summoning III
    } else if (dso_pos == 348) { add_to_power(wizard_powers[3], dso_pos); // Evard's Black Tentacles
    } else {
        fprintf(stderr, "ERRRRRRRRRRRRRR: unknown power: %ld\n", dso_pos);
    }
}

int main(int argc, char *argv[]) {
    ds1_file = fopen("dsun1.dat", "rb");
    ds2_file = fopen("dsun2.dat", "rb");
    dso_file = fopen("dso.dat", "rb");
    dso_extra_file = fopen("dso-extra.dat", "rb");
    size_t ds1_file_size = 0;

    memset(wizard_powers, -1, sizeof(int32_t) * MAX_LEVELS * MAX_POWERS);
    memset(priest_powers, -1, sizeof(int32_t) * MAX_LEVELS * MAX_POWERS);
    memset(psionic_powers, -1, sizeof(int32_t) * MAX_LEVELS * MAX_POWERS);
    memset(innate_powers, -1, sizeof(int32_t) * MAX_POWERS);

    if (!ds1_file) {
        fprintf(stderr, "Unable to open dsun2.dat, please provide.\n");
        return 0;
    }

    if (!ds2_file) {
        fprintf(stderr, "Unable to open dsun2.dat, please provide.\n");
        return 0;
    }

    if (!dso_file) {
        fprintf(stderr, "You need to provide the chunk containing the powers as dso.dat\n");
        return 0;
    }

    if (!dso_file) {
        fprintf(stderr, "You need to provide the dso-extra.dat\n");
        return 0;
    }

    if (argc > 1) {
        base_path = argv[1];
    }
    debug = (argc > 2);
    printf("usage: %s <path to output> <print csv to screen>\n", argv[0]);

    //power_info_t *ds1powers;
    fseek(ds1_file, 0L, SEEK_END);
    ds1_file_size = ftell(ds1_file);
    ds1powers = malloc(ds1_file_size);
    fseek(ds1_file, 0L, SEEK_SET);

    fread(ds1powers, 1, ds1_file_size, ds1_file);

    if (debug) {
        printf("Name, Subname, range, duration, area, target, cast (?), ds1 cast sound, ds2/dso cast sound,"
            " special, thrown (?), ds1 thrown sound, ds2/dso thrown sound, hit (?),"
            " ds1 hit sound, ds2/dso hit sound, aoe id, effect id,"
            " effect type, ds1 damage, ds2 damage, dso damage, save,"
            " ds1 ai, ds2 ai, dso ai \n");
    }
    while (!feof(dso_file) || !feof(dso_extra_file)) {
        if (!feof(dso_file)) {
            if (fread(&dsopw, 1, sizeof(power_entry_t), dso_file) < sizeof(power_entry_t)) {
                continue;
            }
            fread(&ds2pw, 1, sizeof(power_entry_t), ds2_file);
            ds1_pos = ds1_has_power(dsopw.name);
            if (ds1_pos) {
                ds1pw = ds1powers[ds1_pos - 1];
            } else {
                ds1pw = dsopw.info;
            }
        } else {
            if (fread(&dsopw, 1, sizeof(power_entry_t), dso_extra_file) < sizeof(power_entry_t)) {
                continue;
            }
            ds2pw = dsopw;
            ds1pw = dsopw.info;
            ds1_pos = 9999;
        }
        generate_power_csv();
        generate_power(dsopw);
        add_power();
        dso_pos++;
    }

    // Manually generate the 4 powers left over from DS1:
    ds1pw = ds1powers[35];
    dso_pos = 345;
    dsopw.info = ds1pw;
    strcpy(dsopw.name, "MONSTER SUMMONING I");
    strcpy(dsopw.name2, "MONSUMM1");
    ds2pw = dsopw;
    generate_power_csv();
    generate_power(dsopw);
    add_power();

    ds1pw = ds1powers[49];
    dso_pos = 346;
    dsopw.info = ds1pw;
    strcpy(dsopw.name, "MONSTER SUMMONING II");
    strcpy(dsopw.name2, "MONSUMM2");
    ds2pw = dsopw;
    generate_power_csv();
    generate_power(dsopw);
    add_power();

    ds1pw = ds1powers[65];
    dso_pos = 347;
    dsopw.info = ds1pw;
    strcpy(dsopw.name, "MONSTER SUMMONING III");
    strcpy(dsopw.name2, "MONSUMM2");
    ds2pw = dsopw;
    generate_power_csv();
    generate_power(dsopw);
    add_power();

    ds1pw = ds1powers[65];
    dso_pos = 348;
    dsopw.info = ds1pw;
    strcpy(dsopw.name, "EVARD'S BLACK TENTACLES");
    strcpy(dsopw.name2, "EVARDSBT");
    ds2pw = dsopw;
    generate_power_csv();
    generate_power(dsopw);
    add_power();

    generate_tables();

    fclose(ds1_file);
    fclose(ds2_file);
    fclose(dso_file);
    return 0;
}

static int ds1_has_powerp(const power_entry_t *pw) {
    for (int i = 0; i < 900; i++) {
        int ds1_len = strlen(ds1_power_names[i]);
        //printf("POW: '%s' ?= '%s'\n", ds1_power_names[i], pw->name);
        if (!strncmp(ds1_power_names[i], pw->name, ds1_len)) { return i; }
        if (!strncmp(ds1_power_names[i], pw->name2, ds1_len)) { return i; }
    }

    return 0;
}

static int ds1_has_power(const char *name) {
    int name_len = strlen(name);

    for (int i = 0; i < 900; i++) {
        int ds1_len = strlen(ds1_power_names[i]);
        if (ds1_len != name_len) { continue; }
        if (!strncmp(ds1_power_names[i], name, ds1_len)) { return i; }
    }

    return 0;
}

static int priest_index(power_entry_t pw) {
    const char **ptr = priest_spells;
    int pos = 0;

    while (*ptr) {
        if (!strcmp(pw.name2, *ptr)) { return pos; }
        ptr++;
        pos++;
    }

    return -1;
}

static int wizard_index(power_entry_t pw) {
    const char **ptr = wizard_spells;
    int pos = 0;

    while (*ptr) {
        if (!strcmp(pw.name2, *ptr)) { return pos; }
        ptr++;
        pos++;
    }

    return -1;
}

static int psionic_index(power_entry_t pw) {
    const char **ptr = psionics;
    int pos = 0;

    while (*ptr) {
        if (!strcmp(pw.name2, *ptr)) { return pos; }
        ptr++;
        pos++;
    }

    return -1;
}

static int innate_index(power_entry_t pw) {
    const char **fptr = innate_power_full;
    const char **sptr = innate_power_short;
    int pos = 0;

    while (*fptr) {
        if ((*sptr)[0] == ' ' && (*sptr)[1] == '\0') {
            if (!strcmp(pw.name, *fptr)) { return pos; }
        } else {
            if (!strcmp(pw.name2, *sptr)) { return pos; }
        }

        fptr++;
        sptr++;
        pos++;
    }

    return -1;
}

static int psi, wiz, pri, innate;
static char type[64];
static void generate_activate(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_can_activate (power_instance_t *source, const int16_t power_level) {\n", type, name);
    fprintf(file, "    if (!source || !source->entity) { return 0;}\n");
    
    if (psi >= 0) {
    } else if (wiz >= 0) {
        fprintf(file, "    return entity_has_wizard_slot(source->entity, power_level);\n");
    } else if (pri >= 0) {
        fprintf(file, "    return entity_has_priest_slot(source->entity, power_level);\n");
    } else if (psi >= 0) {
        fprintf(file, "    NEED TO DO PSI CALC!\n");
    } else if (innate >= 0) {
        fprintf(file, "    return 1;// innates can always activate\n");
    } else {
        fprintf(file, "    return ???;\n");
    }

    fprintf(file, "}\n");
}

static void generate_pay(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_pay          (power_instance_t *source, const int16_t power_level) {\n", type, name);
    fprintf(file, "    if (!source || !source->entity) { return 0;}\n");
    
    if (psi >= 0) {
    } else if (wiz >= 0) {
        fprintf(file, "    return entity_take_wizard_slot(source->entity, power_level);\n");
    } else if (pri >= 0) {
        fprintf(file, "    return entity_take_priest_slot(source->entity, power_level);\n");
    } else if (psi >= 0) {
        fprintf(file, "    NEED TO DO PSI CALC!\n");
    } else if (innate >= 0) {
        fprintf(file, "    return 1;// innates can always pay\n");
    } else {
        fprintf(file, "    return ???;\n");
    }

    fprintf(file, "}\n");
}

const char *effect_names[] = {
    "EFFECT_POISON",
    "EFFECT_FIRE",
    "EFFECT_COLD",
    "EFFECT_BLUNT",
    "EFFECT_CUTTING",
    "EFFECT_POINTED",
    "EFFECT_ACID",
    "EFFECT_ELECTRIC",
    "EFFECT_DRAINING",
    "EFFECT_MAGIC",
    "EFFECT_MENTAL",
    "EFFECT_DEATH",
    "EFFECT_PLUS1",
    "EFFECT_PLUS2",
    "EFFECT_METAL",
    "EFFECT_OVER2",
};

static void generate_apply(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic void %s_%s_apply        (power_instance_t *source, entity_t *entity) {\n", type, name);
    fprintf(file, "    if (!source || !entity) { return; }\n");
    fprintf(file, "    size_t num_dice = 0, mod = 0, damage = 0;\n");
    fprintf(file, "    uint64_t effect_type = 0;\n");
    fprintf(file, "    int caster_level = (source->entity) ? entity_get_%s_level(source->entity)\n", type);
    fprintf(file, "            : item_get_%s_level(source->item);\n\n", type);
    
    fprintf(file, "    num_dice = ");
    if (pw.info.damage.div > 1) {
        fprintf(file, "(%d * ((%d + caster_level) / %d) + %d)",
                pw.info.damage.level, pw.info.damage.dice_plus, pw.info.damage.div, pw.info.damage.dice);
    } else {
        if (pw.info.damage.dice_plus) {
            if (pw.info.damage.div == 1) {
                fprintf(file, "(%d * caster_level + %d)",
                        pw.info.damage.dice_plus, pw.info.damage.dice);
            } else {
                fprintf(file, "%d", pw.info.damage.dice);
            }
        } else {
            if (pw.info.damage.dice > 0) {
                fprintf(file, "%d", pw.info.damage.dice);
            } else {
                fprintf(file, "0");
            }
        }
    }
    fprintf(file, ";\n");

    fprintf(file, "    mod = ");
    if (pw.info.damage.plus) {
        if (pw.info.damage.div > 1) {
            fprintf(file, " %d * ((%d + caster_level) / %d)",
                    pw.info.damage.level, pw.info.damage.plus, pw.info.damage.div);
        } else {
            if (pw.info.damage.div == 1) {
                fprintf(file , "(%d + %d) * caster_level",
                        pw.info.damage.level, pw.info.damage.plus);
            } else {
                fprintf(file, "%d", pw.info.damage.plus);
            }
        }
    } else if (pw.info.damage.div == 1) {
        if (pw.info.damage.level) {
            fprintf(file, "(%d + %d) * caster_level",
                pw.info.damage.level, pw.info.damage.div);
        } else {
            fprintf(file, "%d * caster_level", pw.info.damage.div);
        }
    } else {
        fprintf(file, "0");
    }
    fprintf(file, ";\n");

    fprintf(file, "    damage = dnd2e_dice_roll(num_dice, %d) + mod;\n", pw.info.damage.sides);
    fprintf(file, "    effect_type = 0");

    int is_first = 1;
    for (int i = 0; i < 16; i++) {
        if (pw.info.effect_type & (1<<i)) {
            fprintf(file, " | %s", effect_names[i]);
            is_first = 0;
        }
    }
    fprintf(file, ";\n");

    if (pw.info.damage.sides > 0) {
        fprintf(file, "    sol_effect_apply_damage(source->entity, entity, damage, effect_type);\n");
    }

    fprintf(file, "}\n");
}

static void generate_affect(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_affect_power (power_instance_t *target) {\n", type, name);
    fprintf(file, "    if (!target) { return 0;}\n");
    
    if (psi >= 0) {
    } else if (wiz >= 0) {
    } else if (pri >= 0) {
    } else {
    }

    fprintf(file, "    return 0; // Doesn't affect powers.\n");

    fprintf(file, "}\n");
}


static void generate_update(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_update       (power_t *power, power_instance_t *source) {\n", type, name);
    fprintf(file, "    if (!power || ! source) { return 0;}\n");
    fprintf(file, "    entity_t *entity = source->entity;\n");
    //fprintf(file, "    int made_save = 0;\n");
    //fprintf(file, "    item_t *item = source->item;\n\n");
    fprintf(file, "\n");

    fprintf(file, "    if (entity) {\n");
    fprintf(file, "        power->range = %d",
        (dsopw.info.range) < 0
        ? 0
        : dsopw.info.range
        );
    if (pw.info.range_per_level > 0) {
        fprintf(file, " + (%d * entity_get_%s_level(entity))",
            dsopw.info.range_per_level, type);
    }
        //snprintf(range_buf, 255, "%d feet + %d per level", pw.info.range, pw.info.range_per_level);
    fprintf(file, ";\n");
    fprintf(file, "        power->aoe = %d",
        (dsopw.info.area) < 0
        ? 0
        : dsopw.info.area
        );
    if (pw.info.area_per_level > 0) {
        fprintf(file, " + (%d * entity_get_%s_level(entity))",
            dsopw.info.area_per_level, type);
    }
        //snprintf(range_buf, 255, "%d feet + %d per level", pw.info.range, pw.info.range_per_level);
    fprintf(file, ";\n");
    fprintf(file, "    }\n\n");

    if (pw.info.damage.savable) {
        //pw.info.damage.save_type
    }

    fprintf(file, "    return 1;\n");
    fprintf(file, "}\n");
}

static char* get_target_shape(power_entry_t pw) {
    switch (pw.info.target) {
        case 0: return "TARGET_NONE";
        case 1: return "TARGET_SINGLE";
        case 2: return "TARGET_RECTANGLE";
        case 3: return "TARGET_ALLY";
        case 4: return "TARGET_ENEMY";
        case 5: return "TARGET_ANY";
        case 6: return "TARGET_CONE";
        case 7: return "TARGET_SELF";
        case 8: return "TARGET_MULTI";
        default:break;
    }
    return "ERROR";
}

static void generate_setup(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nextern void %s_%s_setup  (power_t *power) {\n", type, name);
    fprintf(file, "    power->name                 = \"%s\";\n", pw.name);
    fprintf(file, "    power->description          = power_spin_read_description(power_select_by_game(%d, %d, %d));\n",
            (!ds1_has_powerp(&pw))
            ? -1
            : (dso_pos < MAX_DSO_ICONS)
            ? ds1_icons[dso_pos].spin
            : -1,
            (dso_pos < MAX_DSO_ICONS)
            ? dso_icons[dso_pos].spin
            : -1,
            (dso_pos < MAX_DSO_ICONS)
            ? dso_icons[dso_pos].spin
            : -1);
    fprintf(file, "    power->range                = -99999;\n");
    fprintf(file, "    power->aoe                  = -99999;\n");
    fprintf(file, "    power->level                = %d;\n", get_level());
    fprintf(file, "    power->shape                = %s;\n", get_target_shape(pw));
    fprintf(file, "    power->cast_sound           = power_select_by_game(%d, %d, %d);\n",
            ds1pw.cast_sound, ds2pw.info.cast_sound, dsopw.info.cast_sound);
    fprintf(file, "    power->thrown_sound         = power_select_by_game(%d, %d, %d);\n",
            ds1pw.thrown_sound, ds2pw.info.thrown_sound, dsopw.info.thrown_sound);
    fprintf(file, "    power->hit_sound            = power_select_by_game(%d, %d, %d);\n",
            ds1pw.hit_sound, ds2pw.info.hit_sound, dsopw.info.hit_sound);
    fprintf(file, "    power->actions.can_activate = %s_%s_can_activate;\n", type, name);
    fprintf(file, "    power->actions.pay          = %s_%s_pay;\n", type, name);
    fprintf(file, "    power->actions.apply        = %s_%s_apply;\n", type, name);
    fprintf(file, "    power->actions.affect_power = %s_%s_affect_power;\n", type, name);
    fprintf(file, "    powers_set_icon(power, power_select_by_game(%d, %d, %d));\n",
            (dso_pos < MAX_DSO_ICONS) 
            ? ds1_icons[dso_pos].icon
            : -1,
            (dso_pos < MAX_DSO_ICONS)
            ? dso_icons[dso_pos].icon
            : -1,
            (dso_pos < MAX_DSO_ICONS)
            ? dso_icons[dso_pos].icon
            : -1);
    fprintf(file, "    powers_set_cast(power, power_select_by_game(%d, %d, %d)); // OJFF\n",
            ds1pw.cast, ds2pw.info.cast, dsopw.info.cast);
    fprintf(file, "    powers_set_thrown(power, power_select_by_game(%d, %d, %d)); // OJFF.\n",
            8100 + ds1pw.thrown,
            8100 + ds2pw.info.thrown,
            8100 + dsopw.info.thrown
            );
    fprintf(file, "    powers_set_hit(power, power_select_by_game(%d, %d, %d)); // OJFF\n",
            ds1pw.hit, ds2pw.info.hit, dsopw.info.hit);
    fprintf(file, "}\n");
}

static void generate_tables() {
    FILE *file = fopen ("powers-generator.c", "wb");
    if (!file) { return; }

    fprintf(file, "// This file is generated.\n");
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include \"powers.h\"\n");
    fprintf(file, "#include \"wizard.h\"\n");
    fprintf(file, "\n");
    for (int i = 0; i < MAX_LEVELS; i++) {
        for (int j = 0; j < MAX_POWERS; j++) {
            if (wizard_powers[i][j] >= 0) {
                fprintf(file, "extern void wizard_%s_setup(power_t *p);\n", power_names[wizard_powers[i][j]]);
            }
        }
        fprintf(file, "\n");
    }

    fprintf(file, "\nvoid wizard_setup_powers() {\n");
    fprintf(file, "    power_t *p;\n");
    for (int i = 0; i < MAX_LEVELS; i++) {
        for (int j = 0; j < MAX_POWERS; j++) {
            if (wizard_powers[i][j] >= 0) {
                fprintf(file, "    p = power_create();\n");
                fprintf(file, "    wizard_%s_setup(p);\n", power_names[wizard_powers[i][j]]);
                fprintf(file, "    wizard_add_power(%d, p);\n", i + 1);
            }
        }
        fprintf(file, "\n");
    }
    fprintf(file, "}\n");

    fclose(file);
}

static void generate_power(power_entry_t pw) {
    char path[128];
    char filename[64];
    char name[64];
    FILE *file = NULL;
    int pos = 0;

    psi = psionic_index(pw);
    wiz = wizard_index(pw);
    pri = priest_index(pw);
    innate = innate_index(pw);

    (psi >= 0)
        ? strcpy(type, "psionic")
        : (wiz >= 0)
        ? strcpy(type, "wizard")
        : (pri >= 0)
        ? strcpy(type, "priest")
        : (innate >= 0)
        ? strcpy(type, "innate")
        : strcpy(type, "unknown")
        ;

    for (int i = 0; i < 64 && pw.name[i]; i++) {
        if (pw.name[i] == '\'') { continue; }
        name[pos] = toupper(pw.name[i]);
        filename[pos] = tolower(pw.name[i]);
        if (filename[pos] == ' ') { filename[pos] = '-'; }
        if (name[pos] == ' ') { name[pos] = '_'; }
        pos++;
    }
    filename[pos] = '\0';
    name[pos] = '\0';

    //snprintf(path, 127, "powers/%s-%s.c", type, filename);
    snprintf(path, 127, "%s/%s-%s.c", base_path, type, filename);
    file = fopen(path, "wb+");

    if (!file) {
        fprintf(stderr, "Unable to overwrite '%s'\n", path);
    }

    for (int i = 0; i < 64 && type[i]; i++) {
        type[i] = toupper(type[i]);
    }

    fprintf(file, "/* This file is auto-generated. */\n");
    fprintf(file, "#ifndef %s_%s_POWER_H\n", type, name);
    fprintf(file, "#define %s_%s_POWER_H\n\n", type, name);
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include \"effect.h\"\n");
    fprintf(file, "#include \"entity.h\"\n");
    fprintf(file, "#include \"rules.h\"\n");
    fprintf(file, "#include \"wizard.h\"\n");

    for (int i = 0; i < 64 && name[i]; i++) {
        name[i] = tolower(name[i]);
    }
    for (int i = 0; i < 64 && type[i]; i++) {
        type[i] = tolower(type[i]);
    }

    strcpy(power_names[dso_pos], name);
    generate_activate(pw, name, file);
    generate_pay(pw, name, file);
    generate_apply(pw, name, file);
    generate_affect(pw, name, file);
    generate_update(pw, name, file);
    generate_setup(pw, name, file);

    fprintf(file, "\n#endif\n");
    fclose(file);
};
