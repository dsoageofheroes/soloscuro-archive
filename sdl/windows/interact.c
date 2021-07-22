#include "interact.h"
#include "narrate.h"
#include "../../src/entity.h"
#include "../sprite.h"
#include "../main.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "../../src/settings.h"
#include <string.h>

static uint16_t background;
static uint16_t talk, get, use, bar, bar_border;
static uint16_t box;
static char action_text[16];
static char name_text[32];
static char info_text[32];
static entity_t *entity = NULL;
SDL_Rect action_loc, name_loc, info_loc;

void interact_set_entity(entity_t *_entity) {
    entity = _entity;
    if (entity) {
        strcpy(name_text, entity->name);
        sprintf(info_text, "LEVEL: %d", entity->class[0].level);
    } else {
        name_text[0] = info_text[0] = '\0';
    }
}

uint16_t interact_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

void interact_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    background = sprite_new(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 3020);
    talk = sprite_new(renderer, pal, 3 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15105);
    get = sprite_new(renderer, pal,  23 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15106);
    use = sprite_new(renderer, pal,  43 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15107);
    box = sprite_new(renderer, pal,  60 + x, 59 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15109);
    bar = sprite_new(renderer, pal,  6 + x, 7 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20107);
    bar_border = sprite_new(renderer, pal,  5 + x, 6 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20106);

    action_loc.x = (62 + x) * zoom;
    action_loc.y = (61 + y) * zoom;
    name_loc.x = (6 + x) * zoom;
    name_loc.y = (17 + y) * zoom;
    info_loc.x = (6 + x) * zoom;
    info_loc.y = (24 + y) * zoom;

    memset(action_text, 0x0, sizeof(action_text));
    memset(name_text, 0x0, sizeof(name_text));
    memset(info_text, 0x0, sizeof(info_text));
}

void interact_render(void *data, SDL_Renderer *renderer) {
    float amt = entity
        ? entity->stats.hp / (float) entity->stats.high_hp
        : 0;
    sprite_render(renderer, background);
    sprite_render(renderer, talk);
    sprite_render(renderer, use);
    sprite_render(renderer, get);
    sprite_render(renderer, bar_border);
    sprite_render(renderer, box);
    sprite_render_box(renderer, bar, sprite_getx(bar_border), sprite_gety(bar_border),
        sprite_getw(bar_border) * (amt > 1.0 ? 1.0 : amt), sprite_geth(bar_border));

    print_line_len(renderer, FONT_GREY, action_text, action_loc.x, action_loc.y, sizeof(action_text));
    print_line_len(renderer, FONT_GREYLIGHT, name_text, name_loc.x, name_loc.y, sizeof(name_text));
    print_line_len(renderer, FONT_GREYLIGHT, info_text, info_loc.x, info_loc.y, sizeof(info_text));
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(talk, x, y)) { return talk; }
    if (sprite_in_rect(get, x, y)) { return get; }
    if (sprite_in_rect(use, x, y)) { return use; }
    
    return SPRITE_ERROR;
}

int interact_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }
    
    last_sprite = cur_sprite;
    if (cur_sprite == talk) { strcpy(action_text, "TALK");
    } else if (cur_sprite == get) { strcpy(action_text, " GET");
    } else if (cur_sprite == use) { strcpy(action_text, " USE");
    } else { action_text[0] = '\0';
    }
    return 1;// did not handle
}

int interact_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

int interact_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

void interact_free() {
    sprite_free(background);
    sprite_free(talk);
    sprite_free(get);
    sprite_free(use);
    sprite_free(bar);
    sprite_free(bar_border);
    sprite_free(box);
}

wops_t interact_window = {
    .init = interact_init,
    .cleanup = interact_free,
    .render = interact_render,
    .mouse_movement = interact_handle_mouse_movement,
    .mouse_down = interact_handle_mouse_down,
    .mouse_up = interact_handle_mouse_up,
    .grey_out_map = 1,
    .data = NULL
};
