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

static int die_pos = 0;
static int die_countdown = 0;

static ds_character_t pc; // the character we are creating.

static uint16_t new_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

static void init_pc() {
    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
}

static void new_character_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    init_pc();
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
    sprite_render(renderer, background);
    for (int i = 0; i < 5; i++) {
        sprite_render(renderer, parchment[i]);
    }
    for (int i = 0; i < 8; i++) {
        sprite_render(renderer, classes[i]);
    }
    /*
    for (int i = 0; i < 3; i++) {
        sprite_render(renderer, psionics[i]);
    }
    */
    for (int i = 0; i < 4; i++) {
        sprite_render(renderer, spheres[i]);
    }
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
