#include <math.h>
#include "player.h"
#include "../src/dsl.h"
#include "sprite.h"
#include "../src/trigger.h"
#include "../src/ds-player.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

#define MAX_PCS (4)

static animate_sprite_node_t *player_node = NULL;
static int player_zpos;

typedef struct player_sprites_s {
    uint16_t main;
    uint16_t port;
    inventory_sprites_t inv;
} player_sprites_t;

static scmd_t move_down[] = {
    {.bmp_idx = 3, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t move_up[] = {
    {.bmp_idx = 7, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t move_right[] = {
    {.bmp_idx = 11, .delay = 7, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 7, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 7, .flags = 0x0, .xoffset = 8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t move_left[] = {
    {.bmp_idx = 11, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t stand_down[] = {
    {.bmp_idx = 0, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t stand_up[] = {
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t stand_right[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t stand_left[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static player_t player;
static region_object_t dsl_player;
static player_sprites_t players[MAX_PCS];
static animate_sprite_t anims[MAX_PCS];

#define sprite_t uint16_t

void player_init() {
    player.x = 30;
    player.y = 10;
    player_zpos = 0;
    memset(players, 0x00, sizeof(player_sprites_t) * MAX_PCS);
    memset(anims, 0x00, sizeof(animate_sprite_t) * MAX_PCS);
    for (int i = 0; i < 4; i++) {
        players[i].main = players[i].port = SPRITE_ERROR;
        for (int j = 0; j < sizeof(inventory_sprites_t) / sizeof(sprite_t); j++) {
            ((sprite_t*)&(players[i].inv))[j] = SPRITE_ERROR;
        }
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

    // Set initial location
    dsl_player.mapx = player.x * 16;
    dsl_player.mapy = player.y * 16;
}

static int ticks_per_move = 10;
static int count = 0;
static int direction = 0x0;

static void set_animation(animate_sprite_t *as, scmd_t *scmd) {
    const int diffx = abs(as->x - as->destx);
    const int diffy = abs(as->y - as->desty);
    const float distance = sqrt(diffx * diffx + diffy * diffy);

    as->scmd = scmd;
    as->pos = 0;
    as->move = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);

    sprite_set_frame(as->spr, as->scmd->bmp_idx);
    sprite_set_location(as->spr, as->x, as->y);
}

void player_update() {
    animate_shift_node(player_node, player_zpos);
    if (--count > 0) { return; }

    //printf("%d, %d\n", player.x, player.y);
    trigger_box_check(player.x, player.y);
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

    int nextx = player.x;
    int nexty = player.y;
    if (direction & PLAYER_UP) { nexty -= 1; }
    if (direction & PLAYER_DOWN) { nexty += 1; }
    if (direction & PLAYER_LEFT) { nextx -= 1; }
    if (direction & PLAYER_RIGHT) { nextx += 1; }
    //debug ("tile @ (%d, %d) = %d\n", player.x, player.y, cmap_is_block(player.y, player.x));
    if (direction == 0x0 || cmap_is_block(nexty + 1, nextx)) {
        anims[0].x = anims[0].destx;
        anims[0].y = anims[0].desty;
        if (anims[0].scmd == move_left) {
            set_animation(anims + 0, stand_left);
        } else if (anims[0].scmd == move_right) {
            set_animation(anims + 0, stand_right);
        } else if (anims[0].scmd == move_up) {
            set_animation(anims + 0, stand_up);
        } else if (anims[0].scmd == move_down) {
            set_animation(anims + 0, stand_down);
        } else {
        }
        return;
    }

    player.x = nextx;
    player.y = nexty;

    dsl_player.mapx = player.x * 16;
    dsl_player.mapy = player.y * 16;
    anims[0].x = anims[0].destx;
    anims[0].y = anims[0].desty;
    anims[0].destx = player.x * 16 * 2;
    anims[0].desty = player.y * 16 * 2;

    if (direction & PLAYER_LEFT) { 
        set_animation(anims + 0, move_left);
    } else if (direction & PLAYER_RIGHT) {
        set_animation(anims + 0, move_right);
    } else if (direction & PLAYER_DOWN) {
        set_animation(anims + 0, move_down);
    } else if (direction & PLAYER_UP) {
        set_animation(anims + 0, move_up);
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
    ds_character_t *ch = ds_player_get_char(slot);
    ds1_item_t *inv = (ds1_item_t*)ds_player_get_inv(slot);
    sprite_t *inv_sprs = (sprite_t*)&(players[slot].inv);

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

    for (int i = 0; i < sizeof(inventory_sprites_t) / sizeof(sprite_t) && inv; i++) {
        if (inv[i].id != 0) {
            inv_sprs[i] = sprite_new(renderer, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP,
                    ds_item_get_bmp_id(inv + i));
        }
    }
}

void player_load(SDL_Renderer *renderer, const int slot, const float zoom) {
    load_character_sprite(renderer, slot, zoom);
    anims[slot].spr = players[slot].main;
    set_animation(anims + slot, stand_down);
    player_add_to_animation_list();
}

void player_add_to_animation_list() {
    player_node = animate_list_add(anims + 0, player_zpos);
    anims[0].destx = player.x * 16 * 2;
    anims[0].desty = player.y * 16 * 2;
    anims[0].x = anims[0].destx;
    anims[0].y = anims[0].desty;
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

void player_set_delay(const int amt) {
    if (amt < 0) { return; }

    for (int i = 0; i < 4; i++) {
        move_down[i].delay = amt;
        move_up[i].delay = amt;
        move_left[i].delay = amt;
        move_right[i].delay = amt;
    }
}

void player_set_move(const int amt) {
    if (amt < 0) { return; }
    ticks_per_move = amt;
}

void player_close() {
    if (player_node) {
        animate_list_remove(player_node, player_zpos);
    }

    for (int i = 0; i < MAX_PCS; i++) {
        free_sprites(i);
    }
}
