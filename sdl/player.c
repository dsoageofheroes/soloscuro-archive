#include "player.h"
#include "../src/dsl.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

player_t player;

// thri-keen is 291 & 292.

void player_init() {
    player.x = 30;
    player.y = 10;
}

static SDL_Rect loc;
SDL_Texture *tex;

void player_load_graphics(SDL_Renderer *rend) {
    tex = create_texture(rend, OBJEX_GFF_INDEX, GT_BMP, 291, 0, -1, &loc);
    loc.w *= 2;
    loc.h *= 2;
}

void player_move(const uint8_t direction) {
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
}

void player_render(SDL_Renderer *rend) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    loc.x = -xoffset + player.x * 16 * stretch;
    loc.y = -yoffset + player.y * 16 * stretch;
    SDL_RenderCopy(rend, tex, NULL, &loc);
}
