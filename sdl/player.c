#include "player.h"
#include "../src/dsl.h"
#include "sprite.h"
#include "../src/ds-player.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

#define MAX_PCS (4)

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
} player_sprites_t;

static player_t player;
static region_object_t dsl_player;
static animate_t *anim = NULL;
static player_sprites_t players[MAX_PCS];

// thri-keen is 291 & 292.

void player_init() {
    player.x = 30;
    player.y = 10;
    memset(players, 0x00, sizeof(player_sprites_t) * MAX_PCS);
    for (int i = 0; i < 4; i++) {
        players[i].main = players[i].port = SPRITE_ERROR;
    }
}

void player_load_graphics(SDL_Renderer *rend) {
    dsl_player.flags = 0;
    dsl_player.entry_id = 0;
    dsl_player.bmpx = 0;
    dsl_player.bmpy = 0;
    dsl_player.xoffset = 0;
    dsl_player.yoffset = 0;
    dsl_player.mapx = 30;
    dsl_player.mapy = 10;
    dsl_player.mapz = 0;
    dsl_player.ht_idx = 0;
    dsl_player.gt_idx = 0;
    dsl_player.bmp_idx = 0;
    dsl_player.bmp_width = 0;
    dsl_player.bmp_height = 0;
    dsl_player.cdelay = 0;
    dsl_player.st_idx = 0;
    dsl_player.sc_idx = 0;
    dsl_player.btc_idx = 291;
    dsl_player.disk_idx = 0;
    dsl_player.game_time = 0;
    dsl_player.scmd = NULL;
    anim = animate_add_obj(rend, &dsl_player, OBJEX_GFF_INDEX, -1);

    // Set initial location
    dsl_player.mapx = player.x * 16;
    dsl_player.mapy = player.y * 16;
}

#define TICKS_PER_MOVE (5)

static int count = TICKS_PER_MOVE;

void player_move(const uint8_t direction) {
    if (--count > 0) { return; }
    int nextx = player.x;
    int nexty = player.y;
    switch(direction) {
        case PLAYER_UP:
            nexty -= 1;
            break;
        case PLAYER_DOWN:
            nexty += 1;
            break;
        case PLAYER_LEFT:
            nextx -= 1;
            break;
        case PLAYER_RIGHT:
            nextx += 1;
            break;
        default:
            warn("Unknown player direction: %d\n", direction);
            break;
    }
    //debug ("tile @ (%d, %d) = %d\n", player.x, player.y, cmap_is_block(player.y, player.x));
    if (cmap_is_block(nexty + 1, nextx)) { return; }
    player.x = nextx;
    player.y = nexty;
    dsl_check_t* dsl_check = dsl_find_tile_check(player.x, player.y);
    if (dsl_check) {
        debug("TILE CHECK: Need to execute file = %d, addr = %d, trip = %d\n",
            dsl_check->data.tile_check.file, dsl_check->data.tile_check.addr,
            dsl_check->data.tile_check.trip);
        //dsl_execute_subroutine(dsl_check->data.tile_check.file,
            //dsl_check->data.tile_check.addr, 0);
    }
    dsl_check = dsl_find_box_check(player.x, player.y);
    if (dsl_check) {
        debug("BOX CHECK: Need to execute file = %d, addr = %d, trip = %d\n",
            dsl_check->data.box_check.file, dsl_check->data.box_check.addr,
            dsl_check->data.box_check.trip);
        //dsl_execute_subroutine(dsl_check->data.box_check.file,
            //dsl_check->data.box_check.addr, 0);
    }
    count = TICKS_PER_MOVE;
    dsl_player.mapx = player.x * 16;
    dsl_player.mapy = player.y * 16;
    shift_anim(anim);
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
    ds_character_t *ch = ds_player_get_char(slot);

    free_sprites(slot);

    switch(ch->race) {
        case RACE_HALFELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20006 : 20007);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2095 : 2099);
            break;
        case RACE_HUMAN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20000 : 20001);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2095 : 2099);
            break;
        case RACE_DWARF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20002 : 20003);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2055 : 2053);
            break;
        case RACE_ELF:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20004 : 20005);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2061 : 2059);
            break;
        case RACE_HALFGIANT:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20008 : 20009);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2072 : 2074);
            break;
        case RACE_HALFLING:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, ch->gender == GENDER_MALE ? 20010 : 20011);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, (ch->gender == GENDER_MALE) ? 2068 : 2070);
            break;
        case RACE_MUL:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20012);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2093);
            break;
        case RACE_TRIKEEN:
            players[slot].port = sprite_new(renderer, pal, 0, 0,
                zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20013);
            players[slot].main = sprite_new(renderer, pal, 0, 0,
                zoom, OBJEX_GFF_INDEX, GFF_BMP, 2097);
            break;
    }
}

void player_load(SDL_Renderer *renderer, const int slot, const float zoom) {
    load_character_sprite(renderer, slot, zoom);
}

int32_t player_getx(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return sprite_getx(players[slot].main);
}

int32_t player_gety(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return sprite_gety(players[slot].main);
}

void player_set_loc(const int slot, const int32_t x, const int32_t y) {
    if (slot < 0 || slot >= MAX_PCS) { return; }

    sprite_set_location(players[slot].main, x, y);
}

void player_render(SDL_Renderer *rend, const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    //sprite_set_location(players[slot].main, players[slot].x, players[slot].y);
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

