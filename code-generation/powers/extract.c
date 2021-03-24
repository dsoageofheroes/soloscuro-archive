#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static FILE *ds1_file = NULL;
static FILE *ds2_file = NULL;
static FILE *dso_file = NULL;

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

static power_info_t ds1pw;
static power_entry_t ds2pw;
static power_entry_t dsopw;

static char test_buf[256];
static char* test(power_entry_t pw) { return ""; }
/*
    snprintf(test_buf, 255, "[%d:%d, %d, %d]",
            pw.data1[0] >> 4, pw.data1[1] & 0x0F,
            pw.data1[1],
            pw.data1[2]);

    return test_buf;
}
*/

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
    if (feof(ds1_file)) {
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
    if (feof(ds1_file)) {
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
    if (feof(ds1_file)) {
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

int main(int argc, char *argv[]) {
    ds1_file = fopen("dsun1.dat", "rb");
    ds2_file = fopen("dsun2.dat", "rb");
    dso_file = fopen("dso.dat", "rb");

    if (!ds1_file) {
        fprintf(stderr, "Unable to open dsun2.dat, please provide.\n");
        return 0;
    }

    if (!ds2_file) {
        fprintf(stderr, "Unable to open dsun2.dat, please provide.\n");
        return 0;
    }

    if (!dso_file) {
        fprintf(stderr, "You need to provide the chunk containing the powers as chunk.dat\n");
        return 0;
    }

    //while (!feof(dso_file)) {
    printf("Name, Subname, range, duration, area, target, cast (?), ds1 cast sound, ds2/dso cast sound,"
            " special, thrown (?), ds1 thrown sound, ds2/dso thrown sound, hit (?),"
            " ds1 hit sound, ds2/dso hit sound, aoe id, effect id,"
            " effect type, ds1 damage, ds2 damage, dso damage, save,"
            " ds1 ai, ds2 ai, dso ai \n");
    while (!feof(dso_file)) {
    //for (int i = 0; i < 200; i++) {
    //for (int i = 0; i < 60; i++) {
        fread(&ds2pw, 1, sizeof(power_entry_t), ds2_file);
        fread(&dsopw, 1, sizeof(power_entry_t), dso_file);
        if (!feof(ds1_file)) {
            fread(&ds1pw, 1, sizeof(power_info_t), ds1_file);
        } else {
            ds1pw = ds2pw.info;
        }
        printf("%s, %s", dsopw.name, dsopw.name2);

        printf(", %s", get_range(dsopw));
        printf(", %s", get_duration(dsopw));
        //printf("%s, subname = '%s' %s %s %s %s %s\n", dsopw.name, dsopw.name2, 
                //get_range(dsopw), get_duration(dsopw), get_area(dsopw), get_cast(dsopw),
                //test(dsopw));
        printf(", %s", get_area(dsopw));
        printf(", %s", get_target());
        printf(", %s", get_cast(dsopw));
        printf(", %s", get_special());
        printf(", %s", get_thrown());
        printf(", %s", get_hit());
        printf(", %s", get_aoe_id());
        printf(", %s", get_effect());
        printf(", %s", get_effect_type());
        if (feof(ds1_file)) {
            printf(", ---");
        } else {
            printf(", %s", get_damage(ds1pw.damage));
        }
        printf(", %s", get_damage(ds2pw.info.damage));
        printf(", %s", get_damage(dsopw.info.damage));
        printf(", %s", get_save());

        printf(", %s %d times, %s %d, %s %d times",
                ai[ds1pw.data0 & 0x03],
                ds1pw.data0 >> 2,
                ai[ds2pw.info.data0 & 0x03],
                ds2pw.info.data0 >> 2,
                ai[dsopw.info.data0 & 0x03],
                dsopw.info.data0 >> 2
              );
        printf("\n");
        //fflush(stdout);
    }
    //fread(&power, 1, sizeof(power_entry_t), file);
    //printf("name = %s\n", power.name);

    fclose(ds1_file);
    fclose(ds2_file);
    fclose(dso_file);
    return 0;
}
