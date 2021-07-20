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

static void load_character_sprite(SDL_Renderer *renderer, const int slot, const float zoom);

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
} player_sprites_t;

static player_sprites_t players[MAX_PCS] = { 
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR},
    {SPRITE_ERROR, SPRITE_ERROR}};

enum entity_action_e last_action[MAX_PCS] = { EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN };

#define sprite_t uint16_t

void player_init() {
}

void player_load_graphics(const int slot) {
    dude_t *dude = player_get(slot);
    dude->anim.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    load_character_sprite(main_get_rend(), slot, main_get_zoom());
}

static int ticks_per_move = 30;
static int count = 0;
static int direction = 0x0;

void player_update() {
    entity_t *dude = player_get_active();
    int xdiff = 0, ydiff = 0;
    const enum combat_turn_t combat_turn = combat_player_turn();
    enum entity_action_e action;

    //if (entity_animation_execute(dude)) { --count; return; }
    //entity_animation_list_execute(&(dude->actions), region_manager_get_current());
    if (--count > 0) { return; }

    //if (entity_animation_list_execute(&(dude->actions), region_manager_get_current())) {
        //count = ticks_per_move;
        //return;
    //}

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

    // We aren't moving...
    if (xdiff == 0 && ydiff == 0) {
        dude->anim.movex = dude->anim.movey = 0.0;
        dude->anim.scmd = entity_animation_face_direction(dude->anim.scmd,
            last_action[player_get_active_slot()]);
        entity_animation_list_add(&(dude->actions), EA_NONE, dude, NULL, NULL, 1);
        return;
    }

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

    entity_animation_list_add_speed(&(dude->actions), action, dude, NULL, NULL, ticks_per_move, 2);
    count = ticks_per_move / 2;

    dude->mapx += xdiff;
    dude->mapy += ydiff;
    dude->anim.destx += (xdiff * 32);
    dude->anim.desty += (ydiff * 32);
    region_t *reg = region_manager_get_current();
    if (reg) {
        animation_shift_entity(reg->entities, entity_list_find(reg->entities, dude));
    }
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
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            break;
        case RACE_HUMAN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20000 : 20001);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2095 : 2099;
            break;
        case RACE_DWARF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20002 : 20003);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2055 : 2053);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2055 : 2053;
            break;
        case RACE_ELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20004 : 20005);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2061 : 2059);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2061 : 2059;
            break;
        case RACE_HALFGIANT:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20008 : 20009);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2072 : 2074);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2072 : 2074;
            break;
        case RACE_HALFLING:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20010 : 20011);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2068 : 2070);
            dude->sprite.bmp_id = (dude->gender == GENDER_MALE) ? 2068 : 2070;
            break;
        case RACE_MUL:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            dude->sprite.bmp_id = 2093;
            break;
        case RACE_THRIKREEN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            dude->sprite.bmp_id = 2097;
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
    port_place_entity(player_get(slot));
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
