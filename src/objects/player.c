#include <string.h>
#include <stdlib.h>
#include "combat-region.h"
#include "gff-common.h"
#include "gfftypes.h"
#include "gff.h"
#include "player.h"
#include "map.h"
#include "wizard.h"
#include "rules.h"
#include "region-manager.h"
#include "port.h"
#include "settings.h"
#include "gameloop.h"
#include "narrate.h"
#include "trigger.h"
#include "arbiter.h"

static sol_entity_t *players[MAX_PCS] = {NULL, NULL, NULL, NULL};
static int ai[MAX_PCS] = {0, 0, 0, 0};

typedef struct player_sprites_s {
    sol_sprite_t main;
    sol_sprite_t port;
} player_sprites_t;

static player_sprites_t players_spr[MAX_PCS] = { 
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR}};

#define BUF_MAX (1<<12)

static int active = -1;

static void free_sprites(const int slot) {
    if (players_spr[slot].main != SPRITE_ERROR) {
        sol_status_check(sol_sprite_free(players_spr[slot].main), "Unable to free sprite.");
        players_spr[slot].main = SPRITE_ERROR;
    }
    if (players_spr[slot].port != SPRITE_ERROR) {
        sol_status_check(sol_sprite_free(players_spr[slot].port), "Unable to free sprite.");
        players_spr[slot].port = SPRITE_ERROR;
    }
}

static sol_status_t load_character_sprite(const int slot, const float zoom) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_SUCCESS; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    sol_dude_t *dude;
    sol_player_get(slot, &dude);
    if (!dude) { return SOL_NULL_ARGUMENT; }

    free_sprites(slot);

    switch(dude->race) {
        case RACE_HALFELF:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20006 : 20007,
                &players_spr[slot].port),
                    "Unable to load portrait for half-elf");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099,
                &players_spr[slot].main),
                    "Unable to load main sprite for half-elf");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            break;
        case RACE_HUMAN:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20000 : 20001,
                &players_spr[slot].port),
                    "Unable to load portrait for human");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099,
                &players_spr[slot].main),
                    "Unable to load main sprite for human");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            dude->anim.spr = players_spr[slot].main;
            break;
        case RACE_DWARF:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20002 : 20003,
                &players_spr[slot].port),
                    "Unable to load portrait for dwarf");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2055 : 2053,
                &players_spr[slot].main),
                    "Unable to load main sprite for dwarf");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2055 : 2053;
            break;
        case RACE_ELF:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20004 : 20005,
                &players_spr[slot].port),
                    "Unable to load portrait for elf");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2061 : 2059,
                &players_spr[slot].main),
                    "Unable to load main sprite for elf");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2061 : 2059;
            break;
        case RACE_HALFGIANT:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20008 : 20009,
                &players_spr[slot].port),
                    "Unable to load portrait for half-giant");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2072 : 2074,
                &players_spr[slot].main),
                    "Unable to load main sprite for half-giant");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2072 : 2074;
            break;
        case RACE_HALFLING:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20010 : 20011,
                &players_spr[slot].port),
                    "Unable to load portrait for halfling");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2068 : 2070,
                &players_spr[slot].main),
                    "Unable to load main sprite for halfling");
            dude->anim.bmp_id = (dude->gender == GENDER_MALE) ? 2068 : 2070;
            break;
        case RACE_MUL:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012,
                &players_spr[slot].port),
                    "Unable to load portrait for mul");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093,
                &players_spr[slot].main),
                    "Unable to load main sprite for mul");
            dude->anim.bmp_id = 2093;
            break;
        case RACE_THRIKREEN:
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013,
                &players_spr[slot].port),
                    "Unable to load portrait for thrikreen");
            sol_status_check(sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097,
                &players_spr[slot].main),
                    "Unable to load main sprite for thrikreen");
            dude->anim.bmp_id = 2097;
            break;
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_render(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_SUCCESS; }
    return sol_sprite_render(players_spr[slot].main);
}

extern sol_status_t sol_player_render_portrait(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    sol_sprite_render(players_spr[slot].port);
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_center(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    return sol_sprite_center(players_spr[slot].main, x, y, w, h);
}

extern sol_status_t sol_player_center_portrait(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    return sol_sprite_center(players_spr[slot].port, x, y, w, h);
}

extern sol_status_t sol_player_get_sprite(const int slot, sol_sprite_t *sprite) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }

    if (players_spr[slot].main == SPRITE_ERROR) {
        load_character_sprite(slot, settings_zoom());
    }

    *sprite = players_spr[slot].main;
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_cleanup() {
    sol_region_manager_remove_players();
    for (int i = 0; i < MAX_PCS; i++) {
        sol_player_free(i);
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_free(const int slot) {
    if (players[slot]) {
        sol_entity_free(players[slot]);
    }
    players[slot] = NULL;
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_init() {
    // Setup the slots for reading/writing
    for (int i = 0; i < MAX_PCS; i++) {
        if (!players[i]) {
            sol_player_get(i, &players[i]);
        }
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_get(const int slot, sol_entity_t **e) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_NULL_ARGUMENT; }
    *e = players[slot];
    //printf("player[%d] = %p\n", slot, players[slot]);
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_set(const int slot, sol_entity_t *dude) {
    if (slot < 0 || slot >= MAX_PCS || !dude) { return SOL_OUT_OF_RANGE; }

    players[slot] = dude;

    if (!players[slot] && active == slot) {
        active = -1;
        for (int i = 0; i < MAX_PCS; i++) {
            if (players[i]) { active = i; return SOL_SUCCESS; }
        }
    }

    if (active < 0 && players[slot]) {
        sol_player_set_active(slot);
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    sol_dude_t *player;
    sol_player_get(slot, &player);
    return (player && (player->name != NULL)) ? SOL_SUCCESS : SOL_NOT_FOUND;
}

extern sol_status_t sol_player_set_active(const int slot) {
    int prev = active;
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    if (players[slot]) { active = slot; }
    sol_map_update_active_player(prev);
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_get_active(sol_entity_t **e) {
    if (!e) { return SOL_NULL_ARGUMENT; }
    return sol_player_get(active, e);
}

extern sol_status_t sol_player_get_slot(sol_entity_t *entity, int *slot) {
    if (!entity || !slot) { return SOL_NULL_ARGUMENT; }

    *slot = -1;

    for (int i = 0; *slot == -1 && i < MAX_PCS; i++) {
        if (entity == players[i]) { *slot = i; }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_player_ai(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    return ai[slot] ? SOL_SUCCESS : SOL_NOT_FOUND;
}

extern sol_status_t sol_player_set_ai(const int slot, const int _ai) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    ai[slot] = _ai;
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_close() {
    for (int i = 0; i < MAX_PCS; i++) {
        free_sprites(i);
    }

    sol_player_cleanup();
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_load_graphics(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    sol_dude_t *dude;
    sol_player_get(slot, &dude);
    sol_combat_get_scmd(COMBAT_SCMD_STAND_DOWN, &dude->anim.scmd);
    return load_character_sprite(slot, settings_zoom());
}

extern sol_status_t sol_player_load_zoom(const int slot, const float zoom) {
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }
    return load_character_sprite(slot, zoom);
}

static int count = 0;
static int direction = 0x0;

static int game_over() {
    sol_entity_t *dude;

    for (int i = 0; i < MAX_PCS; i++) {
        sol_player_get(i, &dude);
        if (dude && dude->stats.hp > 0) { return 0; }
    }

    return 1;
}

extern sol_status_t sol_player_update() {
    sol_entity_t *dude;
    int       xdiff = 0, ydiff = 0;
    const int speed = 2;

    sol_player_get_active(&dude);
    if (game_over()) { sol_map_game_over(); return SOL_GAME_OVER; }
    if (sol_started() != SOL_SUCCESS) { return SOL_STOPPED; }

    if (--count > 0) { return SOL_OUT_OF_RANGE; }

    if (direction & PLAYER_UP)    { ydiff -= 1; }
    if (direction & PLAYER_DOWN)  { ydiff += 1; }
    if (direction & PLAYER_LEFT)  { xdiff -= 1; }
    if (direction & PLAYER_RIGHT) { xdiff += 1; }

    if (sol_player_freeze() == SOL_SUCCESS) {
        xdiff = ydiff = 0;
    }

    if (sol_narrate_is_open() != SOL_SUCCESS) {
        //sol_trigger_noorders(dude->mapx, dude->mapy);
        //sol_trigger_los_check();
    }
    if (xdiff != 0 || ydiff != 0) {
        sol_trigger_box_check(dude->mapx, dude->mapy);
        sol_trigger_tile_check(dude->mapx, dude->mapy);
    }

    sol_entity_attempt_move(dude, xdiff, ydiff, speed);

    count = settings_ticks_per_move() / speed;
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_move(const uint8_t _direction) {
    direction |= _direction;
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_unmove(const uint8_t _direction) {
    direction &= ~(_direction);
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_condense() {
    for (int i = 0; i < MAX_PCS; i++) {
        sol_entity_t *player, *active;
        sol_player_get(i, &player);
        sol_player_get_active(&active);
        if (player != active && player->name) {
            sol_entity_animation_update(player, -999, -999);
        }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_player_load(const int slot) {
    sol_dude_t *player;
    if (slot < 0 || slot >= MAX_PCS) { return SOL_OUT_OF_RANGE; }

    if (players[slot]->anim.scmd == NULL) { // load a new char
        sol_combat_set_scmd(players[slot], COMBAT_SCMD_STAND_DOWN);
        players[slot]->anim.spr = SPRITE_ERROR;
    }

    if (players[slot]->anim.spr == SPRITE_ERROR) {
        sol_player_load_zoom(slot, settings_zoom());
        sol_player_load_graphics(slot);
        sol_player_get(slot, &player);
        sol_map_place_entity(player);
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_player_set_delay(const int amt) {
    if (amt < 0) { return SOL_OUT_OF_RANGE; }
    scmd_t *s = NULL;

    for (int i = 0; i < 4; i++) {
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN, &s);
        s[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP, &s);
        s[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT, &s);
        s[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT, &s);
        s[i].delay = amt;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_player_get_inventory(const int slot, sol_inventory_t **i) {
    if (!i) { return SOL_NULL_ARGUMENT; }
    if (!players[slot]->inv) { sol_inventory_create(&players[slot]->inv); }
    *i = (sol_inventory_t*)players[slot]->inv;
    return SOL_SUCCESS;
}

