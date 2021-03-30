#include <math.h>
#include "player.h"
#include "main.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "sprite.h"
#include "screens/narrate.h"
#include "../src/trigger.h"
#include "../src/entity-animation.h"
#include "../src/port.h"
#include "../src/region-manager.h"
#include "../src/ds-player.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

static animate_sprite_node_t *player_node[MAX_PCS] = {NULL, NULL, NULL, NULL} ;
static int player_zpos;

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
    inventory_sprites_t inv;
} player_sprites_t;

static player_sprites_t players[MAX_PCS];
static animate_sprite_t anims[MAX_PCS];

#define sprite_t uint16_t

void player_init() {
    dude_t *dude = player_get_active();
    player_zpos = 0;
    memset(players, 0x00, sizeof(player_sprites_t) * MAX_PCS);
    memset(anims, 0x00, sizeof(animate_sprite_t) * MAX_PCS);
    for (int i = 0; i < 4; i++) {
        players[i].main = players[i].port = SPRITE_ERROR;
        for (int j = 0; j < sizeof(inventory_sprites_t) / sizeof(sprite_t); j++) {
            ((sprite_t*)&(players[i].inv))[j] = SPRITE_ERROR;
        }
    }
    dude->mapx = 30;
    dude->mapy = 10;
}

void player_load_graphics(const int slot) {
    dude_t *dude = player_get_entity(slot);
    dude->mapx = 30;
    dude->mapy = 10;
    dude->sprite.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
}

static int ticks_per_move = 10;
static int count = 0;
static int direction = 0x0;

void player_update() {
    entity_t *dude = player_get_active();
    int slot = player_get_slot(dude);
    animate_shift_node(player_node[slot], player_zpos);
    const enum combat_turn_t combat_turn = combat_player_turn();
    if (--count > 0) { return; }

    // update when we can have the player take a turn.
    if (combat_turn != NO_COMBAT) { return; }

    int nextx = dude->mapx;
    int nexty = dude->mapy;
    if (direction & PLAYER_UP) { nexty -= 1; }
    if (direction & PLAYER_DOWN) { nexty += 1; }
    if (direction & PLAYER_LEFT) { nextx -= 1; }
    if (direction & PLAYER_RIGHT) { nextx += 1; }
    //debug ("tile @ (%d, %d) = %d\n", pc->xpos, pc->ypos, cmap_is_block(pc->xpos, pc->ypos));

    if (!narrate_is_open()) {
        trigger_noorders(dude->mapx, dude->mapy);
    }
    trigger_box_check(dude->mapx, dude->mapy);
    trigger_tile_check(dude->mapx, dude->mapy);

    //printf("HERE: %d %d %d\n", main_player_freeze(), direction, cmap_is_block(nexty, nextx));
    if (main_player_freeze() || direction == 0x0 || cmap_is_block(nexty, nextx)) {
        //printf("HERE! (%d, %d, %d)  (%d, %d)\n", main_player_freeze(), direction, cmap_is_block(nexty + 1, nextx), nexty + 1, nextx);
        anims[0].x = anims[0].destx;
        anims[0].y = anims[0].desty;
        if (anims[0].scmd == combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT)) {
            animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_STAND_LEFT), ticks_per_move);
        } else if (anims[0].scmd == combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT)) {
            animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_STAND_RIGHT), ticks_per_move);
        } else if (anims[0].scmd == combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP)) {
            animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_STAND_UP), ticks_per_move);
        } else if (anims[0].scmd == combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN)) {
            animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_STAND_DOWN), ticks_per_move);
        } else {
            animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_STAND_DOWN), ticks_per_move);
        }
        return;
    }

    dude->mapx = nextx;
    dude->mapy = nexty;

    anims[0].x = anims[0].destx;
    anims[0].y = anims[0].desty;
    anims[0].destx = dude->mapx * 16 * main_get_zoom();
    anims[0].destx -= sprite_getw(anims[0].spr) / 2;
    anims[0].desty = dude->mapy * 16 * main_get_zoom();
    anims[0].desty -= sprite_geth(anims[0].spr) - (8 * main_get_zoom());
    //printf("Going to: (%d, %d) -> %d, %d\n", anims[0].x, anims[0].y, anims[0].destx, anims[0].desty);

    if (direction & PLAYER_LEFT) { 
        //anims[0].scmd = combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT);
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT), ticks_per_move);
        //sprite_set_frame(anims[0].spr, anims[0].scmd->bmp_idx);
    } else if (direction & PLAYER_RIGHT) {
        //anims[0].scmd = combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT);
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT), ticks_per_move);
    } else if (direction & PLAYER_DOWN) {
        anims[0].scmd = combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN);
        //animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN), ticks_per_move);
    } else if (direction & PLAYER_UP) {
        //anims[0].scmd = combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP);
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP), ticks_per_move);
    }
/*
    */

    count = ticks_per_move;
}

void player_move(const uint8_t _direction) {
    direction |= _direction;
}

void player_unmove(const uint8_t _direction) {
    direction &= ~(_direction);
}

static void free_sprites(const int slot) {
    sprite_t *inv_sprs = ((sprite_t*)&(players[slot].inv));
    if (players[slot].main != SPRITE_ERROR) {
        sprite_free(players[slot].main);
        players[slot].main = SPRITE_ERROR;
    }
    if (players[slot].port != SPRITE_ERROR) {
        sprite_free(players[slot].port);
        players[slot].port = SPRITE_ERROR;
    }
    for (size_t i = 0; i < sizeof(inventory_sprites_t) / sizeof(sprite_t); i++) {
        if (inv_sprs[i] != SPRITE_ERROR) {
            sprite_free(inv_sprs[i]);
            inv_sprs[i] = SPRITE_ERROR;
        }
    }
}

static void load_character_sprite(SDL_Renderer *renderer, const int slot, const float zoom) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    dude_t *dude = player_get_entity(slot);

    free_sprites(slot);

    switch(dude->race) {
        case RACE_HALFELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20006 : 20007);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2095 : 2099) + 1);
            break;
        case RACE_HUMAN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20000 : 20001);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2095 : 2099);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2095 : 2099) + 1);
            break;
        case RACE_DWARF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20002 : 20003);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2055 : 2053);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2055 : 2053) + 1);
            break;
        case RACE_ELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20004 : 20005);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2061 : 2059);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2061 : 2059) + 1);
            break;
        case RACE_HALFGIANT:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20008 : 20009);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2072 : 2074);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2072 : 2074) + 1);
            break;
        case RACE_HALFLING:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, dude->gender == GENDER_MALE ? 20010 : 20011);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (dude->gender == GENDER_MALE) ? 2068 : 2070);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, ((dude->gender == GENDER_MALE) ? 2068 : 2070) + 1);
            break;
        case RACE_MUL:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2094);
            break;
        case RACE_THRIKREEN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            sprite_append(players[slot].main, renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2098);
            break;
    }

}

extern void player_condense() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = player_get_entity(i);
        if (player != player_get_active() && player->name) {
            //animate_list_remove(player_node[i]);
            animate_list_remove(player_node[i], player_zpos);
            player_node[i] = NULL;
        }
    }
}

void player_add_to_animation_list(const int slot) {
    entity_t *dude = player_get_entity(slot);
    if (!dude->name) { return; } // !player_exists
    /*
    player_node[slot] = animate_list_add(anims + slot, player_zpos);
    anims[slot].destx = dude->mapx * 16 * main_get_zoom();
    anims[slot].desty = dude->mapy * 16 * main_get_zoom();
    anims[slot].desty -= sprite_geth(anims[slot].spr) - (16 * main_get_zoom());
    anims[slot].x = anims[slot].destx;
    anims[slot].y = anims[slot].desty;
    anims[slot].entity = player_get_entity(slot);
    anims[slot].entity->sprite.data = player_node[slot];//anims + slot;
    */
}

void player_load(const int slot, const float zoom) {
    //gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    entity_t *dude = player_get_entity(slot);
    load_character_sprite(main_get_rend(), slot, zoom);
    anims[slot].scmd = entity_animation_get_scmd(NULL, 0, 1, CA_NONE);
    anims[slot].spr = players[slot].main;
    anims[slot].delay = 0;
    anims[slot].pos = 0;
    anims[slot].x = (dude->mapx * 16 + dude->sprite.xoffset) * zoom;
    anims[slot].y = (dude->mapy * 16 + dude->sprite.yoffset + dude->mapz) * zoom;
    anims[slot].destx = anims[slot].x;
    anims[slot].destx -= sprite_getw(anims[slot].spr) / 2 - (8 * main_get_zoom());
    anims[slot].move = anims[slot].left_over = 0.0;
    anims[slot].entity = dude;
    //anim_nodes[slot] = animate_list_add(anims + anim_pos, entity->mapz);
    player_node[slot] = animate_list_add(anims + slot, dude->mapz);
    //dude->sprite.data = anim_nodes[slot];
    dude->sprite.data = player_node[slot];
    //anim_nodes[slot]->anim->entity = dude;
    player_node[slot]->anim->entity = dude;
    //animate_set_animation(anims + slot, combat_get_scmd(COMBAT_SCMD_STAND_DOWN), ticks_per_move);
    if (player_get_entity(slot) == player_get_active()) {
        //player_add_to_animation_list(slot);
        region_add_entity(region_manager_get_current(), player_get_entity(slot));
        //port_add_entity(player_get_entity(slot), pal);
    }
}

int32_t player_getx(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return sprite_getx(players[slot].main);
}

int32_t player_gety(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return sprite_gety(players[slot].main);
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

inventory_sprites_t* player_get_inventory_sprites(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(players[slot].inv);
}

uint16_t player_get_sprite(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return SPRITE_ERROR; }
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

void player_remove_animation() {
    for (int i = 0; i < MAX_PCS; i++) {
        if (player_node[i]) {
            animate_list_remove(player_node[i], player_zpos);
            player_node[i] = NULL;
        }
    }
}

void player_close() {
    player_remove_animation();

    for (int i = 0; i < MAX_PCS; i++) {
        free_sprites(i);
    }
}
