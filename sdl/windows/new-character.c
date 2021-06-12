#include "new-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "narrate.h"
#include "popup.h"
#include "../textbox.h"
#include "../sprite.h"
#include "../../src/wizard.h"
#include "../../src/player.h"
#include "../../src/rules.h"
#include <string.h>
#include <time.h>

#define BUF_MAX (1<<10)

static uint16_t background;
static uint16_t parchment[5];
static uint16_t done;
static uint16_t text_cursor;
static uint16_t classes[8];
static uint16_t class_sel[8];
static uint16_t stats_align_hp_buttons[8];
static uint16_t psionic_devotion[3];
static uint16_t spheres[4];
static uint16_t ps_sel[4]; // seletion for psionics/spheres.
static uint16_t die[11];
static uint16_t races[14];
static uint16_t spr;// sprite of the character on window
static uint16_t psionic_label; // 2047
static uint16_t sphere_label; // 2046
static uint16_t done_button;
static uint16_t exit_button;
static textbox_t* name_tb;

// Store this so we don't trigger mouseup events in sprites/labels we didn't mousedown in
static uint16_t last_sprite_mousedowned;
static label_t *last_label_mousedowned;

static uint8_t show_psionic_label = 1;
static int8_t sphere_selection = -1;

static int die_pos = 0;
static int die_countdown = 0;

// FIXME - Change these to ds1_race_[...] and then add ones for DS2 and DSO
// H = Human - D = Dwarf - E = Elf - HE = Half-Elf - HG = Half-Giant - HL = Halfling - M = Mul - T = Thri-Kreen
// genderRace                              mH    fH    mD    fD    mE    fE   mHE   fHE   mHG   fHG   mHL   fHL    mM    fM    mT    fT
static int race_portrait_offsets_x[] = {   25,   25,   19,   25,   15,   15,   28,   25,   11,   16,   28,   34,   18,   18,   11,   11 };
static int race_portrait_offsets_y[] = {   12,   17,   26,   27,   14,   16,   14,   13,   15,   17,   27,   30,   07,   07,   13,   13 };
static int race_sprite_offsets_x[]   = {  148,  150,  150,  150,  151,  152,  148,  150,  144,  146,  150,  151,  149,  149,  146,  146 };
static int race_sprite_offsets_y[]   = {   27,   28,   33,   33,   28,   26,   27,   28,   24,   24,   33,   32,   26,   26,   24,   24 };
static int race_sprite_ids[]         = { 2095, 2099, 2055, 2053, 2061, 2059, 2095, 2099, 2072, 2074, 2068, 2070, 2093, 2093, 2097, 2097 };

static int offsetx, offsety;
static SDL_Renderer *renderer;
static entity_t pc;
static psin_t psi; // psi group
static ssi_spell_list_t spells;
static psionic_list_t psionics;
static uint8_t is_valid;
static char sphere_text[32];
static int current_textbox;
static int changed_name;

static void update_ui();
static void select_class(uint8_t class);
static void fix_alignment(int direction);

entity_t* new_character_get_pc() {
    if (!is_valid) { return NULL; }
    return &pc;
}

psin_t* new_character_get_psin() {
    if (!is_valid) { return NULL; }
    return &psi;
}

ssi_spell_list_t* new_character_get_spell_list() {
    if (!is_valid) { return NULL; }
    return &spells;
}

psionic_list_t* new_character_get_psionic_list() {
    if (!is_valid) { return NULL; }
    return &psionics;
}

char* new_character_get_name() {
    return textbox_get_text(name_tb);
}

// FIXME - For DS2/DSO, there may be new random names (I don't think there are?)
// Hard-coding these unless we can come up with a better way of figuring out
// which names are Female/Thri-Kreen
static void get_random_name() {
    uint32_t res_ids[1<<12];
    int name_list_start  = pc.race == RACE_THRIKREEN ? 52 : pc.gender == GENDER_FEMALE ? 33 : 0;
    int name_list_length = pc.race == RACE_THRIKREEN ? 7  : pc.gender == GENDER_FEMALE ? 17 : 32;
    int chosen_name = (rand() % (name_list_length + 1)) + name_list_start;
    char name_text[32];

    memset(&name_text[0], 0, sizeof(name_text));

    // Name resource length is hard-coded now
    gff_get_resource_ids(RESOURCE_GFF_INDEX, GFF_TEXT, res_ids);
    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_TEXT, res_ids[chosen_name]);
    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, name_text, 32);
    textbox_set_text(name_tb, name_text);
}

static int convert_to_actual_class(const uint8_t class) {
    switch (class) {
        case 0: return REAL_CLASS_AIR_CLERIC;
        case 1: return REAL_CLASS_AIR_DRUID;
        case 2: return REAL_CLASS_FIGHTER;
        case 3: return REAL_CLASS_GLADIATOR;
        case 4: return REAL_CLASS_PRESERVER;
        case 5: return REAL_CLASS_PSIONICIST;
        case 6: return REAL_CLASS_AIR_RANGER;
        case 7: return REAL_CLASS_THIEF;
    }

    return 0; // UNKNOWN CLASS
}

static int has_class(const uint16_t class) {
    for (int i = 0; i < 3; i++) {
        if (pc.class[i].class == class) { return 1; }
    }
    return 0;
}

static int is_divine_spell_user() {
    return     has_class(REAL_CLASS_AIR_CLERIC)
            || has_class(REAL_CLASS_EARTH_CLERIC)
            || has_class(REAL_CLASS_FIRE_CLERIC)
            || has_class(REAL_CLASS_WATER_CLERIC)
            || has_class(REAL_CLASS_AIR_DRUID)
            || has_class(REAL_CLASS_EARTH_DRUID)
            || has_class(REAL_CLASS_FIRE_DRUID)
            || has_class(REAL_CLASS_WATER_DRUID)
            || has_class(REAL_CLASS_AIR_RANGER)
            || has_class(REAL_CLASS_EARTH_RANGER)
            || has_class(REAL_CLASS_FIRE_RANGER)
            || has_class(REAL_CLASS_WATER_RANGER);
}

static void set_class_frames() {
    int next_class =
        (pc.class[0].class == -1) ? 0 :
        (pc.class[1].class == -1) ? 1 :
        (pc.class[2].class == -1) ? 2 :
        3;

    for (int i = 0; i < 8; i++) {
        pc.class[next_class].class = convert_to_actual_class(i);
        if (sprite_get_frame(class_sel[i]) != 1) {
            sprite_set_frame(classes[i], 
                (next_class < 3 && dnd2e_is_class_allowed(pc.race, pc.class
                ))
                ? 0 : 2);
        }
    }
    pc.class[next_class].class = -1;
    update_ui();
}

static uint16_t new_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

static void load_character_sprite() {
    const float zoom = main_get_zoom();
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    int race = (pc.race - 1) * 2;
    int gender = pc.gender - 1;

    if (spr != SPRITE_ERROR) {
        sprite_free(spr);
        spr = SPRITE_ERROR;
    }

    spr = new_sprite_create(renderer, pal,
                            offsetx / zoom + race_sprite_offsets_x[race + gender],
                            offsety / zoom + race_sprite_offsets_y[race + gender],
                            main_get_zoom(), OBJEX_GFF_INDEX, GFF_BMP,
                            race_sprite_ids[race + gender]);
}

static void init_pc() {
    memset(&pc, 0x0, sizeof(entity_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.alignment = TRUE_NEUTRAL;
    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    pc.class[0].level = pc.class[1].level = pc.class[2].level = -1;
    memset(&psi, 0x0, sizeof(psi));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    pc.allegiance = 1;
    get_random_name();
}

static void new_character_init(SDL_Renderer* _renderer, const uint32_t _x, const uint32_t _y) {
    gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = main_get_zoom();
    offsetx = _x; offsety = _y;
    uint32_t x = _x / zoom;
    uint32_t y = _y / zoom;
    renderer = _renderer;
    current_textbox = TEXTBOX_NONE;

    is_valid = 0;
    spr = SPRITE_ERROR;
    background = new_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13001);
    parchment[0] = new_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20084);
    parchment[1] = new_sprite_create(renderer, pal, 135 + x, 20 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20083);
    parchment[2] = new_sprite_create(renderer, pal, 130 + x, 70 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20085);
    parchment[3] = new_sprite_create(renderer, pal, 210 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20086);
    parchment[4] = new_sprite_create(renderer, pal, 210 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20087);
    done = new_sprite_create(renderer, pal, 250 + x, 160 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2000);
    sphere_label = new_sprite_create(renderer, pal, 217 + x, 140 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2046);
    psionic_label = new_sprite_create(renderer, pal, 217 + x, 140 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2047);
    done_button = new_sprite_create(renderer, pal, 240 + x, 174 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2000);
    exit_button = new_sprite_create(renderer, pal, 255 + x, 156 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2058);
    name_tb = textbox_create(32, 34 + offsetx / zoom, 124 + offsety / zoom);
    main_set_textbox(name_tb);

    init_pc();
    text_cursor = new_sprite_create(renderer, pal, 170 + x, 150 + y, // Blinking text cursor (for the player's name)
        zoom, RESOURCE_GFF_INDEX, GFF_ICON, 100);
    sprite_set_frame(text_cursor, 1);

    float shrink = .28 / zoom;
    float magnify = 1.0 / shrink;
    for (int i = 0; i < 6; i++) { // STR, DEX, CON, INT, WIS, CHA BUTTONS
        stats_align_hp_buttons[i] = sprite_new(renderer, pal, (4 + x) * magnify, (138 + y + (i * 7.5)) * magnify,
            zoom * shrink, RESOURCE_GFF_INDEX, GFF_BMP, 5013);
        sprite_set_frame(stats_align_hp_buttons[i], 1);
    }
    stats_align_hp_buttons[6] = new_sprite_create(renderer, pal, (79 + x) * magnify, (145 + y) * magnify, // ALIGNMENT BUTTON
        zoom * shrink, RESOURCE_GFF_INDEX, GFF_BMP, 5013);
    sprite_set_frame(stats_align_hp_buttons[6], 1);
    stats_align_hp_buttons[7] = new_sprite_create(renderer, pal, (89 + x) * magnify, (175 + y) * magnify, // HP BUTTON
        zoom * shrink, RESOURCE_GFF_INDEX, GFF_BMP, 5013);
    sprite_set_frame(stats_align_hp_buttons[7], 1);


    for (int i = 0; i < 8; i++) {
        classes[i] = new_sprite_create(renderer, pal, 220 + x, 10 + y + (i * 8),
            zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2002 + i);
        class_sel[i] = new_sprite_create(renderer, pal, 220 + x, 11 + y + (i * 8),
            zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20047);
        sprite_set_frame(classes[i], 2);
    }
    for (int i = 0; i < 3; i++) {
        psionic_devotion[i] = new_sprite_create(renderer, pal, 217 + x, 105 + y + (i * 8),
            zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2038 + i);
        sprite_set_frame(psionic_devotion[i], 0);
    }
    for (int i = 0; i < 4; i++) {
        spheres[i] = new_sprite_create(renderer, pal, 216 + x, 105 + y + (i * 8),
            zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2042 + i);
        ps_sel[i] = new_sprite_create(renderer, pal, 218 + x, 106 + y + (i * 8),
            zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20047);
        sprite_set_frame(spheres[i], 0);
    }
    for (int i = 0; i < 11; i++) {
        die[i] = new_sprite_create(renderer, pal, 142 + x, 65 + y,
            zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20048 + i);
    }
    for (int i = 0; i < 14; i++) {
        races[i] = new_sprite_create(renderer, pal,
                                     x + race_portrait_offsets_x[i == 13 ? 15 : i], // FIXME - Hack until/if female Muls are implemented
                                     y + race_portrait_offsets_y[i == 13 ? 15 : i], // FIXME - Hack until/if female Muls are implemented
                                     zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20000 + i);
    }

    strcpy(sphere_text, "PSI DISCIPLINES");

    srand(time(NULL));
    load_character_sprite();
    dnd2e_randomize_stats_pc(&pc);
    set_class_frames(); // go ahead and setup the new class frames
    select_class(2); // Fighter is the default class
    label_create_group();
}

static void update_die_countdown() {
    static int last_die_pos = -1;
    if (die_countdown) {
        die_countdown--;
        if (die_countdown > 30) {
            die_pos = 1;
        } else if (die_countdown > 20) {
            die_pos = 2;
        } else if (die_countdown > 10) {
            die_pos = 3;
        } else if (die_countdown > 0) {
            die_pos = 4;
        } else {
            die_pos = 5 + (rand() % 6);
        }
        if (die_pos != last_die_pos) {
            dnd2e_randomize_stats_pc(&pc);
        }
        last_die_pos = die_pos;
    }
}

static int get_race_id() { // for the large portrait
    if (pc.race < RACE_MUL) { return 2 * (pc.race - 1) + (pc.gender - 1); }
    return 12 + (pc.race - RACE_MUL);
}

void new_character_render(void* data, SDL_Renderer* renderer) {

    sprite_render(renderer, background);
    for (int i = 0; i < 5; i++) {
        sprite_render(renderer, parchment[i]);
    }
    for (int i = 0; i < 8; i++) {
        sprite_render(renderer, stats_align_hp_buttons[i]);
    }
    for (int i = 0; i < 8; i++) {
        sprite_render(renderer, classes[i]);
        sprite_render(renderer, class_sel[i]);
    }
    sprite_render(renderer, spr);
    update_die_countdown();
    sprite_render(renderer, die[die_pos]);
    sprite_render(renderer, races[get_race_id()]);

    show_psionic_label ? strcpy(sphere_text, "PSI DISCIPLINES") :
        strcpy(sphere_text, "CLERICAL SPHERE");

    print_line_len(renderer, FONT_BLACKDARK, sphere_text, 446 + offsetx, 193 + offsety, 1 << 12);

    for (int i = 0; i < 4; i++) {
        if (i < 3 && show_psionic_label) {
            sprite_render(renderer, psionic_devotion[i]);
        }
        if (!show_psionic_label) {
            sprite_render(renderer, spheres[i]);
        }
        sprite_render(renderer, ps_sel[i]);
    }

    sprite_render(renderer, show_psionic_label ? sphere_label : psionic_label);
    sprite_render(renderer, done_button);
    sprite_render(renderer, exit_button);

    label_group_set_font(FONT_GREYLIGHT);
    label_set_group(&pc, SCREEN_NEW_CHARACTER);
    label_set_positions(9, 249, SCREEN_NEW_CHARACTER);
    label_render_full(offsetx, offsety);

    textbox_render(name_tb);
}

void update_stats_alignment_hp(int i, uint32_t button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            switch (i) {
                case 0: // STR
                    pc.stats.str++;
                    break;
                case 1: // DEX
                    pc.stats.dex++;
                    break;
                case 2: // CON
                    pc.stats.con++;
                    break;
                case 3: // INT
                    pc.stats.intel++;
                    break;
                case 4: // WIS
                    pc.stats.wis++;
                    break;
                case 5: // CHA
                    pc.stats.cha++;
                    break;
                case 6:
                    pc.alignment++;
                    fix_alignment(1); // 1 = next alignment
                    break;
                case 7:
                    pc.stats.hp++;
                    pc.stats.high_hp++;
                    break;
                default:
                    break;
            }
        break;
        case SDL_BUTTON_RIGHT:
            switch (i) {
                case 0: // STR
                    pc.stats.str--;
                    break;
                case 1: // DEX
                    pc.stats.dex--;
                    break;
                case 2: // CON
                    pc.stats.con--;
                    break;
                case 3: // INT
                    pc.stats.intel--;
                    break;
                case 4: // WIS
                    pc.stats.wis--;
                    break;
                case 5: // CHA
                    pc.stats.cha--;
                    break;
                case 6:
                    pc.alignment--;
                    fix_alignment(-1); // -1 = previous alignment
                    break;
                case 7:
                    pc.stats.hp--;
                    pc.stats.high_hp--;
                    break;
                default:
                    break;
            }
        break;
    }

    dnd2e_loop_creation_stats(&pc);
}

static int find_class_selection(const uint8_t real_class) {
    for (int i = 0; i < 3; i++) {
        if (pc.class[i].class == real_class) { return i; }
    }
    return -1;
}

static int find_class_selection_position(const uint8_t class_selection) {
    int sel = -1;
    switch(class_selection) {
        case 0:
            sel = find_class_selection(REAL_CLASS_AIR_CLERIC);
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_EARTH_CLERIC); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_FIRE_CLERIC); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_WATER_CLERIC); }
            return sel;
        case 1:
            sel = find_class_selection(REAL_CLASS_AIR_DRUID);
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_EARTH_DRUID); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_FIRE_DRUID); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_WATER_DRUID); }
            return sel;
        case 2: return find_class_selection(REAL_CLASS_FIGHTER);
        case 3: return find_class_selection(REAL_CLASS_GLADIATOR);
        case 4: return find_class_selection(REAL_CLASS_PRESERVER);
        case 5: return find_class_selection(REAL_CLASS_PSIONICIST);
        case 6:
            sel = find_class_selection(REAL_CLASS_AIR_RANGER);
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_EARTH_RANGER); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_FIRE_RANGER); }
            if (sel == -1) { sel = find_class_selection(REAL_CLASS_WATER_RANGER); }
            return sel;
        case 7: return find_class_selection(REAL_CLASS_THIEF);
    }
    return -1;
}

static void set_ps_sel_frames() {
    int ps_selections = 0;

    // Setup selection correctly
    if (show_psionic_label) {
        sprite_set_frame(ps_sel[0], spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC));
        sprite_set_frame(ps_sel[1], spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM));
        sprite_set_frame(ps_sel[2], spell_has_psin(&psi, PSIONIC_TELEPATH));
        sprite_set_frame(ps_sel[3], 0);

        ps_selections = spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC)
               + spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM)
               + spell_has_psin(&psi, PSIONIC_TELEPATH);

        if (ps_selections > 0) {
            sprite_set_frame(psionic_devotion[0], spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC) ? 0 : 2);
            sprite_set_frame(psionic_devotion[1], spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM) ? 0 : 2);
            sprite_set_frame(psionic_devotion[2], spell_has_psin(&psi, PSIONIC_TELEPATH) ? 0 : 2);
        } else {
            sprite_set_frame(psionic_devotion[0], 0);
            sprite_set_frame(psionic_devotion[1], 0);
            sprite_set_frame(psionic_devotion[2], 0);
        }
    } else {
        sprite_set_frame(ps_sel[0], (sphere_selection == 0) ? 1 : 0);
        sprite_set_frame(ps_sel[1], (sphere_selection == 1) ? 1 : 0);
        sprite_set_frame(ps_sel[2], (sphere_selection == 2) ? 1 : 0);
        sprite_set_frame(ps_sel[3], (sphere_selection == 3) ? 1 : 0);

        if (sphere_selection == -1) {
            sprite_set_frame(spheres[0], 0);
            sprite_set_frame(spheres[1], 0);
            sprite_set_frame(spheres[2], 0);
            sprite_set_frame(spheres[3], 0);
        } else {
            sprite_set_frame(spheres[0], (sphere_selection == 0) ? 0 : 2);
            sprite_set_frame(spheres[1], (sphere_selection == 1) ? 0 : 2);
            sprite_set_frame(spheres[2], (sphere_selection == 2) ? 0 : 2);
            sprite_set_frame(spheres[3], (sphere_selection == 3) ? 0 : 2);
        }
    }
}

static void toggle_psi(const uint16_t i) {
    int cframe = sprite_get_frame(ps_sel[i]);
    int ps_selections = spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC)
           + spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM)
           + spell_has_psin(&psi, PSIONIC_TELEPATH);

    if (ps_selections > 2) { return; }

    spell_set_psin(&psi, i, cframe == 1 ? 0 : 1);

    set_ps_sel_frames();
}

static void toggle_sphere(const uint16_t i) {
    int cframe = sprite_get_frame(ps_sel[i]);
    sprite_set_frame(ps_sel[i], cframe == 1 ? 0 : 1);

    sphere_selection = (cframe) ? -1 : i;

    set_ps_sel_frames();
}

static void deselect_class(uint8_t class_selection) {
    int class_pos = find_class_selection_position(class_selection);
    for (int i = class_pos; i < 2; i++) {
        pc.class[i] = pc.class[i + 1];
    }

    if (convert_to_actual_class(class_selection) == REAL_CLASS_PSIONICIST) {
        spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 0);
        spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 0);
        spell_set_psin(&psi, PSIONIC_TELEPATH, 0);
    }

    // If the player doesn't have a Cleric, Druid, or Ranger class, reset their sphere
    if (!is_divine_spell_user())
        sphere_selection = -1;

    memset(pc.class + 2, 0x0, sizeof(class_t));
    pc.class[2].level = pc.class[2].class = -1;
    sprite_set_frame(class_sel[class_selection], 0);
    show_psionic_label = 1;

    set_class_frames();
    set_ps_sel_frames();

    dnd2e_set_exp(&pc, 4000);
    
    if (pc.class[0].class == -1) {
        pc.alignment = TRUE_NEUTRAL;
    }
}

static void select_class(uint8_t class) {
    int pos = 0;
    if (pc.class[pos].class != -1) { pos++; }
    if (pc.class[pos].class != -1) { pos++; }
    sprite_set_frame(class_sel[class], 1);

    pc.alignment = pc.class[0].class == -1 ? TRUE_NEUTRAL : pc.alignment;
    pc.class[pos].class = convert_to_actual_class(class);
    pc.class[pos].level = 0;
    set_class_frames();

    if (pc.class[pos].class == REAL_CLASS_PSIONICIST) {
        spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);
        spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 1);
        spell_set_psin(&psi, PSIONIC_TELEPATH, 1);
    } else if (!spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC) &&
               !spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM) &&
               !spell_has_psin(&psi, PSIONIC_TELEPATH)) {
        spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1); // Default psi discipline
    }

    // Force Cleric/Druid/Ranger to have a sphere chosen
    if (is_divine_spell_user() && sphere_selection == -1)
        sphere_selection = 0;

    set_ps_sel_frames();
    dnd2e_set_exp(&pc, 4000);
    fix_alignment(1); // 1 = next alignment
    dnd2e_loop_creation_stats(&pc); // in case something need adjustment
}

static void fix_race_gender() { // move the race/gender to the appropiate spot
    if (pc.gender > GENDER_FEMALE) {
        pc.gender = GENDER_MALE;
        pc.race++;
    }
    else if (pc.gender < GENDER_MALE) {
        pc.gender = GENDER_FEMALE;
        pc.race--;
    }

    if (pc.race < RACE_HUMAN) { pc.race = RACE_THRIKREEN; }
    if (pc.race > RACE_THRIKREEN) { pc.race = RACE_HUMAN; }

    // FIXME: Add (optional?) support for Female Muls/Male Thri-Kreen if art assets
    // are ever created for them
    if (pc.race == RACE_MUL && pc.gender == GENDER_FEMALE) {
        pc.race = RACE_THRIKREEN;
    }

    if (pc.race == RACE_THRIKREEN && pc.gender == GENDER_MALE) {
        pc.race = RACE_MUL;
    }

    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    pc.class[0].level = pc.class[1].level = pc.class[2].level = -1;
    for (int i = 0; i < 8; i++)
        sprite_set_frame(class_sel[i], 0);

    if (!changed_name) { // Only automatically generate a name if the player hasn't modified it
        get_random_name();
    }

    load_character_sprite(); // go ahead and get the new sprite
    set_class_frames(); // go ahead and setup the new class frames
    select_class(2); // Default to Fighter whenever race changes
    dnd2e_randomize_stats_pc(&pc);
    dnd2e_loop_creation_stats(&pc); // in case something need adjustment
}

static void fix_alignment(int direction) { // direction: -1 = previous alignment, 1 = next alignment
    if ((int8_t)pc.alignment < LAWFUL_GOOD) { // pc.alignment is unsigned, need to convert to get negatives
        pc.alignment = CHAOTIC_EVIL;
    } else if (pc.alignment > CHAOTIC_EVIL) {
        pc.alignment = LAWFUL_GOOD;
    }

    if (!dnd2e_is_alignment_allowed(pc.alignment, pc.class, 1))
    {
        for (int i = pc.alignment + direction; i != pc.alignment; i += direction) {
            if (i < LAWFUL_GOOD) {
                i = CHAOTIC_EVIL;
                direction = -1;
            } else if (i > CHAOTIC_EVIL) {
                i = LAWFUL_GOOD;
                direction = 1;
            }

            if (dnd2e_is_alignment_allowed(i, pc.class, 1))
            {
                pc.alignment = i;
                break;
            }
        }
    }
}

int new_character_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_spr = SPRITE_ERROR;
    uint16_t cspr = SPRITE_ERROR;

    for (int i = 0; i < 8; i++) {
        if (sprite_in_rect(classes[i], x, y)) {
            cspr = classes[i];
        }
    }

    if (sprite_in_rect(sphere_label, x, y)) {
        cspr = show_psionic_label ? sphere_label : psionic_label;
    }

    if (show_psionic_label) {
        for (int i = 0; i < 3; i++) {
            if (sprite_in_rect(psionic_devotion[i], x, y)) {
                cspr = psionic_devotion[i];
            }
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (sprite_in_rect(spheres[i], x, y)) {
                cspr = spheres[i];
            }
        }
    }
    sprite_set_frame(done_button, 0);
    sprite_set_frame(exit_button, 0);
    if (sprite_in_rect(done_button, x, y)) {
        cspr = done_button;
    }
    if (sprite_in_rect(exit_button, x, y)) {
        cspr = exit_button;
    }

    if (sprite_get_frame(cspr) < 2) {
        sprite_set_frame(cspr, 1);
    }

    if (last_spr != SPRITE_ERROR && last_spr != cspr) {
        if (sprite_get_frame(last_spr) < 2) {
            sprite_set_frame(last_spr, 0);
        }
    }

    last_spr = cspr;

    return 1;// handle
}

int new_character_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    // Only support Left and Right mouse buttons for now
    if (button != SDL_BUTTON_LEFT && button != SDL_BUTTON_RIGHT) {
        return 1; // Handle
    }

    last_sprite_mousedowned = 0;
    last_label_mousedowned  = label_group_point_in(x - offsetx, y - offsety);

    if (sprite_in_rect(done_button, x, y)) {
        last_sprite_mousedowned = done_button;
        sprite_set_frame(done_button, 2);
    }

    if (sprite_in_rect(exit_button, x, y)) {
        last_sprite_mousedowned = exit_button;
        sprite_set_frame(exit_button, 2);
    }

    for (int i = 0; i < 8; i++) {
        if (sprite_in_rect(stats_align_hp_buttons[i], x, y)) {
            last_sprite_mousedowned = stats_align_hp_buttons[i];
        }
    }

    if (sprite_in_rect(parchment[2], x, y)) { // Was die[die_pos]
        last_sprite_mousedowned = parchment[2];
    }

    for (int i = 0; i < 8; i++) {
        if (sprite_in_rect(classes[i], x, y)) {
            last_sprite_mousedowned = classes[i];
        }
    }

    if (sprite_in_rect(parchment[0], x, y)) { // Change race/gender via portrait - Was races[pc.race]
        last_sprite_mousedowned = parchment[0];
    }

    if (sprite_in_rect(parchment[1], x, y)) { // Change race/gender via sprite
        last_sprite_mousedowned = parchment[1];
    }

    for (int i = 0; i < 4; i++) {
        if (i < 3 && show_psionic_label) {
            if (sprite_in_rect(psionic_devotion[i], x, y)) {
                last_sprite_mousedowned = psionic_devotion[i];
            }
        }

        if (!show_psionic_label) {
            if (sprite_in_rect(spheres[i], x, y)) {
                last_sprite_mousedowned = spheres[i];
            }
        }
    }

    if (show_psionic_label && sprite_get_frame(psionic_label) < 2 && sprite_in_rect(psionic_label, x, y)) {
        last_sprite_mousedowned = psionic_label;
    } else if (!show_psionic_label && sprite_get_frame(sphere_label) < 2 && sprite_in_rect(sphere_label, x, y)) {
        last_sprite_mousedowned = sphere_label;
    }
    
    textbox_set_focus(name_tb, (textbox_is_in(name_tb, x, y)));

    return 1; // handle
}

int new_character_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    // Only support Left and Right mouse buttons for now
    if (button != SDL_BUTTON_LEFT && button != SDL_BUTTON_RIGHT) {
        return 1;
    }

    if (last_label_mousedowned != NULL &&
        last_label_mousedowned == label_group_point_in(x - offsetx, y - offsety))
    {
        switch (last_label_mousedowned->id)
        {
            case LABEL_STR:
            case LABEL_STR_VAL:
                update_stats_alignment_hp(0, button);
                break;
            case LABEL_DEX:
            case LABEL_DEX_VAL:
                update_stats_alignment_hp(1, button);
                break;
            case LABEL_CON:
            case LABEL_CON_VAL:
                update_stats_alignment_hp(2, button);
                break;
            case LABEL_INT:
            case LABEL_INT_VAL:
                update_stats_alignment_hp(3, button);
                break;
            case LABEL_WIS:
            case LABEL_WIS_VAL:
                update_stats_alignment_hp(4, button);
                break;
            case LABEL_CHA:
            case LABEL_CHA_VAL:
                update_stats_alignment_hp(5, button);
                break;
            case LABEL_ALIGNMENT:
                update_stats_alignment_hp(6, button);
                break;
            case LABEL_HP:
                update_stats_alignment_hp(7, button);
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < 8; i++) {
        if (last_sprite_mousedowned == stats_align_hp_buttons[i] && sprite_in_rect(stats_align_hp_buttons[i], x, y)) {
            update_stats_alignment_hp(i, button);
        }
    }

    if (last_sprite_mousedowned == parchment[2] && sprite_in_rect(parchment[2], x, y)) {
        die_countdown = 40;
    }

    for (int i = 0; i < 8; i++) {
        if (last_sprite_mousedowned == classes[i] && sprite_in_rect(classes[i], x, y)) {
            if (sprite_get_frame(class_sel[i]) == 1) {
                deselect_class(i);
            } else if (sprite_get_frame(classes[i]) < 2) {
                select_class(i);
            } 
        }
    }

    // Change race/gender via portrait
    if (last_sprite_mousedowned == parchment[0] && sprite_in_rect(parchment[0], x, y)) {
        if (button == SDL_BUTTON_LEFT) {
            pc.gender++;
        } else if (button == SDL_BUTTON_RIGHT) {
            pc.gender--;
        }

        fix_race_gender();
    }

    // Change race/gender via sprite - FIXME: This should change just the SPRITE (not race/gender) in DSO!
    if (last_sprite_mousedowned == parchment[1] && sprite_in_rect(parchment[1], x, y)) {
        if (button == SDL_BUTTON_LEFT) {
            pc.gender++;
        } else if (button == SDL_BUTTON_RIGHT) {
            pc.gender--;
        }

        fix_race_gender();
    }

    for (int i = 0; i < 4; i++) {
        if (i < 3 && show_psionic_label) {
            if (last_sprite_mousedowned == psionic_devotion[i] && sprite_in_rect(psionic_devotion[i], x, y)) {
                if (sprite_get_frame(psionic_devotion[i]) < 2) {
                    toggle_psi(i);
                }
            }
        }

        if (!show_psionic_label) {
            if (last_sprite_mousedowned == spheres[i] && sprite_in_rect(spheres[i], x, y)) {
                if (sprite_get_frame(spheres[i]) < 2) {
                    toggle_sphere(i);
                }
            }
        }

        sprite_render(renderer, ps_sel[i]);
    }

    if (show_psionic_label && sprite_get_frame(psionic_label) < 2 && last_sprite_mousedowned == psionic_label && sprite_in_rect(psionic_label, x, y)) {
        show_psionic_label = 0;
        set_ps_sel_frames();
    } else if (!show_psionic_label && sprite_get_frame(sphere_label) < 2 && last_sprite_mousedowned == sphere_label && sprite_in_rect(sphere_label, x, y)) {
        show_psionic_label = 1;
        set_ps_sel_frames();
    }

    if (last_sprite_mousedowned == done_button && sprite_in_rect(done_button, x, y)) {
        is_valid = 1;
        window_pop();
    }

    if (last_sprite_mousedowned == exit_button && sprite_in_rect(exit_button, x, y)) {
        is_valid = 0;
        window_pop();
    }

    return 1; // handle
}

static void update_ui() {
    int show_spheres = is_divine_spell_user();
    sprite_set_frame(sphere_label, show_spheres ? 0 : 2);
    sprite_set_frame(psionic_label, show_spheres ? 0 : 2);
}

void new_character_free() {
    sprite_free(background);
    for (int i = 0; i < 5; i++) {
        sprite_free(parchment[i]);
    }
    for (int i = 0; i < 8; i++) {
        sprite_free(classes[i]);
        sprite_free(class_sel[i]);
    }
    for (int i = 0; i < 3; i++) {
        sprite_free(psionic_devotion[i]);
    }
    for (int i = 0; i < 4; i++) {
        sprite_free(spheres[i]);
        sprite_free(ps_sel[i]);
    }
    for (int i = 0; i < 11; i++) {
        sprite_free(die[i]);
    }
    for (int i = 0; i < 14; i++) {
        sprite_free(races[i]);
    }
    sprite_free(done);
    sprite_free(sphere_label);
    sprite_free(psionic_label);
    sprite_free(done_button);
    sprite_free(exit_button);
    sprite_free(spr);
    main_set_textbox(NULL);
}

wops_t new_character_window = {
    .init = new_character_init,
    .cleanup = new_character_free,
    .render = new_character_render,
    .mouse_movement = new_character_handle_mouse_movement,
    .mouse_down = new_character_handle_mouse_down,
    .mouse_up = new_character_handle_mouse_up,
    .return_control = NULL,
    .grey_out_map = 1,
    .data = NULL
};
