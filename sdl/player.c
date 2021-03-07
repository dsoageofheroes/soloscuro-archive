#include <math.h>
#include "player.h"
#include "main.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "sprite.h"
#include "screens/narrate.h"
#include "../src/trigger.h"
#include "../src/ds-player.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

static animate_sprite_node_t *player_node = NULL;
static int player_zpos;

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
    inventory_sprites_t inv;
} player_sprites_t;

static region_object_t dsl_player[MAX_PCS];
static player_sprites_t players[MAX_PCS];
static animate_sprite_t anims[MAX_PCS];

#define sprite_t uint16_t

void player_init() {
    dude_t *dude = player_get_entity(ds_player_get_active());
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

region_object_t* player_get_robj(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return dsl_player + slot;
}

void player_load_graphics(const int slot) {
    dude_t *dude = player_get_entity(slot);
    dude->mapx = 30;
    dude->mapy = 10;
    dude->sprite.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);

    dsl_player[slot].flags = 0;
    dsl_player[slot].entry_id = 0;
    dsl_player[slot].bmpx = 0;
    dsl_player[slot].bmpy = 0;
    dsl_player[slot].xoffset = 0;
    dsl_player[slot].yoffset = 0;
    dsl_player[slot].mapx = 30;
    dsl_player[slot].mapy = 10;
    dsl_player[slot].mapz = 0;
    dsl_player[slot].ht_idx = 0;
    dsl_player[slot].gt_idx = 0;
    dsl_player[slot].bmp_idx = 0;
    dsl_player[slot].bmp_width = 0;
    dsl_player[slot].bmp_height = 0;
    dsl_player[slot].cdelay = 0;
    dsl_player[slot].st_idx = 0;
    dsl_player[slot].sc_idx = 0;
    dsl_player[slot].btc_idx = 291;
    dsl_player[slot].disk_idx = 0;
    dsl_player[slot].game_time = 0;
    dsl_player[slot].scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);

    // Set initial location
    dsl_player[slot].mapx = dude->mapx;
    dsl_player[slot].mapy = dude->mapy;
}

static int ticks_per_move = 10;
static int count = 0;
static int direction = 0x0;

void player_update() {
    int slot = ds_player_get_active();
    entity_t *dude = player_get_entity(slot);
    animate_shift_node(player_node, player_zpos);
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
        //trigger_noorders(pc->xpos, pc->ypos);
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
        }
        return;
    }

    dude->mapx = nextx;
    dude->mapy = nexty;

    dsl_player[slot].mapx = dude->mapx;
    dsl_player[slot].mapy = dude->mapy;
    anims[0].x = anims[0].destx;
    anims[0].y = anims[0].desty;
    anims[0].destx = dude->mapx * 16 * main_get_zoom();
    anims[0].desty = dude->mapy * 16 * main_get_zoom();
    anims[0].desty -= sprite_geth(anims[0].spr) - (8 * main_get_zoom());
    //printf("Going to: (%d, %d) -> %d, %d\n", anims[0].x, anims[0].y, anims[0].destx, anims[0].desty);

    if (direction & PLAYER_LEFT) { 
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT), ticks_per_move);
    } else if (direction & PLAYER_RIGHT) {
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT), ticks_per_move);
    } else if (direction & PLAYER_DOWN) {
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN), ticks_per_move);
    } else if (direction & PLAYER_UP) {
        animate_set_animation(anims + 0, combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP), ticks_per_move);
    }

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
    for (int i = 0; i < sizeof(inventory_sprites_t) / sizeof(sprite_t); i++) {
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
    ds1_item_t *inv = (ds1_item_t*)ds_player_get_inv(slot);
    sprite_t *inv_sprs = (sprite_t*)&(players[slot].inv);

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

    for (int i = 0; i < sizeof(inventory_sprites_t) / sizeof(sprite_t) && inv; i++) {
        if (inv[i].id != 0) {
            inv_sprs[i] = sprite_new(renderer, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP,
                    ds_item_get_bmp_id(inv + i));
        }
    }
}

void player_add_to_animation_list(const int slot) {
    entity_t *dude = player_get_entity(slot);
    if (!dude->name) { return; } // !player_exists
    player_node = animate_list_add(anims + slot, player_zpos);
    anims[slot].destx = dude->mapx * 16 * main_get_zoom();
    anims[slot].desty = dude->mapy * 16 * main_get_zoom();
    anims[slot].desty -= sprite_geth(anims[slot].spr) - (16 * main_get_zoom());
    anims[slot].x = anims[slot].destx;
    anims[slot].y = anims[slot].desty;
    printf("%d, %d\n", anims[slot].x, anims[slot].y);
    anims[slot].entity = player_get_entity(slot);
    anims[slot].entity->sprite.data = anims + slot;
}

void player_load(const int slot, const float zoom) {
    load_character_sprite(main_get_rend(), slot, zoom);
    anims[slot].spr = players[slot].main;
    animate_set_animation(anims + slot, combat_get_scmd(COMBAT_SCMD_STAND_DOWN), ticks_per_move);
    if (slot == ds_player_get_active()) {
        player_add_to_animation_list(slot);
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
    if (player_node) {
        animate_list_remove(player_node, player_zpos);
        player_node = NULL;
    }

}

void player_close() {
    player_remove_animation();

    for (int i = 0; i < MAX_PCS; i++) {
        free_sprites(i);
    }
}
