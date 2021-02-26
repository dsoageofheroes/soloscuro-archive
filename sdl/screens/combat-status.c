#include "combat-status.h"
#include "popup.h"
#include "narrate.h"
#include "../main.h"
#include "../sprite.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include <string.h>

static uint16_t background;
static combat_status_t combat_status;
static uint32_t xoffset, yoffset;

combat_status_t* combat_status_get() { return &combat_status; }

#define MAX_STATUS 9
const static char *statuses[] = {
    "New",
    "Okay",
    "Stunned",
    "Out Cold",
    "Dying",
    "Animated",
    "Petrified",
    "Dead",
    "Gone"
};

void combat_status_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    xoffset = x * main_get_zoom();
    yoffset = y * main_get_zoom();

    background = sprite_new(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5016);
}

void combat_status_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, background);
    float zoom = main_get_zoom();
    SDL_Rect loc;
    char buf[128];

    loc.x = xoffset;
    loc.y = yoffset + 2 * zoom;
    loc.w = sprite_getw(background);

    font_render_center(renderer, FONT_GREYLIGHT, combat_status.name, loc);
    snprintf(buf, 127, "%d/%d", combat_status.current_hp, combat_status.max_hp);
    loc.y += 6 * zoom;
    font_render_center(renderer, FONT_GREYLIGHT, buf, loc);
    loc.y += 6 * zoom;
    if (combat_status.status >= 0 && combat_status.status < MAX_STATUS) {
        font_render_center(renderer, FONT_GREYLIGHT, statuses[combat_status.status], loc);
    }
    loc.y += 6 * zoom;
    snprintf(buf, 127, "Move : %d", combat_status.move);
    font_render_center(renderer, FONT_GREYLIGHT, buf, loc);
}

int combat_status_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 1;// did not handle
}

int combat_status_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

int combat_status_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void combat_status_free() {
    sprite_free(background);
}

sops_t combat_status_screen = {
    .init = combat_status_init,
    .cleanup = combat_status_free,
    .render = combat_status_render,
    .mouse_movement = combat_status_handle_mouse_movement,
    .mouse_down = combat_status_handle_mouse_down,
    .mouse_up = combat_status_handle_mouse_up,
    .data = NULL
};
