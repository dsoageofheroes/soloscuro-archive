#include "new-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "narrate.h"
#include "popup.h"
#include "../font.h"
#include "../sprite.h"
#include <time.h>

static uint16_t background;
static uint16_t parchment[5];
static uint16_t done;
static uint16_t classes[8];
static uint16_t psionics[3];
static uint16_t spheres[4];
static uint16_t die[11];
static uint16_t races[14];
static uint16_t spr;// sprite of the character on screen

//trikeen 750/751, GFF_BMP segobjex
//1052, female something.
//1169/1170, male somthing
//2053/2054, dwarf (male?)
//2055/2056, dwarf (female?)
//2059/2060, elf (female?)
//2061/2062, elf (male?)
//2068/2069, halfling male
//2070/2071, halfling female
//2072/2073, half-giant male
//2074/2075, half-giant female
//2093/2094, mul
//2095/2096, male human or half-elf
//2097/2098, trikeen
//2099/2100, female human or half-elf

uint8_t classes_allowed[][8] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 1, 1, 0},
    {1, 1, 1, 1, 0, 1, 1, 1},
    {1, 1, 1, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 0, 1, 1, 0},
};

uint8_t races_allowed[][8] = {
    {1, 1, 1, 1, 1, 1, 1, 1}, //Cleric
    {1, 0, 0, 1, 0, 1, 1, 1}, //Druid
    {1, 1, 1, 1, 1, 1, 1, 1}, //Fighter
    {1, 1, 1, 1, 1, 1, 1, 1}, //Gladiator
    {1, 0, 1, 1, 0, 0, 0, 0}, //Preserver
    {1, 1, 1, 1, 1, 1, 1, 1}, //Psionicist
    {1, 0, 1, 1, 1, 1, 0, 1}, //Ranger
    {1, 0, 1, 1, 1, 1, 0, 1}, //Thief
};

static int die_pos = 0;
static int die_countdown = 0;

static int offsetx, offsety;
static float zoom;
static SDL_Renderer *renderer;
static ds_character_t pc; // the character we are creating.

/*static int get_class_idx(const uint8_t class) {
    if (class < 1) { return -1; }
    if (class < 5) { return 1; } // Cleric
    if (class < 9) { return 2; } // Druid
    if (class < 13) { return class - 6; } // Fighter - Psionicist
    if (class < 17) { return 6; } // Ranger
    if (class < 18) { return 7; } // Thief

    return -1; // UNKNOWN CLASS
}*/

static int is_class_allowed(const uint8_t race, const int8_t classes[3]) {
    if (classes[0] == -1) { return 0; }

    printf("race = %d (%d, %d, %d)\n", race, classes[0], classes[1], classes[2]);
    if (classes[2] == -1 && classes[1] == -1) {
        printf("HERE! race = %d, ac[0] = %d, returning = %d\n", race, classes[0], races_allowed[race-1][classes[0]]);
        return races_allowed[race - 1][classes[0]];
    }

    return 0;
}

static uint16_t new_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

static void load_character_sprite() {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    if (spr != SPRITE_ERROR) {
        sprite_free(spr);
        spr = SPRITE_ERROR;
    }
    switch(pc.race) {
        case RACE_HALFELF:
        case RACE_HUMAN:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (pc.gender == GENDER_MALE) ? 2095 : 2099);
            break;
        case RACE_DWARF:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (pc.gender == GENDER_MALE) ? 2055 : 2053);
            break;
        case RACE_ELF:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (pc.gender == GENDER_MALE) ? 2061 : 2059);
            break;
        case RACE_HALFGIANT:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (pc.gender == GENDER_MALE) ? 2072 : 2074);
            break;
        case RACE_HALFLING:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (pc.gender == GENDER_MALE) ? 2068 : 2070);
            break;
        case RACE_MUL:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            break;
        case RACE_TRIKEEN:
            spr = new_sprite_create(renderer, pal, 150 + offsetx, 28 + offsetx,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            break;
    }
}

static void init_pc() {
    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
}

static void new_character_init(SDL_Renderer *_renderer, const uint32_t x, const uint32_t y, const float _zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    offsetx = x; offsety = y;
    zoom = _zoom;
    renderer = _renderer;

    init_pc();
    spr = SPRITE_ERROR;
    //load_character_sprite(renderer);
    background = new_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13001);
    parchment[0] = new_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20084);
    parchment[1] = new_sprite_create(renderer, pal, 135 + x, 20 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20083);
    parchment[2] = new_sprite_create(renderer, pal, 130 + x, 70 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20085);
    parchment[3] = new_sprite_create(renderer, pal, 210 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20086);
    parchment[4] = new_sprite_create(renderer, pal, 210 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20087);
    done = new_sprite_create(renderer, pal, 250 + x, 160 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2000);

    for (int i = 0; i < 8; i++) {
        classes[i] = new_sprite_create(renderer, pal, 220 + x, 10 + y + (i*8),
                zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2002 + i);
        sprite_set_frame(classes[i], 2);
    }
    for (int i = 0; i < 3; i++) {
        psionics[i] = new_sprite_create(renderer, pal, 220 + x, 108 + y + (i*8),
                zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2038 + i);
        sprite_set_frame(psionics[i], 2);
    }
    for (int i = 0; i < 4; i++) {
        spheres[i] = new_sprite_create(renderer, pal, 220 + x, 108 + y + (i*8),
                zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2042 + i);
        sprite_set_frame(spheres[i], 2);
    }
    for (int i = 0; i < 11; i++) {
        die[i] = new_sprite_create(renderer, pal, 142, 65,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20048 + i);
    }
    for (int i = 0; i < 14; i++) {
        races[i] = new_sprite_create(renderer, pal, 25, 12,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20000 + i);
    }

    srand(time(NULL));
    load_character_sprite(renderer);
}

static void update_die_countdown() {
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
    }
}

static int get_race_id() { // for the large portrait
    if (pc.race < RACE_MUL) { return 2 * (pc.race - 1) + (pc.gender - 1); }
    return 12 + (pc.race - RACE_MUL);
}

void new_character_render(void *data, SDL_Renderer *renderer) {
    int8_t class_sel[3];
    class_sel[0] = class_sel[1] = class_sel[2] = -1;
    sprite_render(renderer, background);
    for (int i = 0; i < 5; i++) {
        sprite_render(renderer, parchment[i]);
    }
    for (int i = 0; i < 8; i++) {
        class_sel[0] = i;
        sprite_set_frame(classes[i], 
             (is_class_allowed(pc.race, class_sel))
             ? 0 : 2);
        sprite_render(renderer, classes[i]);
    }
    for (int i = 0; i < 4; i++) {
        sprite_render(renderer, spheres[i]);
    }
    sprite_render(renderer, spr);
    update_die_countdown();
    sprite_render(renderer, die[die_pos]);
    sprite_render(renderer, races[get_race_id()]);
}

int new_character_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 1;// handle
}

int new_character_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1;// handle
}

static void fix_race_gender() { // move the race/gender to the appropiate spot
    if (pc.gender > GENDER_FEMALE) {
        pc.gender = GENDER_MALE;
        pc.race++;
    } else if (pc.gender < GENDER_MALE) {
        pc.gender = GENDER_FEMALE;
        pc.race--;
    }
    if (pc.race < RACE_HUMAN) { pc.race = RACE_TRIKEEN; }
    if (pc.race > RACE_TRIKEEN) { pc.race = RACE_HUMAN; }

    if (pc.race == RACE_MUL && pc.gender == GENDER_FEMALE) {
        pc.race = RACE_TRIKEEN;
    }

    if (pc.race == RACE_TRIKEEN && pc.gender == GENDER_MALE) {
        pc.race = RACE_MUL;
    }
    load_character_sprite(); // go ahead and get the new sprite
}

int new_character_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(die[die_pos], x, y)) {
        die_countdown = 40;
    }
    if (sprite_in_rect(races[pc.race], x, y)) {
        if (button == SDL_BUTTON_LEFT) {
            pc.gender++;
        } else {
            pc.gender--;
        }
        fix_race_gender();
    }
    return 1;// handle
}

void new_character_free() {
    sprite_free(background);
    for (int i = 0; i < 5; i++) {
        sprite_free(parchment[i]);
    }
    for (int i = 0; i < 8; i++) {
        sprite_free(classes[i]);
    }
    for (int i = 0; i < 3; i++) {
        sprite_free(psionics[i]);
    }
    for (int i = 0; i < 4; i++) {
        sprite_free(spheres[i]);
    }
    for (int i = 0; i < 11; i++) {
        sprite_free(die[i]);
    }
}

sops_t new_character_screen = {
    .init = new_character_init,
    .cleanup = new_character_free,
    .render = new_character_render,
    .mouse_movement = new_character_handle_mouse_movement,
    .mouse_down = new_character_handle_mouse_down,
    .mouse_up = new_character_handle_mouse_up,
    .return_control = NULL,
    .data = NULL
};
