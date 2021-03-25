#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* DS1 Notes:
 * It appears Monster Summoning I and Evard's Black Tentacles are unique to DS1.
 * There is acollision between Hold person and hold person/mammal, both should map to the same spell.
 * Same with Dispel Magic
 */

static FILE *ds1_file = NULL;
static FILE *ds2_file = NULL;
static FILE *dso_file = NULL;
static FILE *dso_extra_file = NULL;
static const char *ds1_power_names[];
static size_t ds1_pos = 0;

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

static power_info_t ds1pw;
static power_entry_t ds2pw;
static power_entry_t dsopw;

static char test_buf[256];
static void generate_power(power_entry_t pw);

const char *saves[] = {
    "NONE", "Poison", "Wands", "Petrification", "Breath", "Spells", "Paralyze", "Death", "Magic"
};

static char save_buf[256];
static char* get_save() {
    //uint8_t savable   : 1;
    //uint8_t save_mod  : 4;
    //uint8_t save_type : 3;
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
//typedef struct damage_info_s {
    //uint8_t plus      : 5;
    //uint8_t dice_plus : 3;
    //uint8_t div       : 3;
    //uint8_t dice      : 5;
    //uint8_t sides     : 4;
    //uint8_t level     : 4;
    //uint8_t savable   : 1;
    //uint8_t save_mod  : 4;
    //uint8_t save_type : 3;
//} __attribute__ ((__packed__)) damage_info_t;
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
        pos += snprintf(effect_type + pos, 255 - pos, "Magic-fire ");
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
    /*
    if (ds1pw.effect != ds2pw.info.effect || ds2pw.info.effect != dsopw.info.effect) {
        fprintf(stderr, "%s has different effect (%d, %d, %d).\n",
                dsopw.name,
                ds1pw.effect,
                ds2pw.info.effect,
                dsopw.info.effect
               );
    }
    */
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

int main() {
    ds1_file = fopen("dsun1.dat", "rb");
    ds2_file = fopen("dsun2.dat", "rb");
    dso_file = fopen("dso.dat", "rb");
    dso_extra_file = fopen("dso-extra.dat", "rb");
    size_t ds1_file_size = 0;

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

    //power_info_t *ds1powers;
    fseek(ds1_file, 0L, SEEK_END);
    ds1_file_size = ftell(ds1_file);
    ds1powers = malloc(ds1_file_size);
    fseek(ds1_file, 0L, SEEK_SET);

    fread(ds1powers, 1, ds1_file_size, ds1_file);

    printf("Name, Subname, range, duration, area, target, cast (?), ds1 cast sound, ds2/dso cast sound,"
            " special, thrown (?), ds1 thrown sound, ds2/dso thrown sound, hit (?),"
            " ds1 hit sound, ds2/dso hit sound, aoe id, effect id,"
            " effect type, ds1 damage, ds2 damage, dso damage, save,"
            " ds1 ai, ds2 ai, dso ai \n");
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

        generate_power(dsopw);
    }

    fclose(ds1_file);
    fclose(ds2_file);
    fclose(dso_file);
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

static const char *priest_spells[] = { "BLESS", "CURSE", "CURLTWND", "CASLTWND", "ENTANGLE", "INVISUND", "MAGSTONE", "PROTEVIL", "REMFEAR", "CAUSFEAR", "SANCTURY", "SHILELAG", "AID", "ALTRUISM", "BARKSKIN", "CHNSTNCH", "CHRMPRML", "DUSTDEVL", "FNDTRAPS", "FLMBLADE", "HESITATN", "HLDPERSP", "MUSICSPH", "RESTFIRE", "RESTCOLD", "SILENC15", "SPRHAMER", "STELBRTH", "AIR_LENS", "BRMBSTAF", "CRTSMOKE", "CURBLIDF", "CASBLIDF", "CURDISES", "CASDISES", "DISMAGCP", "HEATXHST", "MAGCVEST", "NGPLNPRO", "PLNTGROW", "PRAYER", "PROTFIRE", "REMCURSE", "BSTCURSE", "REMPARAL", "SANDSPRA", "SPIKEGRO", "STONESHA", "STRNGTHO", "SUMINSCT", "ABJURE", "BLOODFLW", "CLKBRAVY", "CLKOFEAR", "CONDENSE", "CURSRWND", "CASSRWND", "DEHYDRAT", "DUSTCLOD", "FOCSHEAT", "FREEACT", "LUNGWATR", "MAGMABLD", "NEUTPOIS", "POISON", "PRODFIRE", "QUCHFIRE", "PROEVL10", "PROLIGHT", "PROWEATH", "INTENSFW", "RETEARTH", "SOOTHE", "THRNBIND", "CONAIRE", "CONFIREE", "CONEARHE", "CONWATRE", "CRUMBLE", "CURECRIT", "CAUSCRIT", "DEFLECTN", "DISPEVIL", "FLMSTRIK", "INSPLAGE", "IRONSKIN", "RAINBOW", "QUIKSAND", "RASEDEAD", "SLAYLIVG", "SANDSTOR", "SPIKSTON", "WALLFIRE", "BLADEBAR", "FIRESEED", "HEAL", "HARM", "HEARTSEE", "SUNSTROK", "WALTHORN", "WATOLIFE", "CONFUSIN", "CONGEL_A", "CONGEL_F", "CONGEL_E", "CONGEL_W", "CREEPING", "FIRESTOR", "GLASSTOR", "RESTORE", "ENERGYDR", "SCIROCCO", "SUNRAY", "SYM_HOPL", "SYM_PAIN", "SYM_PERS", "REV_WIND", "DISRUPTN", "INSCHOST", "RIFTPR", NULL };

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

static const char *wizard_spells[] = { "ARMOR", "BURNINGH", "CHARMPER", "CHILL", "COLORSPR", "ENLARGE", "GAZEREFL", "GREASE", "MAGICMIS", "SHIELD", "SHOKGRSP", "WALLOFOG", "BLUR", "DETINVIS", "FLMSPHER", "FOGCLOUD", "GLITTERD", "INVISBEL", "ACIDAROW", "MIRRORIM", "PROPARAL", "SCARE", "STINKCLD", "STRENGTH", "WEB", "BLINK", "DISWIZRD", "FIREBALL", "FLMARROW", "HASTE", "HOLDWIZ", "HLDUNDED", "LITNBOLT", "MINMETOR", "MINMALIS", "PRONMMIS", "SLOW", "SPIRARMR", "VAMTOUCH", "CHARMMON", "CNFUSWIZ", "FEAR", "FIRESHLD", "ICESTORM", "IMPVISIB", "MGBINVUL", "MINSPTUR", "ORSPHERE", "PSIDAMPR", "RAINBOWP", "SOLIDFOG", "SSTRAND", "STONSKIN", "PEBTOBOD", "WALLFIRW", "WALLOICE", "CHAOS", "CLODKILL", "CNOFCOLD", "CONELEMN", "DISMISAL", "DOMINATE", "FEEBLEMD", "HLDMONST", "LOWRESMG", "SUMSHADW", "WALLFORC", "WALLSTON", "ANTIMAGS", "CHAINLIT", "DEATHFOG", "DEATHSPL", "DISNTWIZ", "GLOBEINV", "IMPHASTE", "IMPSLOW", "MONSUMM4", "REINCARN", "STNTFLSH", "FLSHTSTN", "TNSRTRAN", "CGELMENT", "CNUNDEAD", "DELAYFBA", "FINGEROD", "FORCAGE", "MASINVSB", "MONSUMM5", "MORDSWRD", "POWSTUN", "PRISPRAY", "SPELTURN", "BIGBYFST", "INCNDCLD", "MASCHARM", "MINDBLKW", "MONSUMM6", "OTSPHERE", "OTTODANC", "POWBLIND", "PRISWALL", "SSPELLIM", "BIGBYHND", "CRYSBRTL", "LVLDRAIN", "METEORSW", "MONSUMM7", "MORDDISJ", "POWKILL", "PRISPHER", "TIMESTOP", "DOMEINVL", "MAGPLAGU", "RIFT", "WALLASH", NULL };

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

static const char *psionics[] = { "DETONATE", "DISNTGRT", "PRJTFRCE", "BLSTCATK", "CNTLBODY", "INERTBAR", "ANMAFFIN", "ENRGECON", "LIFDRAIN", "ABSRBDIS", "ADRENCNT", "BIOFEDBK", "BODYWEAP", "CELLADG", "DISPLACE", "ENSTRENG", "FLESHARM", "GRAFWEAP", "LNDHEALH", "SHARESTR", "PSIDOMNT", "MASSDOM", "PSICRUSH", "SUPINVIS", "TWRIRONW", "EGO_WHIP", "IDINSINT", "INTELFOR", "MENTLBAR", "MIND_BAR", "MNDBLANK", "PSIBLAST", "SYNPSTAT", "THOTSHLD", NULL };

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

static const char *innate_power_full[] = { "COLD TOUCH", "PARALYSIS TOUCH", "ACID TOUCH", "WEB", "POISON 10", "POISON 30", "CORROSIVE POISON", "PARALYSIS GAZE", "LAUNCH DARTS", "FIRE BERATH", "STUN SCREAM", "ENFEEBLEMENT GAZE", "ROTTING DISEASE", "FIRE BREATH", "CORRODE ARMOR", "CORRODE WEAPON", "CONE OF SKULLS", "STUN", "CLAMP AND SHAKE", "SWALLOW", "GATE_AIR", "GATE_EARTH", "GATE_FIRE", "GATE_WATER", "EARTHQUAKE", "FEYR_FEAR", "SMASH_FISTS", "RADIATE_HEAT", "BURNING TOUCH", "CRUSH", "AURA_OF_FEAR", "MIND_BLAST", "EAT_BRAIN", "TERRASQUE_TRAMPLE", "CONSTRICT", "PARALYSE_SPIT", "THRIKREEN POISON", "TURN UNDEAD", "TWINKLE", "GENERIC ZAP", "BURNING_WEB", "ETC_POISON_PARALYZING   ", "ETC_SCHRAPNEL_D10     ", "ROD_OF_TEETH", "NINE_LIFE_STEALER", "RAINBOW BOW", "HEART_SEEKER ", "SWORD_OF_MAGMA", "SWORD_OF_WOUNDING", "LOCK_JAWS", "UMBERHULK", "POISON", "SCREECH", "DETONATE", "SLIME", "POISON", "BANSHEE GAZE", "FIRE EEL", "GHOST FEAR", "MASTRYIAL", "BLACK MASTRYIAL", "DISEASE", "RAMPAGER_FEAR", "DISEASE", "HOWL", "PSIONIC BLAST", "GREATER SHADOW", "SLAAD DISEASE", "RED SLAAD STUN", "POISON", "SPINE LAUNCH", "STYR BREATH", "TANARRI SCREECH", "TYRIAN SLIME", "XORN CORROSION", "ZOMBIE", NULL };

static const char *innate_power_short[] = { " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "RODOFTTH", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "PSIBLAST", " ", " ", " ", "POISON", "SPINES", " ", " ", " ", " ", " ", " ", NULL};

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
static const char *ds1_power_names[] = {"DNE", "ARMOR", "BURNING HANDS", "CHARM PERSON", "CHILL TOUCH", "COLOR SPRAY", "ENLARGE", "GAZE REFLECTION", "GREASE", "MAGIC MISSILE", "SHIELD", "SHOCKING GRASP", "WALL OF FOG", "BLUR", "DETECT INVISIBLE", "FLAMING SPHERE", "FOG CLOUD", "GLITTER DUST", "INVISIBILITY", "ACID ARROW", "MIRROR IMAGE", "PROTECTION FROM PARALYSIS", "SCARE", "STINKING CLOUD", "STRENGTH", "WEB", "BLINK", "DISPEL MAGIC", "FIREBALL", "FLAMING ARROW", "HASTE", "HOLD PERSON", "HOLD UNDEAD", "LIGHTNING BOLT", "MINUTE METEORS", "MINOR MALISON", "MONSTER SUMMONING I", "PROTECTION FROM MISSILES", "SLOW", "SPIRIT ARMOR", "VAMPIRIC TOUCH", "CHARM MONSTER", "CONFUSION", "EVARD'S BLACK TENTACLES", "FEAR", "FIRE SHIELD", "ICE STORM", "IMPROVED INVISIBILITY", "M GLOBE OF INVULNERABILITY", "MINOR SPELL TURNING", "MONSTER SUMMONING II", "RAINBOW PATTERN", "SOLID FOG", "STONE SKIN", "PEBBLE TO BOULDER", "WALL OF FIRE", "WALL OF ICE", "CHAOS", "CLOUD KILL", "CONE OF COLD", "CONJURE ELEMENTAL", "DISMISSAL", "DOMINATE", "FEEBLE MIND", "HOLD MONSTER", "LOWER RESISTANCE", "MONSTER SUMMONING III", "SUMMON SHADOW", "WALL OF FORCE", "WALL OF STONE", "BLESS", "CURSE", "CURE LIGHT WOUNDS", "CAUSE LIGHT WOUNDS", "ENTANGLE", "INVISIBLE TO UNDEAD", "MAGIC STONE", "PROTECTION FROM EVIL", "REMOVE FEAR", "CAUSE FEAR", "SHILLELAGH", "AID", "BARKSKIN", "CHARM MAMMAL", "DUSTDEVIL", "FIND TRAPS", "FLAME BLADE", "HOLD PERSON", "RESIST FIRE", "RESIST COLD", "SPIRITUAL HAMMER", "CONJURE AIR ELEMENTAL", "CONJURE FIRE ELEMENTAL", "CONJURE EARTH ELEMENTAL", "CONJURE WATER ELEMENTAL", "CURE BLINDNESS", "CAUSE BLINDNESS", "CURE DISEASE", "CAUSE DISEASE", "DISPEL MAGIC", "MAGIC VESTMENT", "NEGATIVE PLANE PROTECTION", "PRAYER", "PROTECTION FROM FIRE", "REMOVE CURSE", "BESTOW CURSE", "REMOVE PARALYSIS", "SUMMON INSECTS", "ABJURE", "BLOOD FLOW", "CLOAK OF BRAVERY", "CLOAK OF FEAR", "CONDENSE", "CURE SERIOUS WOUNDS", "CAUSE SERIOUS WOUNDS", "DEHYDRATE", "DUST CLOUD", "FOCUS HEAT", "FREE ACTION", "NEUTRALIZE POISON", "POISON", "PRODUCE FIRE", "PROTECTION FROM EVIL 10", "PROTECTION FROM LIGHTNING", "CONJURE GREATER AIR ELEMENTAL", "CONJURE GREATER FIRE ELEMENTAL", "CONJURE GREATER EARTH ELEMENTAL", "CONJURE GREATER WATER ELEMENTAL", "CURE CRITICAL WOUNDS", "CAUSE CRITICAL WOUNDS", "DEFLECTION", "DISPEL EVIL", "FLAME STRIKE", "INSECT PLAGUE", "IRON SKIN", "QUICKSAND", "RAISE DEAD", "SLAY LIVING", "WALL OF FIRE", "DETONATE", "DISINTEGRATE", "PROJECT FORCE", "BALLISTIC ATTACK", "CONTROL BODY", "INERTIAL BARRIER", "ANIMAL AFFINITY", "ENERGY CONTROL", "LIFE DRAIN", "ABSORB DISEASE", "ADRENALIN CONTROL", "BIOFEEDBACK", "BODY WEAPONRY", "CELLULAR ADJUSTMENT", "DISPLACEMENT", "ENHANCED STRENGTH", "FLESH ARMOR", "GRAFT WEAPON", "LEND HEALTH", "SHARE STRENGTH", "DOMINATION", "MASS DOMINATION", "PSYCHIC CRUSH", "SUPERIOR INVISIBILITY", "TOWER OF IRON WILL", "EGO WHIP", "ID INSINUATION", "INTELLECT FORTRESS", "MENTAL BARRIER", "PSIONIC MIND BAR", "MIND BLANK", "PSIONIC BLAST", "SYNAPTIC STATIC", "THOUGHT SHIELD", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "TURN UNDEAD", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "DNE", "TURN UNDEAD" };

/*
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
*/

static int psi, wiz, pri, innate;
static char type[64];
static void generate_activate(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_can_activate (power_instance_t *source, const int16_t power_level) {\n", type, name);
    fprintf(file, "    if (!source || !source->entity) { return 0;}\n");
    
    if (psi >= 0) {
    } else if (wiz >= 0) {
        fprintf(file, "    return entity_has_wizard_slot(power_level);\n");
    } else if (pri >= 0) {
        fprintf(file, "    return entity_has_priest_slot(power_level);\n");
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
        fprintf(file, "    return entity_take_wizard_slot(power_level);\n");
    } else if (pri >= 0) {
        fprintf(file, "    return entity_take_priest_slot(power_level);\n");
    } else if (psi >= 0) {
        fprintf(file, "    NEED TO DO PSI CALC!\n");
    } else if (innate >= 0) {
        fprintf(file, "    return 1;// innates can always pay\n");
    } else {
        fprintf(file, "    return ???;\n");
    }

    fprintf(file, "}\n");
}

static void generate_apply(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nstatic int %s_%s_apply        (power_instance_t *source, entity_t *entity) {\n", type, name);
    fprintf(file, "    if (!source || !source->entity) { return 0;}\n");
    
    if (psi >= 0) {
    } else if (wiz >= 0) {
    } else if (pri >= 0) {
    } else {
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
        //case 7: return "Self";
        //case 8: return "Two?";
        default:break;
    }
    return "ERROR";
}

static void generate_setup(power_entry_t pw, char *name, FILE *file) {
    fprintf(file, "\nextern void %s_%s_setup  (power_t *spell) {\n", type, name);
    fprintf(file, "    spell->name                 = \"%s\";\n", pw.name);
    fprintf(file, "    spell->description          = %s_read_description(%d);\n", type, index);
    fprintf(file, "    spell->range                = -99999;\n");
    fprintf(file, "    spell->aoe                  = -99999;\n");
    fprintf(file, "    spell->shape                = %s;\n", get_target_shape(pw));
    fprintf(file, "    spell->cast_sound           = select_by_game(%d, %d, %d);\n",
            ds1pw.cast_sound, ds2pw.info.cast_sound, dsopw.info.cast_sound);
    fprintf(file, "    spell->thrown_sound         = select_by_game(%d, %d, %d);\n",
            ds1pw.thrown_sound, ds2pw.info.thrown_sound, dsopw.info.thrown_sound);
    fprintf(file, "    spell->hit_sound            = select_by_game(%d, %d, %d);\n",
            ds1pw.hit_sound, ds2pw.info.hit_sound, dsopw.info.hit_sound);
    fprintf(file, "    spell->actions.can_activate = %s_%s_can_activate;\n", type, name);
    fprintf(file, "    spell->actions.pay          = %s_%s_pay;\n", type, name);
    fprintf(file, "    spell->actions.apply        = %s_%s_apply;\n", type, name);
    fprintf(file, "    spell->actions.still_active = %s_%s_still_active;\n", type, name);
    fprintf(file, "    spell->actions.affect_power = %s_%s_affect_power;\n", type, name);
    fprintf(file, "    spells_set_icon(spell, 21000);\n");
    fprintf(file, "    spells_set_thrown(spell, 2315);\n");
    //fprintf(file, "    spells_set_hit(spell, 2314);\n");
    fprintf(file, "    spells_set_hit(spell, select_by_game(%d, %d, %d));\n",
            ds1pw.hit, ds2pw.info.hit, dsopw.info.hit);
    fprintf(file, "}\n");
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

    snprintf(path, 127, "powers/%s-%s.c", type, filename);
    file = fopen(path, "wb+");

    if (!file) {
        fprintf(stderr, "Unable to overwrite '%s'\n", path);
    }

    for (int i = 0; i < 64 && type[i]; i++) {
        type[i] = toupper(type[i]);
    }

    fprintf(file, "/* This file is auto-generated. */\n");
    fprintf(file, "#ifndef %s_%s_POWER_H\n", type, name);
    fprintf(file, "#define %s_%s_POWER_H\n", type, name);
    fprintf(file, "\n#include \"../spells.h\"\n");

    for (int i = 0; i < 64 && name[i]; i++) {
        name[i] = tolower(name[i]);
    }
    for (int i = 0; i < 64 && type[i]; i++) {
        type[i] = tolower(type[i]);
    }

    generate_activate(pw, name, file);
    generate_pay(pw, name, file);
    generate_apply(pw, name, file);
    generate_affect(pw, name, file);
    generate_setup(pw, name, file);

    fprintf(file, "#endif\n");
    fclose(file);
};
