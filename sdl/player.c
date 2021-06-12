#include <math.h>
#include "player.h"
#include "main.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "sprite.h"
#include "windows/narrate.h"
#include "../src/trigger.h"
#include "../src/entity-animation.h"
#include "../src/port.h"
#include "../src/region-manager.h"
#include "../src/player.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

//static animate_sprite_node_t *player_node[MAX_PCS] = {NULL, NULL, NULL, NULL} ;
static uint32_t get_bmp_idx(const entity_t *dude);
//static int player_zpos = 0;

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
} player_sprites_t;

static player_sprites_t players[MAX_PCS] = { 
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR}};

#define sprite_t uint16_t

void player_init() {
    //dude_t *dude = player_get_active();

    //dude->mapx = 30;
    //dude->mapy = 10;
}

void player_load_graphics(const int slot) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    dude_t *dude = player_get(slot);
    dude->sprite.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    if (!dude->sprite.data) {
        dude->sprite.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
        dude->sprite.bmp_id = get_bmp_idx(dude);
        dude->sprite.xoffset = 0;
        dude->sprite.yoffset = 0;
        dude->mapx = 30;
        dude->mapy = 10;
        dude->mapz = 0;
        dude->ds_id = 2095;
        region_add_entity(region_manager_get_current(), dude);
        port_add_entity(dude, pal);
        if (dude != player_get_active()) {
            port_update_entity(dude, -999, -999);
        }
    }
}

static int ticks_per_move = 10;
static int count = 0;
static int direction = 0x0;

void player_update() {
    entity_t *dude = player_get_active();
    int xdiff = 0, ydiff = 0;
    const enum combat_turn_t combat_turn = combat_player_turn();

    if (--count > 0) { return; }

    if (entity_animation_list_execute(&(dude->actions), region_manager_get_current())) {
        count = ticks_per_move;
        return;
    }

    // update when we can have the player take a turn.
    if (combat_turn != NO_COMBAT) { return; }

    if (direction & PLAYER_UP)    { ydiff -= 1; }
    if (direction & PLAYER_DOWN)  { ydiff += 1; }
    if (direction & PLAYER_LEFT)  { xdiff -= 1; }
    if (direction & PLAYER_RIGHT) { xdiff += 1; }

    if (main_player_freeze() || region_is_block(region_manager_get_current(),
                //dude->mapx + xdiff, dude->mapy + ydiff)) {
                dude->mapy + ydiff, dude->mapx + xdiff)) {
        xdiff = ydiff = 0;
    }

    if (!narrate_is_open()) {
        trigger_noorders(dude->mapx, dude->mapy);
    }
    trigger_box_check(dude->mapx, dude->mapy);
    trigger_tile_check(dude->mapx, dude->mapy);

    dude->sprite.scmd = entity_animation_get_scmd(dude->sprite.scmd,
            xdiff, ydiff, EA_NONE);
    port_update_entity(dude, xdiff, ydiff);

    // We aren't moving...
    if (xdiff == 0 && ydiff == 0) { return; }

    count = ticks_per_move;
}

void player_move(const uint8_t _direction) {
    direction |= _direction;
}

void player_unmove(const uint8_t _direction) {
    direction &= ~(_direction);
}

static void free_sprites(const int slot) {
    if (players[slot].main != SPRITE_ERROR) {
        sprite_free(players[slot].main);
        players[slot].main = SPRITE_ERROR;
    }
    if (players[slot].port != SPRITE_ERROR) {
        sprite_free(players[slot].port);
        players[slot].port = SPRITE_ERROR;
    }
}

static uint32_t get_bmp_idx(const entity_t *dude) {
    switch(dude->race) {
        case RACE_HALFELF:
            return (dude->gender == GENDER_MALE) ? 2095 : 2099;
        case RACE_HUMAN:
            return (dude->gender == GENDER_MALE) ? 2095 : 2099;
        case RACE_DWARF:
            return (dude->gender == GENDER_MALE) ? 2055 : 2053;
        case RACE_ELF:
            return (dude->gender == GENDER_MALE) ? 2061 : 2059;
        case RACE_HALFGIANT:
            return (dude->gender == GENDER_MALE) ? 2072 : 2074;
        case RACE_HALFLING:
            return (dude->gender == GENDER_MALE) ? 2068 : 2070;
        case RACE_MUL:
            return (dude->gender == GENDER_MALE) ? 2093 : 2093;
        case RACE_THRIKREEN:
            return (dude->gender == GENDER_MALE) ? 2097 : 2097;
    }

    return 2095; // when in doubt, male human.
}

static void load_character_sprite(SDL_Renderer *renderer, const int slot, const float zoom) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    dude_t *dude = player_get(slot);
    if (!dude) { return; }

    free_sprites(slot);

    switch(dude->race) {
        case RACE_HALFELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20006 : 20007);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2095 : 2099) + 1);
            break;
        case RACE_HUMAN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20000 : 20001);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2095 : 2099) + 1);
            break;
        case RACE_DWARF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20002 : 20003);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2055 : 2053);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2055 : 2053) + 1);
            break;
        case RACE_ELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20004 : 20005);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2061 : 2059);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2061 : 2059) + 1);
            break;
        case RACE_HALFGIANT:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20008 : 20009);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2072 : 2074);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2072 : 2074) + 1);
            break;
        case RACE_HALFLING:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20010 : 20011);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2068 : 2070);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2068 : 2070) + 1);
            break;
        case RACE_MUL:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, 2094);
            break;
        case RACE_THRIKREEN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            //sprite_append(players[slot].main, renderer, pal, 0, 0,
                //zoom, OBJEX_GFF_INDEX, GFF_BMP, 2098);
            break;
    }

}

extern void player_condense() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = player_get(i);
        if (player != player_get_active() && player->name) {
            port_update_entity(player, -999, -999);
        }
    }
}

extern void port_player_load(const int slot) {
    player_load(slot, main_get_zoom());
    player_load_graphics(slot);
}

void player_load(const int slot, const float zoom) {
    load_character_sprite(main_get_rend(), slot, zoom);
}

void player_render(SDL_Renderer *rend, const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sprite_render(rend, players[slot].main);
}

void player_render_portrait(SDL_Renderer *rend, const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sprite_render(rend, players[slot].port);
}

void player_center(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sprite_center(players[slot].main, x, y, w, h);
}

void player_center_portrait(const int slot, const int x, const int y, const int w, const int h) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    sprite_center(players[slot].port, x, y, w, h);
}

uint16_t player_get_sprite(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SPRITE_ERROR; }

    if (players[slot].main == SPRITE_ERROR) {
        load_character_sprite(main_get_rend(), slot, main_get_zoom());
    }

    return players[slot].main;
}

void player_set_delay(const int amt) {
    if (amt < 0) { return; }

    for (int i = 0; i < 4; i++) {
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT)[i].delay = amt;
    }
}

void player_set_move(const int amt) {
    if (amt < 0) { return; }
    ticks_per_move = amt;
}

void player_close() {
    //dude_t *dude;

    for (int i = 0; i < MAX_PCS; i++) {
        //dude = player_get(i);
        free_sprites(i);
        //port_remove_entity(dude);
        //printf("dude = %p\n", dude);
        //if (dude && dude->sprite.data) {
            //sprite_free(((animate_sprite_node_t*)dude->sprite.data)->anim->spr);
        //}
    }
    player_cleanup();
}
