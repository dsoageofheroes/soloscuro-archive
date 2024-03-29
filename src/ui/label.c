#include "class.h"
#include "alignment.h"
#include "label.h"
#include "font.h"
#include "rules.h"
#include "settings.h"

#include <string.h>

static sol_label_t labels[LABEL_END];
static int labels_created = 0;
static sol_screen_type_t screen_type;

#define BUF_MAX (1<<10)

extern sol_status_t sol_label_render(struct sol_label_s* label) {
    if (label->visible) {
        return sol_print_line_len(label->font, label->text, label->x, label->y, strlen(label->text));
    }
    return SOL_LABEL_NOT_FOUND;
}

extern sol_status_t sol_label_set(sol_label_t *label, const char *buf, const int32_t x, const int32_t y) {
    if (!label || !buf) { return SOL_NULL_ARGUMENT; }
    sol_label_set_text(label, buf);
    label->x = x;
    label->y = y;
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_set_text(sol_label_t *label, const char* string) {
    if (!label || !string) { return SOL_NULL_ARGUMENT; }

    free(label->text);
    label->text = (char*)malloc( (strlen(string) + 1) * sizeof(string) );
    if (label->text == NULL) {
        printf("couldn't malloc in sol_label_set_text() - label id = %d\n", label->id);
        return SOL_MEMORY_ERROR;
    }

    strcpy(label->text, string);
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_pixel_width(struct sol_label_s* label, uint32_t *width) {
    if (!label) { return SOL_NULL_ARGUMENT; }
    if (label->__m_pixel_width_do_not_use == 0 || label->text != label->__m_old_text_do_not_use)
    {
        label->__m_old_text_do_not_use = label->text;
        sol_font_pixel_width(label->font,
                label->text, strlen(label->text),
                &label->__m_pixel_width_do_not_use);
    }

    *width = label->__m_pixel_width_do_not_use;
    return SOL_SUCCESS;
}


extern sol_label_t create_label(int parent, int id, char* text, sol_font_t font) {
    sol_label_t new_label;

    new_label.parent = parent;
    new_label.id = id;
    new_label.text = NULL; // must be set so we can free whenever sol_label_set_text is called
    sol_label_set_text(&new_label, text);
    new_label.font = font;
    new_label.visible = 1;

    return new_label;
}

extern sol_status_t sol_label_create_at_pos(int parent, int id, char* text, sol_font_t font, int16_t x, int16_t y, sol_label_t *l) {
    sol_label_t new_label = create_label(parent, id, text, font);

    new_label.x = x;
    new_label.y = y;

    *l = new_label;
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_free(sol_label_t *l) {
    if (!l) { return SOL_NULL_ARGUMENT; }

    if (l->text) {
        free(l->text);
        l->text = NULL;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_label_point_in(sol_label_t *label, const int32_t x, const int32_t y, sol_label_t **l) {
    uint32_t font_h, font_w;

    if (!label) { return SOL_NULL_ARGUMENT; }
    
    sol_font_pixel_height(FONT_GREYLIGHT, &font_h);
    sol_label_pixel_width(label, &font_w);
    if (!((x >= label->x && x <= (label->x + font_w)) &&
         (y >= label->y && y <= (label->y + font_h)))) {
        return SOL_OUT_OF_RANGE;
    }

    if (l) { *l = label; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_group_point_in(const int32_t x, const int32_t y, sol_label_t **l) {
    if (!l) { return SOL_NULL_ARGUMENT; }
    for (int i = 0; i < LABEL_END; i++) {
        if (sol_label_point_in(labels + i, x, y, NULL)) {
            *l = labels + i;
            return SOL_SUCCESS;
        }
    }
    return SOL_LABEL_NOT_FOUND;
}

extern sol_status_t sol_label_group_set_font(sol_font_t font) {
    for (int i = 0; i < LABEL_END; i++) {
        labels[i].font = font;
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_create_group() {
    int oX = 8, oY = 249;
    char buf[64];
    const float zoom = settings_zoom();

    if (labels_created) { return SOL_SUCCESS; }

    strcpy(buf, "");
    sol_label_create_at_pos(0, LABEL_NAME,
                            "NAME:", FONT_GREYLIGHT,
                            0 + oX, 0 + oY,
                            &labels[LABEL_NAME]);

    sol_label_create_at_pos(0, LABEL_STR,
                            "STR:", FONT_GREYLIGHT,
                            (oX += 12) + 0, (oY += 20) + 0,
                            &labels[LABEL_STR]);

    sol_label_create_at_pos(0, LABEL_STR_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_STR_VAL]);
    
    sol_label_create_at_pos(0, LABEL_DEX,
                            "DEX:", FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_DEX]);

    sol_label_create_at_pos(0, LABEL_DEX_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_DEX_VAL]);
    
    sol_label_create_at_pos(0, LABEL_CON,
                            "CON:", FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_CON]);
    sol_label_create_at_pos(0, LABEL_CON_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_CON_VAL]);
    
    sol_label_create_at_pos(0, LABEL_INT,
                            "INT:", FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_INT]);
    sol_label_create_at_pos(0, LABEL_INT_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_INT_VAL]);

    sol_label_create_at_pos(0, LABEL_WIS,
                            "WIS:", FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_WIS]);

    sol_label_create_at_pos(0, LABEL_WIS_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_WIS_VAL]);

    sol_label_create_at_pos(0, LABEL_CHA,
                            "CHA:", FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_CHA]);

    sol_label_create_at_pos(0, LABEL_CHA_VAL,
                            buf, FONT_GREYLIGHT,
                            oX + 26 * zoom, oY,
                            &labels[LABEL_CHA_VAL]);

    sol_label_create_at_pos(0, LABEL_GENDER,
                            buf, FONT_GREYLIGHT,
                            0, 0,
                            &labels[LABEL_GENDER]);

    sol_label_create_at_pos(0, LABEL_RACE,
                            buf, FONT_GREYLIGHT,
                            0, 0,
                            &labels[LABEL_RACE]);

    sol_label_create_at_pos(0, LABEL_ALIGNMENT,
                            buf, FONT_GREYLIGHT,
                            0, 0,
                            &labels[LABEL_ALIGNMENT]);

    sol_label_create_at_pos(0, LABEL_CLASSES,
                            buf, FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_CLASSES]);

    sol_label_create_at_pos(0, LABEL_LEVELS,
                            buf, FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_LEVELS]);

    sol_label_create_at_pos(0, LABEL_EXP_TNL,
                            buf, FONT_GREYLIGHT,
                            0 + oX + 70, 0 + oY,
                            &labels[LABEL_EXP_TNL]);

    sol_label_create_at_pos(0, LABEL_AC,
                            buf, FONT_GREYLIGHT,
                            0 + oX, (oY += 15) + 0,
                            &labels[LABEL_AC]);

    sol_label_create_at_pos(0, LABEL_DAM,
                            buf, FONT_GREYLIGHT,
                            0 + oX + 70, 0 + oY,
                            &labels[LABEL_DAM]);

    sol_label_create_at_pos(0, LABEL_HP,
                            buf, FONT_GREYLIGHT,
                            0 + (oX += 20), 0 + (oY += 15),
                            &labels[LABEL_HP]);

    sol_label_create_at_pos(0, LABEL_PSP,
                            buf, FONT_GREYLIGHT,
                            0 + (oX), 0 + (oY += 15),
                            &labels[LABEL_PSP]);
    for (int i = 0; i < LABEL_END; i++) {
        labels[i].id = i;
    }
    labels_created = 1;
    return SOL_SUCCESS;
}

static const char* get_gender_as_string(sol_entity_t *pc) {
    switch(pc->gender) {
        case GENDER_MALE:   return "MALE";
        case GENDER_FEMALE: return "FEMALE";
        case GENDER_NONE:   return "NONE";       
    }
    return "UNKNOWN";
}

static const char* get_race_as_string(sol_entity_t *pc) {
    switch(pc->race) {
        case RACE_HUMAN:     return "HUMAN";
        case RACE_DWARF:     return "DWARF";
        case RACE_ELF:       return "ELF";
        case RACE_HALFELF:   return "HALF-ELF";
        case RACE_HALFGIANT: return "HALF-GIANT";
        case RACE_HALFLING:  return "HALFLING";
        case RACE_MUL:       return "MUL";
        case RACE_THRIKREEN: return "THRI-KREEN";
    }
    return "UNKNOWN";
}

static const char* get_alignment_as_string(sol_entity_t *pc) {
    switch(pc->alignment) {
        case LAWFUL_GOOD:     return "LAWFUL GOOD";
        case LAWFUL_NEUTRAL:  return "LAWFUL NEUTRAL";
        case LAWFUL_EVIL:     return "LAWFUL EVIL";
        case NEUTRAL_GOOD:    return "NEUTRAL GOOD";
        case TRUE_NEUTRAL:    return "TRUE NEUTRAL";
        case NEUTRAL_EVIL:    return "NEUTRAL EVIL";
        case CHAOTIC_GOOD:    return "CHAOTIC GOOD";
        case CHAOTIC_NEUTRAL: return "CHAOTIC NEUTRAL";
        case CHAOTIC_EVIL:    return "CHAOTIC EVIL";
    }
    return "UNKNOWN";
}

static const char* get_class_name(const uint8_t class) {
    switch (class) {
        case REAL_CLASS_AIR_CLERIC:
        case REAL_CLASS_EARTH_CLERIC:
        case REAL_CLASS_FIRE_CLERIC:
        case REAL_CLASS_WATER_CLERIC:
            return "CLERIC";
        case REAL_CLASS_AIR_DRUID:
        case REAL_CLASS_EARTH_DRUID:
        case REAL_CLASS_FIRE_DRUID:
        case REAL_CLASS_WATER_DRUID:
            return "DRUID";
        case REAL_CLASS_FIGHTER: return "FIGHTER";
        case REAL_CLASS_GLADIATOR: return "GLADIATOR";
        case REAL_CLASS_PRESERVER: return "PRESERVER";
        case REAL_CLASS_PSIONICIST: return "PSIONICIST";
        case REAL_CLASS_AIR_RANGER:
        case REAL_CLASS_EARTH_RANGER:
        case REAL_CLASS_FIRE_RANGER:
        case REAL_CLASS_WATER_RANGER:
            return "RANGER";
        case REAL_CLASS_THIEF: return "THIEF";
    }

    return 0; // UNKNOWN CLASS
}

static void copy_classes_string(sol_entity_t *pc, char* storage) {
    int pos = 0;

    for (int i = 0; i < 3; i++) {
        if (pc->class[i].class > 0) {
            pos += snprintf(storage + pos, BUF_MAX - pos, "%s%s", i > 0 ? "/" : "", get_class_name(pc->class[i].class));
        }
    }

    storage[pos] = '\0';
}

static void copy_levels_string(sol_entity_t *pc, char* storage) {
    int pos = 0;

    for (int i = 0; i < 3; i++) {
        if (pc->class[i].class > 0) {
            pos += snprintf(storage + pos, BUF_MAX - pos, "%s%d", i > 0 ? "/" : "", pc->class[i].level);
        }
    }

    storage[pos] = '\0';
}

static void copy_exp_tnl_string(sol_entity_t *pc, char* storage) {
    int32_t next_exp, total_exp;
    if (pc->class[0].class > -1) {
        sol_status_check(
                sol_dnd2e_class_exp_to_next_level(pc, &next_exp),
                "Unable to get the experience for next level");
        sol_entity_get_total_exp(pc, &total_exp);
        snprintf(storage, BUF_MAX, "EXP: %d (%d)", total_exp, next_exp);
    }
    else {
        storage[0] = '\0';
    }
}

static void copy_dam_string(sol_entity_t *pc, char* storage) {
    int pos = 0;
    sol_item_t *items = pc ? pc->inv : NULL;
    int attack_num = dnd2e_get_attack_num(pc, items ? items + 3 : NULL);
    int sides = dnd2e_get_attack_sides_pc(pc, items ? items + 3 : NULL);
    int mod = dnd2e_get_attack_mod_pc(pc, items ? items + 3 : NULL);

    pos = snprintf(storage, BUF_MAX, "DAM: %d%s",
            attack_num >> 1, attack_num & 0x01 ? ".5" : "");
    pos += snprintf(storage + pos, BUF_MAX - pos, "x1D%d", sides);
    pos += snprintf(storage + pos, BUF_MAX - pos, "+%d", mod);
}


extern sol_status_t sol_label_set_group(sol_entity_t *dude, sol_screen_type_t _screen_type) {
    char buf[BUF_MAX];
    int show_hp_psp;
    if (!dude) { return SOL_NULL_ARGUMENT; }

    screen_type = _screen_type;
    show_hp_psp = (screen_type == SCREEN_VIEW_CHARACTER);

    snprintf(buf, 127, "%d", dude->stats.str);
    sol_label_set_text(labels + LABEL_STR_VAL, buf);

    snprintf(buf, 127, "%d", dude->stats.dex);
    sol_label_set_text(labels + LABEL_DEX_VAL, buf);

    snprintf(buf, 127, "%d", dude->stats.con);
    sol_label_set_text(labels + LABEL_CON_VAL, buf);

    snprintf(buf, 127, "%d", dude->stats.intel);
    sol_label_set_text(labels + LABEL_INT_VAL, buf);

    snprintf(buf, 127, "%d", dude->stats.wis);
    sol_label_set_text(labels + LABEL_WIS_VAL, buf);

    snprintf(buf, 127, "%d", dude->stats.cha);
    sol_label_set_text(labels + LABEL_CHA_VAL, buf);

    if (dude->name) {
        sol_label_set_text(labels + LABEL_NAME, dude->name);
    }

    sol_label_set_text(labels + LABEL_GENDER, get_gender_as_string(dude));
    sol_label_set_text(labels + LABEL_RACE, get_race_as_string(dude));
    sol_label_set_text(labels + LABEL_ALIGNMENT, get_alignment_as_string(dude));
    copy_classes_string(dude, buf);
    sol_label_set_text(labels + LABEL_CLASSES, buf);

    copy_levels_string(dude, buf);
    sol_label_set_text(labels + LABEL_LEVELS, buf);

    copy_exp_tnl_string(dude, buf);
    sol_label_set_text(labels + LABEL_EXP_TNL, buf);

    snprintf(buf, BUF_MAX, "AC: %d", dnd2e_get_ac_pc(dude));
    sol_label_set_text(labels + LABEL_AC, buf);

    copy_dam_string(dude, buf);
    sol_label_set_text(labels + LABEL_DAM, buf);

    snprintf(buf, BUF_MAX, "%s%d/%d",
            (show_hp_psp) ? "HP: " : "",
            dude->stats.hp, dude->stats.high_hp);
    sol_label_set_text(labels + LABEL_HP, buf);

    snprintf(buf, BUF_MAX, "%s%d/%d",
            (show_hp_psp) ? "PSP: " : "",
            dude->stats.psp, dude->stats.high_psp);
    sol_label_set_text(labels + LABEL_PSP, buf);

    return SOL_SUCCESS;
}

extern sol_status_t sol_label_set_positions(int32_t oX, int32_t oY, const sol_screen_type_t _screen_type) {
    screen_type = _screen_type;
    int tab_hp_psp = (screen_type != SCREEN_VIEW_CHARACTER);
    int exp_new_line = (screen_type == SCREEN_VIEW_CHARACTER);
    int yadj = (screen_type == SCREEN_VIEW_CHARACTER) ? 13 : 15;
    uint32_t width;

    labels[LABEL_NAME].x = oX;
    labels[LABEL_NAME].y = oY;

    labels[LABEL_STR].x = (oX += 12);
    labels[LABEL_STR].y = (oY += yadj + 5);
    labels[LABEL_STR_VAL].x = oX + (strlen(labels[LABEL_STR_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_STR_VAL].y = oY;

    labels[LABEL_DEX].x = oX;
    labels[LABEL_DEX].y = (oY += yadj);
    labels[LABEL_DEX_VAL].x = oX + (strlen(labels[LABEL_DEX_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_DEX_VAL].y = oY;

    labels[LABEL_CON].x = oX;
    labels[LABEL_CON].y = (oY += yadj);
    labels[LABEL_CON_VAL].x = oX + (strlen(labels[LABEL_CON_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_CON_VAL].y = oY;
    
    labels[LABEL_INT].x = oX;
    labels[LABEL_INT].y = (oY += yadj);
    labels[LABEL_INT_VAL].x = oX + (strlen(labels[LABEL_INT_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_INT_VAL].y = oY;

    labels[LABEL_WIS].x = oX;
    labels[LABEL_WIS].y = (oY += yadj);
    labels[LABEL_WIS_VAL].x = oX + (strlen(labels[LABEL_WIS_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_WIS_VAL].y = oY;

    labels[LABEL_CHA].x = oX;
    labels[LABEL_CHA].y = (oY += yadj);
    labels[LABEL_CHA_VAL].x = oX + (strlen(labels[LABEL_CHA_VAL].text) > 1 ? 52 : 60);
    labels[LABEL_CHA_VAL].y = oY;

    labels[LABEL_GENDER].x = (oX = 170);
    labels[LABEL_GENDER].y = (oY = 270);

    sol_font_pixel_width(FONT_GREYLIGHT, labels[LABEL_RACE].text, strlen(labels[LABEL_RACE].text), &width);
    labels[LABEL_RACE].x = oX + width;
    labels[LABEL_RACE].y = (oY = 270);

    labels[LABEL_ALIGNMENT].x = oX;
    labels[LABEL_ALIGNMENT].y = (oY += yadj);

    labels[LABEL_CLASSES].x = oX;
    labels[LABEL_CLASSES].y = (oY += yadj);

    labels[LABEL_LEVELS].x = oX;
    labels[LABEL_LEVELS].y = (oY += yadj);

    labels[LABEL_EXP_TNL].x = oX + (exp_new_line ? 0 : 70);
    labels[LABEL_EXP_TNL].y = (oY += (exp_new_line ? yadj : 0));

    labels[LABEL_AC].x = oX;
    labels[LABEL_DAM].x = oX + 70;
    labels[LABEL_HP].x = (oX += (tab_hp_psp) ? 20 : 0);
    labels[LABEL_PSP].x = oX;

    if (screen_type == SCREEN_VIEW_CHARACTER) {
        labels[LABEL_HP].y = (oY += yadj);
        labels[LABEL_PSP].y = (oY += yadj);
        labels[LABEL_AC].y = (oY += yadj);
        labels[LABEL_DAM].y = oY;
    } else {
        labels[LABEL_AC].y = (oY += yadj);
        labels[LABEL_DAM].y = oY;
        labels[LABEL_HP].y = (oY += yadj);
        labels[LABEL_PSP].y = (oY += yadj);
    }

    return SOL_SUCCESS;
}

static sol_status_t sol_label_render_offset(sol_label_t *label, const int16_t offsetx, const int16_t offsety) {
    return sol_print_line_len(label->font, label->text, offsetx + label->x, offsety + label->y, strlen(label->text));
}

extern sol_status_t sol_label_render_gra(const int16_t offsetx, const int16_t offsety) {
    sol_status_t status;
    if((status = sol_label_render_offset(labels + LABEL_GENDER, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_RACE, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_ALIGNMENT, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_render_class_and_combat(const int16_t offsetx, const int16_t offsety) {
    sol_status_t status;
    if((status = sol_label_render_offset(labels + LABEL_CLASSES, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_LEVELS, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_EXP_TNL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_AC, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_DAM, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_HP, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_PSP, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_render_full(const int16_t offsetx, const int16_t offsety) {
    sol_status_t status;
    if((status = sol_label_render_stats(offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_NAME, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_gra(offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_class_and_combat(offsetx, offsety)) != SOL_SUCCESS) { return status; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_label_render_stats(const int16_t offsetx, const int16_t offsety) {
    sol_status_t status;
    if((status = sol_label_render_offset(labels + LABEL_STR, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_STR_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_DEX, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_DEX_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_CON, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_CON_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_INT, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_INT_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_WIS, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_WIS_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_CHA, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    if((status = sol_label_render_offset(labels + LABEL_CHA_VAL, offsetx, offsety)) != SOL_SUCCESS) { return status; }
    return SOL_SUCCESS;
}
