#include <string.h>
#include <stdlib.h>
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

static entity_t *players[MAX_PCS] = {NULL, NULL, NULL, NULL};
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
        sol_sprite_free(players_spr[slot].main);
        players_spr[slot].main = SPRITE_ERROR;
    }
    if (players_spr[slot].port != SPRITE_ERROR) {
        sol_sprite_free(players_spr[slot].port);
        players_spr[slot].port = SPRITE_ERROR;
    }
}

static void load_character_sprite(const int slot, const float zoom) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    dude_t *dude = player_get(slot);
    if (!dude) { return; }

    free_sprites(slot);

    switch(dude->race) {
        case RACE_HALFELF:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20006 : 20007);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            break;
        case RACE_HUMAN:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20000 : 20001);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            dude->anim.spr = players_spr[slot].main;
            break;
        case RACE_DWARF:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20002 : 20003);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2055 : 2053);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2055 : 2053;
            break;
        case RACE_ELF:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20004 : 20005);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2061 : 2059);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2061 : 2059;
            break;
        case RACE_HALFGIANT:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20008 : 20009);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2072 : 2074);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2072 : 2074;
            break;
        case RACE_HALFLING:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20010 : 20011);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2068 : 2070);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2068 : 2070;
            break;
        case RACE_MUL:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            dude->sprite.bmp_id = 2093;
            break;
        case RACE_THRIKREEN:
            players_spr[slot].port = sol_sprite_new(pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013);
            players_spr[slot].main = sol_sprite_new(pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            dude->sprite.bmp_id = 2097;
            break;
    }
}

void sol_player_render(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sol_sprite_render(players_spr[slot].main);
}

void sol_player_render_portrait(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sol_sprite_render(players_spr[slot].port);
}

void sol_player_center(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sol_sprite_center(players_spr[slot].main, x, y, w, h);
}

void sol_player_center_portrait(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sol_sprite_center(players_spr[slot].port, x, y, w, h);
}

uint16_t sol_player_get_sprite(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SPRITE_ERROR; }

    if (players_spr[slot].main == SPRITE_ERROR) {
        load_character_sprite(slot, settings_zoom());
    }

    return players_spr[slot].main;
}

extern void player_cleanup() {
    sol_region_manager_remove_players();
    for (int i = 0; i < MAX_PCS; i++) {
        player_free(i);
    }
}

extern void player_free(const int slot) {
    if (players[slot]) {
        entity_free(players[slot]);
    }
    players[slot] = NULL;
}

extern void sol_player_init() {
    // Setup the slots for reading/writing
    for (int i = 0; i < MAX_PCS; i++) {
        if (!players[i]) {
            players[i] = player_get(i);
        }
    }
}

extern entity_t* player_get(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return players[slot];
}

extern void player_set(const int slot, entity_t *dude) {
    if (slot < 0 || slot >= MAX_PCS || !dude) { return; }
    players[slot] = dude;
    if (!players[slot] && active == slot) {
        active = -1;
        for (int i = 0; i < MAX_PCS; i++) {
            if (players[i]) { active = i; return; }
        }
    }
    if (active < 0 && players[slot]) {
        player_set_active(slot);
    }
}

extern int player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    dude_t *player = player_get(slot);
    return player && (player->name != NULL);
}

extern void player_set_active(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    if (players[slot]) { active = slot; }
}

extern entity_t* player_get_active() {
    return player_get(active);
}

extern int player_get_active_slot() {
    return active;
}

extern int player_get_slot(entity_t *entity) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (entity == players[i]) { return i; }
    }

    return -1;
}

extern int player_ai(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return ai[slot];
}

extern void player_set_ai(const int slot, const int _ai) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    ai[slot] = _ai;
}

void sol_player_close() {
    //dude_t *dude;

    for (int i = 0; i < MAX_PCS; i++) {
        //dude = player_get(i);
        free_sprites(i);
        //port_remove_entity(dude);
        //printf("dude = %p\n", dude);
        //if (dude && dude->sprite.data) {
            //sol_sprite_free(((animate_sprite_node_t*)dude->sprite.data)->anim->spr);
        //}
    }
    player_cleanup();
}

void sol_player_load_graphics(const int slot) {
    dude_t *dude = player_get(slot);
    dude->anim.scmd = sol_combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    load_character_sprite(slot, settings_zoom());
}

extern void sol_player_load_zoom(const int slot, const float zoom) {
    load_character_sprite(slot, zoom);
}

enum entity_action_e last_action[MAX_PCS] = { EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN };

static int ticks_per_move = 30;
static int count = 0;
static int direction = 0x0;

extern void sol_player_update() {
    entity_t *dude = player_get_active();
    int xdiff = 0, ydiff = 0;
    combat_turn_t combat_turn = sol_combat_player_turn();
    enum entity_action_e action;
    static int moving = 0;

    if (!sol_started()) { return; }

    if (--count > 0) { return; }

    // update when we can have the player take a turn.
    if (combat_turn != NO_COMBAT) { return; }

    if (direction & PLAYER_UP)    { ydiff -= 1; }
    if (direction & PLAYER_DOWN)  { ydiff += 1; }
    if (direction & PLAYER_LEFT)  { xdiff -= 1; }
    if (direction & PLAYER_RIGHT) { xdiff += 1; }

    if (sol_player_freeze() || sol_region_is_block(sol_region_manager_get_current(),
                dude->mapy + ydiff, dude->mapx + xdiff)) {
        xdiff = ydiff = 0;
    }

    if (!narrate_is_open()) {
        sol_trigger_noorders(dude->mapx, dude->mapy);
    }
    sol_trigger_box_check(dude->mapx, dude->mapy);
    sol_trigger_tile_check(dude->mapx, dude->mapy);

    // We aren't moving...
    if (xdiff == 0 && ydiff == 0) {
        dude->anim.movex = dude->anim.movey = 0.0;
        if (moving && dude->actions.head == NULL) {
            entity_animation_list_add(&(dude->actions), EA_NONE, dude, NULL, NULL, 1);
            moving = 0;
        }
        return;
    }

    moving = 1;
    action =
          (xdiff == 1 && ydiff == 1) ? EA_WALK_DOWNRIGHT
        : (xdiff == 1 && ydiff == -1) ? EA_WALK_UPRIGHT
        : (xdiff == -1 && ydiff == -1) ? EA_WALK_UPLEFT
        : (xdiff == -1 && ydiff == 1) ? EA_WALK_DOWNLEFT
        : (xdiff == 1) ? EA_WALK_RIGHT
        : (xdiff == -1) ? EA_WALK_LEFT
        : (ydiff == 1) ? EA_WALK_DOWN
        : (ydiff == -1) ? EA_WALK_UP
        : EA_NONE;
    last_action[player_get_active_slot()] = action;

    entity_animation_list_add_speed(&(dude->actions), action, dude, NULL, NULL, ticks_per_move, 2, 0);
    count = ticks_per_move / 2;

    dude->mapx += xdiff;
    dude->mapy += ydiff;
    dude->anim.destx += (xdiff * 32);
    dude->anim.desty += (ydiff * 32);
    sol_region_t *reg = sol_region_manager_get_current();
    if (reg) {
        animation_shift_entity(reg->entities, entity_list_find(reg->entities, dude));
    }
}

extern void sol_player_move(const uint8_t _direction) {
    direction |= _direction;
}

extern void sol_player_unmove(const uint8_t _direction) {
    direction &= ~(_direction);
}

extern void sol_player_condense() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = player_get(i);
        if (player != player_get_active() && player->name) {
            port_update_entity(player, -999, -999);
        }
    }
}

extern void sol_player_load(const int slot) {
    if (slot >= 0 && slot < MAX_PCS && players[slot]->anim.spr == SPRITE_ERROR) {
        sol_player_load_zoom(slot, settings_zoom());
        sol_player_load_graphics(slot);
        sol_map_place_entity(player_get(slot));
    }
}

extern void sol_player_set_delay(const int amt) {
    if (amt < 0) { return; }

    for (int i = 0; i < 4; i++) {
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN)[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP)[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT)[i].delay = amt;
        sol_combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT)[i].delay = amt;
    }
}

extern void sol_player_set_move(const int amt) {
    if (amt < 0) { return; }
    ticks_per_move = amt;
}

extern inventory_t* sol_player_get_inventory(const int slot) {
    if (!players[slot]->inv) { players[slot]->inv = sol_inventory_create(); }
    return (inventory_t*)players[slot]->inv;
}

