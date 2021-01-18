#include "player.h"
#include "../src/dsl.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

static player_t player;
static region_object_t dsl_player;
static animate_t *anim = NULL;

// thri-keen is 291 & 292.

void player_init() {
    player.x = 30;
    player.y = 10;
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

void player_render(SDL_Renderer *rend) {
}
